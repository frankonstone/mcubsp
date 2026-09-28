"""Exercise the shared CMake/IDE artifact path and physical runner boundary."""
import argparse
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

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tool'))
import hardware
import mcuhw
from mcuhwlib import environment, runners, workflow


class HardwareWorkflowTests(unittest.TestCase):
    requirements = ('REQ-0049', 'REQ-0050')

    def test_flash_region_numbers_follow_decimal_and_hex_semantics(self):
        self.assertEqual(runners._regions({'flash_regions': [{'start': '0008', 'size': '0X10'}]}),
                         [(8, 16)])

    def test_openocd_fixture_uses_shared_probe_board_settings_and_selected_image(self):
        with tempfile.TemporaryDirectory(prefix='mcuhw hardware [fixture] ') as directory:
            elf = Path(directory) / 'selected $firmware.elf'
            image = bytearray(84)
            image[:6] = b'\x7fELF\x01\x01'
            struct.pack_into('<I', image, 28, 52)
            struct.pack_into('<HH', image, 42, 32, 1)
            struct.pack_into('<IIIII', image, 52, 1, 0, 0x08000000, 0x08000000, 84)
            elf.write_bytes(image)
            args = argparse.Namespace(board='st/nucleo_h753zi', revision=None, elf=elf,
                sdk_build=None, port=None, probe='fixture-serial', tool_environment='OPENOCD_EXE',
                commands='reset halt; resume; sleep 250; halt; shutdown')
            with mock.patch.dict(os.environ, {'OPENOCD_EXE': sys.executable}):
                plan = hardware.flash_plan(args)
            command = plan['commands'][0]
            self.assertEqual(command[0], str(Path(sys.executable).absolute()))
            self.assertIn('adapter serial "fixture-serial"', command)
            self.assertIn('target/stm32h7x.cfg', command)
            self.assertIn('selected \\$firmware.elf', command[-1])
            self.assertTrue(command[-1].endswith(args.commands))
            self.assertEqual(len(plan['commands']), 1)

    def test_build_record_rejects_another_board_or_startup_selection(self):
        with tempfile.TemporaryDirectory() as directory:
            project = Path(directory)
            elf = project / 'firmware.elf'
            elf.write_bytes(b'fixture artifact')
            manifest = workflow.record_build(project, board='arduino/uno_r4_minima', revision='r4',
                                             startup='supplied', root=ROOT, artifact=elf)
            config = {'board': 'st/nucleo_h753zi', 'revision': 'r4', 'startup': 'supplied'}
            with self.assertRaisesRegex(environment.EnvironmentError, 'Build board'):
                workflow.load_build(project, config, str(manifest))
            config['board'] = 'arduino/uno_r4_minima'
            config['startup'] = 'external'
            with self.assertRaisesRegex(environment.EnvironmentError, 'Build startup'):
                workflow.load_build(project, config, str(manifest))

    def test_tool_metadata_drives_cmake_and_environment_without_exporting_ambient_secrets(self):
        state = {'tools': {'new-sdk': {'kind': 'directory', 'path': '/sdk/root',
                                    'cmake_variable': 'MY_SDK_ROOT', 'environment': 'MY_SDK_PATH'},
                           'compiler': {'kind': 'executable', 'path': '/compiler/bin/cc'}}}
        with mock.patch.dict(os.environ, {'SECRET_FIXTURE': 'never export this'}):
            self.assertEqual(environment.cmake_variables(state, ['new-sdk']), {'MY_SDK_ROOT': '/sdk/root'})
            exported = environment.preset_environment(state, ['new-sdk', 'compiler'])
        self.assertEqual(exported['MY_SDK_PATH'], '/sdk/root')
        self.assertNotIn('SECRET_FIXTURE', exported)
        self.assertTrue(exported['PATH'].endswith('$penv{PATH}'))

    def test_sdk_record_tracks_flashed_images_in_addition_to_the_elf(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            sdk = build / 'sdk'
            sdk.mkdir()
            elf = build / 'firmware.elf'
            elf.write_bytes(b'ELF fixture')
            (sdk / 'firmware.bin').write_bytes(b'original flash image')
            (sdk / 'flasher_args.json').write_text(json.dumps({'flash_files': {'0x10000': 'firmware.bin'}}))
            config = {'board': 'waveshare/esp32p4_nano', 'revision': 'development-v1.0', 'startup': 'supplied'}
            manifest = workflow.record_build(build, **config, root=ROOT, artifact=elf, sdk_build=sdk)
            workflow.load_build(build, config, str(manifest))
            (sdk / 'firmware.bin').write_bytes(b'changed flash image')
            with self.assertRaisesRegex(environment.EnvironmentError, 'flash images or settings changed'):
                workflow.load_build(build, config, str(manifest))


@unittest.skipUnless(all(shutil.which(tool) for tool in ('cmake', 'ninja', 'arm-none-eabi-g++')),
                     'A real Arm toolchain exercises the shared IDE build path')
class CMakeWorkflowIntegration(unittest.TestCase):
    requirements = ('REQ-0049', 'REQ-0050', 'REQ-0052')

    def test_setup_preset_direct_cmake_build_records_flashable_artifact(self):
        with tempfile.TemporaryDirectory(prefix='mcuhw IDE workflow ') as directory:
            project = Path(directory) / 'application'
            def cli(*args):
                output, errors = io.StringIO(), io.StringIO()
                with contextlib.redirect_stdout(output), contextlib.redirect_stderr(errors):
                    status = mcuhw.main(list(args))
                self.assertEqual(status, 0, output.getvalue() + errors.getvalue())
            cli('new', str(project), '--board', 'arduino/uno_r4_minima')
            cli('setup', '--project', str(project), '--offline', '--allow-version-override')
            # These are the same ordinary commands an IDE invokes; the frontend
            # does not create the record on this path.
            for command in (['cmake', '--preset', 'mcuhw'], ['cmake', '--build', '--preset', 'mcuhw']):
                result = subprocess.run(command, cwd=project, text=True, capture_output=True)
                self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            config = environment.read_json(project / 'mcuhw.json')
            record = workflow.load_build(project, config)
            self.assertEqual(record['board'], 'arduino/uno_r4_minima')
            self.assertTrue(Path(record['artifacts']['elf']).is_file())
            self.assertEqual(Path(record['build_dir']), Path(workflow.selected_preset(project)['binaryDir']))
            self.assertTrue((project / '.mcuhw/last-build.json').is_file())
            # Direct CMake also checks the recorded tools; bypassing the frontend
            # must not silently bypass the imported environment's integrity.
            state_path = project / '.mcuhw/environment.json'
            state = environment.read_json(state_path)
            state['tools']['ninja']['sha256'] = 'changed'
            environment.write_json(state_path, state)
            result = subprocess.run(['cmake', '--preset', 'mcuhw'], cwd=project, text=True, capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('recorded sha256 changed', result.stdout + result.stderr)


if __name__ == '__main__':
    unittest.main()
