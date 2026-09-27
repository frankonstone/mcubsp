#!/usr/bin/env python3
"""Measure catalog growth against an unchanged, actually configured/built consumer.

Synthetic entries stay in a temporary tree and never claim hardware support.
@req REQ-0052
"""
from __future__ import annotations

import argparse
import copy
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tempfile
import time

from mcuhwlib import catalog, provenance


def run(argv, *, cwd=None):
    result = subprocess.run(list(map(str, argv)), cwd=cwd, text=True, capture_output=True)
    if result.returncode:
        raise RuntimeError(f"Command failed: {argv!r}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def timed(function):
    begin=time.perf_counter()
    result=function()
    return result, round(time.perf_counter()-begin, 6)


def copy_resources(root, destination):
    """Copy a self-contained resource tree without caches or fetched SDK/builds."""
    ignore=shutil.ignore_patterns("build", "__pycache__", ".git", "*.pdf", "*.png", "*.html", "*.svg", "*.pyc")
    for directory in ("include", "cmake", "tools", "tests", "board", "device", "mcu", "ip", "catalog", "doc", "examples"):
        shutil.copytree(root/directory, destination/directory, ignore=ignore)
    shutil.copy2(root/"CMakeLists.txt", destination/"CMakeLists.txt")
    for filename in ("README.md", "requirements.md", "implementation-plan.md"):
        if (root/filename).is_file(): shutil.copy2(root/filename,destination/filename)


def add_synthetic(root, boards=300, devices=3000):
    """Derive minimal local entries from real metadata, with no support claims."""
    board=catalog.load(root,"board","arduino/uno_r4_minima")
    device=catalog.load(root,"device","ti/tmp102")
    for kind,count,original in (("board",boards,board),("device",devices,device)):
        for number in range(count):
            value=copy.deepcopy(original)
            value["id"]=f"scale_fixture/{kind}_{number:04d}"
            value["name"]=f"Synthetic scaling {kind} {number}"
            value["summary"]="Synthetic scalability fixture only; not supported hardware."
            value["ownership"]={"maintainers":["synthetic benchmark fixture"]}
            value["tests"]=[]; value["examples"]=[]; value["evidence"]=[]
            value["limitations"]=["Synthetic metadata only. No host, build or physical support claim."]
            path=catalog.manifest_path(root,kind,value["id"])
            path.parent.mkdir(parents=True,exist_ok=True)
            path.write_text(json.dumps(value,indent=2)+"\n",encoding="utf-8")


def inventory(root):
    components,discovery=timed(lambda:catalog.discover(root))
    errors,validation=timed(lambda:catalog.validate(root))
    if errors:
        raise RuntimeError("Catalog validation failed:\n"+"\n".join(errors))
    selected,selection=timed(lambda:catalog.closure(root,"board","arduino/uno_r4_minima"))
    return {"component_count":len(components),"board_count":sum(c["kind"]=="board" for c in components),
            "device_count":sum(c["kind"]=="device" for c in components),
            "discovery_seconds":discovery,"validation_seconds":validation,"selection_seconds":selection,
            "selected_components":[f"{c['kind']}:{c['id']}" for c in selected]}


def targets_from_file_api(build):
    replies=build/".cmake/api/v1/reply"
    index=json.loads(next(replies.glob("index-*.json")).read_text())
    model=json.loads((replies/index["reply"]["codemodel-v2"]["jsonFile"]).read_text())
    return sorted(target["name"] for config in model["configurations"] for target in config["targets"])


def measure_build(root, application, build, mode, cmake, ninja, compiler):
    root, application, build = Path(root).resolve(), Path(application).resolve(), Path(build).resolve()
    if build.exists(): shutil.rmtree(build)
    query=build/".cmake/api/v1/query"
    query.mkdir(parents=True)
    (query/"codemodel-v2").write_text("")
    command=[cmake,"-S",application,"-B",build,"-G","Ninja",f"-DCMAKE_MAKE_PROGRAM={ninja}",
             "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",f"-DMCUHW_SOURCE_DIR={root}","-DMCUHW_INSTALL=OFF"]
    target="firmware.elf" if mode=="arm-blink" else "scale_consumer"
    if mode=="arm-blink":
        command += [f"-DCMAKE_TOOLCHAIN_FILE={root}/cmake/McuhwToolchain.cmake", f"-DMCUHW_ROOT={root}",
                    "-DMCUHW_BOARD=arduino/uno_r4_minima", "-DMCUHW_BOARD_REVISION=r4"]
    elif compiler:
        command += [f"-DCMAKE_CXX_COMPILER={compiler}"]
    configure_log,configure_seconds=timed(lambda:run(command))
    build_log,build_seconds=timed(lambda:run([cmake,"--build",build,"--target",target]))
    extra=[root/"board/arduino/uno_r4_minima/component.json",root/"board/arduino/uno_r4_minima/board.cmake"]
    identity=provenance.identity(root,build,target,ninja=ninja,extra=extra)
    targets=targets_from_file_api(build)
    compile_commands=json.loads((build/"compile_commands.json").read_text())
    source_files=sorted({Path(item["file"]).resolve().relative_to(root).as_posix()
                         if Path(item["file"]).resolve().is_relative_to(root) else
                         "application:"+Path(item["file"]).name for item in compile_commands})
    commands=run([ninja,"-t","commands",target],cwd=build).splitlines()
    return {"configure_seconds":configure_seconds,"build_seconds":build_seconds,
            "target_count":len(targets),"targets":targets,"compile_count":len(compile_commands),
            "compiled_sources":source_files,"build_command_count":len(commands),
            "identity":identity,"configure_log":configure_log,"build_log":build_log}


def benchmark(root, output, *, boards=300, devices=3000, mode="auto", compiler=None, cmake="cmake", ninja="ninja"):
    root=Path(root).resolve()
    if boards<300 or devices<3000:
        raise ValueError("Qualification requires at least 300 synthetic boards and 3000 devices")
    if mode=="auto": mode="arm-blink" if shutil.which("arm-none-eabi-g++") else "host-device"
    if mode=="host-device" and not compiler:
        compiler=os.environ.get("CXX") or shutil.which("g++-15") or shutil.which("c++")
    with tempfile.TemporaryDirectory(prefix="mcuhw-scale-") as temporary:
        work=Path(temporary).resolve(); copied=work/"resources"; application=work/"external application"; build=work/"build"
        copy_resources(root,copied)
        if mode=="arm-blink":
            shutil.copytree(copied/"examples/apps/blink",application)
        else:
            application.mkdir()
            (application/"CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.25)
project(mcuhw_scale_consumer LANGUAGES CXX)
add_subdirectory("${MCUHW_SOURCE_DIR}" mcuhw-library)
add_executable(scale_consumer main.cpp)
target_link_libraries(scale_consumer PRIVATE mcuhw::device)
''')
            (application/"main.cpp").write_text('''#include <mcuhw/device/sensor/tmp102.hpp>
int main() {
    constexpr mcuhw::device::tmp102_temperature sample{16};
    static_assert(sample.micro_celsius() == 1000000);
    return 0;
}
''')
        baseline=inventory(copied)
        baseline["build"]=measure_build(copied,application,build,mode,cmake,ninja,compiler)
        _,growth=timed(lambda:add_synthetic(copied,boards,devices))
        expanded=inventory(copied)
        expanded["build"]=measure_build(copied,application,build,mode,cmake,ninja,compiler)
        comparisons={key:baseline["build"][key]==expanded["build"][key]
                     for key in ("targets","compile_count","compiled_sources","build_command_count")}
        comparisons["selected_components"]=baseline["selected_components"]==expanded["selected_components"]
        comparisons["selected_input_files"]=baseline["build"]["identity"]["files"]==expanded["build"]["identity"]["files"]
        comparisons["selected_identity"]=baseline["build"]["identity"]["identity"]==expanded["build"]["identity"]["identity"]
        result={"schema_version":1,"requirement":"REQ-0052","state":"passed" if all(comparisons.values()) else "failed",
                "scope":"Clean external application builds before/after synthetic metadata growth; no physical evidence.",
                "mode":mode,"host":{"system":platform.system(),"release":platform.release(),"machine":platform.machine(),"python":platform.python_version()},
                "tools":{"cmake":run([cmake,"--version"]).splitlines()[0],"ninja":run([ninja,"--version"]).strip()},
                "synthetic":{"boards":boards,"devices":devices,"creation_seconds":growth},
                "baseline":baseline,"expanded":expanded,"unchanged":comparisons}
        revision=subprocess.run(["git","-C",str(root),"rev-parse","HEAD"],capture_output=True,text=True)
        result["repository_revision"]=revision.stdout.strip() if revision.returncode==0 else None
        # Temporary host paths are labeled in logs. Dependency identities preserve
        # exact compiler/system inputs to keep this a measured report.
        text=json.dumps(result,indent=2,sort_keys=True).replace(str(work),"$TEMP")+"\n"
        output=Path(output); output.parent.mkdir(parents=True,exist_ok=True); output.write_text(text,encoding="utf-8")
        return result


def main(argv=None):
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root",type=Path,default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output",type=Path)
    parser.add_argument("--boards",type=int,default=300)
    parser.add_argument("--devices",type=int,default=3000)
    parser.add_argument("--mode",choices=("auto","arm-blink","host-device"),default="auto")
    parser.add_argument("--compiler")
    parser.add_argument("--cmake",default="cmake")
    parser.add_argument("--ninja",default="ninja")
    args=parser.parse_args(argv)
    try:
        result=benchmark(args.root,args.output or args.root/"build/adoption/scaling.json",boards=args.boards,devices=args.devices,
                         mode=args.mode,compiler=args.compiler,cmake=args.cmake,ninja=args.ninja)
        print(f"{result['state']}: {result['synthetic']['boards']} boards + {result['synthetic']['devices']} devices; "
              f"{result['expanded']['build']['target_count']} selected targets, "
              f"{result['expanded']['build']['compile_count']} compiled sources ({result['mode']})")
        return 0 if result["state"]=="passed" else 1
    except (OSError,ValueError,RuntimeError) as error:
        print(f"scale: {error}",file=sys.stderr)
        return 1


if __name__=="__main__":
    raise SystemExit(main())
