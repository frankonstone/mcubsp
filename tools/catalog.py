#!/usr/bin/env python3
"""Validate and generate the hardware catalog without extra Python packages.

@req REQ-0048 REQ-0051
"""
from __future__ import annotations

import argparse
import os
from pathlib import Path
import sys

from mcuhwlib import catalog


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[1], help="repository or installed resource root")
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("validate", help="validate all manifests, references, source anchors and dependency graphs")
    for command in ("index", "docs"):
        current = sub.add_parser(command, help="generate deterministic " + ("JSON" if command == "index" else "Markdown"))
        current.add_argument("--output", type=Path, help="write file (default: stdout); docs links follow the output location")
        current.add_argument("--query", default="")
        for field in ("kind", "category", "capability", "revision", "runner", "stage", "state"):
            current.add_argument("--" + field)
    args = parser.parse_args(argv)
    try:
        errors = catalog.validate(args.root)
        if errors:
            print("\n".join(errors), file=sys.stderr)
            return 1
        if args.command == "validate":
            print(f"Catalog valid: {len(catalog.discover(args.root))} components")
            return 0
        components = catalog.search(args.root, args.query, **{
            field:getattr(args, field) for field in ("kind", "category", "capability", "revision", "runner", "stage", "state")
        })
        if args.command == "index":
            text = catalog.render_index(args.root, components)
        else:
            prefix = os.path.relpath(args.root.resolve(), args.output.resolve().parent).replace(os.sep, "/") if args.output else ""
            text = catalog.render_markdown(args.root, components, link_prefix=prefix)
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_text(text, encoding="utf-8")
        else:
            print(text, end="")
        return 0
    except (catalog.CatalogError, OSError) as error:
        print(f"catalog: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
