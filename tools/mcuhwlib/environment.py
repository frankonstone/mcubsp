"""Explicit, pinned local tool imports and checksum-verified setup caches.

Only setup may fetch archives. Build and doctor inspect recorded local paths.
The bundled lock records observed versions, not an unverified download matrix.
"""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import stat
import subprocess
import tarfile
import tempfile
import urllib.request
import zipfile

from . import platforms


class EnvironmentError(ValueError):
    """An actionable setup or integrity diagnostic."""


def digest(path: Path) -> str:
    value = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            value.update(block)
    return value.hexdigest()


def read_json(path: Path) -> dict:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, ValueError) as error:
        raise EnvironmentError(f"Cannot read {path}: {error}") from error
    if not isinstance(value, dict) or value.get("schema_version") != 1:
        raise EnvironmentError(f"{path}: expected object with schema_version 1")
    return value


def write_json(path: Path, value: dict) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    # Replace only after the complete state is written, including on Windows.
    with tempfile.NamedTemporaryFile("w", dir=path.parent, encoding="utf-8", delete=False) as stream:
        temporary = Path(stream.name)
        json.dump(value, stream, indent=2, sort_keys=True)
        stream.write("\n")
    temporary.replace(path)


def host_id() -> str:
    machine = platform.machine().lower()
    machine = {"amd64": "x86_64", "aarch64": "arm64"}.get(machine, machine)
    return f"{platform.system().lower()}-{machine}"


def default_cache() -> Path:
    base = os.environ.get("LOCALAPPDATA") if os.name == "nt" else os.environ.get("XDG_CACHE_HOME")
    return Path(base or Path.home() / ".cache") / "mcuhw"


def run_output(argv: list[str], *, cwd: Path | None = None, env: dict | None = None) -> str:
    try:
        result = subprocess.run(argv, cwd=cwd, env=env, stdin=subprocess.DEVNULL,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, timeout=60, check=False)
    except (OSError, subprocess.TimeoutExpired) as error:
        raise EnvironmentError(f"Cannot run {argv[0]}: {error}") from error
    # J-Link's version/help switch prints the version but may return nonzero.
    if result.returncode and not ("SEGGER J-Link" in result.stdout and "-?" in argv):
        raise EnvironmentError(f"{argv[0]} exited {result.returncode}: {result.stdout.strip()}")
    return result.stdout.strip()


def load_lock(path: Path) -> dict:
    lock = read_json(path)
    if not isinstance(lock.get("tools"), dict):
        raise EnvironmentError(f"{path}: tools must be an object")
    for name, spec in lock["tools"].items():
        if not isinstance(spec, dict) or spec.get("kind") not in ("git", "executable", "directory"):
            raise EnvironmentError(f"{path}: invalid tool definition {name}")
        if not isinstance(spec.get("version"), str) or not spec["version"]:
            raise EnvironmentError(f"{path}: {name} requires an exact version")
        if spec['kind'] == 'executable':
            if not isinstance(spec.get('command'), str) or not spec['command']:
                raise EnvironmentError(f'{path}: {name} requires an executable command name')
            if not isinstance(spec.get('version_args', []), list) or not all(isinstance(value, str) for value in spec.get('version_args', [])):
                raise EnvironmentError(f'{path}: {name} version_args must be a string array')
    return lock


def required_tools(board: dict, operation: str) -> list[str]:
    build = board["build"]
    if operation == "flash":
        runner = build.get("runner") or {}
        if not runner.get("tool"):
            raise EnvironmentError(f"{board['id']} has no flash runner")
        # A completed image needs only its runner, never a compiler or Ninja.
        return [runner["tool"]]
    return list(build.get("tools", ["cmake", "ninja", "arm-gcc"]))


def _version_matches(output: str, expected: str) -> bool:
    return re.search(r"(?<![\d.])" + re.escape(expected) + r"(?![\d.])", output) is not None


def git_worktree_changes(path: Path) -> dict[str, str]:
    """Check every SDK worktree despite per-repository submodule ignore settings.

    The foreach command is a fixed Git-owned shell command; no path or other
    caller input is interpolated into it. Git enters each submodule itself.
    Checking each worktree independently also catches ignored nested submodule
    dirtiness that the superproject's diff/status may omit.
    """
    options = ['status', '--porcelain=v1', '--untracked-files=all', '--ignore-submodules=none']
    return {
        'SDK': run_output(['git', '-C', str(path), *options]),
        'SDK submodule': run_output(['git', '-C', str(path), 'submodule', 'foreach',
                                    '--quiet', '--recursive', 'git ' + ' '.join(options)]),
    }


