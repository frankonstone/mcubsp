#!/usr/bin/env python3
"""Cross-build a declared board example with an explicitly provisioned toolchain."""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys

from mcuhwlib import boards, environment, workflow
from mcuhwlib.verification import board_actions


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--board", required=True)
    parser.add_argument("--revision")
    parser.add_argument("--example", required=True, help="Declared CMake project, e.g. examples/apps/blink")
    parser.add_argument("--build", type=Path, required=True)
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--jobs", type=int, default=2)
    args = parser.parse_args(argv)
    root, build = args.root.resolve(), args.build.resolve()
    report = build / "artifacts/cross-build.json"
    result = {"schema_version": 1, "stage": "cross-build", "state": "failed",
              "requested": {"board": args.board, "revision": args.revision, "example": args.example},
              "scope": "Compile/link only; no physical hardware execution."}
    try:
        # A failed selection/configure must never leave a previous successful
        # result or its copied identity evidence available for upload.
        for filename in ('cross-build.json', 'firmware.build.json', 'firmware.identity.json',
                         'firmware.bootloader.identity.json', 'firmware.flash.identity.json'):
            (report.parent / filename).unlink(missing_ok=True)
        environment.write_json(report, result)
        board = boards.load(root, args.board, args.revision)
        revision = args.revision or board["default_revision"]
        selected = [item for item in board_actions(root, [args.board])
                    if item["revision"] == revision and item["example"] == args.example]
        if len(selected) != 1:
            raise ValueError(f"{args.example} is not a declared example for {args.board}@{revision}")
        action = selected[0]
        result['action'] = action
        result['tools'] = {'python': {'path': sys.executable, 'version': platform.python_version()}}
        if action['environment'] == 'esp-idf':
            # The provisioned image is a transport for the SDK, not permission
            # to silently use a different SDK/compiler than the tool lock.
            lock = environment.load_lock(root / 'tools/toolchains.lock.json')
            sdk = os.environ.get('IDF_PATH')
            if not sdk:
                raise ValueError('ESP cross-build requires the provisioned IDF_PATH environment')
            result['tools']['esp-idf'] = environment.inspect_tool('esp-idf', lock['tools']['esp-idf'], Path(sdk))
            compiler = shutil.which(lock['tools']['riscv-gcc']['command'])
            if not compiler:
                raise ValueError('ESP cross-build requires the locked RISC-V compiler in PATH')
            result['tools']['riscv-gcc'] = environment.inspect_tool('riscv-gcc', lock['tools']['riscv-gcc'], Path(compiler))
        configure = [args.cmake, "-S", str(root / args.example), "-B", str(build), "-G", "Ninja",
                     f"-DMCUHW_SOURCE_DIR={root}", f"-DMCUHW_BOARD={args.board}",
                     f"-DMCUHW_BOARD_REVISION={revision}", "-DMCUHW_STARTUP=supplied",
                     f"-DCMAKE_TOOLCHAIN_FILE={root}/cmake/McuhwToolchain.cmake",
                     f"-DPython3_EXECUTABLE={sys.executable}", f"-DPYTHON={sys.executable}"]
        commands = [configure, [args.cmake, "--build", str(build), "--target", "firmware",
                                "--parallel", str(args.jobs)]]
        result['commands'] = commands
        environment.write_json(report, result)
        for command in commands:
            subprocess.run(command, check=True)
        record = workflow.load_build(build, {'board': args.board, 'revision': revision, 'startup': 'supplied'},
                                     str(build / 'firmware.build.json'))
        result['artifact'] = record
        result['evidence'] = []
        for filename in ('firmware.build.json', 'firmware.identity.json',
                         'firmware.bootloader.identity.json', 'firmware.flash.identity.json'):
            source = build / filename
            if source.is_file():
                shutil.copyfile(source, report.parent / filename)
                result['evidence'].append(filename)
        result["state"] = "passed"
        environment.write_json(report, result)
        return 0
    except (ValueError, OSError, KeyError, TypeError, subprocess.CalledProcessError) as error:
        result['error'] = str(error)
        try:
            environment.write_json(report, result)
        except OSError:
            pass  # Preserve the original failure diagnostic if output is unwritable.
        print(f"mcuhw cross-build: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
