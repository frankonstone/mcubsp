#!/usr/bin/env python3
"""Select full native and board verification without dependency metadata."""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys

from mcuhwlib.boards import discover
from mcuhwlib.verification import matrices


def selection(root, event, base=None, head="HEAD"):
    # Without a dependency catalog, run every native suite and declared board build.
    identifiers = sorted(board["id"] for board in discover(root))
    return {"schema_version": 1, "mode": "all", "changed_paths": [],
            "boards": identifiers, "suites": ["all"],
            "cmake_args": ["-DMCUHW_TEST_SUITES=all"],
            "reasons": ["Full verification without dependency selection"]}


def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument("--event",required=True)
    parser.add_argument("--base")
    parser.add_argument("--output",type=Path,required=True)
    parser.add_argument("--github-output",type=Path)
    args=parser.parse_args(argv)
    result=selection(args.root,args.event,args.base)
    result["cross_builds"]=matrices(args.root,result["boards"])
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(result,indent=2,sort_keys=True)+"\n",encoding="utf-8")
    if args.github_output:
        with args.github_output.open("a",encoding="utf-8") as output:
            output.write("suites="+";".join(result["suites"])+"\n")
            output.write("run_native="+("true" if result["suites"] else "false")+"\n")
            for environment, matrix in result["cross_builds"].items():
                name=environment.replace("-","_")
                output.write(name+"_matrix="+json.dumps(matrix,separators=(",",":"))+"\n")
                output.write("run_"+name+"="+("true" if matrix["include"] else "false")+"\n")
    print(json.dumps(result,indent=2,sort_keys=True))
    return 0


if __name__=="__main__":
    raise SystemExit(main())
