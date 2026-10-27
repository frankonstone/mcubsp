#!/usr/bin/env python3
"""Preserve all selected coverage modules in bounded, uniquely named manifests."""
import argparse
import json
from pathlib import Path


def split_manifests(inputs, output, prefixes=()):
    output.mkdir(parents=True, exist_ok=True)
    if any(output.iterdir()):
        raise ValueError('manifest output directory must be empty')
    count = 0
    for source in inputs:
        paths = sorted(source.glob('*.mcucov.json')) if source.is_dir() else [source]
        if not paths:
            raise ValueError(f'no coverage manifests: {source}')
        for path in paths:
            manifest = json.loads(path.read_text())
            for module in manifest['modules']:
                if prefixes and not module['path'].startswith(tuple(prefixes)):
                    continue
                destination = output / f'{count}.mcucov.json'
                destination.write_text(json.dumps({**manifest, 'modules': [module]}))
                count += 1
    if not count:
        raise ValueError('no selected coverage modules')
    return count


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input', type=Path, action='append', required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--prefix', action='append', default=[])
    args = parser.parse_args()
    split_manifests(args.input, args.output, args.prefix)


if __name__ == '__main__':
    main()
