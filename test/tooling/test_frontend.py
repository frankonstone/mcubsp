"""Portable first-build, offline-cache and runner planning checks. Never flashes.

@req REQ-0049 REQ-0050
"""
from __future__ import annotations

import contextlib
import io
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest import mock
import zipfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tool'))
import mcuhw
from mcuhwlib import boards, environment, runners, workflow


class FrontendTests(unittest.TestCase):
    requirements = ('REQ-0049', 'REQ-0050')
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='mcuhw frontend space ')
        self.directory = Path(self.temporary.name)

    def tearDown(self):
        self.temporary.cleanup()

    def cli(self, *args):
        output, errors = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(errors):
            status = mcuhw.main(list(args))
        return status, output.getvalue(), errors.getvalue()

    def new(self, board='arduino/uno_r4_minima'):
        project = self.directory / 'application with spaces'
        result = self.cli('new', str(project), '--board', board)
        self.assertEqual(result[0], 0, result)
        return project

    def fake_lock(self):
        # Import the actual running Python under each role; no fake shell script
        # or POSIX-only executable fixture is needed on Windows CI.
        lock = self.directory / 'lock.json'
        environment.write_json(lock, {'schema_version': 1, 'tools': {
            name: {'kind': 'executable', 'command': sys.executable, 'version': sys.version.split()[0],
                   'version_args': ['--version']}
            for name in ('cmake', 'ninja', 'arm-gcc', 'jlink')}})
        return lock

    def setup_project(self, project):
        result = self.cli('setup', '--project', str(project), '--lock', str(self.fake_lock()), '--offline')
        self.assertEqual(result[0], 0, result)

    def test_new_external_application_and_portable_argument_planning(self):
        project = self.new()
        self.setup_project(project)
        self.assertTrue((project / 'main.cpp').is_file())
        self.assertTrue((project / 'CMakeLists.txt').is_file())
        config = environment.read_json(project / 'mcuhw.json')
        self.assertEqual(config['revision'], 'r4')
        result = self.cli('build', '--project', str(project), '--dry-run')
        self.assertEqual(result[0], 0, result)
        plan = json.loads(result[1])
        self.assertEqual(str(project.resolve()), plan['cwd'])
        self.assertEqual(plan['preset']['cacheVariables']['MCUHW_STARTUP'], 'supplied')
        self.assertEqual(plan['preset']['cacheVariables']['MCUHW_BOARD'], 'arduino/uno_r4_minima')
        self.assertEqual(plan['commands'][0][1:], ['--preset', 'mcuhw'])
        self.assertEqual(plan['commands'][1][1:], ['--build', '--preset', 'mcuhw'])
        # A dry-run neither creates the build directory nor invokes CMake.
        self.assertFalse(Path(plan['build_dir']).exists())

    def test_new_refuses_existing_content_and_unknown_board(self):
        project = self.new()
        result = self.cli('new', str(project), '--board', 'arduino/uno_r4_minima')
        self.assertEqual(result[0], 2)
        self.assertIn('not empty', result[2])
        result = self.cli('new', str(self.directory / 'other'), '--board', 'unknown/board')
        self.assertEqual(result[0], 2)
        self.assertFalse((self.directory / 'other').exists())
        result = self.cli('new', str(self.directory / 'nucleo'), '--board', 'st/nucleo_h753zi')
        self.assertEqual(result[0], 2)
        self.assertIn('not declared', result[2])
        self.assertFalse((self.directory / 'nucleo').exists())

    def test_no_setup_diagnostic_and_changed_lock(self):
        project = self.new()
        result = self.cli('build', '--project', str(project))
        self.assertIn('run mcuhw.py setup', result[2])
        self.setup_project(project)
        with (project / '.mcuhw/toolchains.lock.json').open('a') as output:
            output.write('\n')
        result = self.cli('doctor', '--project', str(project))
        self.assertIn('tool lock changed', result[2])

    def test_build_does_not_need_flash_tool_and_flash_does_not_need_compiler(self):
        project = self.new()
        self.setup_project(project)
        result = self.cli('doctor', '--project', str(project), '--operation', 'build')
        self.assertEqual(result[0], 0, result)
        self.assertNotIn('jlink', json.loads(result[1])['tools'])
        result = self.cli('doctor', '--project', str(project), '--operation', 'flash')
        self.assertEqual(result[0], 1, result)
        self.assertEqual(set(json.loads(result[1])['tools']), {'jlink'})

    def test_subprocess_path_only_includes_tools_for_the_requested_operation(self):
        state = {'tools': {
            'cmake': {'kind': 'executable', 'path': str(self.directory / 'build tools/cmake')},
            'jlink': {'kind': 'executable', 'path': str(self.directory / 'flash tools/JLinkExe')},
            'esp-idf': {'kind': 'git', 'path': str(self.directory / 'unselected SDK')},
        }}
        with mock.patch.dict(os.environ, {'PATH': 'original-path'}, clear=True):
            build = environment.process_environment(state, ['cmake'])
            flash = environment.process_environment(state, ['jlink'])
        self.assertEqual(build['PATH'].split(os.pathsep), [str(self.directory / 'build tools'), 'original-path'])
        self.assertEqual(flash['PATH'].split(os.pathsep), [str(self.directory / 'flash tools'), 'original-path'])
        self.assertNotIn('IDF_PATH', build)

    def test_flash_import_keeps_build_directory_identity(self):
        project = self.new()
        self.setup_project(project)
        first = json.loads(self.cli('build', '--project', str(project), '--dry-run')[1])['build_dir']
        result = self.cli('setup', '--project', str(project), '--operation', 'flash', '--tool', f'jlink={sys.executable}')
        self.assertEqual(result[0], 0, result)
        second = json.loads(self.cli('build', '--project', str(project), '--dry-run')[1])['build_dir']
        self.assertEqual(first, second)
        # A completed build, including one made directly in an IDE, records its
        # actual artifact instead of relying on the current compiler identity.
        artifact = self.directory / 'IDE output' / 'custom-name.elf'
        artifact.parent.mkdir()
        image = bytearray(84)
        image[:6] = b'\x7fELF\x01\x01'
        struct.pack_into('<I', image, 28, 52)
        struct.pack_into('<HH', image, 42, 32, 1)
        struct.pack_into('<IIIII', image, 52, 1, 0, 0x4000, 0x4000, 84)
        artifact.write_bytes(image)
        workflow.record_build(artifact.parent, board='arduino/uno_r4_minima', revision='r4',
            startup='supplied', root=ROOT, artifact=artifact, pointer=project / '.mcuhw/last-build.json')
        state = environment.read_json(project / '.mcuhw/environment.json')
        state['tools'].pop('arm-gcc')  # Flashing an existing image needs no compiler.
        environment.write_json(project / '.mcuhw/environment.json', state)
        with mock.patch('mcuhw.subprocess.run', side_effect=AssertionError('must not execute')):
            # Inspection also runs executables; mock only its verified output here.
            with mock.patch('mcuhw.environment.verify', return_value=[]):
                result = self.cli('flash', '--project', str(project), '--probe', '123456', '--dry-run')
        self.assertEqual(result[0], 0, result)
        argv = json.loads(result[1])['commands'][0]
        self.assertEqual(argv[argv.index('-SelectEmuBySN') + 1], '123456')
        self.assertIn(str(artifact), next(iter(json.loads(result[1])['files'].values())))
        self.assertFalse(Path(first).exists())

    @unittest.skipUnless(shutil.which('cmake'), 'CMake validates the actual preset format')
    def test_setup_exposes_same_configuration_to_cmake_and_preserves_user_presets(self):
        project = self.new()
        user = {'version': 3, 'configurePresets': [{'name': 'personal', 'generator': 'Ninja',
                                                 'binaryDir': str(project / 'personal-build')}]}
        (project / 'CMakeUserPresets.json').write_text(json.dumps(user))
        self.setup_project(project)
        actual = json.loads((project / 'CMakeUserPresets.json').read_text())
        self.assertEqual(actual['configurePresets'], user['configurePresets'])
        self.assertEqual(actual['include'], ['.mcuhw/CMakePresets.json'])
        result = subprocess.run(['cmake', '--list-presets'], cwd=project, text=True, capture_output=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertIn('"mcuhw"', result.stdout)
        self.assertIn('"personal"', result.stdout)
        # Repeated setup does not add duplicate includes or mutate user choices.
        self.setup_project(project)
        self.assertEqual(json.loads((project / 'CMakeUserPresets.json').read_text()), actual)

    def test_flash_refuses_unbuilt_or_changed_artifacts_even_for_dry_run(self):
        project = self.new()
        self.setup_project(project)
        self.assertEqual(self.cli('setup', '--project', str(project), '--operation', 'flash',
                                  '--tool', f'jlink={sys.executable}')[0], 0)
        self.assertIn('No completed build record', self.cli('flash', '--project', str(project),
            '--probe', '123', '--dry-run')[2])
        artifact = self.directory / 'firmware.elf'
        artifact.write_bytes(b'original artifact')
        workflow.record_build(self.directory, board='arduino/uno_r4_minima', revision='r4',
            startup='supplied', root=ROOT, artifact=artifact, pointer=project / '.mcuhw/last-build.json')
        artifact.write_bytes(b'changed artifact')
        result = self.cli('flash', '--project', str(project), '--probe', '123', '--dry-run')
        self.assertEqual(result[0], 2)
        self.assertIn('Recorded firmware is missing or changed', result[2])

    def test_changed_compiler_or_revision_changes_build_directory(self):
        project = self.new()
        self.setup_project(project)
        state = environment.read_json(project / '.mcuhw/environment.json')
        config = environment.read_json(project / 'mcuhw.json')
        first = mcuhw._build_dir(project, config, state)
        state['tools']['arm-gcc']['sha256'] = 'changed'
        self.assertNotEqual(first, mcuhw._build_dir(project, config, state))
        config['revision'] = 'another'
        self.assertNotEqual(first, mcuhw._build_dir(project, config, state))

    def test_version_override_is_explicit_and_recorded(self):
        spec = {'kind': 'executable', 'version': '0.0.0', 'version_args': ['--version']}
        with self.assertRaisesRegex(environment.EnvironmentError, 'expected 0.0.0'):
            environment.inspect_tool('python', spec, Path(sys.executable))
        result = environment.inspect_tool('python', spec, Path(sys.executable), allow_override=True)
        self.assertTrue(result['override'])
        result['sha256'] = 'changed'
        errors = environment.verify({'tools': {'python': result}}, {'tools': {'python': spec}}, ['python'])
        self.assertIn('sha256 changed', errors[0])

    def test_cpp_library_failure_has_focused_remediation(self):
        with mock.patch.object(environment, 'run_output', side_effect=environment.EnvironmentError('expected header missing')):
            with self.assertRaisesRegex(environment.EnvironmentError, 'matching libstdc'):
                environment.check_cpp_library(Path(sys.executable))



class ArchiveTests(unittest.TestCase):
    requirements = ('REQ-0050',)
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='mcuhw cache ')
        self.directory = Path(self.temporary.name)
        self.archive = self.directory / 'tool.zip'
        with zipfile.ZipFile(self.archive, 'w') as bundle:
            bundle.writestr('tool/bin/compiler', 'compiler payload')
        self.checksum = environment.digest(self.archive)
        self.spec = {'archives': {environment.host_id(): {'url': self.archive.as_uri(), 'sha256': self.checksum,
                                                      'format': 'zip', 'path': 'tool/bin/compiler'}}}
        self.cache = self.directory / 'cache'

    def tearDown(self):
        self.temporary.cleanup()

    def test_explicit_setup_then_offline_reuse(self):
        tool = environment.setup_archive(self.spec, self.cache, offline=False)
        self.assertEqual(tool.read_text(), 'compiler payload')
        self.archive.unlink()
        tool.write_text('modified extraction')
        with mock.patch('urllib.request.urlopen', side_effect=AssertionError('offline must not fetch')):
            tool = environment.setup_archive(self.spec, self.cache, offline=True)
        self.assertEqual(tool.read_text(), 'compiler payload')

    def test_offline_cache_miss_and_tamper(self):
        with self.assertRaisesRegex(environment.EnvironmentError, 'Offline cache entry missing'):
            environment.setup_archive(self.spec, self.cache, offline=True)
        environment.setup_archive(self.spec, self.cache, offline=False)
        (self.cache / 'archives' / self.checksum).write_text('tampered')
        with self.assertRaisesRegex(environment.EnvironmentError, 'Cached archive integrity mismatch'):
            environment.setup_archive(self.spec, self.cache, offline=True)

    def test_download_integrity_and_archive_path_traversal(self):
        self.spec['archives'][environment.host_id()]['sha256'] = '0' * 64
        with self.assertRaisesRegex(environment.EnvironmentError, 'integrity mismatch'):
            environment.setup_archive(self.spec, self.cache, offline=False)
        with zipfile.ZipFile(self.archive, 'w') as bundle:
            bundle.writestr('../outside', 'unsafe')
        self.spec['archives'][environment.host_id()]['sha256'] = environment.digest(self.archive)
        with self.assertRaisesRegex(environment.EnvironmentError, 'Unsafe archive member'):
            environment.setup_archive(self.spec, self.cache, offline=False)
        self.assertFalse((self.cache / 'unpacked/outside').exists())

    def test_windows_device_stream_and_ambiguous_archive_names_are_rejected_on_every_host(self):
        for name in ('tool/AUX.txt', 'tool/file:stream', 'tool/COM1', 'tool/trailing.', 'tool/file '):
            with self.subTest(name=name):
                with zipfile.ZipFile(self.archive, 'w') as bundle:
                    bundle.writestr(name, 'unsafe')
                self.spec['archives'][environment.host_id()]['sha256'] = environment.digest(self.archive)
                with self.assertRaisesRegex(environment.EnvironmentError, 'Unsafe archive member'):
                    environment.setup_archive(self.spec, self.cache, offline=False)


