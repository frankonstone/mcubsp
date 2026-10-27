#!/usr/bin/env python3
"""Shared explicit board runner. Planning never opens a probe or serial port."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def executable(command, override=None):
    selected = os.environ.get(override, '') if override else ''
    selected = selected or command
    found = shutil.which(selected)
    if not found:
        raise ValueError(f'{selected}: executable unavailable; install {command}' +
                         (f' or set {override} to its full path' if override else ''))
    # Python locates a virtual environment from its invoked path. Resolving a
    # venv's interpreter symlink would silently select the base installation.
    return str(Path(found).absolute())


def board_info(board):
    if not re.fullmatch(r'[a-z0-9_-]+/[a-z0-9_-]+', board):
        raise ValueError('board must be vendor/name')
    entry = ROOT / 'src' / 'board' / board / 'board.cmake'
    if not entry.is_file():
        raise ValueError(f'unknown board: {board}')
    with tempfile.TemporaryDirectory(prefix='mcubsp runner ') as directory:
        output = Path(directory) / 'board.json'
        result = subprocess.run([executable('cmake'), f'-DBOARD_ENTRY={entry}',
            f'-DOUTPUT={output}', '-P', str(ROOT / 'tool/RunnerInfo.cmake')],
            capture_output=True, text=True)
        if result.returncode:
            raise ValueError(result.stderr.strip())
        return json.loads(output.read_text())


def regions(runner):
    return [(int(item['start'], 0), int(item['size'], 0)) for item in runner['flash_regions']]


def fits(start, size, allowed):
    return size > 0 and any(start >= base and start + size <= base + length for base, length in allowed)


def elf_loads(path, machine=40):
    data = Path(path).read_bytes()
    if len(data) < 52 or data[:4] != b'\x7fELF' or data[4] not in (1, 2) or data[5] not in (1, 2):
        raise ValueError('firmware is not a supported ELF image')
    endian = '<' if data[5] == 1 else '>'
    if data[4] == 1:
        header = struct.unpack_from(endian + 'HHIIIIIHHHHHH', data, 16)
        offset, stride, count = header[4], header[8], header[9]
        format_ = endian + 'IIIIIIII'
    else:
        if len(data) < 64:
            raise ValueError('truncated ELF header')
        header = struct.unpack_from(endian + 'HHIQQQIHHHHHH', data, 16)
        offset, stride, count = header[4], header[8], header[9]
        format_ = endian + 'IIQQQQQQ'
    if header[1] != machine:
        raise ValueError('ELF machine does not match the selected board architecture')
    if stride < struct.calcsize(format_) or offset + count * stride > len(data):
        raise ValueError('truncated ELF program headers')
    loads = []
    for index in range(count):
        item = struct.unpack_from(format_, data, offset + index * stride)
        if item[0] != 1:
            continue
        if data[4] == 1:
            file_offset, virtual, physical, size, memory = item[1:6]
        else:
            file_offset, virtual, physical, size, memory = item[2:7]
        if size > memory or file_offset + size > len(data):
            raise ValueError('invalid ELF load segment')
        if size:
            loads.append((physical or virtual, size))
    if not loads:
        raise ValueError('firmware has no loadable file data')
    return loads


def artifact_record(elf, board, revision, info):
    if not elf.is_file():
        raise ValueError(f'missing firmware: {elf}; build the selected application first')
    path = elf.with_name(elf.name + '.build.json')
    if not path.is_file():
        raise ValueError(f'missing artifact record: {path}; rebuild using the current firmware helper')
    record = json.loads(path.read_text())
    revision = revision or info['default_revision']
    if revision not in info['revisions']:
        raise ValueError(f'unsupported {board} revision: {revision}')
    if record.get('board') != board or record.get('revision') != revision:
        raise ValueError('artifact board/revision does not match requested hardware')
    if record.get('sha256') != digest(elf) or record.get('bytes') != elf.stat().st_size:
        raise ValueError('firmware differs from its completed build record; rebuild before flashing')
    inputs = record.get('inputs', {})
    if inputs.get('status') == 'recorded':
        sidecar = elf.with_name(elf.name + '.inputs.json')
        if inputs.get('file') != sidecar.name or not sidecar.is_file() or inputs.get('sha256') != digest(sidecar):
            raise ValueError('firmware input record is missing or differs from its completed build record')
    return record


def tcl_quote(value):
    value = str(value).replace('\\', '/')
    for old, new in [('"', '\\"'), ('$', '\\$'), ('[', '\\['), (']', '\\]')]:
        value = value.replace(old, new)
    if '\n' in value or '\r' in value:
        raise ValueError('newlines are not supported in debugger arguments')
    return '"' + value + '"'


def plan_flash(args):
    info = board_info(args.board)
    runner = info['runner']
    elf = Path(args.elf).resolve()
    record = artifact_record(elf, args.board, args.revision, info)
    allowed = regions(runner)
    plan = {'board': args.board, 'revision': record['revision'], 'elf': str(elf),
            'sha256': record['sha256'], 'runner': runner['type'],
            'inputs': record.get('inputs', {'status': 'unavailable', 'reason': 'legacy build record'})}
    if runner['type'] in ('jlink', 'openocd'):
        loads = elf_loads(elf)
        if any(not fits(address, size, allowed) for address, size in loads):
            raise ValueError('ELF load segment is outside declared application flash regions')
        probe = args.probe or os.environ.get('MCUBSP_PROBE_SERIAL', '')
        if not probe or not re.fullmatch(r'[A-Za-z0-9_.:-]+', probe):
            raise ValueError('select an explicit probe serial with --probe or MCUBSP_PROBE_SERIAL')
        plan['probe'] = probe
        plan['load_segments'] = loads
        if runner['type'] == 'jlink':
            if args.commands:
                raise ValueError('--commands is supported only by the OpenOCD runner')
            if '"' in str(elf) or '\n' in str(elf) or '\r' in str(elf):
                raise ValueError('quotes/newlines are not supported in J-Link image paths')
            tool = executable('JLinkExe', args.tool_environment or 'JLINK_EXE')
            plan['command'] = [tool, '-device', runner['device'], '-if', runner['interface'],
                '-speed', str(runner['speed']), '-USB', probe, '-autoconnect', '1',
                '-ExitOnError', '1', '-CommandFile', '{command_file}']
            plan['command_file'] = f'r\nloadfile "{elf.as_posix()}"\nr\ng\nq\n'
        else:
            tool = executable('openocd', args.tool_environment or 'OPENOCD_EXE')
            plan['command'] = [tool, '-f', runner['interface'], '-f', runner['target'],
                '-c', f'adapter serial {tcl_quote(probe)}',
                '-c', f'program {tcl_quote(elf.as_posix())} verify',
                '-c', args.commands or 'reset run; shutdown']
    # @req REQ-0053
    elif runner['type'] == 'uf2':
        if args.commands or args.probe or args.port or args.sdk_build:
            raise ValueError('UF2 upload uses --mount, without probe/port/SDK/commands')
        mount_arg = getattr(args, 'mount', None)
        if not mount_arg:
            raise ValueError('select the explicitly mounted UF2 drive with --mount')
        mount = Path(mount_arg).resolve()
        info_file = mount / 'INFO_UF2.TXT'
        if not mount.is_dir() or not info_file.is_file():
            raise ValueError('selected mount has no INFO_UF2.TXT; enter and mount the UF2 bootloader')
        identity = info_file.read_text(encoding='utf-8', errors='replace')
        board_id = re.search(r'^Board-ID:\s*(\S+)', identity, re.MULTILINE | re.IGNORECASE)
        if not board_id or not board_id[1].lower().startswith(runner['board_id'].lower() + '-'):
            raise ValueError('mounted UF2 Board-ID does not match the selected MCU family')
        spec = importlib.util.spec_from_file_location('mcubsp_uf2', ROOT / 'tool/uf2.py')
        uf2 = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(uf2)
        if len(allowed) != 1:
            raise ValueError('UF2 runner requires one application flash region')
        start, size = allowed[0]
        family = int(runner['family'], 0)
        expected = uf2.encode(elf.read_bytes(), family=family, start=start, size=size)
        image = elf.with_name(elf.name + '.uf2')
        if not image.is_file() or image.read_bytes() != expected:
            raise ValueError('missing or stale UF2 image; rebuild the selected application with --clean-first')
        plan['mount'] = str(mount)
        plan['board_id'] = board_id[1]
        plan['uf2'] = str(image)
        plan['uf2_sha256'] = digest(image)
        plan['command'] = [sys.executable, str(ROOT / 'tool/uf2.py'), '--elf', str(elf),
            '--output', str(mount / 'FIRMWARE.UF2'), '--family', hex(family),
            '--start', hex(start), '--size', hex(size)]
    elif runner['type'] == 'esp-idf':
        if args.commands:
            raise ValueError('--commands is supported only by the OpenOCD runner')
        if not args.port:
            raise ValueError('select an explicit serial port with --port')
        if not args.sdk_build:
            raise ValueError('ESP flashing requires --sdk-build containing the completed flash bundle')
        build = Path(args.sdk_build).resolve()
        manifest_path = build / 'flasher_args.json'
        manifest = json.loads(manifest_path.read_text())
        if manifest.get('extra_esptool_args', {}).get('chip') != runner['chip']:
            raise ValueError('SDK flash manifest selects another chip')
        if record.get('flash_manifest_sha256') != digest(manifest_path):
            raise ValueError('flash bundle differs from its build record; rebuild the SDK application')
        sdk_elf = build / elf.name
        if not sdk_elf.is_file() or digest(sdk_elf) != record['sha256']:
            raise ValueError('SDK build ELF does not match the selected completed image')
        if {int(offset, 0) for offset in manifest['flash_files']} != {base for base, _ in allowed}:
            raise ValueError('SDK flash bundle must contain the declared bootloader, partition table and application')
        files, spans = [], []
        for offset_text, name in sorted(manifest['flash_files'].items(), key=lambda pair: int(pair[0], 0)):
            offset = int(offset_text, 0)
            image = (build / name).resolve()
            if not image.is_relative_to(build) or not image.is_file():
                raise ValueError('SDK image is missing or escapes its build directory')
            size = image.stat().st_size
            if not fits(offset, size, allowed):
                raise ValueError('SDK image is outside declared flash regions')
            if any(offset < end and start < offset + size for start, end in spans):
                raise ValueError('SDK flash images overlap')
            spans.append((offset, offset + size))
            saved = record.get('flash_files', {}).get(offset_text, {})
            if saved.get('file') != name or saved.get('sha256') != digest(image):
                raise ValueError('SDK image differs from its completed flash-bundle record')
            files.extend([hex(offset), str(image)])
        app = manifest.get('app', {})
        if app.get('file') != elf.with_suffix('.bin').name or app.get('offset') not in manifest['flash_files']:
            raise ValueError('SDK flash manifest does not identify the selected application')
        settings = manifest.get('write_flash_args', [])
        if len(settings) % 2 or any(settings[i] not in ('--flash-mode', '--flash-size', '--flash-freq')
                                  for i in range(0, len(settings), 2)):
            raise ValueError('unsupported SDK flash options')
        tool = executable('python', args.tool_environment or 'IDF_PYTHON')
        plan['command'] = [tool, '-m', 'esptool', '--chip', runner['chip'], '--port', args.port,
                           '--before', 'default-reset', '--after', 'hard-reset', 'write-flash', *settings, *files]
        plan['port'] = args.port
    else:
        raise ValueError(f'unsupported runner: {runner["type"]}')
    return plan


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest='action', required=True)
    tool = commands.add_parser('tool')
    tool.add_argument('--name', required=True)
    tool.add_argument('--command', required=True)
    tool.add_argument('--override-env')
    field = commands.add_parser('runner-field')
    field.add_argument('--board', required=True)
    field.add_argument('--field', required=True)
    quote = commands.add_parser('tcl-quote')
    quote.add_argument('--value', required=True)
    artifact = commands.add_parser('artifact-dir')
    artifact.add_argument('--elf', required=True)
    flash = commands.add_parser('flash')
    flash.add_argument('--board', required=True)
    flash.add_argument('--revision')
    flash.add_argument('--elf', required=True)
    flash.add_argument('--probe')
    flash.add_argument('--port')
    flash.add_argument('--mount', help='explicit mounted UF2 bootloader directory')
    flash.add_argument('--sdk-build')
    flash.add_argument('--commands')
    flash.add_argument('--tool-environment')
    flash.add_argument('--dry-run', action='store_true')
    args = parser.parse_args()
    try:
        if args.action == 'tool':
            print(executable(args.command, args.override_env))
        elif args.action == 'runner-field':
            print(board_info(args.board)['runner'][args.field])
        elif args.action == 'tcl-quote':
            print(tcl_quote(args.value))
        elif args.action == 'artifact-dir':
            elf = Path(args.elf).resolve()
            root = next((parent for parent in elf.parents if (parent / 'CMakeCache.txt').is_file()), elf.parent)
            print(root / 'artifact')
        else:
            plan = plan_flash(args)
            if args.dry_run:
                print(json.dumps(plan, indent=2))
            elif 'command_file' in plan:
                with tempfile.TemporaryDirectory(prefix='mcubsp flash ') as directory:
                    path = Path(directory) / 'flash.jlink'
                    path.write_text(plan['command_file'])
                    command = [str(path) if arg == '{command_file}' else arg for arg in plan['command']]
                    subprocess.run(command, check=True)
            else:
                subprocess.run(plan['command'], check=True)
    except (ValueError, OSError, KeyError, struct.error, subprocess.CalledProcessError) as error:
        parser.exit(2, f'mcubsp runner: {error}\n')


if __name__ == '__main__':
    main()
