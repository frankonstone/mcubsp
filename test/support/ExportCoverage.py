#!/usr/bin/env python3
"""Join executed native captures with their exact, explicitly registered manifests."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile


def export(registry, tool, source_root, output):
    output.mkdir(parents=True, exist_ok=True)
    # A failed export must not leave a previous report looking current.
    for name in ('native.mcucov.json', 'native.mcucov.html', 'native.lcov', 'producers.json', 'mcutrace.toml'):
        (output / name).unlink(missing_ok=True)
    records = [line.split('|') for line in registry.read_text().splitlines() if line]
    if not records or any(len(record) != 3 for record in records):
        raise ValueError('invalid or empty coverage registry')
    captures, producers = [], []
    with tempfile.TemporaryDirectory(prefix='mcubsp coverage ') as temporary:
        manifests = Path(temporary)
        index = 0
        for name, capture_path, manifest_dir in records:
            capture = Path(capture_path)
            if not capture.is_file() or not capture.stat().st_size:
                raise ValueError(f'{name}: missing executed capture: {capture}')
            paths = sorted(Path(manifest_dir).glob('*.mcucov.json'))
            if not paths:
                raise ValueError(f'{name}: no instrumentation manifests: {manifest_dir}')
            hashes = []
            for path in paths:
                data = json.loads(path.read_text())
                if not data.get('modules'):
                    continue
                hashes.append({'path': str(path), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()})
                # The host reader has a bounded JSON arena. One module per file
                # keeps FULL-profile reports within that bound without dropping
                # modules or changing capture/manifest identities.
                for module in data['modules']:
                    split = {**data, 'modules': [module]}
                    (manifests / f'{index}.mcucov.json').write_text(json.dumps(split))
                    index += 1
            captures.append(str(capture))
            producers.append({'suite': name, 'capture': str(capture),
                'sha256': hashlib.sha256(capture.read_bytes()).hexdigest(), 'manifests': hashes})
        if not index:
            raise ValueError('coverage manifests contain no modules')
        merged = output / 'native.mcuv'
        subprocess.run([str(tool), 'merge', *captures, '--output', str(merged)], check=True)
        for format_, name in [('json', 'native.mcucov.json'), ('html', 'native.mcucov.html'), ('lcov', 'native.lcov')]:
            subprocess.run([str(tool), 'report', '--input', str(merged), '--manifest', str(manifests),
                '--source-root', str(source_root), '--format', format_, '--output', str(output / name)], check=True)
        (output / 'producers.json').write_text(json.dumps(producers, indent=2) + '\n')
        # Optional analyzed graph: ordinary consumption keeps coverage optional.
        # This snapshot imports the instrumented run, not another build's tests.
        text = (source_root / 'mcutrace.toml').read_text()
        if 'root = "."' not in text or 'path = "output/artifact/native.json"' not in text:
            raise ValueError('update coverage graph export for the current project configuration')
        text = text.replace('root = "."', 'root = ' + json.dumps(str(source_root.resolve())), 1)
        text = text.replace('path = "output/artifact/native.json"',
            'path = ' + json.dumps(str((registry.parent / 'native.json').resolve())), 1)
        text = text.replace('[validation]', '[[artifacts]]\npath = ' +
            json.dumps(str((output / 'native.mcucov.json').resolve())) +
            '\nimporter = "mcucov"\n\n[validation]', 1)
        (output / 'mcutrace.toml').write_text(text)



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--registry', type=Path, required=True)
    parser.add_argument('--tool', type=Path, required=True)
    parser.add_argument('--source-root', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    export(args.registry, args.tool, args.source_root, args.output)


if __name__ == '__main__':
    main()
