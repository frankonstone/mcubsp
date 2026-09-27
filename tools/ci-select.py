#!/usr/bin/env python3
"""Translate a GitHub PR diff into the shared conservative impact selection.

Event names and commit IDs are data passed through environment/arguments, never
interpolated shell programs. Diff/catalog uncertainty selects all native suites.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
import re
import subprocess
import sys

from mcuhwlib.catalog import CatalogError, discover
from mcuhwlib.impact import impact
from mcuhwlib.verification import matrices


def selection(root, event, base=None, head="HEAD"):
    root=Path(root).resolve()

    def all_suites(reason):
        result={"schema_version":1,"mode":"all","changed_paths":[],"components":[],"boards":[],
                "suites":["all"],"cmake_args":["-DMCUHW_TEST_SUITES=all"],"reasons":[reason]}
        try:
            components=discover(root)
            result["components"]=sorted(f"{item['kind']}:{item['id']}" for item in components)
            result["boards"]=sorted(item["id"] for item in components if item["kind"]=="board")
        except (CatalogError,OSError):
            # The independent catalog validation step reports the actual error.
            # Suite selection must never turn uncertain metadata into a skip.
            pass
        return result

    if event != "pull_request":
        return all_suites(f"Full native verification for {event}")
    if not base or not re.fullmatch(r"[0-9a-fA-F]{40,64}",base):
        return all_suites("PR base commit unavailable; dependency scope uncertain")
    try:
        # Compare the checked-out PR merge result to its base. -z preserves paths
        # with spaces/newlines; no file names become shell syntax.
        diff=subprocess.run(["git","-C",str(root),"diff","--name-only","--no-renames","-z",base,head,"--"],
                            capture_output=True,check=True)
        paths=[path for path in diff.stdout.decode("utf-8").split("\0") if path]
        return impact(root,paths)
    except (CatalogError,OSError,UnicodeError,subprocess.CalledProcessError) as error:
        return all_suites(f"Cannot establish complete dependency scope: {error}")


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
