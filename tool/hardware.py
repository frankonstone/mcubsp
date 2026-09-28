#!/usr/bin/env python3
"""Shared host operations used by board-local physical test fixtures.

Fixtures own observations, timing and result interpretation. This entry point
owns tool selection, board runner configuration, flashing and artifact paths.
"""
from __future__ import annotations
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

from mcuhwlib import boards, environment, runners

ROOT = Path(__file__).resolve().parents[1]


def recorded_tools() -> dict:
    filename = os.environ.get('MCUHW_ENVIRONMENT_JSON')
    return environment.read_json(Path(filename))['tools'] if filename else {}


def resolve_tool(name: str, command: str, override: str | None = None) -> str:
    selected = recorded_tools().get(name, {})
    candidate = (os.environ.get(override) if override else None) or selected.get('path') or command
    path = shutil.which(candidate)
    if not path and name == 'mcucov' and candidate == command:
        # Preserve the established local-development layout in one place.
        paths = sorted((ROOT.parent / 'mcucov/output/install').glob('**/bin/mcucov'))
        path = next((str(item) for item in paths if os.access(item, os.X_OK)), None)
    if not path:
        raise environment.EnvironmentError(f'Missing {name}: {candidate}; install it or set {override or "MCUHW_ENVIRONMENT_JSON"}')
    return str(Path(path).absolute())


def flash_plan(args) -> dict:
    board = boards.load(ROOT, args.board, revision=args.revision)
    board['selected_revision'] = args.revision or board['default_revision']
    runner = board['build']['runner']
    spec = environment.load_lock(ROOT / 'tool/toolchains.lock.json')['tools'][runner['tool']]
    command = spec.get('windows_command', spec.get('command')) if os.name == 'nt' else spec.get('command')
    tool = resolve_tool(runner['tool'], command, args.tool_environment)
    return runners.plan(board, args.elf.resolve().parent, {runner['tool']: {'path': tool}},
                        probe=args.probe, port=args.port, artifact=args.elf.resolve(),
                        sdk_build=args.sdk_build.resolve() if args.sdk_build else None,
                        openocd_tail=args.commands, require_artifacts=True)


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='operation', required=True)
    tool = sub.add_parser('tool')
    tool.add_argument('--name', required=True)
    tool.add_argument('--command', required=True)
    tool.add_argument('--override-env')
    directory = sub.add_parser('artifact-dir')
    directory.add_argument('--elf', type=Path, required=True)
    field = sub.add_parser('runner-field')
    field.add_argument('--board', required=True)
    field.add_argument('--field', required=True)
    flash = sub.add_parser('flash')
    flash.add_argument('--board', required=True)
    flash.add_argument('--revision')
    flash.add_argument('--elf', type=Path, required=True)
    flash.add_argument('--sdk-build', type=Path)
    flash.add_argument('--probe', default=os.environ.get('MCUHW_PROBE_SERIAL'))
    flash.add_argument('--port', default=os.environ.get('MCUHW_SERIAL_PORT'))
    flash.add_argument('--tool-environment')
    flash.add_argument('--commands', help='Board fixture OpenOCD commands after programming; retains one debug session')
    flash.add_argument('--dry-run', action='store_true')
    args = parser.parse_args(argv)
    try:
        if args.operation == 'tool':
            print(resolve_tool(args.name, args.command, args.override_env))
        elif args.operation == 'artifact-dir':
            print(args.elf.resolve().parent / 'artifact')
        elif args.operation == 'runner-field':
            print(boards.load(ROOT, args.board)['build']['runner'][args.field])
        else:
            plan = flash_plan(args)
            if args.dry_run:
                print(json.dumps(plan, indent=2))
            else:
                runners.execute(plan)
        return 0
    except (ValueError, OSError, KeyError, subprocess.CalledProcessError) as error:
        print(f'mcuhw hardware: {error}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
