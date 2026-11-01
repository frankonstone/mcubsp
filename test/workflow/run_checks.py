#!/usr/bin/env python3
"""Build workflow regressions; no downloads, hardware access or sibling projects."""
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

parser = argparse.ArgumentParser()
parser.add_argument('--compiler', required=True)
parser.add_argument('--cmake', default='cmake')
parser.add_argument('--contract-fixture', required=True)
args = parser.parse_args()
ROOT = Path(__file__).resolve().parents[2]

def run(*command, success=True):
    result = subprocess.run([str(x) for x in command], text=True, capture_output=True)
    if (result.returncode == 0) != success:
        raise AssertionError(f'{command}\n{result.stdout}\n{result.stderr}')
    return result

class Workflows(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='mcubsp build checks ')
        self.root = Path(self.tmp.name)
    def tearDown(self):
        self.tmp.cleanup()
    def configure(self, source, build, *options):
        return run(args.cmake, '-S', source, '-B', build, '-G', 'Ninja',
                   f'-DCMAKE_CXX_COMPILER={args.compiler}', *options)
    def consumer(self, installed=False):
        source = self.root / 'consumer'
        source.mkdir(exist_ok=True)
        (source / 'main.cpp').write_text('#include <mcubsp/io/board.hpp>\n#include <mcubsp/device/sensor/tmp102.hpp>\nint main() {}\n')
        dependency = 'find_package(mcubsp 0.1 CONFIG REQUIRED)' if installed else f'add_subdirectory("{ROOT.as_posix()}" mcubsp EXCLUDE_FROM_ALL)'
        (source / 'CMakeLists.txt').write_text(f'''cmake_minimum_required(VERSION 3.25)
project(consumer LANGUAGES CXX)
enable_testing()
{dependency}
if(NOT COMMAND mcubsp_add_firmware)
  message(FATAL_ERROR "public firmware helper is unavailable")
endif()
get_property(consumer_languages GLOBAL PROPERTY ENABLED_LANGUAGES)
if("ASM" IN_LIST consumer_languages)
  message(FATAL_ERROR "library inclusion enabled an unrequested language")
endif()
add_executable(consumer main.cpp)
target_link_libraries(consumer PRIVATE mcubsp::io mcubsp::device)
add_test(NAME parent_test COMMAND consumer)
if(TARGET native-tests OR TARGET firmware OR TARGET check-headers OR TARGET mcuip::mcuip OR TARGET mcuintegration::mcubsp_mcuip)
  message(FATAL_ERROR "mcubsp created unrequested targets")
endif()
''')
        return source
    def test_source_consumer_preserves_parent_tests(self):
        build = self.root / 'source-build'
        self.configure(self.consumer(), build, '-DMCUBSP_BOARD=unselected/board')
        run(args.cmake, '--build', build)
        run('ctest', '--test-dir', build, '--output-on-failure')
    def test_library_only_install_and_relocated_consumer(self):
        build = self.root / 'library-build'
        self.configure(ROOT, build)
        run(args.cmake, '--build', build)
        run(args.cmake, '--install', build, '--prefix', self.root / 'install')
        shutil.move(self.root / 'install', self.root / 'relocated')
        for path in (self.root / 'relocated' / 'lib' / 'cmake').rglob('*.cmake'):
            self.assertNotIn(str(ROOT), path.read_text())
        installed_root = self.root / 'relocated' / 'share' / 'mcubsp'
        self.assertFalse((installed_root / 'include/mcubsp/integration').exists())
        runner = run(__import__('sys').executable, installed_root / 'tool/hardware.py',
                     'runner-field', '--board', 'arduino/uno_r4_minima', '--field', 'device')
        self.assertEqual(runner.stdout.strip(), 'R7FA4M1AB')
        self.assertFalse((installed_root / 'tool/coverage-manifests.py').exists())
        self.assertTrue((installed_root / 'tool/firmware-inputs.py').is_file())
        self.assertTrue((installed_root / 'src/board/arduino/uno_r4_minima/startup/startup.S').is_file())
        self.assertTrue((installed_root / 'src/board/st/nucleo_h753zi/linker.ld').is_file())
        self.assertTrue((installed_root / 'src/board/waveshare/esp32p4_nano/startup/bootloader_components/main/CMakeLists.txt').is_file())
        self.assertTrue((installed_root / 'example/blink/CMakeLists.txt').is_file())
        for excluded in ('test', 'src/core', 'doc/acceptance', 'doc/development',
                         'requirements.md', 'implementation-plan.md',
                         'src/board/arduino/uno_r4_minima/CMakeLists.txt'):
            self.assertFalse((installed_root / excluded).exists(), excluded)
        self.assertEqual(list((installed_root / 'src').rglob('test')), [])
        self.assertEqual(list(installed_root.rglob('evidence')), [])
        self.configure(self.consumer(installed=True), self.root / 'installed-build',
                       f'-DCMAKE_PREFIX_PATH={self.root / "relocated"}')
        run(args.cmake, '--build', self.root / 'installed-build')
    def test_compile_failure_must_match_diagnostic(self):
        source = self.root / 'unrelated.cpp'
        source.write_text('#include <mcubsp_deliberately_missing_header.hpp>\n')
        result = run(args.cmake, f'-DCXX_COMPILER={args.compiler}',
                     f'-DSOURCE={source}', f'-DINCLUDE_DIR={ROOT / "include"}',
                     '-DEXPECT_SUCCESS=FALSE', '-DEXPECT_DIAGNOSTIC=exclusive pin assigned',
                     '-P', ROOT / 'test/compile/CompileCase.cmake', success=False)
        self.assertIn('unexpected reason', result.stderr)
    def test_pending_operation_diagnostic(self):
        result = run(args.contract_fixture, success=False)
        self.assertEqual(result.returncode, 77)
        self.assertIn('operation:pending operation destroyed', result.stdout)
        self.assertIn(':0:1', result.stdout)
    def test_merge_uses_registry_and_rejects_missing_reports(self):
        producer = self.root / 'fresh.json'
        producer.write_text(json.dumps({'format': 'mcutest-results', 'version': 1,
            'summary': {'tests': 1, 'failures': 0},
            'tests': [{'name': 'case', 'status': 'passed', 'requirements': ['REQ-0052']}]}))
        (self.root / 'stale.json').write_text('{"summary":{"failures":1}}')
        registry = self.root / 'registry.txt'
        registry.write_text(str(producer) + '\n')
        output = self.root / 'merged.json'
        shards, published = self.root / 'shards', self.root / 'published'
        command = [__import__('sys').executable, ROOT / 'test/support/MergeResults.py',
                   '--registry', registry, '--output', output,
                   '--shard-dir', shards, '--publish-dir', published]
        run(*command)
        self.assertEqual(json.loads(output.read_text())['summary']['tests'], 1)
        cases = [case for path in sorted(shards.glob('*.json'))
                 for case in json.loads(path.read_text())['tests']]
        self.assertEqual([case['name'] for case in cases], ['fresh/case'])
        self.assertEqual(len(list(shards.glob('*.json'))), 8)
        self.assertEqual([p.read_bytes() for p in sorted(shards.glob('*.json'))],
                         [p.read_bytes() for p in sorted(published.glob('*.json'))])
        producer.unlink()
        run(*command, success=False)
        self.assertFalse(output.exists())
        self.assertEqual(list(shards.glob('*.json')), [])
        self.assertEqual(list(published.glob('*.json')), [])
    def test_large_native_reports_preserve_every_case_in_bounded_trace_shards(self):
        producer, registry = self.root / 'large.json', self.root / 'registry.txt'
        cases = [{'name': f'{i}/' + 'long_case_name_' * 8, 'status': 'passed',
                  'requirements': ['REQ-0052']} for i in range(1000)]
        producer.write_text(json.dumps({'format': 'mcutest-results', 'version': 1,
            'summary': {'tests': len(cases), 'failures': 0}, 'tests': cases}))
        registry.write_text(str(producer) + '\n')
        output, shards = self.root / 'merged.json', self.root / 'shards'
        run(__import__('sys').executable, ROOT / 'test/support/MergeResults.py',
            '--registry', registry, '--output', output, '--shard-dir', shards)
        combined = json.loads(output.read_text())['tests']
        imported = [case for path in sorted(shards.glob('*.json'))
                    for case in json.loads(path.read_text())['tests']]
        self.assertEqual(len(imported), len(cases))
        self.assertEqual({c['name'] for c in imported}, {c['name'] for c in combined})
        for path in shards.glob('*.json'):
            self.assertLess(path.stat().st_size, 65534)
    def test_active_documentation_links_resolve(self):
        import re
        files = [ROOT / 'README.md', ROOT / 'doc/README.md',
                 *sorted((ROOT / 'example').rglob('README.md')),
                 *sorted((ROOT / 'doc/getting-started').glob('*.md')),
                 *sorted((ROOT / 'doc/development').glob('*.md'))]
        for path in files:
            for link in re.findall(r'\]\(([^)]+)\)', path.read_text()):
                target = link.split('#')[0]
                if not target or '://' in target or target.startswith('mailto:'):
                    continue
                if '_vendor/' in target or '_generated/' in target:
                    continue  # Optional local vendor material has its own source manifest.
                resolved = (path.parent / target).resolve()
                if not resolved.is_relative_to(ROOT):
                    continue  # Sibling projects are independent, optional checkouts.
                self.assertTrue(resolved.exists(), f'{path.relative_to(ROOT)}: {target}')
    def test_board_validation_precedes_platform_setup(self):
        source = self.root / 'bad-board'
        source.mkdir()
        (source / 'CMakeLists.txt').write_text(f'''cmake_minimum_required(VERSION 3.25)
project(bad LANGUAGES CXX)
add_subdirectory("{ROOT.as_posix()}" mcubsp)
mcubsp_add_firmware(firmware BOARD missing/board STARTUP supplied SOURCES main.cpp)
''')
        result = run(args.cmake, '-S', source, '-B', self.root / 'bad-build', '-G', 'Ninja',
                     f'-DCMAKE_CXX_COMPILER={args.compiler}', success=False)
        self.assertIn('Unknown mcubsp board', result.stderr)

    def test_board_toolchain_validates_before_compiler_discovery(self):
        source = self.root / 'toolchain-consumer'
        source.mkdir()
        (source / 'CMakeLists.txt').write_text('cmake_minimum_required(VERSION 3.25)\nproject(consumer LANGUAGES CXX)\n')
        cases = [([], 'Select -DMCUBSP_BOARD'),
                 (['-DMCUBSP_BOARD=missing/board'], 'Unknown mcubsp board'),
                 (['-DMCUBSP_BOARD=arduino/uno_r4_minima', '-DMCUBSP_BOARD_REVISION=wrong'], 'Unsupported'),
                 (['-DMCUBSP_BOARD=arduino/uno_r4_minima', '-DMCUBSP_CONFIGURED_BOARD=st/nucleo_h753zi'], 'fresh build directory')]
        for index, (options, message) in enumerate(cases):
            with self.subTest(message=message):
                result = run(args.cmake, '-S', source, '-B', self.root / f'invalid-{index}', '-G', 'Ninja',
                    f'-DCMAKE_TOOLCHAIN_FILE={ROOT / "cmake/board-toolchain.cmake"}',
                    f'-DCMAKE_CXX_COMPILER={self.root / "missing-compiler"}', *options, success=False)
                self.assertIn(message, ' '.join(result.stderr.split()))

    def test_public_firmware_rejects_misspelled_or_empty_arguments(self):
        for index, invocation in enumerate([
                'mcubsp_add_firmware(app BOARD arduino/uno_r4_minima STARTUP supplied SOURCSE main.cpp)',
                'mcubsp_add_firmware(app BOARD arduino/uno_r4_minima STARTUP SOURCES main.cpp)']):
            source = self.root / f'arguments-{index}'
            source.mkdir()
            (source / 'CMakeLists.txt').write_text(f'''cmake_minimum_required(VERSION 3.25)
project(consumer LANGUAGES CXX)
add_subdirectory("{ROOT.as_posix()}" mcubsp EXCLUDE_FROM_ALL)
{invocation}
''')
            result = run(args.cmake, '-S', source, '-B', source / 'build', '-G', 'Ninja',
                         f'-DCMAKE_CXX_COMPILER={args.compiler}', success=False)
            self.assertIn('Invalid mcubsp_add_firmware arguments', result.stderr)

    def test_board_bindings_are_independent_of_io_and_backends(self):
        source = self.root / 'board-bindings.cpp'
        source.write_text('''#include <mcubsp/board/arduino/uno_r4_minima/bindings.hpp>
#include <mcubsp/board/st/nucleo_h753zi/bindings.hpp>
#include <mcubsp/board/waveshare/esp32p4_nano/bindings.hpp>
using uno = mcubsp::board::arduino::uno_r4_minima_pins;
using st = mcubsp::board::st::nucleo_h753zi_bindings;
using esp = mcubsp::board::waveshare::esp32p4_nano_bindings;
static_assert(uno::led_builtin_d13.port == 1 && uno::led_builtin_d13.index == 11);
static_assert(st::ethernet_binding::rmii_pins.size() == 9);
static_assert(esp::sdmmc_slot0_binding::slot == 0);
static_assert(esp::sdmmc_slot1_wifi_binding::uses_gpio_matrix);
''')
        command = [args.compiler, '-std=c++23', '-I', ROOT / 'include', source]
        run(*command, '-fsyntax-only')
        dependencies = run(*command, '-MM').stdout.replace('\\', '/')
        self.assertNotIn('mcubsp/io/', dependencies)
        self.assertNotIn('mcubsp/backend/', dependencies)