def inspect_tool(name: str, spec: dict, path: Path, *, allow_override: bool = False) -> dict:
    path = path.expanduser().absolute()
    # Do not resolve interpreter symlinks: venv identity depends on their location.
    if spec['kind'] == 'directory':
        files = {}
        for relative, expected in spec.get('files', {}).items():
            filename = _safe_member(path, relative)
            if not filename.is_file():
                raise EnvironmentError(f"{name}: required file missing: {filename}. {spec.get('remediation', '')}")
            files[relative] = digest(filename)
            if files[relative] != expected and not allow_override:
                raise EnvironmentError(f"{name}: {relative} differs from the locked SHA-256; use --allow-version-override to record an explicit override")
        if not files:
            raise EnvironmentError(f'{name}: directory imports require locked files')
        return {'kind': 'directory', 'path': str(path), 'version': spec['version'],
                'files': files, 'override': files != spec['files']}
    if spec["kind"] == "git":
        if not (path / spec["required_file"]).is_file():
            raise EnvironmentError(f"{name}: {path} lacks {spec['required_file']}. {spec.get('remediation', '')}")
        revision = run_output(["git", "-C", str(path), "rev-parse", "HEAD"])
        submodules = run_output(["git", "-C", str(path), "submodule", "status", "--recursive"])
        if any(line.startswith(("-", "+", "U")) for line in submodules.splitlines()):
            raise EnvironmentError(f"{name}: submodules do not match the SDK revision; run git submodule update --init --recursive")
        changes = git_worktree_changes(path)
        for worktree, status in changes.items():
            if status:
                raise EnvironmentError(f"{name}: {worktree} has uncommitted files ({status.splitlines()[0]}); commit or remove them before importing a source override")
        if revision != spec["version"] and not allow_override:
            raise EnvironmentError(f"{name}: expected clean revision {spec['version']}, found {revision}; use --allow-version-override to record a deliberate override")
        return {"path": str(path), "version": revision, "submodules": submodules,
                "diff_sha256": hashlib.sha256(b'').hexdigest(),
                "override": revision != spec["version"], "kind": "git"}
    if not path.is_file():
        raise EnvironmentError(f"{name}: executable does not exist: {path}. {spec.get('remediation', '')}")
    output = run_output([str(path), *spec.get("version_args", ["--version"])])
    matches = _version_matches(output, spec["version"])
    if not matches and not allow_override:
        raise EnvironmentError(f"{name}: expected {spec['version']}, found {output.splitlines()[0]}. {spec.get('remediation', '')}")
    identity = {"kind": "executable", "path": str(path), "sha256": digest(path),
                "version": output, "override": not matches}
    if spec.get('compiler'):
        files = {}
        for component, option in [('cc1plus', '-print-prog-name=cc1plus'),
                             ('assembler', '-print-prog-name=as'),
                             ('linker', '-print-prog-name=ld'),
                             ('libstdc++', '-print-file-name=libstdc++.a')]:
            reported = run_output([str(path), option])
            filename = Path(reported)
            if not filename.is_absolute():
                adjacent = path.resolve().parent / reported
                filename = adjacent if adjacent.is_file() else Path(shutil.which(reported) or reported)
            if not filename.is_file():
                raise EnvironmentError(f'{path}: compiler installation lacks {component}: {reported}; install the complete matching toolchain')
            files[component] = {'path': str(filename.resolve()), 'sha256': digest(filename)}
        identity['compiler_files'] = files
    if spec.get("python_packages"):
        modules = spec["python_packages"]
        if not isinstance(modules, list) or not all(isinstance(item, str) and re.fullmatch(r'[A-Za-z_][A-Za-z0-9_.]*', item) for item in modules):
            raise EnvironmentError(f'{name}: python_packages must list importable module names')
        run_output([str(path), "-c", "; ".join("import " + item for item in modules)])
        packages = run_output([str(path), "-c", "import importlib.metadata,json; print(json.dumps(sorted((d.metadata['Name'], d.version) for d in importlib.metadata.distributions())))"])
        identity["packages"] = json.loads(packages)
    return identity


