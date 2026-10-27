#!/usr/bin/env python3
"""Merge only the reports explicitly registered by the configured CTest suites."""
import argparse
import hashlib
import json
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument('--registry', type=Path, required=True)
parser.add_argument('--output', type=Path, required=True)
args = parser.parse_args()
args.output.unlink(missing_ok=True)
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
args.output.parent.mkdir(parents=True, exist_ok=True)
args.output.write_text(json.dumps({'format': 'mcutest-results', 'version': 1,
    'summary': {'tests': len(tests), 'failures': 0}, 'tests': tests,
    'producers': producers}, indent=2) + '\n')
