"""Selected Ninja input provenance, plus preprocessing for embedded source IDs."""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import subprocess
import sys


def run(argv, cwd=None):
    result = subprocess.run(list(map(str, argv)), cwd=cwd, text=True, capture_output=True)
    if result.returncode:
        raise ValueError(f"{argv[0]} failed: {result.stderr or result.stdout}")
    return result.stdout


def digest(data):
    return hashlib.sha256(data).hexdigest()


def identity(root, build, target, ninja="ninja", extra=()):
    """Read only the selected build edges and recorded compiler dependencies."""
    root, build = Path(root).resolve(), Path(build).resolve()
    def canonical(path):
        path = Path(path).resolve()
        for prefix, name in ((build, "build"), (root, "source")):
            if path.is_relative_to(prefix):
                return name + ":" + path.relative_to(prefix).as_posix()
        return "external:" + path.as_posix()

    inputs = run([ninja, "-t", "inputs", target], build).splitlines()
    files, objects = set(), []
    for value in inputs:
        if value.startswith(("'", '"')):
            words = shlex.split(value, posix=os.name != "nt")
            if len(words) != 1:
                raise ValueError(f"Invalid Ninja input path: {value}")
            value = words[0]
            if os.name == "nt" and value.startswith('"') and value.endswith('"'):
                value = value[1:-1]
        path = Path(value)
        path = path if path.is_absolute() else build / path
        if path.suffix in (".o", ".obj"):
            if not path.is_file():
                raise ValueError(f"No compiled dependency graph for {target}; build it first with Ninja")
            objects.append(value)
        elif path.is_file() and path.suffix not in (".a", ".elf", ".bin", ".map"):
            files.add(path.resolve())
        elif path.is_file() and path.suffix == ".a" and not path.resolve().is_relative_to(build):
            files.add(path.resolve())
    # SDK images contain hundreds of objects. Batch dependency queries while
    # keeping each command comfortably below Windows command-line limits.
    # Compiled objects are represented by their source closure and commands.
    # Imported objects have no compiler dependency record, so their binary bytes
    # are the only available evidence of what the linker actually consumed.
    objects_without_dependencies = {
        (Path(value) if Path(value).is_absolute() else build / value).resolve()
        for value in objects
    }
    for index in range(0, len(objects), 32):
        dependencies = run([ninja, "-t", "deps", *objects[index:index + 32]], build)
        for line in dependencies.splitlines():
            if not line.startswith("    "):
                if ": #deps " in line:
                    value, details = line.rsplit(": #deps ", 1)
                    if int(details.split(",", 1)[0]) > 0:
                        path = Path(value)
                        path = path if path.is_absolute() else build / path
                        objects_without_dependencies.discard(path.resolve())
                continue
            path = Path(line.strip())
            path = path if path.is_absolute() else build / path
            if not path.is_file():
                raise ValueError(f"Selected dependency disappeared: {path}")
            files.add(path.resolve())
    files.update(objects_without_dependencies)
    files.update(Path(path).resolve() for path in extra)
    if not objects or not files:
        raise ValueError(f"No compiled dependency graph for {target}; build it first with Ninja")
    commands = run([ninja, "-t", "commands", target], build)
    # Relocation of the source/package and build directory is not a source change.
    for path, replacement in ((build, "$BUILD"), (root, "$SOURCE")):
        commands = commands.replace(str(path), replacement).replace(path.as_posix(), replacement)
    records = [{"path": canonical(path), "sha256": digest(path.read_bytes())}
               for path in sorted(files, key=canonical)]
    compilers = {}
    cache = build / "CMakeCache.txt"
    if cache.is_file():
        cache_text = cache.read_text()
        locations = dict(re.findall(r"^CMAKE_(CXX|C|ASM)_COMPILER:[^=]+=(.+)$", cache_text, re.M))
        version_parts = [re.search(rf"^CMAKE_CACHE_{part}_VERSION:[^=]+=(.+)$", cache_text, re.M)
                         for part in ("MAJOR", "MINOR", "PATCH")]
        if all(version_parts):
            version = ".".join(part.group(1) for part in version_parts)
            for language in ("C", "CXX", "ASM"):
                compiler_file = build / "CMakeFiles" / version / f"CMake{language}Compiler.cmake"
                if language not in locations and compiler_file.is_file():
                    match = re.search(rf'set\(CMAKE_{language}_COMPILER "([^"]+)"\)', compiler_file.read_text())
                    if match:
                        locations[language] = match.group(1)
        for language, compiler in locations.items():
            compilers[language] = {"version": run([compiler, "--version"]),
                                   "target": run([compiler, "-dumpmachine"]).strip()}
    material = {"files": records, "commands": commands, "toolchains": compilers}
    value = digest(json.dumps(material, sort_keys=True, separators=(",", ":")).encode())
    revision = subprocess.run(["git", "-C", str(root), "rev-parse", "HEAD"],
                              text=True, capture_output=True)
    return {"schema_version": 1, "target": target, "identity": value,
            "repository_revision": revision.stdout.strip() if revision.returncode == 0 else None,
            "source_count": len(records), "object_count": len(objects), **material}


