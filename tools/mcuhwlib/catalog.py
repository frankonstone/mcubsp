"""Versioned local component catalog; normal selection performs no catalog scan.

@req REQ-0048 REQ-0051
Public APIs accept pathlib.Path or string repository roots and return JSON dicts.
"""
from __future__ import annotations

import hashlib
import json
import re
from pathlib import Path, PurePosixPath

KINDS = ("board", "device", "ip", "mcu")
ID_PATTERN = r"[a-z0-9][a-z0-9_-]*/[a-z0-9][a-z0-9_.-]*"
REF_PATTERN = rf"(board|device|ip|mcu):({ID_PATTERN})(?:@([a-z0-9][a-z0-9_.-]*))?"
FILE_FIELDS = ("headers", "sources", "docs", "tests", "examples")


class CatalogError(ValueError):
    """An invalid manifest, reference or requested composition."""


def _pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise CatalogError(f"duplicate JSON key {key!r}")
        result[key] = value
    return result


def _json(path):
    try:
        return json.loads(Path(path).read_text(encoding="utf-8"), object_pairs_hook=_pairs)
    except (OSError, UnicodeError, json.JSONDecodeError) as error:
        raise CatalogError(f"{path}: {error}") from error


def _schema_errors(value, schema, at="manifest"):
    """Validate exactly the JSON Schema vocabulary used in catalog/schema.json."""
    errors = []
    types = {"object": dict, "array": list, "string": str, "integer": int, "boolean": bool}
    wanted = schema.get("type")
    if wanted and (not isinstance(value, types[wanted]) or wanted == "integer" and isinstance(value, bool)):
        return [f"{at}: expected {wanted}"]
    if "const" in schema and value != schema["const"]:
        errors.append(f"{at}: unsupported value {value!r}; expected {schema['const']!r}")
    if "enum" in schema and value not in schema["enum"]:
        errors.append(f"{at}: expected one of {schema['enum']}")
    if isinstance(value, str):
        if len(value) < schema.get("minLength", 0):
            errors.append(f"{at}: must not be empty")
        if "pattern" in schema and not re.fullmatch(schema["pattern"], value):
            errors.append(f"{at}: invalid value {value!r}")
    if isinstance(value, dict):
        for key in schema.get("required", []):
            if key not in value:
                errors.append(f"{at}: missing {key}")
        properties = schema.get("properties", {})
        for key, item in value.items():
            if key in properties:
                errors.extend(_schema_errors(item, properties[key], f"{at}.{key}"))
            elif schema.get("additionalProperties") is False:
                errors.append(f"{at}: unknown field {key}")
            elif isinstance(schema.get("additionalProperties"), dict):
                errors.extend(_schema_errors(item, schema["additionalProperties"], f"{at}.{key}"))
    if isinstance(value, list):
        if len(value) < schema.get("minItems", 0):
            errors.append(f"{at}: requires at least {schema['minItems']} item(s)")
        if schema.get("uniqueItems") and len({json.dumps(x, sort_keys=True) for x in value}) != len(value):
            errors.append(f"{at}: duplicate items")
        for index, item in enumerate(value):
            errors.extend(_schema_errors(item, schema.get("items", {}), f"{at}[{index}]"))
    return errors


def _path(root, value):
    path = PurePosixPath(value)
    if path.is_absolute() or ".." in path.parts or "\\" in value or not path.parts:
        raise CatalogError(f"invalid repository-relative path {value!r}")
    resolved = (Path(root) / path).resolve()
    if not resolved.is_relative_to(Path(root).resolve()):
        raise CatalogError(f"path escapes repository: {value!r}")
    return resolved


def _key(component):
    return f"{component['kind']}:{component['id']}"


def split_reference(reference):
    match = re.fullmatch(REF_PATTERN, reference)
    if not match:
        raise CatalogError(f"invalid component reference {reference!r}; use kind:vendor/name[@revision]")
    return match.groups()


