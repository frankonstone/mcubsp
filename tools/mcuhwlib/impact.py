"""Conservative changed-component and transitive-dependent verification selection.

@req REQ-0052
"""
from __future__ import annotations

from pathlib import Path, PurePosixPath
from . import catalog


def _key(value):
    return f"{value['kind']}:{value['id']}"


def impact(root, paths):
    """Return CMake suites/boards; uncertain, deleted and shared inputs select all.

    This is a CI/contributor operation and may enumerate manifests. It is never
    required by an application configure/build. Paths may be absolute inside root
    or slash-separated relative paths. Missing/deleted paths select all even if an
    old manifest still mentions them, since their previous dependency scope is not
    available from the current tree.
    """
    root = Path(root).resolve()
    components = catalog.discover(root)
    by_key = {_key(value):value for value in components}
    all_ids = sorted(by_key)
    changed, reasons, owners, reverse = [], [], {}, {key:set() for key in all_ids}
    for value in components:
        key = _key(value)
        owned = [f"{value['kind']}/{value['id']}/component.json"]
        owned += [path for field in catalog.FILE_FIELDS for path in value[field]]
        owned += [value["build"][field] for field in ("entry", "cmake", "profile_header") if value.get("build", {}).get(field)]
        for path in owned:
            owners.setdefault(path, set()).add(key)
        for dependency in value["depends"]:
            kind, identifier, _ = catalog.split_reference(dependency)
            dependency_key = f"{kind}:{identifier}"
            if dependency_key not in reverse:
                reasons.append(f"unresolved dependency {dependency_key}")
            else:
                reverse[dependency_key].add(key)
    seeds = set()
    shared = ("include/mcuhw/core/", "include/mcuhw/driver/", "include/mcuhw/drivers/", "include/mcuhw/arch/",
              "include/mcuhw/execution/", "include/mcuhw/testing/", "include/mcuhw/io/detail/", "cmake/", "tools/", "catalog/", ".github/")
    for raw in paths:
        path = Path(raw)
        if path.is_absolute():
            try:
                relative = path.resolve().relative_to(root).as_posix()
            except ValueError:
                changed.append(str(raw)); reasons.append(f"path outside repository: {raw}"); continue
        else:
            relative = PurePosixPath(str(raw).replace("\\", "/")).as_posix()
        changed.append(relative)
        if ".." in PurePosixPath(relative).parts:
            reasons.append(f"uncertain path: {relative}"); continue
        if not (root / relative).is_file():
            reasons.append(f"deleted or missing input: {relative}"); continue
        if relative.startswith(shared):
            reasons.append(f"shared infrastructure: {relative}"); continue
        file_owners = owners.get(relative, set())
        # Any new board/device-local narrative, source or fixture remains local
        # even before it is added to the explicit file arrays.
        if not file_owners:
            parts = PurePosixPath(relative).parts
            if len(parts) >= 4 and parts[0] in catalog.KINDS:
                component_key = f"{parts[0]}:{parts[1]}/{parts[2]}"
                if component_key in by_key:
                    file_owners = {component_key}
        if not file_owners:
            reasons.append(f"unknown dependency scope: {relative}")
        elif len(file_owners) > 1:
            reasons.append(f"shared component input: {relative}")
        else:
            seeds.update(file_owners)
    selected = set(all_ids) if reasons else set(seeds)
    queue = list(selected)
    while queue:
        for dependent in reverse.get(queue.pop(), ()):
            if dependent not in selected:
                selected.add(dependent); queue.append(dependent)
    if reasons:
        suites = ["all"]
    else:
        suites = {key for key in selected if key.split(":", 1)[0] in ("mcu", "ip", "device")}
        if selected:
            suites.update(("io", "compile"))
        if any(key.startswith(("mcu:", "ip:")) for key in selected):
            suites.update(("drivers", "descriptions"))
        # Board-local changes warrant the native contracts of the board's exact
        # dependency closure in addition to its external cross-build.
        for key in selected:
            value = by_key[key]
            if value["kind"] == "board":
                for dependency in catalog.closure(root, "board", value["id"])[1:]:
                    if dependency["kind"] in ("mcu", "ip", "device"):
                        suites.add(_key(dependency))
        suites = sorted(suites)
    return {"schema_version":1, "mode":"all" if reasons else "affected",
            "changed_paths":sorted(set(changed)), "reasons":sorted(set(reasons)),
            "components":sorted(selected), "boards":sorted(by_key[key]["id"] for key in selected if by_key[key]["kind"] == "board"),
            "suites":suites, "cmake_args":["-DMCUHW_TEST_SUITES=" + ";".join(suites)] if suites else []}
