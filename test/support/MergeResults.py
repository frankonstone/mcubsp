#!/usr/bin/env python3
"""Merge only the reports explicitly registered by the configured CTest suites."""
import argparse
import hashlib
import json
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--registry', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
parser.add_argument('--shard-dir', type=Path)
parser.add_argument('--publish-dir', type=Path)
args = parser.parse_args()
if args.publish_dir and not args.shard_dir:
    parser.error('--publish-dir requires --shard-dir')
# Eight fixed, configured imports preserve the same tests and evidence policy
# while keeping each mcutrace/mcujson DOM below its fixed string arena.
SHARDS = 8
args.output.unlink(missing_ok=True)
for directory in (args.shard_dir, args.publish_dir):
    if directory:
        for index in range(SHARDS):
            (directory / f'{index}.json').unlink(missing_ok=True)
reports = [Path(line) for line in args.registry.read_text().splitlines() if line]
if not reports or len(set(reports)) != len(reports):
    raise SystemExit('Native report registry is empty or contains duplicates')
tests, producers = [], []
for path in reports:
    raw = path.read_bytes()
    report = json.loads(raw)
    if report.get('format') != 'mcutest-results' or report.get('version') != 1:
        raise SystemExit(f'{path}: unsupported result format')
    cases = report['tests']
    if report['summary']['tests'] != len(cases):
        raise SystemExit(f'{path}: test count does not match producer data')
    if report['summary']['failures'] or any(case['status'] != 'passed' for case in cases):
        raise SystemExit(f'{path}: producer contains unsuccessful or skipped cases')
    for case in cases:
        tests.append({**case, 'name': path.stem + '/' + case['name']})
    producers.append({'path': str(path), 'sha256': hashlib.sha256(raw).hexdigest()})
merged = {'format': 'mcutest-results', 'version': 1,
    'summary': {'tests': len(tests), 'failures': 0}, 'tests': tests,
    'producers': producers}

def write_atomic(path, report):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(report, indent=2) + '\n')
    temporary.replace(path)

if args.shard_dir:
    partitions = [[] for _ in range(SHARDS)]
    for case in tests:
        index = int.from_bytes(hashlib.sha256(case['name'].encode()).digest()[:4], 'big') % SHARDS
        partitions[index].append(case)
    for index, cases in enumerate(partitions):
        report = {**merged, 'summary': {'tests': len(cases), 'failures': 0}, 'tests': cases}
        write_atomic(args.shard_dir / f'{index}.json', report)
        if args.publish_dir:
            write_atomic(args.publish_dir / f'{index}.json', report)
write_atomic(args.output, merged)