def manifest_path(root, kind, identifier):
    if kind not in KINDS or not re.fullmatch(ID_PATTERN, identifier):
        raise CatalogError(f"invalid component identity {kind}:{identifier}; use kind and vendor/name")
    return _path(root, f"{kind}/{identifier}/component.json")


def _read(root, path, check_location=True):
    value = _json(path)
    schema = _json(Path(root) / "catalog/schema.json")
    errors = _schema_errors(value, schema)
    if errors:
        raise CatalogError(f"{path}: " + "; ".join(errors))
    if check_location and path.resolve() != manifest_path(root, value["kind"], value["id"]):
        raise CatalogError(f"{path}: identity {_key(value)} must live at {value['kind']}/{value['id']}/component.json")
    if value["default_revision"] not in value["revisions"]:
        raise CatalogError(f"{path}: default_revision is not listed in revisions")
    capabilities = set(value["provides"]) | set(value.get("described", []))
    if set(value["capability_sources"]) != capabilities:
        raise CatalogError(f"{path}: capability_sources must cover exactly provides and described capabilities")
    if value["kind"] == "board":
        for key in ("entry", "toolchain", "profile_header", "profile_type", "architecture"):
            if not value.get("build", {}).get(key):
                raise CatalogError(f"{path}: board requires build.{key}")
        runner = value["build"].get("runner")
        if runner:
            required = {"jlink": ("device", "interface", "speed_khz", "artifact"),
                        "openocd": ("interface", "target", "artifact"),
                        "esp-idf": ("chip", "build_dir")}[runner["type"]]
            for field in required:
                if not runner.get(field):
                    raise CatalogError(f"{path}: {runner['type']} runner requires {field}")
            if runner.get("speed_khz", 1) < 1:
                raise CatalogError(f"{path}: runner speed_khz must be positive")
            for field in ("artifact", "build_dir", "recovery_doc"):
                if field in runner:
                    _path(root, runner[field])
            regions = sorted((int(item["start"], 16 if item["start"].lower().startswith("0x") else 10),
                              int(item["size"], 16 if item["size"].lower().startswith("0x") else 10))
                             for item in runner["flash_regions"])
            end = 0
            for start, size in regions:
                if size <= 0 or start < end:
                    raise CatalogError(f"{path}: runner flash_regions must be nonempty and nonoverlapping")
                end = start + size
    if value["kind"] == "mcu" and not value.get("variants"):
        raise CatalogError(f"{path}: MCU requires explicit variants")
    variant_ids = [variant["id"] for variant in value.get("variants", [])]
    if len(variant_ids) != len(set(variant_ids)):
        raise CatalogError(f"{path}: duplicate MCU variant IDs")
    for evidence in value["evidence"]:
        if evidence["revision"] not in value["revisions"]:
            raise CatalogError(f"{path}: evidence revision {evidence['revision']!r} is not supported")
        if evidence["state"] != "missing" and not evidence.get("path"):
            raise CatalogError(f"{path}: non-missing evidence requires a path")
    return value


def load(root, kind, identifier, revision=None):
    """Read one known component directly, without directory enumeration."""
    path = manifest_path(root, kind, identifier)
    if not path.is_file():
        raise CatalogError(f"unknown {kind} {identifier!r}; expected {path}; run mcuhw list {kind}s")
    component = _read(root, path)
    if revision is not None and revision not in component["revisions"]:
        raise CatalogError(f"{kind}:{identifier}: unsupported revision {revision!r}; choose {', '.join(component['revisions'])}")
    return component


def _manifest_paths(root):
    return sorted(path for kind in KINDS for path in (Path(root) / kind).glob("*/*/component.json"))


def discover(root):
    """Enumerate local manifests deterministically; no handwritten registry."""
    components = []
    seen = set()
    for path in _manifest_paths(root):
        value = _read(root, path, check_location=False)
        if _key(value) in seen:
            raise CatalogError(f"duplicate component ID {_key(value)}")
        seen.add(_key(value))
        if path.resolve() != manifest_path(root, value["kind"], value["id"]):
            raise CatalogError(f"{path}: component location does not match {_key(value)}")
        components.append(value)
    return sorted(components, key=_key)