@unittest.skipUnless(shutil.which('git'), 'SDK identity checks require Git')
class SdkIdentityTests(unittest.TestCase):
    requirements = ('REQ-0050',)

    def test_nested_dirty_submodule_is_rejected_despite_ignore_config_and_explicit_override(self):
        # Actual local Git repositories prove we are not relying on the parent
        # diff honoring an inherited ignore-submodules configuration. No network.
        with tempfile.TemporaryDirectory(prefix='mcuhw SDK identity ') as temporary:
            directory = Path(temporary)

            def git(path, *arguments):
                return environment.run_output(['git', '-C', str(path), *arguments])

            def commit(path):
                git(path, 'add', '.')
                git(path, '-c', 'user.name=mcuhw fixture', '-c', 'user.email=fixture@example.invalid',
                    '-c', 'commit.gpgsign=false', 'commit', '-qm', 'fixture')

            def repository(name):
                path = directory / name
                path.mkdir()
                git(path, 'init', '-q')
                (path / 'source.cpp').write_text('original\n')
                commit(path)
                return path

            leaf = repository('leaf source')
            middle = repository('middle source')
            git(middle, '-c', 'protocol.file.allow=always', 'submodule', 'add', '-q', str(leaf), 'nested child')
            commit(middle)
            sdk = repository('SDK checkout')
            git(sdk, '-c', 'protocol.file.allow=always', 'submodule', 'add', '-q', str(middle), 'middle child')
            commit(sdk)
            git(sdk, '-c', 'protocol.file.allow=always', 'submodule', 'update', '--init', '--recursive')
            revision = git(sdk, 'rev-parse', 'HEAD')
            spec = {'kind': 'git', 'required_file': 'source.cpp', 'version': revision}
            self.assertFalse(environment.inspect_tool('sdk', spec, sdk)['override'])
            different_pin = {**spec, 'version': '0' * 40}
            with self.assertRaisesRegex(environment.EnvironmentError, 'allow-version-override'):
                environment.inspect_tool('sdk', different_pin, sdk)
            self.assertTrue(environment.inspect_tool('sdk', different_pin, sdk, allow_override=True)['override'])

            git(sdk, 'config', 'diff.ignoreSubmodules', 'all')
            git(sdk, 'config', 'submodule.middle child.ignore', 'all')
            git(sdk / 'middle child', 'config', 'submodule.nested child.ignore', 'all')
            nested = sdk / 'middle child/nested child'
            (nested / 'source.cpp').write_text('changed dependency\n')
            self.assertEqual(git(sdk, 'diff', 'HEAD', '--', '.'), '')
            for allow in (False, True):
                with self.subTest(allow_version_override=allow):
                    with self.assertRaisesRegex(environment.EnvironmentError, 'SDK submodule has uncommitted files'):
                        environment.inspect_tool('sdk', spec, sdk, allow_override=allow)
            (nested / 'source.cpp').write_text('original\n')
            (nested / 'added-source.cpp').write_text('untracked dependency\n')
            with self.assertRaisesRegex(environment.EnvironmentError, 'uncommitted files'):
                environment.inspect_tool('sdk', spec, sdk, allow_override=True)