def scan_sources(compiler, roots, sources, flags):
    """Compiler-preprocessed selected source closure; no recursive filesystem walk."""
    files = set(Path(source).resolve() for source in sources)
    for source in sources:
        if Path(source).suffix not in (".cpp", ".cc", ".c", ".S"):
            continue
        output = run([compiler, "-MM", "-MT", "mcuhw", *flags, source])
        output = output.replace("\\\n", " ")
        _, dependencies = output.split(":", 1)
        for value in shlex.split(dependencies):
            files.add(Path(value).resolve())
    def name(path):
        for label, root in roots:
            if path.is_relative_to(root):
                return label + ":" + path.relative_to(root).as_posix()
        return path.as_posix()
    records = [{"path": name(path), "sha256": digest(path.read_bytes())}
               for path in sorted(files, key=name)]
    compiler_version = run([compiler, "--version"])
    return {"identity": digest(json.dumps({"files": records, "flags": flags,
                                           "compiler": compiler_version}, sort_keys=True).encode()),
            "files": records, "compiler": compiler_version, "flags": flags}


def flash_bundle(build, manifest):
    """Identify the exact ESP flash settings, offsets and image bytes, separately
    from each ELF's selected source/dependency identity.
    """
    build = Path(build).resolve()
    settings = json.loads(Path(manifest).read_text())
    flash_files = settings.get("flash_files")
    if not isinstance(flash_files, dict) or not flash_files:
        raise ValueError("Flash manifest must name at least one image")
    images = []
    for address, relative in sorted(flash_files.items(), key=lambda item: int(item[0], 0)):
        offset = int(address, 0)
        path = (build / relative).resolve()
        if offset < 0 or not path.is_relative_to(build):
            raise ValueError(f"Invalid flash image address/path: {address} {relative}")
        data = path.read_bytes()
        images.append({"offset": offset, "path": path.relative_to(build).as_posix(),
                       "size": len(data), "sha256": digest(data)})
    material = {"manifest": settings, "images": images}
    value = digest(json.dumps(material, sort_keys=True, separators=(",", ":")).encode())
    return {"schema_version": 1, "scope": "flash-bundle", "identity": value,
            "image_count": len(images), **material}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, required=True)
    parser.add_argument("--build", type=Path)
    parser.add_argument("--target")
    parser.add_argument("--flash-manifest", type=Path)
    parser.add_argument("--ninja", default="ninja")
    parser.add_argument("--extra", action="append", default=[])
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--compiler")
    parser.add_argument("--source", action="append", default=[])
    parser.add_argument("--include", action="append", default=[])
    parser.add_argument("--flag", action="append", default=[])
    args = parser.parse_args()
    try:
        if args.flash_manifest:
            if not args.build:
                parser.error("--flash-manifest requires --build")
            result = flash_bundle(args.build, args.flash_manifest)
        elif args.compiler:
            flags = args.flag + ["-I" + path for path in args.include]
            result = scan_sources(args.compiler, [("mcuhw", args.root.resolve())], args.source, flags)
        else:
            result = identity(args.root, args.build, args.target, args.ninja, args.extra)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + "\n")
        print(result["identity"])
        return 0
    except (ValueError, OSError) as error:
        print(f"mcuhw identity: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
