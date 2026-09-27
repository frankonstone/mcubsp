"""Portable flash command planning, separate from compilation and execution."""
from __future__ import annotations

import json
from pathlib import Path
import struct
import subprocess

from .environment import EnvironmentError


def _tcl(value: str) -> str:
    """One literal Tcl argument, even with spaces, brackets or dollar signs."""
    return '"' + ''.join('\\' + char if char in '\\"$[]' else
                         '\\n' if char == '\n' else '\\r' if char == '\r' else char
                         for char in value) + '"'


def _jlink_path(value: str) -> str:
    if any(char in value for char in ('"', '\n', '\r')):
        raise EnvironmentError("J-Link command paths cannot contain quotes or line breaks")
    return '"' + value + '"'


def _regions(runner: dict) -> list[tuple[int, int]]:
    try:
        def number(value: str) -> int:
            return int(value, 16 if value.lower().startswith('0x') else 10)
        return [(number(value['start']), number(value['size'])) for value in runner['flash_regions']]
    except (KeyError, ValueError, TypeError) as error:
        raise EnvironmentError("Runner must declare exact flash_regions start and size") from error


def _in_region(address: int, size: int, regions: list[tuple[int, int]]) -> bool:
    return any(start <= address and address + size <= start + length for start, length in regions)


def check_elf_regions(path: Path, runner: dict) -> None:
    """Verify loadable file bytes fit the declared flash map before executing."""
    data = path.read_bytes()
    if len(data) < 52 or data[:4] != b'\x7fELF' or data[4] != 1 or data[5] != 1:
        raise EnvironmentError(f"Expected a 32-bit little-endian firmware ELF: {path}")
    offset = struct.unpack_from('<I', data, 28)[0]
    size, count = struct.unpack_from('<HH', data, 42)
    if size < 32 or offset + size * count > len(data):
        raise EnvironmentError(f"Malformed ELF program headers: {path}")
    segments = 0
    for index in range(count):
        kind, file_offset, _, address, length = struct.unpack_from('<IIIII', data, offset + size * index)
        if kind == 1 and length:
            segments += 1
            if file_offset + length > len(data):
                raise EnvironmentError(f"Truncated ELF load segment: {path}")
            if not _in_region(address, length, _regions(runner)):
                raise EnvironmentError(f"ELF load at 0x{address:x} ({length} bytes) exceeds the board flash regions")
    if not segments:
        raise EnvironmentError(f"ELF has no loadable file bytes: {path}")


def plan(board: dict, build_dir: Path, tools: dict, *, probe: str | None = None,
         port: str | None = None, require_artifacts: bool = True,
         artifact: Path | None = None, sdk_build: Path | None = None,
         openocd_tail: str | None = None) -> dict:
    runner = board.get('build', {}).get('runner', {})
    kind = runner.get('type')
    executable = tools.get(runner.get('tool'), {}).get('path')
    if not executable:
        raise EnvironmentError(f"Missing {runner.get('tool', 'runner')} tool; run setup --operation flash")
    result = {'runner': kind, 'board': board['id'], 'revision': board.get('selected_revision', board.get('default_revision')),
              'flash_regions': runner.get('flash_regions', []), 'commands': [], 'files': {}}
    if kind in ('jlink', 'openocd'):
        if not probe or any(char in probe for char in ('\n', '\r')):
            raise EnvironmentError('Select a debug probe explicitly with --probe SERIAL')
        artifact = artifact or build_dir / runner.get('artifact', 'firmware.elf')
        if require_artifacts:
            if not artifact.is_file():
                raise EnvironmentError(f"Firmware image missing: {artifact}; build this board first")
            check_elf_regions(artifact, runner)
        if kind == 'jlink':
            command_file = build_dir / 'mcuhw-flash.jlink'
            result['files'][str(command_file)] = '\n'.join(['r', 'h', f'loadfile {_jlink_path(str(artifact))}', 'r', 'g', 'exit', ''])
            result['commands'] = [[executable, '-Device', runner['device'], '-If', runner.get('interface', 'SWD'),
                                   '-Speed', str(runner.get('speed_khz', 4000)), '-SelectEmuBySN', probe,
                                   '-AutoConnect', '1', '-ExitOnError', '1', '-CommandFile', str(command_file)]]
        else:
            program = f'program {_tcl(artifact.as_posix())} verify reset exit'
            if openocd_tail is not None:
                program = (f'adapter speed {runner.get("speed_khz", 4000)}; init; reset halt; '
                           f'program {_tcl(artifact.as_posix())} verify; {openocd_tail}')
            result['commands'] = [[executable, '-f', runner['interface'], '-f', runner['target'],
                                   '-c', f'adapter serial {_tcl(probe)}', '-c',
                                   program]]
    elif kind == 'esp-idf':
        if not port:
            raise EnvironmentError('Select the ESP32 serial port explicitly with --port PORT')
        sdk_build = sdk_build or build_dir / runner.get('build_dir', 'esp-idf-build')
        settings_path = sdk_build / 'flasher_args.json'
        if not settings_path.is_file():
            raise EnvironmentError(f"Missing SDK flash manifest: {settings_path}; build this board first")
        settings = json.loads(settings_path.read_text(encoding='utf-8'))
        chip = settings.get('extra_esptool_args', {}).get('chip')
        if chip != runner['chip']:
            raise EnvironmentError(f"Flash manifest chip {chip!r} does not match {runner['chip']}")
        regions = _regions(runner)
        flash_files = settings.get('flash_files', {})
        expected = {address for address, _ in regions}
        try:
            actual = {int(address, 0) for address in flash_files}
        except (ValueError, TypeError) as error:
            raise EnvironmentError('Malformed ESP flash addresses') from error
        if actual != expected:
            raise EnvironmentError('SDK flash manifest does not match the board bootloader, partition and application regions')
        flags = settings.get('flash_settings', {})
        argv = [executable, '-m', 'esptool', '--chip', chip, '--port', port,
                '--before', 'default-reset', '--after', 'hard-reset', 'write-flash']
        for key, flag in [('flash_mode', '--flash-mode'), ('flash_size', '--flash-size'), ('flash_freq', '--flash-freq')]:
            if key not in flags or not isinstance(flags[key], str):
                raise EnvironmentError(f'Missing SDK flash setting {key}')
            argv += [flag, flags[key]]
        for address, relative in sorted(flash_files.items(), key=lambda value: int(value[0], 0)):
            image = (sdk_build / relative).resolve()
            if sdk_build.resolve() not in image.parents:
                raise EnvironmentError(f'Flash image outside selected SDK build: {relative}')
            if require_artifacts:
                if not image.is_file():
                    raise EnvironmentError(f'Flash image missing: {image}')
                if not _in_region(int(address, 0), image.stat().st_size, regions):
                    raise EnvironmentError(f'Flash image exceeds declared region: {image}')
            argv += [address, str(image)]
        result['commands'] = [argv]
    else:
        raise EnvironmentError(f"Unsupported runner {kind!r} for {board['id']}")
    return result


def execute(plan: dict, *, env: dict | None = None) -> None:
    """Execute a reviewed plan; application and physical fixtures share this path."""
    for filename, content in plan['files'].items():
        Path(filename).write_text(content, encoding='utf-8')
    for command in plan['commands']:
        subprocess.run(command, check=True, env=env)