def _safe_member(destination: Path, name: str) -> Path:
    normalized = name.replace("\\", "/")
    if normalized.startswith("/") or re.match(r"^[A-Za-z]:", normalized):
        raise EnvironmentError(f"Unsafe archive member: {name}")
    # Validate Windows filesystem semantics on every host so the same locked
    # archive cannot name a device or an NTFS alternate stream on Windows.
    reserved = re.compile(r'^(?:CON|PRN|AUX|NUL|COM[1-9]|LPT[1-9])(?:\.|$)', re.IGNORECASE)
    for part in normalized.split('/'):
        if part and part != '.' and (':' in part or part.endswith((' ', '.')) or
                                     reserved.match(part) or any(ord(char) < 32 for char in part)):
            raise EnvironmentError(f"Unsafe archive member: {name}")
    result = (destination / normalized).resolve()
    if result != destination.resolve() and destination.resolve() not in result.parents:
        raise EnvironmentError(f"Unsafe archive member: {name}")
    return result


def _unpack(archive: Path, destination: Path, archive_format: str) -> None:
    """Extract regular files only; do not follow links from an archive."""
    if archive_format == "zip":
        with zipfile.ZipFile(archive) as bundle:
            for item in bundle.infolist():
                target = _safe_member(destination, item.filename)
                mode = item.external_attr >> 16
                if stat.S_ISLNK(mode):
                    raise EnvironmentError(f"Archive symlink is unsupported: {item.filename}")
                if item.is_dir():
                    target.mkdir(parents=True, exist_ok=True)
                else:
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with bundle.open(item) as source, target.open("wb") as output:
                        shutil.copyfileobj(source, output)
                    if mode & 0o777:
                        target.chmod(mode & 0o777)
    elif archive_format == "tar":
        with tarfile.open(archive) as bundle:
            for item in bundle:
                target = _safe_member(destination, item.name)
                if item.isdir():
                    target.mkdir(parents=True, exist_ok=True)
                elif item.isfile():
                    target.parent.mkdir(parents=True, exist_ok=True)
                    with bundle.extractfile(item) as source, target.open("wb") as output:
                        shutil.copyfileobj(source, output)
                    target.chmod(item.mode & 0o777)
                else:
                    raise EnvironmentError(f"Archive links/devices are unsupported: {item.name}")
    else:
        raise EnvironmentError(f"Unsupported archive format {archive_format!r}; use zip or tar")


def setup_archive(spec: dict, cache: Path, *, offline: bool) -> Path:
    """Retrieve a host-specific immutable archive only during explicit setup."""
    item = spec.get("archives", {}).get(host_id())
    if not item:
        raise EnvironmentError(f"No pinned archive for {host_id()}; import an installed tool with --tool NAME=PATH")
    checksum = item.get("sha256", "")
    if not re.fullmatch(r"[0-9a-f]{64}", checksum):
        raise EnvironmentError("Archive setup requires a lowercase SHA-256 in the lock")
    archive = cache / "archives" / checksum
    destination = cache / "unpacked" / checksum
    if not archive.is_file():
        if offline:
            raise EnvironmentError(f"Offline cache entry missing: {archive}; run setup online once or import --tool NAME=PATH")
        url = item.get("url", "")
        if not url.startswith(("https://", "file://")):
            raise EnvironmentError("Archive URL must use HTTPS or a local file URI")
        archive.parent.mkdir(parents=True, exist_ok=True)
        temporary = None
        try:
            with tempfile.NamedTemporaryFile(dir=archive.parent, delete=False) as output:
                temporary = Path(output.name)
                with urllib.request.urlopen(url, timeout=60) as source:
                    shutil.copyfileobj(source, output)
        except Exception:
            if temporary is not None:
                temporary.unlink(missing_ok=True)
            raise
        if digest(temporary) != checksum:
            temporary.unlink()
            raise EnvironmentError(f"Archive integrity mismatch for {url}")
        temporary.replace(archive)
    if digest(archive) != checksum:
        raise EnvironmentError(f"Cached archive integrity mismatch: {archive}; remove this entry and rerun setup")
    # Fresh extraction prevents modifications to a previous extraction surviving setup.
    destination.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(dir=destination.parent) as temporary:
        extracted = Path(temporary) / "contents"
        extracted.mkdir()
        _unpack(archive, extracted, item.get("format", "tar"))
        target = _safe_member(extracted, item["path"])
        if not target.exists():
            raise EnvironmentError(f"Archive lacks configured tool path {item['path']}")
        if destination.exists():
            shutil.rmtree(destination)
        extracted.replace(destination)
    return _safe_member(destination, item["path"])