def closure(root, kind, identifier, revision=None, capabilities=()):
    """Resolve only the selected component and its transitive manifest references."""
    result, visited, active, selected_revisions = [], set(), set(), {}

    def visit(kind, identifier, revision):
        key = f"{kind}:{identifier}"
        if key in active:
            raise CatalogError(f"dependency cycle at {key}")
        value = load(root, kind, identifier, revision)
        selected = revision or value["default_revision"]
        if key in selected_revisions and selected_revisions[key] != selected:
            raise CatalogError(f"conflicting revisions for {key}: {selected_revisions[key]} and {selected}")
        selected_revisions[key] = selected
        if key in visited:
            return
        active.add(key)
        result.append(value)
        for reference in sorted(value["depends"]):
            visit(*split_reference(reference))
        active.remove(key)
        visited.add(key)

    visit(kind, identifier, revision)
    available = set(result[0]["provides"])
    missing = set(capabilities) - available
    if missing:
        raise CatalogError(f"{kind}:{identifier}: unsupported capabilities: {', '.join(sorted(missing))}")
    return result


def compose(root, board_id, device_ids=(), revision=None):
    """Check capability compatibility. This never establishes physical evidence."""
    selected = closure(root, "board", board_id, revision)
    available = set(selected[0]["provides"])
    for identifier in device_ids:
        device = load(root, "device", identifier)
        missing = set(device["requires"]) - available
        if missing:
            raise CatalogError(f"device:{identifier} requires {', '.join(sorted(missing))}, unavailable on board:{board_id}")
        selected.extend(closure(root, "device", identifier))
    return list({_key(value): value for value in selected}.values())


def evidence_state(root, evidence):
    """Keep failed/stale/missing records separate; do not upgrade any result."""
    state = evidence["state"]
    if state in ("failed", "stale", "missing"):
        return state
    path = _path(root, evidence["path"])
    if not path.is_file():
        return "missing"
    digest = evidence.get("sha256")
    if digest and hashlib.sha256(path.read_bytes()).hexdigest() != digest:
        return "stale"
    return state


def validate(root):
    """Return deterministic errors from all manifests, sources and dependencies."""
    errors, components, seen, test_owners = [], [], set(), {}
    for path in _manifest_paths(root):
        try:
            value = _read(root, path, check_location=False)
            key = _key(value)
            if key in seen:
                errors.append(f"duplicate component ID {key}")
            seen.add(key)
            if path.resolve() != manifest_path(root, value["kind"], value["id"]):
                errors.append(f"{path}: component location does not match {key}")
            components.append(value)
            for test in value["tests"]:
                if test in test_owners:
                    errors.append(f"{key}: test {test} already owned by {test_owners[test]}")
                test_owners[test] = key
            references = [item for field in FILE_FIELDS for item in value[field]]
            references += [value["build"][field] for field in ("entry", "cmake", "toolchain", "profile_header") if value.get("build", {}).get(field)]
            if value.get("build", {}).get("test_project"):
                references.append(value["build"]["test_project"] + "/CMakeLists.txt")
            for reference in references:
                if not _path(root, reference).is_file():
                    errors.append(f"{key}: missing referenced file {reference}")
            anchors = list(value["capability_sources"].values())
            anchors += [variant["source"] for variant in value.get("variants", [])]
            for anchor in anchors:
                source = _path(root, anchor["path"])
                if not source.is_file():
                    errors.append(f"{key}: missing authoritative source {anchor['path']}")
                elif anchor["contains"] not in source.read_text(encoding="utf-8"):
                    errors.append(f"{key}: source summary anchor absent in {anchor['path']}: {anchor['contains']!r}")
            for evidence in value["evidence"]:
                if evidence.get("path"):
                    _path(root, evidence["path"])
            selected = closure(root, value["kind"], value["id"])
            if value["kind"] == "board":
                available = set(value["provides"])
                for dependency in selected[1:]:
                    if dependency["kind"] == "device":
                        missing = set(dependency["requires"]) - available
                        if missing:
                            errors.append(f"{key}: mounted {_key(dependency)} requires unavailable capabilities: {', '.join(sorted(missing))}")
                recovery = value.get("build", {}).get("runner", {}).get("recovery_doc")
                if recovery and not _path(root, recovery).is_file():
                    errors.append(f"{key}: missing recovery document {recovery}")
        except (CatalogError, OSError, UnicodeError) as error:
            errors.append(str(error))
    return sorted(set(errors))


