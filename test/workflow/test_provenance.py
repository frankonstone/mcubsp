#!/usr/bin/env python3
"""Selected-input provenance regressions using real Ninja compiler dependencies."""
import argparse
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('inputs', ROOT / 'tool/firmware-inputs.py')
inputs = importlib.util.module_from_spec(spec)
spec.loader.exec_module(inputs)


class Provenance(unittest.TestCase):
    def test_dependency_formats_and_stale_log_rejection(self):
        quoted = '"source with spaces.cpp"' if os.name == 'nt' else "'source with spaces.cpp'"
        self.assertEqual(list(inputs.input_names(quoted + '\nplain.cpp\n')),
                         ['source with spaces.cpp', 'plain.cpp'])
        text = 'C:/build/firmware.elf: source\\ file.cpp \\\n C:/sdk/lib.a cash$$name.h\nsource\\ file.cpp:\n'
        self.assertEqual(inputs.make_dependencies(text), ['source file.cpp', 'C:/sdk/lib.a', 'cash$name.h'])
        ld_text = 'target with spaces.elf: ' + '\\' + '\n  /package with spaces/linker.ld ' + '\\' + '\n  C:/sdk with spaces/cash$name.a\n\n/package with spaces/linker.ld:\n'
        self.assertEqual(inputs.make_dependencies(ld_text),
                         ['/package with spaces/linker.ld', 'C:/sdk with spaces/cash$name.a'])
        self.assertEqual(list(inputs.dependency_paths('a.obj: #deps 1, deps mtime 1 (VALID)\n    selected.hpp\n', {'a.obj'})), ['selected.hpp'])
        for text in ('', 'a.obj: #deps 1, deps mtime 1 (STALE)\n    selected.hpp\n'):
            with self.assertRaisesRegex(ValueError, 'missing|stale'):
                list(inputs.dependency_paths(text, {'a.obj'}))
        with self.assertRaises(ValueError):
            inputs.make_dependencies('')

    def test_identity_tracks_selected_inputs_and_separates_repository_revision(self):
        with tempfile.TemporaryDirectory(prefix='mcubsp provenance with spaces ') as temporary:
            root = Path(temporary).resolve()
            source, build = root / 'source', root / 'build'
            source.mkdir()
            (source / 'CMakeLists.txt').write_text('''cmake_minimum_required(VERSION 3.25)
project(provenance LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(VALUE 2 CACHE STRING "generated value")
set(FLAG 1 CACHE STRING "compile flag")
configure_file(config.hpp.in config.hpp @ONLY)
add_library(selected STATIC selected.cpp)
target_include_directories(selected PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(selected PRIVATE FLAG=${FLAG})
add_executable(sample main.cpp)
target_link_libraries(sample PRIVATE selected)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/selected-target.txt" CONTENT "$<TARGET_FILE:sample>")
''')
            (source / 'config.hpp.in').write_text('#define CONFIG_VALUE @VALUE@\n')
            (source / 'nested.hpp').write_text('inline constexpr int nested = 3;\n')
            (source / 'selected.cpp').write_text('#include "nested.hpp"\n#include "config.hpp"\nint selected() { return nested + CONFIG_VALUE + FLAG; }\n')
            (source / 'main.cpp').write_text('int selected(); int main() { return selected(); }\n')
            def command(*args):
                return inputs.run(args)
            command(args.git, 'init', '-q', source)
            command(args.git, '-C', source, 'add', '.')
            def commit(message):
                command(args.git, '-C', source, '-c', 'user.name=Provenance test',
                        '-c', 'user.email=test@example.invalid', '-c', 'commit.gpgsign=false',
                        '-c', f'core.hooksPath={root / "no hooks"}', 'commit', '-qm', message)
            commit('selected inputs')
            def configure(*extra):
                command(args.cmake, '-S', source, '-B', build, '-G', 'Ninja',
                        f'-DCMAKE_CXX_COMPILER={args.compiler}', *extra)
                command(args.cmake, '--build', build)
            configure()
            image = Path((build / 'selected-target.txt').read_text())
            # Native platform linkers differ. The fixture supplies one explicit
            # linker dependency; real GNU firmware builds verify automatic ld output.
            selected_link = source / 'selected link input.bin'
            selected_link.write_bytes(b'initial linker input')
            dependency = image.with_name(image.name + '.link.d')
            dependency.write_text(image.name + ': ' + selected_link.as_posix().replace(' ', '\\ ') + '\n')
            output = image.with_name(image.name + '.inputs.json')
            def record():
                meta = inputs.record(args.ninja, build, str(image.relative_to(build)), args.compiler,
                                     source, ROOT, output, 'test/fixture', 'r1', 'external')
                return meta['identity_sha256'], json.loads(output.read_text())
            baseline, report = record()
            self.assertTrue(any('-DFLAG=1' in item for item in report['commands']))
            self.assertTrue(any(item['path'].endswith('/nested.hpp') for item in report['files']))
            self.assertTrue(any(item['path'].endswith('/config.hpp') for item in report['files']))
            self.assertTrue(any('selected' in item['path'] and item['path'].endswith(('.a', '.lib')) for item in report['files']))
            (source / 'unrelated.cpp').write_text('invalid unselected source\n')
            command(args.git, '-C', source, 'add', 'unrelated.cpp')
            commit('unrelated addition')
            unchanged, next_report = record()
            self.assertEqual(baseline, unchanged)
            self.assertNotEqual(report['repository']['revision'], next_report['repository']['revision'])
            (source / 'nested.hpp').write_text('inline constexpr int nested = 4;\n')
            configure()
            changed, _ = record()
            self.assertNotEqual(unchanged, changed)
            configure('-DVALUE=7')
            generated, _ = record()
            self.assertNotEqual(changed, generated)
            configure('-DFLAG=9')
            flags, _ = record()
            self.assertNotEqual(generated, flags)
            selected_link.write_bytes(b'changed selected linker input')
            linked, _ = record()
            self.assertNotEqual(flags, linked)
            dependency.unlink()
            with self.assertRaisesRegex(ValueError, 'linker input record'):
                record()
            self.assertFalse(output.exists())

    def test_deleted_response_file_contents_are_preserved(self):
        with tempfile.TemporaryDirectory(prefix='mcubsp response file ') as temporary:
            root = Path(temporary)
            (root / 'source.cpp').write_text('selected source')
            (root / 'writer.py').write_text('''from pathlib import Path
import sys
if sys.argv[1] == 'compile':
    Path('source.o').write_bytes(b'object')
    Path('source.o.d').write_text('source.o: source.cpp\\n')
else:
    Path('image').write_text(Path(sys.argv[1][1:]).read_text())
''')
            python = subprocess.list2cmdline([sys.executable]) if os.name == 'nt' else __import__('shlex').quote(sys.executable)
            (root / 'build.ninja').write_text(f'''rule compile
  command = {python} writer.py compile
  depfile = source.o.d
  deps = gcc
rule link
  command = {python} writer.py @$out.rsp
  rspfile = $out.rsp
  rspfile_content = -DSELECTED=7
build source.o: compile source.cpp
build image: link source.o
''')
            inputs.run([args.ninja], root)
            self.assertFalse((root / 'image.rsp').exists())
            _, commands = inputs.collect(args.ninja, root, ['image'])
            self.assertTrue(any('-DSELECTED=7' in command for command in commands))
            self.assertFalse(any('@image.rsp' in command for command in commands))


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--compiler', required=True)
    parser.add_argument('--cmake', required=True)
    parser.add_argument('--ninja', required=True)
    parser.add_argument('--git', default='git')
    args = parser.parse_args()
    result = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(Provenance))
    failed = {test.id() for test, _ in result.failures + result.errors}
    print(json.dumps({'format': 'mcutest-results', 'version': 1,
        'summary': {'tests': result.testsRun, 'failures': len(failed)},
        'tests': [{'name': 'provenance/' + name,
                   'status': 'failed' if f'__main__.Provenance.{name}' in failed else 'passed',
                   'requirements': ['REQ-0052']} for name in unittest.defaultTestLoader.getTestCaseNames(Provenance)]}))
    raise SystemExit(not result.wasSuccessful())
