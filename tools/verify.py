"""Configure selected native suites, run them, and merge only this run's evidence.

@req REQ-0035 REQ-0036 REQ-0052
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def evidence_paths(registry, build):
    """CTest is authoritative: never glob old artifacts from other selections."""
    paths = []
    for test in registry["tests"]:
        for argument in test.get("command", []):
            if argument.startswith("-DOUTPUT_FILE="):
                path = Path(argument.partition("=")[2]).resolve()
                if not path.is_relative_to(build.resolve()):
                    raise ValueError(f"Test evidence escapes build directory: {path}")
                paths.append(path)
    return sorted(set(paths))


def merge_evidence(paths, output):
    tests, artifacts = [], []
    for path in paths:
        if not path.is_file():
            raise ValueError(f"Selected test did not write evidence: {path}")
        report = json.loads(path.read_text())
        if report.get("format") != "mcutest-results" or report.get("version") != 1:
            raise ValueError(f"Unsupported test evidence: {path}")
        tests.extend(report["tests"])
        artifacts.append(str(path))
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps({"format": "mcutest-results", "version": 1,
        "summary": {"tests": len(tests), "failures": sum(t["status"] == "failed" for t in tests)},
        "artifacts": artifacts, "tests": tests}, indent=2) + "\n")
    return len(tests)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=ROOT / "build/verification")
    parser.add_argument("--suites", default="all", help="all or semicolon-separated native suite IDs")
    parser.add_argument("--mcutest", type=Path, default=ROOT.parent / "mcutest")
    parser.add_argument("--compiler")
    parser.add_argument("--tooling", action="store_true")
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--ctest", default="ctest")
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--existing", action="store_true", help="Build/test an already configured maintainer tree")
    args = parser.parse_args()
    build = args.build.resolve()
    output = build / "artifacts/mcutest-native.json"
    # Invalidate aggregate evidence before any producer step can fail.
    output.unlink(missing_ok=True)
    try:
        if not args.existing:
            command = [args.cmake, "-S", str(ROOT), "-B", str(build), "-G", "Ninja",
                "-DMCUHW_BUILD_TESTS=ON", "-DMCUHW_BUILD_PROBES=OFF",
                "-DMCUHW_BUILD_MCUIP_CONTRACTS=OFF", "-DMCUHW_CHECK_HEADERS=OFF",
                "-DMCUHW_WRITE_TOOLCHAIN_REPORT=ON", "-DMCUHW_BOARD_TESTS=",
                "-DMCUHW_BUILD_TOOLING_TESTS=" + ("ON" if args.tooling else "OFF"),
                "-DMCUHW_TEST_SUITES=" + args.suites,
                "-DMCUHW_MCUTEST_SOURCE_DIR=" + str(args.mcutest.resolve()),
                "-DPython3_EXECUTABLE=" + sys.executable]
            if args.compiler:
                command.append("-DCMAKE_CXX_COMPILER=" + args.compiler)
            subprocess.run(command, check=True)
        subprocess.run([args.cmake, "--build", str(build), "--parallel", str(args.jobs)], check=True)
        command = [args.ctest, "--test-dir", str(build)]
        registry = json.loads(subprocess.check_output(command + ["--show-only=json-v1"], text=True))
        if not registry["tests"]:
            raise ValueError("No selected tests; refusing an empty verification run")
        paths = evidence_paths(registry, build)
        for path in paths:
            path.unlink(missing_ok=True)
        result = subprocess.run(command + ["--output-on-failure", "--parallel", str(args.jobs)])
        count = merge_evidence(paths, output)
        print(f"Recorded {count} native cases from {len(paths)} selected suites: {output}")
        return result.returncode
    except (subprocess.CalledProcessError, OSError, ValueError, KeyError) as error:
        print(f"mcuhw verification: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
