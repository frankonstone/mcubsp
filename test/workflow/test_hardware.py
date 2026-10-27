#!/usr/bin/env python3
"""Exercise image validation and runner planning without opening any device."""
import argparse
import hashlib
import importlib.util
import json
import os
from pathlib import Path
from types import SimpleNamespace
import struct
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('mcubsp_hardware', ROOT / 'tool/hardware.py')
hardware = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hardware)


def elf(path, address, machine=40):
    identity = b'\x7fELF' + bytes([1, 1, 1]) + bytes(9)
    header = identity + struct.pack('<HHIIIIIHHHHHH', 2, machine, 1, address, 52, 0, 0, 52, 32, 1, 0, 0, 0)
    program = struct.pack('<IIIIIIII', 1, 256, address, address, 16, 16, 5, 4)
    path.write_bytes((header + program).ljust(256, b'\0') + bytes(range(16)))


class Runner(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(prefix='mcubsp runner checks ')
        self.root = Path(self.tmp.name)
        self.image = self.root / 'firmware with spaces.elf'
        elf(self.image, 0x4000)
        self.record()
        self.env = patch.dict(os.environ, {'JLINK_EXE': sys.executable, 'OPENOCD_EXE': sys.executable,
            'IDF_PYTHON': sys.executable, 'MCUBSP_PROBE_SERIAL': ''})
        self.env.start()
    def tearDown(self):
        self.env.stop()
        self.tmp.cleanup()
    def record(self, board='arduino/uno_r4_minima', revision='r4', **extra):
        data = {'artifact': str(self.image), 'board': board, 'revision': revision,
                'startup': 'supplied', 'bytes': self.image.stat().st_size,
                'sha256': hardware.digest(self.image), **extra}
        self.image.with_name(self.image.name + '.build.json').write_text(json.dumps(data))
    def args(self, **changes):
        data = {'board': 'arduino/uno_r4_minima', 'revision': None, 'elf': str(self.image),
            'probe': '123456789', 'port': None, 'sdk_build': None, 'commands': None, 'tool_environment': None}
        return SimpleNamespace(**{**data, **changes})
    def sdk(self):
        self.image = self.root / 'firmware.elf'
        elf(self.image, 0x10000, 243)
        files = {'0x2000': 'bootloader.bin', '0x8000': 'partition-table.bin', '0x10000': 'firmware.bin'}
        for name in files.values():
            (self.root / name).write_bytes(bytes(32))
        manifest = {'flash_files': files, 'write_flash_args': ['--flash-mode', 'dio'],
            'extra_esptool_args': {'chip': 'esp32p4'}, 'app': {'offset': '0x10000', 'file': 'firmware.bin'}}
        path = self.root / 'flasher_args.json'
        path.write_text(json.dumps(manifest))
        self.record('waveshare/esp32p4_nano', 'development-v1.0',
            flash_manifest_sha256=hardware.digest(path),
            flash_files={offset: {'file': name, 'sha256': hardware.digest(self.root / name)}
                         for offset, name in files.items()})
        return self.args(board='waveshare/esp32p4_nano', probe=None, port='selected-port', sdk_build=str(self.root))
    def test_jlink_has_explicit_probe_and_preserves_spaced_path(self):
        plan = hardware.plan_flash(self.args())
        self.assertIn('-USB', plan['command'])
        self.assertIn('123456789', plan['command'])
        self.assertIn(f'loadfile "{self.image.resolve().as_posix()}"', plan['command_file'])
        self.assertFalse((self.root / 'flash.jlink').exists())
    def test_bootloader_and_wrong_architecture_are_rejected(self):
        elf(self.image, 0x2000)
        self.record()
        with self.assertRaisesRegex(ValueError, 'outside declared'):
            hardware.plan_flash(self.args())
        elf(self.image, 0x4000, 62)
        self.record()
        with self.assertRaisesRegex(ValueError, 'machine'):
            hardware.plan_flash(self.args())
    def test_missing_probe_and_changed_firmware_are_rejected(self):
        with self.assertRaisesRegex(ValueError, 'explicit probe'):
            hardware.plan_flash(self.args(probe=None))
        self.image.write_bytes(self.image.read_bytes() + b'changed')
        with self.assertRaisesRegex(ValueError, 'differs'):
            hardware.plan_flash(self.args())
    def test_identity_and_missing_record_are_rejected(self):
        with self.assertRaisesRegex(ValueError, 'unsupported'):
            hardware.plan_flash(self.args(revision='wrong'))
        self.record(revision='wrong')
        with self.assertRaisesRegex(ValueError, 'does not match'):
            hardware.plan_flash(self.args())
        self.image.with_name(self.image.name + '.build.json').unlink()
        with self.assertRaisesRegex(ValueError, 'missing artifact record'):
            hardware.plan_flash(self.args())
    def test_selected_input_sidecar_integrity_is_required(self):
        sidecar = self.image.with_name(self.image.name + '.inputs.json')
        sidecar.write_text('{"identity_sha256":"selected-inputs"}')
        self.record(inputs={'status': 'recorded', 'file': sidecar.name,
                            'sha256': hardware.digest(sidecar)})
        hardware.plan_flash(self.args())
        sidecar.write_text('{}')
        with self.assertRaisesRegex(ValueError, 'input record'):
            hardware.plan_flash(self.args())
        sidecar.unlink()
        with self.assertRaisesRegex(ValueError, 'input record'):
            hardware.plan_flash(self.args())
    def test_openocd_escapes_tcl_metacharacters(self):
        self.image = self.root / 'firmware $test [value].elf'
        elf(self.image, 0x08000000)
        self.record('st/nucleo_h753zi', 'mb1364-rev-e')
        plan = hardware.plan_flash(self.args(board='st/nucleo_h753zi'))
        command = next(arg for arg in plan['command'] if arg.startswith('program '))
        self.assertIn('verify', command)
        self.assertIn('\\$test', command)
        self.assertIn('\\[value\\]', command)
    def test_sdk_bundle_records_all_regions_and_selected_port(self):
        plan = hardware.plan_flash(self.sdk())
        self.assertIn('--port', plan['command'])
        self.assertIn('selected-port', plan['command'])
        self.assertIn('0x2000', plan['command'])
        self.assertIn('0x8000', plan['command'])
        self.assertIn('0x10000', plan['command'])
    def test_sdk_changed_image_and_manifest_are_rejected(self):
        args = self.sdk()
        (self.root / 'bootloader.bin').write_bytes(b'changed')
        with self.assertRaisesRegex(ValueError, 'image differs'):
            hardware.plan_flash(args)
        args = self.sdk()
        (self.root / 'flasher_args.json').write_text('{}')
        with self.assertRaisesRegex(ValueError, 'another chip|differs'):
            hardware.plan_flash(args)
    def test_sdk_region_limits_and_missing_bootloader_are_rejected(self):
        args = self.sdk()
        (self.root / 'bootloader.bin').write_bytes(bytes(0x6001))
        record = self.image.with_name(self.image.name + '.build.json')
        data = json.loads(record.read_text())
        data['flash_files']['0x2000']['sha256'] = hardware.digest(self.root / 'bootloader.bin')
        record.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'outside declared'):
            hardware.plan_flash(args)
        args = self.sdk()
        path = self.root / 'flasher_args.json'
        manifest = json.loads(path.read_text())
        del manifest['flash_files']['0x2000']
        path.write_text(json.dumps(manifest))
        data = json.loads(record.read_text())
        data['flash_manifest_sha256'] = hardware.digest(path)
        record.write_text(json.dumps(data))
        with self.assertRaisesRegex(ValueError, 'must contain'):
            hardware.plan_flash(args)
    def test_sdk_missing_port_or_wrong_build_is_rejected(self):
        args = self.sdk()
        args.port = None
        with self.assertRaisesRegex(ValueError, 'explicit serial'):
            hardware.plan_flash(args)
        args.port = 'selected-port'
        original = self.image
        self.image = self.root / 'copied' / original.name
        self.image.parent.mkdir()
        elf(self.image, 0x10000, 243)
        self.image.write_bytes(self.image.read_bytes() + b'different')
        saved = json.loads(original.with_name(original.name + '.build.json').read_text())
        self.record('waveshare/esp32p4_nano', 'development-v1.0',
            flash_manifest_sha256=saved['flash_manifest_sha256'], flash_files=saved['flash_files'])
        args.elf = str(self.image)
        with self.assertRaisesRegex(ValueError, 'SDK build ELF'):
            hardware.plan_flash(args)
    def test_dry_run_does_not_execute_a_debugger(self):
        result = subprocess.run([sys.executable, str(ROOT / 'tool/hardware.py'), 'flash',
            '--board', 'arduino/uno_r4_minima', '--elf', str(self.image), '--probe', '123', '--dry-run'],
            capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)
        plan = json.loads(result.stdout)
        self.assertEqual(plan['probe'], '123')
        self.assertEqual(plan['command'][-1], '{command_file}')
        self.assertFalse((self.root / 'flash.jlink').exists())
    def test_missing_explicit_tool_does_not_fall_back(self):
        with patch.dict(os.environ, {'JLINK_EXE': str(self.root / 'missing')}):
            with self.assertRaisesRegex(ValueError, 'JLINK_EXE'):
                hardware.executable('python', 'JLINK_EXE')
        selected = str(self.root / 'venv' / 'bin' / 'python')
        with patch.object(hardware.shutil, 'which', return_value=selected), \
                patch.object(Path, 'resolve', side_effect=AssertionError('dereferenced interpreter')):
            self.assertEqual(hardware.executable('python', 'IDF_PYTHON'), selected)
    def test_truncated_program_headers_are_rejected(self):
        self.image.write_bytes(self.image.read_bytes()[:60])
        self.record()
        with self.assertRaisesRegex(ValueError, 'truncated'):
            hardware.plan_flash(self.args())
    def test_firmware_record_tracks_sdk_file_hashes(self):
        args = self.sdk()
        subprocess.run(['cmake', f'-DFIRMWARE={self.image}', f'-Dsdk_build={self.root}',
            '-Dboard=waveshare/esp32p4_nano', '-Drevision=development-v1.0', '-Dstartup=supplied',
            '-Dcompiler=test', '-P', str(ROOT / 'cmake/FirmwareArtifact.cmake')], check=True)
        self.assertEqual(hardware.plan_flash(args)['runner'], 'esp-idf')
        failure = subprocess.run(['cmake', f'-DFIRMWARE={self.image}', '-Dgenerator=Ninja',
            f'-Dframework_root={ROOT}', f'-Dbuild_root={self.root}', f'-Dpython={self.root / "missing-python"}',
            '-P', str(ROOT / 'cmake/FirmwareArtifact.cmake')], capture_output=True, text=True)
        self.assertNotEqual(failure.returncode, 0)
        self.assertFalse(self.image.with_name(self.image.name + '.build.json').exists())

    def test_board_selection_ignores_unrelated_board_rules(self):
        (self.root / 'tool').mkdir()
        (self.root / 'tool/RunnerInfo.cmake').write_bytes((ROOT / 'tool/RunnerInfo.cmake').read_bytes())
        for name, body in [('selected', 'set(MCUBSP_BOARD_DEFAULT_REVISION r1)\nset(MCUBSP_BOARD_REVISIONS r1)\nset(MCUBSP_BOARD_RUNNER [=[{"type":"jlink","device":"selected"}]=])'),
                           ('unrelated', 'message(FATAL_ERROR "unselected board was evaluated")')]:
            folder = self.root / 'src' / 'board' / 'vendor' / name
            folder.mkdir(parents=True)
            (folder / 'board.cmake').write_text(body)
        with patch.object(hardware, 'ROOT', self.root):
            self.assertEqual(hardware.board_info('vendor/selected')['runner']['device'], 'selected')

    def test_serial_banner_spans_reads_and_times_out(self):
        spec = importlib.util.spec_from_file_location('banner', ROOT / 'tool/read-serial-banner.py')
        banner = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(banner)
        now = [0.0]
        chunks = iter([b'noise\r\nMCUBSP H753 ', b'USART3 OK\r\n'])
        def read(wait):
            now[0] += wait
            return next(chunks, b'')
        banner.scan(read, b'MCUBSP H753 USART3 OK', 1.0, clock=lambda: now[0])
        with self.assertRaises(TimeoutError):
            banner.scan(read, b'missing', 0.3, clock=lambda: now[0])
        with self.assertRaises(ValueError):
            banner.scan(read, b'', 1, clock=lambda: now[0])

    def test_manifest_split_preserves_multiple_sources_and_module_identity(self):
        spec = importlib.util.spec_from_file_location('manifests', ROOT / 'tool/coverage-manifests.py')
        manifests = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(manifests)
        inputs = []
        for index in range(2):
            folder = self.root / str(index)
            folder.mkdir()
            inputs.append(folder)
            (folder / 'same.mcucov.json').write_text(json.dumps({'build_id': 123,
                'modules': [{'id': index, 'path': 'src/board/test.cpp', 'probes': [index]},
                            {'id': index + 2, 'path': 'include/test.hpp', 'probes': [index + 2]}]}))
        output = self.root / 'split'
        self.assertEqual(manifests.split_manifests(inputs, output), 4)
        records = [json.loads(path.read_text()) for path in output.glob('*.mcucov.json')]
        self.assertEqual({item['modules'][0]['id'] for item in records}, {0, 1, 2, 3})
        self.assertTrue(all(item['build_id'] == 123 and len(item['modules']) == 1 for item in records))
        filtered = self.root / 'filtered'
        self.assertEqual(manifests.split_manifests(inputs, filtered, ['src/board/']), 2)
        with self.assertRaisesRegex(ValueError, 'empty'):
            manifests.split_manifests(inputs, output)
        (self.root / 'empty').mkdir()
        with self.assertRaisesRegex(ValueError, 'no coverage manifests'):
            manifests.split_manifests([self.root / 'empty'], self.root / 'empty-output')


if __name__ == '__main__':
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(Runner)
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    failed = {test.id() for test, _ in result.failures + result.errors}
    names = unittest.defaultTestLoader.getTestCaseNames(Runner)
    print(json.dumps({'format': 'mcutest-results', 'version': 1,
        'summary': {'tests': result.testsRun, 'failures': len(failed)},
        'tests': [{'name': 'runner/' + name, 'status': 'failed' if f'__main__.Runner.{name}' in failed else 'passed',
                   'requirements': (['REQ-0048'] if name == 'test_board_selection_ignores_unrelated_board_rules'
                       else ['REQ-0039', 'REQ-0052'] if name.startswith('test_manifest_split')
                       else ['REQ-0049', 'REQ-0050'])} for name in names]}))
    raise SystemExit(not result.wasSuccessful())
