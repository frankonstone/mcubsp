#!/usr/bin/env python3
"""Record selected Ninja firmware inputs after successful compilation and linking."""
# @req REQ-0052
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shlex
import shutil
import subprocess


def digest(path):
    with path.open('rb') as stream:
        result = hashlib.file_digest(stream, 'sha256') if hasattr(hashlib, 'file_digest') else None
        if result is None:
            result = hashlib.sha256()
            for block in iter(lambda: stream.read(1024 * 1024), b''):
                result.update(block)
    return result.hexdigest()


def run(command, directory=None):
    arguments = [str(item) for item in command]
    result = subprocess.run(arguments, cwd=directory, capture_output=True, text=True)
    if result.returncode:
        raise ValueError(f'command failed ({result.returncode}): {arguments!r}\n{result.stdout}{result.stderr}')
    return result.stdout


def input_names(text):
    # Ninja's inputs tool emits shell-escaped paths, one per line.
    for line in text.splitlines():
        if os.name == 'nt':
            yield line.strip('"')
        else:
            fields = shlex.split(line)
            if len(fields) != 1:
                raise ValueError(f'unrecognized Ninja input: {line}')
            yield fields[0]


def dependency_paths(text, selected):
    current = None
    found = set()
    for line in text.splitlines():
        match = re.match(r'(.+): #deps \d+, .*\((VALID|STALE)\)$', line)
        if match:
            current = match[1] if match[1] in selected else None
            if current:
                if match[2] != 'VALID':
                    raise ValueError(f'stale compiler dependencies: {current}')
                found.add(current)
        elif current and line.startswith('    '):
            yield line[4:]
    missing = selected - found
    if missing:
        raise ValueError('missing compiler dependencies: ' + ', '.join(sorted(missing)))


def make_dependencies(text):
    # GNU ld writes a make rule and optional empty rules for removed inputs.
    # The target separator is followed by whitespace (unlike a Windows drive).
    physical_lines = text.splitlines()
    if physical_lines and re.search(r':\s*\\$', physical_lines[0]):
        # ld emits one path per continuation line, but does not make-escape
        # spaces in those paths. Joining then tokenizing would split a valid
        # installed package path. Preserve these physical path boundaries.
        result = []
        for line in physical_lines[1:]:
            path = line.strip()
            continuation = path.endswith(' \\')
            if continuation:
                path = path[:-2]
            if not path:
                raise ValueError('empty linker input path')
            result.append(path)
            if not continuation:
                return result
        raise ValueError('incomplete linker dependency file')
    lines = text.replace('\\\r\n', ' ').replace('\\\n', ' ').splitlines()
    if not lines:
        raise ValueError('empty linker dependency file')
    rule = lines[0]
    match = re.search(r':(?:\s|$)', rule)
    if not match:
        raise ValueError('invalid linker dependency file')
    words, word, escaped = [], '', False
    for char in rule[match.end():]:
        if escaped:
            word += char
            escaped = False
        elif char == '\\':
            escaped = True
        elif char.isspace():
            if word:
                words.append(word.replace('$$', '$'))
                word = ''
        else:
            word += char
    if escaped:
        raise ValueError('incomplete linker dependency escape')
    if word:
        words.append(word.replace('$$', '$'))
    if not words:
        raise ValueError('empty linker dependency file')
    return words


def collect(ninja, build, targets):
    names = list(input_names(run([ninja, '-t', 'inputs', *targets], build)))
    files = set()
    for name in names:
        path = Path(name)
        path = path if path.is_absolute() else build / path
        if path.is_file():
            files.add(path.resolve())
    objects = {name for name in names if Path(name).suffix in ('.o', '.obj')}
    ordered = sorted(objects)
    for index in range(0, len(ordered), 24):
        batch = set(ordered[index:index + 24])
        dependencies = run([ninja, '-t', 'deps', *sorted(batch)], build)
        for name in dependency_paths(dependencies, batch):
            path = Path(name)
            path = path if path.is_absolute() else build / path
            if not path.is_file():
                raise ValueError(f'missing selected compiler input: {path}')
            files.add(path.resolve())
    commands = run([ninja, '-t', 'commands', *targets], build).splitlines()
    # Response files are often deleted after linking. Ninja expands them from
    # the selected rule's recorded content even when no on-disk rsp file remains.
    raw = json.loads(run([ninja, '-t', 'compdb'], build))
    expanded = json.loads(run([ninja, '-t', 'compdb', '-x'], build))
    by_output = {item['output']: item['command'] for item in expanded}
    replacements = {item['command']: by_output[item['output']] for item in raw}
    commands = [replacements.get(command, command) for command in commands]
    if not objects or not commands:
        raise ValueError('selected target has no compiled inputs or build commands')
    return files, commands