suite = unittest.defaultTestLoader.loadTestsFromTestCase(Workflows)
result = unittest.TextTestRunner(verbosity=2).run(suite)
failed = {test.id() for test, _ in result.failures + result.errors}
names = unittest.defaultTestLoader.getTestCaseNames(Workflows)
requirements = {
    'test_active_documentation_links_resolve': ['REQ-0051'],
    'test_merge_uses_registry_and_rejects_missing_reports': ['REQ-0039', 'REQ-0052'],
    'test_large_native_reports_preserve_every_case_in_bounded_trace_shards': ['REQ-0039', 'REQ-0052'],
    'test_source_consumer_preserves_parent_tests': ['REQ-0047'],
    'test_library_only_install_and_relocated_consumer': ['REQ-0047'],
    'test_compile_failure_must_match_diagnostic': ['REQ-0035', 'REQ-0052'],
    'test_board_validation_precedes_platform_setup': ['REQ-0049'],
    'test_board_toolchain_validates_before_compiler_discovery': ['REQ-0049', 'REQ-0050'],
    'test_public_firmware_rejects_misspelled_or_empty_arguments': ['REQ-0049', 'REQ-0050'],
    'test_board_bindings_are_independent_of_io_and_backends': ['REQ-0048', 'REQ-0049'],
    'test_pending_operation_diagnostic': ['REQ-0018', 'REQ-0045'],
}
print(json.dumps({'format': 'mcutest-results', 'version': 1,
                  'summary': {'tests': result.testsRun, 'failures': len(failed)},
                  'tests': [{'name': 'build/' + name, 'status': 'failed' if f'__main__.Workflows.{name}' in failed else 'passed',
                             'requirements': requirements[name]} for name in names]}))
raise SystemExit(not result.wasSuccessful())
