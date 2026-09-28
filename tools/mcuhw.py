#!/usr/bin/env python3
"""Optional mcuhw frontend. Every build remains an ordinary CMake project."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys

from mcuhwlib import boards, environment, runners, workflow

ROOT = Path(__file__).resolve().parents[1]


def project_context(args: argparse.Namespace) -> tuple[Path, dict, Path, dict]:
    project = Path(args.project).expanduser().resolve()
    config = environment.read_json(project / 'mcuhw.json')
    root = Path(config['mcuhw_root']).expanduser()
    if not root.is_absolute():
        root = (project / root).resolve()
    board = boards.load(root, config['board'], revision=config['revision'])
    board['selected_revision'] = config['revision']
    return project, config, root, board


def _lock_path(project: Path) -> Path:
    return project / '.mcuhw' / 'toolchains.lock.json'


def _state_path(project: Path) -> Path:
    return project / '.mcuhw' / 'environment.json'


def _state(project: Path) -> dict:
    path = _state_path(project)
    if not path.is_file():
        raise environment.EnvironmentError(f'No recorded setup in {project}; run mcuhw.py setup --project "{project}"')
    state = environment.read_json(path)
    if state.get('lock_sha256') != environment.digest(_lock_path(project)):
        raise environment.EnvironmentError('The project tool lock changed; run setup explicitly to record it')
    interpreter = state.get('frontend_python')
    if not interpreter or not Path(interpreter['path']).is_file() or environment.digest(Path(interpreter['path'])) != interpreter['sha256']:
        raise environment.EnvironmentError('The recorded frontend Python is missing or changed; run setup explicitly to record this interpreter')
    return state


def _build_dir(project: Path, config: dict, state: dict) -> Path:
    return workflow.build_directory(project, config, state)


def create(args: argparse.Namespace) -> int:
    root = Path(args.root).expanduser().resolve()
    board = boards.load(root, args.board, revision=args.revision)
    revision = args.revision or board['default_revision']
    entry = f'examples/apps/{args.example}/CMakeLists.txt'
    if entry not in board['examples']:
        raise environment.EnvironmentError(
            f'Example {args.example} is not declared for {args.board}')
    destination = Path(args.destination).expanduser().absolute()
    if destination.exists() and any(destination.iterdir()):
        raise environment.EnvironmentError(f'Application directory is not empty: {destination}')
    source = root / 'examples' / 'apps' / args.example
    if not source.is_dir():
        raise environment.EnvironmentError(f'Example template missing: {source}')
    shutil.copytree(source, destination, dirs_exist_ok=True,
                    ignore=shutil.ignore_patterns('build', '__pycache__', '.mcuhw', 'mcuhw.json'))
    config = {'schema_version': 1, 'mcuhw_root': str(root), 'board': args.board,
              'revision': revision, 'startup': args.startup, 'example': args.example}
    environment.write_json(destination / 'mcuhw.json', config)
    (destination / '.mcuhw').mkdir(exist_ok=True)
    shutil.copyfile(root / 'tools' / 'toolchains.lock.json', _lock_path(destination))
    (destination / '.gitignore').write_text('build/\nCMakeUserPresets.json\n.mcuhw/environment.json\n.mcuhw/CMakePresets.json\n.mcuhw/verify-environment.cmake\n.mcuhw/last-build.json\n', encoding='utf-8')
    print(f'Created {args.example} for {args.board}@{revision} in {destination}')
    print(f'Next: python "{root / "tools/mcuhw.py"}" setup --project "{destination}"')
    return 0


def setup(args: argparse.Namespace) -> int:
    project, config, root, board = project_context(args)
    lock_path = Path(args.lock).expanduser().resolve() if args.lock else _lock_path(project)
    lock = environment.load_lock(lock_path)
    overrides = {}
    for value in args.tool:
        name, separator, path = value.partition('=')
        if not separator or not path:
            raise environment.EnvironmentError('--tool requires NAME=/path/to/tool')
        overrides[name] = path
    names = environment.required_tools(board, args.operation)
    state = environment.setup(lock, names, overrides, Path(args.cache).expanduser(),
                              offline=args.offline, allow_override=args.allow_version_override)
    # Imports and cache setup are validated before replacing a working environment.
    errors = environment.verify(state, lock, names)
    if errors:
        raise environment.EnvironmentError('\n'.join(errors))
    old_path = _state_path(project)
    if old_path.is_file() and lock_path == _lock_path(project):
        old = environment.read_json(old_path)
        if old.get('lock_sha256') == environment.digest(lock_path):
            state['tools'] = {**old['tools'], **state['tools']}
    if lock_path != _lock_path(project):
        environment.write_json(_lock_path(project), lock)
    state['lock_sha256'] = environment.digest(_lock_path(project))
    state['build_tools'] = environment.required_tools(board, 'build')
    state['board'] = config['board']
    state['revision'] = config['revision']
    state['mcuhw_root'] = str(root)
    state['frontend_python'] = {'path': sys.executable, 'version': sys.version,
                                'sha256': environment.digest(Path(sys.executable))}
    if all(name in state['tools'] for name in state['build_tools']):
        workflow.install_presets(project, config, root, state)
    environment.write_json(old_path, state)
    print(json.dumps(state, indent=2, sort_keys=True))
    return 0


def doctor(args: argparse.Namespace) -> int:
    project, config, _, board = project_context(args)
    lock = environment.load_lock(_lock_path(project))
    state = _state(project)
    names = environment.required_tools(board, args.operation)
    errors = environment.verify(state, lock, names)
    report = {'board': config['board'], 'revision': config['revision'], 'startup': config['startup'],
              'operation': args.operation, 'host': environment.host_id(),
              'frontend_python': state['frontend_python'],
              'tools': {name: state['tools'].get(name) for name in names}, 'errors': errors}
    print(json.dumps(report, indent=2, sort_keys=True))
    return 1 if errors else 0


def build_commands(project: Path, config: dict, root: Path, state: dict) -> tuple[Path, list[list[str]]]:
    preset = workflow.selected_preset(project)
    selected = preset['cacheVariables']
    for field, variable in [('board', 'MCUHW_BOARD'), ('revision', 'MCUHW_BOARD_REVISION'), ('startup', 'MCUHW_STARTUP')]:
        if selected[variable] != config[field]:
            raise environment.EnvironmentError('Application selection changed; run setup to update the shared CMake preset')
    cmake = state['tools']['cmake']['path']
    return Path(preset['binaryDir']), [[cmake, '--preset', workflow.PRESET],
                                      [cmake, '--build', '--preset', workflow.PRESET]]


def build(args: argparse.Namespace) -> int:
    project, config, root, board = project_context(args)
    state = _state(project)
    lock = environment.load_lock(_lock_path(project))
    errors = environment.verify(state, lock, environment.required_tools(board, 'build'))
    if errors:
        raise environment.EnvironmentError('\n'.join(errors))
    directory, commands = build_commands(project, config, root, state)
    if args.dry_run:
        print(json.dumps({'build_dir': str(directory), 'cwd': str(project), 'commands': commands,
                          'preset': workflow.selected_preset(project)}, indent=2))
        return 0
    env = environment.process_environment(state, environment.required_tools(board, 'build'))
    for command in commands:
        subprocess.run(command, check=True, env=env, cwd=project)
    # CMake also writes this record for IDE/direct builds. Keep a fallback for
    # projects created against an older build helper during migration.
    manifest = directory / 'firmware.build.json'
    if manifest.is_file():
        workflow.load_build(project, config, str(manifest))
        environment.write_json(project / '.mcuhw/last-build.json', {'schema_version': 1, 'manifest': str(manifest)})
    else:
        workflow.record_build(directory, board=config['board'], revision=config['revision'],
            startup=config['startup'], root=root, artifact=directory / 'firmware.elf',
            sdk_build=directory / 'esp-idf-build' if (directory / 'esp-idf-build/flasher_args.json').is_file() else None,
            pointer=project / '.mcuhw/last-build.json')
    print(f'Built {directory / "firmware.elf"}')
    return 0


def flash(args: argparse.Namespace) -> int:
    project, config, _, board = project_context(args)
    state = _state(project)
    lock = environment.load_lock(_lock_path(project))
    errors = environment.verify(state, lock, environment.required_tools(board, 'flash'), probe=False)
    if errors:
        raise environment.EnvironmentError('\n'.join(errors))
    record = workflow.load_build(project, config, args.build)
    directory = Path(record['build_dir'])
    plan = runners.plan(board, directory, state['tools'], probe=args.probe, port=args.port,
                        artifact=Path(record['artifacts']['elf']),
                        sdk_build=Path(record['artifacts']['sdk_build']) if record['artifacts'].get('sdk_build') else None,
                        require_artifacts=True)
    print(json.dumps(plan, indent=2))
    if args.dry_run:
        return 0
    # Executing flash is reachable only through the user's explicit flash command.
    runners.execute(plan, env=environment.process_environment(state, environment.required_tools(board, 'flash')))
    return 0


def parser() -> argparse.ArgumentParser:
    main = argparse.ArgumentParser(description=__doc__)
    sub = main.add_subparsers(dest='command', required=True)
    item = sub.add_parser('new', help='Create a complete external CMake application')
    item.add_argument('destination')
    item.add_argument('--board', required=True)
    item.add_argument('--revision')
    item.add_argument('--example', choices=('blink', 'tmp102'), default='blink')
    item.add_argument('--startup', choices=('supplied', 'external'), default='supplied')
    item.add_argument('--root', default=str(ROOT))
    item.set_defaults(function=create)
    for command, function in [('setup', setup), ('doctor', doctor), ('build', build), ('flash', flash)]:
        item = sub.add_parser(command)
        item.add_argument('--project', default='.')
        item.set_defaults(function=function)
        if command in ('setup', 'doctor'):
            item.add_argument('--operation', choices=('build', 'flash'), default='build')
        if command == 'setup':
            item.add_argument('--tool', action='append', default=[], metavar='NAME=PATH')
            item.add_argument('--cache', default=str(environment.default_cache()))
            item.add_argument('--lock', help='Explicit alternate tool lock to copy into the project')
            item.add_argument('--offline', action='store_true')
            item.add_argument('--allow-version-override', action='store_true')
        if command in ('build', 'flash'):
            item.add_argument('--dry-run', action='store_true')
        if command == 'flash':
            item.add_argument('--build', help='Completed build directory or mcuhw-build.json (default: last recorded build)')
            item.add_argument('--probe', help='J-Link or ST-LINK probe serial number')
            item.add_argument('--port', help='ESP32 serial port, for example COM4 or /dev/ttyACM0')
    return main


def main(argv: list[str] | None = None) -> int:
    args = parser().parse_args(argv)
    try:
        return args.function(args)
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError) as error:
        print(f'mcuhw: {error}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    sys.exit(main())