def toolchain(compiler):
    compiler = Path(compiler).absolute()
    paths = {compiler}
    for name in ('cc1', 'cc1plus', 'as', 'ld', 'collect2', 'lto-wrapper'):
        selected = run([compiler, f'-print-prog-name={name}']).strip()
        path = Path(selected)
        if not path.is_absolute():
            beside = compiler.parent / path
            path = beside if beside.is_file() else Path(shutil.which(selected) or selected)
        if path.is_file():
            paths.add(path.absolute())
    return [{'path': str(path), 'sha256': digest(path)} for path in sorted(paths)], run([compiler, '--version']).strip()


def record(ninja, build, target, compiler, source_root, framework_root, output,
           board, revision, startup, sdk=False):
    output.unlink(missing_ok=True)
    build, source_root, framework_root = build.resolve(), source_root.resolve(), framework_root.resolve()
    targets = [target]
    if sdk:
        targets.append('partition_table/partition-table.bin')
    files, commands = collect(ninja, build, targets)
    graphs = [(build, target)]
    if sdk:
        boot = build / 'bootloader'
        boot_files, boot_commands = collect(ninja, boot, ['bootloader.elf'])
        files.update(boot_files)
        commands.extend(boot_commands)
        graphs.append((boot, 'bootloader.elf'))
        files.add((build / 'config/sdkconfig.h').resolve())
    for directory, image in graphs:
        dependency = directory / (image + '.link.d')
        if not dependency.is_file():
            raise ValueError(f'missing linker input record: {dependency}; relink the firmware')
        for name in make_dependencies(dependency.read_text()):
            path = Path(name)
            path = path if path.is_absolute() else directory / path
            if not path.is_file():
                raise ValueError(f'missing selected linker input: {path}')
            files.add(path.resolve())
    for command in commands:
        for match in re.finditer(r'--?specs=([^\s]+)', command):
            name = match[1].strip('"\'')
            path = Path(name)
            if not path.is_absolute():
                path = Path(run([compiler, f'-print-file-name={name}']).strip())
            if not path.is_file():
                raise ValueError(f'missing selected compiler specification: {name}')
            files.add(path.resolve())
    anchors = sorted([(build, '<build>'), (source_root, '<source>'), (framework_root, '<mcubsp>')],
                     key=lambda item: len(str(item[0])), reverse=True)
    def normalized(value):
        value = str(value)
        for root, label in anchors:
            value = value.replace(str(root), label)
            value = value.replace(root.as_posix(), label)
        return value
    entries = [{'path': str(path), 'name': normalized(path), 'sha256': digest(path)} for path in sorted(files)]
    tools, version = toolchain(compiler)
    identity = {'board': board, 'revision': revision, 'startup': startup,
        'files': [{'name': item['name'], 'sha256': item['sha256']} for item in entries],
        'commands': [normalized(command) for command in commands],
        'tools': [{'path': item['path'], 'sha256': item['sha256']} for item in tools],
        'compiler_version': version}
    encoded = json.dumps(identity, sort_keys=True, separators=(',', ':')).encode()
    repository = {'revision': None, 'dirty': None}
    try:
        repository = {'revision': run(['git', '-C', source_root, 'rev-parse', 'HEAD']).strip(),
            'dirty': bool(run(['git', '-C', source_root, 'status', '--porcelain', '--untracked-files=no']).strip())}
    except (OSError, ValueError):
        pass
    report = {'format': 'mcubsp-firmware-inputs', 'version': 1,
        'identity_sha256': hashlib.sha256(encoded).hexdigest(), 'repository': repository,
        'files': entries, 'commands': identity['commands'], 'tools': tools,
        'compiler_version': version, 'board': board, 'revision': revision, 'startup': startup}
    temporary = output.with_name(output.name + '.tmp')
    temporary.write_text(json.dumps(report, indent=2) + '\n')
    temporary.replace(output)
    return {'status': 'recorded', 'file': output.name, 'sha256': digest(output),
            'identity_sha256': report['identity_sha256'], 'files': len(entries)}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for name in ('ninja', 'target', 'compiler', 'board', 'revision', 'startup'):
        parser.add_argument('--' + name, required=True)
    for name in ('build', 'source-root', 'framework-root', 'output'):
        parser.add_argument('--' + name, type=Path, required=True)
    parser.add_argument('--sdk', action='store_true')
    args = vars(parser.parse_args())
    try:
        print(json.dumps(record(**args)))
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        parser.exit(2, f'mcubsp inputs: {error}\n')


if __name__ == '__main__':
    main()
