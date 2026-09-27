#!/usr/bin/env python3
"""Select native CMake suites and board builds for changed repository paths."""
import argparse
import json
from pathlib import Path
import sys
from mcuhwlib.catalog import CatalogError
from mcuhwlib.impact import impact


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument("paths",nargs="*",help="changed paths; deleted/unknown/shared changes select all")
    args=parser.parse_args()
    try:
        print(json.dumps(impact(args.root,args.paths),indent=2,sort_keys=True))
        return 0
    except (CatalogError,OSError) as error:
        print(f"impact: {error}",file=sys.stderr)
        return 1

if __name__ == "__main__":
    raise SystemExit(main())