def search(root, query="", *, kind=None, category=None, capability=None,
           revision=None, runner=None, stage=None, state=None):
    """Filter searchable metadata; evidence filters apply to the same record."""
    result = []
    for value in discover(root):
        if kind and value["kind"] != kind or category and value["category"] != category:
            continue
        if capability and capability not in value["provides"] + value["requires"]:
            continue
        if revision and revision not in value["revisions"]:
            continue
        runner_value = value.get("build", {}).get("runner", {})
        runner_name = runner_value.get("name", runner_value.get("type", "")) if isinstance(runner_value, dict) else runner_value
        if runner and runner != runner_name:
            continue
        if query and not all(term in json.dumps(value, sort_keys=True).lower() for term in query.lower().split()):
            continue
        if stage or state:
            if not any((not stage or item["stage"] == stage) and
                       (not state or evidence_state(root, item) == state) and
                       (not revision or item["revision"] == revision) for item in value["evidence"]):
                continue
        result.append(value)
    return result


def render_index(root, components=None):
    values = discover(root) if components is None else components
    # Computed status is an output property, never a mutation of the manifest.
    return json.dumps({"schema_version": 1, "components": [dict(value, evidence=[
        dict(item, effective_state=evidence_state(root, item)) for item in value["evidence"]
    ]) for value in sorted(values, key=_key)]}, indent=2, sort_keys=True) + "\n"


def render_markdown(root, components=None, link_prefix=""):
    """Render Markdown; prefix links relative to the eventual output location."""
    values = discover(root) if components is None else components

    def clean(value):
        return str(value).replace("|", "\\|").replace("\n", " ")

    def link(path):
        return "/".join(part for part in (link_prefix.rstrip("/"), path) if part)

    lines = ["# Generated hardware catalog", "", "Regenerate from component manifests; do not edit this output.", "",
             "Capabilities describe implemented contracts. They do not imply a physically tested composition. "
             "Each evidence record retains its revision, scope and effective state; historical observations apply only to the named firmware and fixture.", "",
             "| Component | Revisions | Category | Implemented / required capabilities | Build / runner |", "| --- | --- | --- | --- | --- |"]
    for value in sorted(values, key=_key):
        build = value.get("build", {})
        runner = build.get("runner", {})
        runner_name = runner.get("name", runner.get("type", "none")) if isinstance(runner, dict) else runner
        lines.append("| " + " | ".join(map(clean, [
            _key(value), ", ".join(value["revisions"]), value["category"],
            ", ".join(value["provides"]) + " / " + (", ".join(value["requires"]) or "none"),
            f"{build.get('toolchain', 'header-only')} / {runner_name or 'none'}"])) + " |")
    for value in sorted(values, key=_key):
        lines += ["", f"## {clean(_key(value))}", "", clean(value["summary"]), ""]
        for field in ("docs", "examples"):
            lines.append(f"{field.title()}: " + (", ".join(f"[{Path(path).name}]({link(path)})" for path in value[field]) or "none") + ".")
        lines += ["", "Described-only capabilities: " + (", ".join(value.get("described", [])) or "none") + ".", ""]
        for item in value["evidence"]:
            source = f" ([source]({link(item['path'])}))" if item.get("path") else ""
            lines.append(f"- {item['stage']} **{evidence_state(root, item)}**, revision `{item['revision']}`: {clean(item['scope'])}{source}")
        lines += ["", "Limitations: " + " ".join(clean(item) for item in value["limitations"])]
    return "\n".join(lines) + "\n"
