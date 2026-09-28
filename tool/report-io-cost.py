#!/usr/bin/env python3
"""Build fresh paired RA4M1 linked size probes; never runs or flashes them."""

from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import shutil
import subprocess


ROOT = Path(__file__).resolve().parent.parent
SCENARIOS = ("gpio", "uart_blocking", "uart_async", "spi_dma")
FLAGS = [
    "-std=c++23", "-Os", "-mcpu=cortex-m4", "-mthumb", "-ffreestanding",
    "-fno-exceptions", "-fno-rtti", "-fno-threadsafe-statics",
    "-fno-use-cxa-atexit", "-ffunction-sections", "-fdata-sections",
    "-fno-unwind-tables", "-fno-asynchronous-unwind-tables", "-fstack-usage",
    "-Wall", "-Wextra", "-Wpedantic", "-Werror",
]


def run(command: list[str], cwd: Path | None = None) -> str:
    return subprocess.run(command, cwd=cwd, check=True, text=True,
                          stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def input_hashes() -> dict[str, str]:
    inputs = sorted((ROOT / "include" / "mcuhw").rglob("*.hpp"))
    inputs += sorted((ROOT / "test" / "io_cost").glob("*"))
    inputs += [Path(__file__).resolve()]
    return {str(path.relative_to(ROOT)): digest(path) for path in inputs
            if path.is_file()}


def sections(size_tool: str, elf: Path) -> dict[str, int]:
    result = {}
    for line in run([size_tool, "-A", str(elf)]).splitlines():
        fields = line.split()
        if len(fields) >= 3 and fields[0].startswith("."):
            result[fields[0]] = int(fields[1], 0)
    unexpected = set(result) - {
        ".text", ".ARM.extab", ".ARM.exidx", ".init_array", ".data",
        ".bss", ".ARM.attributes",
    }
    if unexpected:
        raise RuntimeError(f"Unclassified sections in {elf}: {unexpected}")
    return result


def stack_records(directory: Path) -> list[dict[str, str | int]]:
    records = []
    for source in sorted(directory.glob("*.su")):
        for line in source.read_text().splitlines():
            fields = line.rsplit("\t", 2)
            if len(fields) == 3:
                records.append({"function": fields[0], "bytes": int(fields[1]),
                                "kind": fields[2]})
    return sorted(records, key=lambda item: int(item["bytes"]), reverse=True)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", default="arm-none-eabi-g++")
    parser.add_argument("--size-tool", default="arm-none-eabi-size")
    parser.add_argument("--output-dir", type=Path,
                        default=ROOT / "build" / "io-cost")
    parser.add_argument("--scenario", choices=SCENARIOS, action="append")
    args = parser.parse_args()
    compiler = shutil.which(args.compiler)
    size_tool = shutil.which(args.size_tool)
    if compiler is None or size_tool is None:
        parser.error("arm-none-eabi-g++ and arm-none-eabi-size must be available")
    output = args.output_dir.resolve()
    output.mkdir(parents=True, exist_ok=True)
    # A failed invocation must never leave an older report looking current.
    for filename in ("report.json", "report.md"):
        (output / filename).unlink(missing_ok=True)

    hashes = input_hashes()
    report = {
        "schema_version": 1,
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "target": "RA4M1 / Cortex-M4 / GNU Arm EABI",
        "compiler": {"path": compiler, "version": run([compiler, "--version"]).strip()},
        "size_tool": {"path": size_tool, "version": run([size_tool, "--version"]).strip()},
        "source_sha256": hashlib.sha256(json.dumps(hashes, sort_keys=True).encode()).hexdigest(),
        "input_sha256": hashes,
        "notes": [
            "Build-only probes: no reset vector or startup. Do not flash these ELFs.",
            "Same target, optimization, compiler, runtime, linker script and application buffers per pair.",
            "Baseline uses existing checked drivers; facade adds board composition and shared ownership.",
            "Flash counts text, read-only data, unwind/init tables, and initialized data load image.",
            "Static RAM counts data + bss; excludes heap and stack. No heap is provided.",
            "Compiler stack records are per-function frames, not worst-case call-chain/interrupt stack usage.",
            "These numbers do not establish execution latency or functional hardware behavior.",
        ],
        "probes": [],
    }
    scenarios = args.scenario or SCENARIOS
    for scenario in scenarios:
        for interface in ("driver", "facade"):
            directory = output / f"{scenario}_{interface}"
            directory.mkdir(exist_ok=True)
            for stale in directory.glob("*"):
                if stale.is_file():
                    stale.unlink()
            commands = []
            objects = []
            for source in (ROOT / "test/io_cost/runtime.cpp",
                           ROOT / f"test/io_cost/{scenario}.cpp"):
                obj = directory / f"{source.stem}.o"
                command = [compiler, *FLAGS, f"-I{ROOT / 'include'}",
                           f"-DMCUHW_IO_COST_FACADE={int(interface == 'facade')}",
                           "-c", str(source), "-o", str(obj)]
                run(command, directory)
                commands.append(command)
                objects.append(str(obj))
            elf = directory / "probe.elf"
            command = [compiler, "-mcpu=cortex-m4", "-mthumb", "-nostdlib",
                       f"-T{ROOT / 'test/io_cost/probe.ld'}",
                       "-Wl,--gc-sections", "-Wl,--build-id=none", "-Wl,--strip-debug",
                       f"-Wl,-Map,{directory / 'probe.map'}", *objects,
                       "-lgcc", "-o", str(elf)]
            run(command, directory)
            commands.append(command)
            sizes = sections(size_tool, elf)
            records = stack_records(directory)
            report["probes"].append({
                "scenario": scenario, "interface": interface,
                "flash_bytes": sum(value for section, value in sizes.items()
                                   if section not in (".bss", ".ARM.attributes")),
                "static_ram_bytes": sizes.get(".data", 0) + sizes.get(".bss", 0),
                "sections": sizes,
                "largest_function_stack_frame_bytes": max(
                    (int(record["bytes"]) for record in records), default=0),
                "stack_records": records, "commands": commands,
                "elf_sha256": digest(elf), "elf": str(elf),
            })
    if hashes != input_hashes():
        raise RuntimeError("Probe sources changed during compilation; rerun for a consistent report")
    (output / "report.json").write_text(json.dumps(report, indent=2) + "\n")
    lines = ["# RA4M1 I/O facade cost", "", report["compiler"]["version"].splitlines()[0],
             "", "Flags: `" + " ".join(FLAGS) + "`", "",
             "| Probe | Driver flash | Facade flash | Δ flash | Driver RAM | Facade RAM | Δ RAM |",
             "|---|---:|---:|---:|---:|---:|---:|"]
    for scenario in scenarios:
        driver, facade = [probe for probe in report["probes"] if probe["scenario"] == scenario]
        lines.append(f"| {scenario} | {driver['flash_bytes']} | {facade['flash_bytes']} | "
                     f"{facade['flash_bytes'] - driver['flash_bytes']:+d} | "
                     f"{driver['static_ram_bytes']} | {facade['static_ram_bytes']} | "
                     f"{facade['static_ram_bytes'] - driver['static_ram_bytes']:+d} |")
    lines += ["", "All sizes are bytes. Source SHA-256: `" + report["source_sha256"] + "`.", ""]
    lines += ["- " + note for note in report["notes"]]
    lines += ["", "See `report.json` for all commands, source hashes, sections and compiler stack records.", ""]
    (output / "report.md").write_text("\n".join(lines))
    print("\n".join(lines))


if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as error:
        raise SystemExit(f"Command failed: {error.cmd!r}\n{error.stdout}{error.stderr}") from error