class RunnerTests(unittest.TestCase):
    requirements = ('REQ-0049', 'REQ-0050')
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix='mcuhw runner [space] ')
        self.directory = Path(self.temporary.name)

    def tearDown(self):
        self.temporary.cleanup()

    def board(self, identifier):
        return boards.load(ROOT, identifier)

    def test_probe_required_and_jlink_paths_quoted(self):
        board = self.board('arduino/uno_r4_minima')
        tools = {'jlink': {'path': 'JLink.exe'}}
        with self.assertRaisesRegex(environment.EnvironmentError, '--probe'):
            runners.plan(board, self.directory, tools, require_artifacts=False)
        result = runners.plan(board, self.directory, tools, probe='123', require_artifacts=False)
        self.assertIn('-SelectEmuBySN', result['commands'][0])
        self.assertIn(f'loadfile "{self.directory / "firmware.elf"}"', next(iter(result['files'].values())))
        self.assertFalse((self.directory / 'mcuhw-flash.jlink').exists())

    def test_openocd_tcl_literals(self):
        board = self.board('st/nucleo_h753zi')
        path = self.directory / 'extra $sub [expression]'
        result = runners.plan(board, path, {'openocd': {'path': 'openocd'}}, probe='serial;$x[exec]', require_artifacts=False)
        argv = result['commands'][0]
        self.assertIn('target/stm32h7x.cfg', argv)
        self.assertIn('adapter serial "serial;\\$x\\[exec\\]"', argv)
        self.assertIn('\\$sub', argv[-1])
        self.assertIn('\\[expression\\]', argv[-1])

    def test_elf_loads_must_fit_declared_region(self):
        board = self.board('arduino/uno_r4_minima')
        image = bytearray(84)
        image[:6] = b'\x7fELF\x01\x01'
        struct.pack_into('<I', image, 28, 52)
        struct.pack_into('<HH', image, 42, 32, 1)
        struct.pack_into('<IIIII', image, 52, 1, 0, 0x4000, 0x4000, 84)
        path = self.directory / 'firmware.elf'
        path.write_bytes(image)
        runners.check_elf_regions(path, board['build']['runner'])
        struct.pack_into('<I', image, 52 + 12, 0)
        path.write_bytes(image)
        with self.assertRaisesRegex(environment.EnvironmentError, 'exceeds the board flash regions'):
            runners.check_elf_regions(path, board['build']['runner'])

    def test_truncated_elf_is_rejected_before_runner_execution(self):
        board = self.board('arduino/uno_r4_minima')
        image = bytearray(84)
        image[:6] = b'\x7fELF\x01\x01'
        struct.pack_into('<I', image, 28, 52)
        struct.pack_into('<HH', image, 42, 32, 1)
        struct.pack_into('<IIIII', image, 52, 1, 80, 0x4000, 0x4000, 8)
        (self.directory / 'firmware.elf').write_bytes(image)
        with self.assertRaisesRegex(environment.EnvironmentError, 'Truncated ELF load segment'):
            runners.plan(board, self.directory, {'jlink': {'path': 'JLinkExe'}}, probe='123')

    def test_esp_flash_uses_generated_offsets_and_requires_explicit_port(self):
        board = self.board('waveshare/esp32p4_nano')
        sdk = self.directory / 'esp-idf-build'
        sdk.mkdir()
        settings = {'extra_esptool_args': {'chip': 'esp32p4'},
                    'flash_settings': {'flash_mode': 'dio', 'flash_size': '16MB', 'flash_freq': '80m'},
                    'flash_files': {'0x2000': 'bootloader.bin', '0x8000': 'partition.bin', '0x10000': 'firmware.bin'}}
        for name in settings['flash_files'].values():
            (sdk / name).write_bytes(b'firmware')
        (sdk / 'flasher_args.json').write_text(json.dumps(settings))
        tools = {'idf-python': {'path': sys.executable}}
        with self.assertRaisesRegex(environment.EnvironmentError, '--port'):
            runners.plan(board, self.directory, tools)
        result = runners.plan(board, self.directory, tools, port='COM4')
        argv = result['commands'][0]
        self.assertEqual(argv[:6], [sys.executable, '-m', 'esptool', '--chip', 'esp32p4', '--port'])
        self.assertIn('COM4', argv)
        for address in settings['flash_files']:
            self.assertIn(address, argv)
        settings['flash_files']['0x0'] = settings['flash_files'].pop('0x2000')
        (sdk / 'flasher_args.json').write_text(json.dumps(settings))
        with self.assertRaisesRegex(environment.EnvironmentError, 'does not match the board'):
            runners.plan(board, self.directory, tools, port='COM4')


if __name__ == '__main__':
    unittest.main()