def setup(lock: dict, names: list[str], overrides: dict[str, str], cache: Path,
          *, offline: bool = False, allow_override: bool = False) -> dict:
    unknown = set(overrides) - set(lock["tools"])
    if unknown:
        raise EnvironmentError(f"Unknown tool override: {', '.join(sorted(unknown))}")
    identities = {}
    for name in dict.fromkeys([*names, *overrides]):
        if name not in lock["tools"]:
            raise EnvironmentError(f"No locked definition for tool {name}")
        spec = lock["tools"][name]
        explicit = overrides.get(name)
        command = spec.get("windows_command", spec.get("command")) if os.name == "nt" else spec.get("command")
        discovered = os.environ.get(spec.get("environment", "")) if spec["kind"] in ("git", "directory") else shutil.which(command)
        if explicit:
            path = Path(explicit)
            source = "explicit-import"
        elif spec.get("archives", {}).get(host_id()):
            path = setup_archive(spec, cache, offline=offline)
            source = "verified-archive"
        elif discovered:
            path = Path(discovered)
            source = "environment-import"
        else:
            raise EnvironmentError(f"Missing {name}. {spec.get('remediation', '')}")
        identities[name] = inspect_tool(name, spec, path, allow_override=allow_override)
        identities[name]["source"] = source
        for field in ('cmake_variable', 'environment', 'adapter'):
            if field in spec:
                identities[name][field] = spec[field]
    return {"schema_version": 1, "host": host_id(), "tools": identities}


def verify(state: dict, lock: dict, names: list[str], *, probe: bool = True) -> list[str]:
    errors = []
    for name in names:
        saved = state.get("tools", {}).get(name)
        if not saved:
            errors.append(f"Missing recorded {name}; run mcuhw.py setup --operation {'flash' if name in ('jlink', 'openocd') else 'build'} --tool {name}=/absolute/path")
            continue
        try:
            actual = inspect_tool(name, lock["tools"][name], Path(saved["path"]), allow_override=saved.get("override", False))
            for key in ("sha256", "version", "submodules", "diff_sha256", "packages", "files", "compiler_files"):
                if actual.get(key) != saved.get(key):
                    raise EnvironmentError(f"{name}: recorded {key} changed; rerun explicit setup to accept the new installation")
            if probe and lock["tools"][name].get("compiler"):
                check_cpp_library(Path(saved["path"]))
        except (EnvironmentError, KeyError) as error:
            errors.append(str(error))
    if not errors:
        for name in names:
            adapter = lock['tools'][name].get('adapter')
            if adapter:
                try:
                    platforms.check(adapter, state, names, run_output, process_environment(state, names))
                except ValueError as error:
                    errors.append(str(error))
    return errors


def check_cpp_library(compiler: Path) -> None:
    source = "#include <concepts>\n#include <expected>\n#include <span>\n#include <bit>\nstatic_assert(__cpp_lib_expected >= 202202L);\nstatic_assert(std::same_as<decltype(std::expected<int,int>{}),std::expected<int,int>>);\n"
    with tempfile.TemporaryDirectory(prefix="mcuhw-cxx23-") as directory:
        path = Path(directory) / "probe.cpp"
        path.write_text(source, encoding="utf-8")
        try:
            run_output([str(compiler), "-std=c++23", "-fsyntax-only", str(path)])
        except EnvironmentError as error:
            raise EnvironmentError(f"C++23 compiler/library probe failed for {compiler}. Install its matching libstdc++ headers. {error}") from error


def process_environment(state: dict, names: list[str] | None = None) -> dict[str, str]:
    result = dict(os.environ)
    selected = {name: value for name, value in state['tools'].items() if names is None or name in names}
    paths = [str(Path(value["path"]).parent) for value in selected.values() if value["kind"] == "executable"]
    result["PATH"] = os.pathsep.join([*paths, result.get("PATH", "")])
    result.update(_selected_environment(state, selected))
    return result


def _selected_environment(state: dict, selected: dict) -> dict[str, str]:
    result = {value['environment']: value['path'] for value in selected.values() if value.get('environment')}
    for value in selected.values():
        if value.get('adapter'):
            result.update(platforms.environment(value['adapter'], state, list(selected)))
    return result


def preset_environment(state: dict, names: list[str]) -> dict[str, str]:
    """Export selected tool paths without recording inherited variables/secrets."""
    selected = {name: value for name, value in state['tools'].items() if name in names}
    paths = [str(Path(value['path']).parent) for value in selected.values() if value['kind'] == 'executable']
    return {'PATH': os.pathsep.join([*paths, '$penv{PATH}']), **_selected_environment(state, selected)}


def cmake_variables(state: dict, names: list[str]) -> dict[str, str]:
    return {value['cmake_variable']: value['path'] for name, value in state['tools'].items()
            if name in names and value.get('cmake_variable')}
