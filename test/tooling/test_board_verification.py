"""A changed component must select its owned tests and real cross-build actions."""
import importlib.util
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tool"))
from mcuhwlib.verification import board_actions, matrices

spec = importlib.util.spec_from_file_location("verify_boards", ROOT / "tool/verify-boards.py")
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)


class BoardVerificationTests(unittest.TestCase):
    requirements = ("REQ-0048", "REQ-0052")

    def test_board_actions_use_local_build_settings(self):
        actions = board_actions(ROOT, ["arduino/uno_r4_minima"])
        self.assertEqual({action["example"] for action in actions},
                         {"example/app/blink", "example/app/tmp102"})
        self.assertEqual({action["revision"] for action in actions}, {"r4"})
        self.assertEqual(matrices(ROOT, ["waveshare/esp32p4_nano"])["esp-idf"]["include"], [])

    def test_missing_tool_fails_instead_of_skipping_cross_build(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch.object(verify.subprocess, "run", side_effect=FileNotFoundError("cmake missing")):
                status = verify.main(["--board", "arduino/uno_r4_minima", "--example", "example/app/blink",
                                      "--build", directory])
            self.assertEqual(status, 1)
            result = json.loads((Path(directory) / "artifact/cross-build.json").read_text())
            self.assertEqual(result["state"], "failed")
            self.assertEqual(result["stage"], "cross-build")

    def test_success_requires_firmware_artifact(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch.object(verify.subprocess, "run"):
                status = verify.main(["--board", "arduino/uno_r4_minima", "--example", "example/app/blink",
                                      "--build", directory])
            self.assertEqual(status, 1)
            self.assertEqual(json.loads((Path(directory) / "artifact/cross-build.json").read_text())["state"], "failed")

    def test_invalid_selection_replaces_previous_success_and_its_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            artifacts = Path(directory) / 'artifact'
            artifacts.mkdir()
            report = artifacts / 'cross-build.json'
            report.write_text(json.dumps({'state': 'passed'}))
            (artifacts / 'firmware.identity.json').write_text('old passing identity')
            with patch.object(verify.subprocess, 'run') as run:
                status = verify.main(['--board', 'missing/board', '--example', 'example/app/blink',
                                      '--build', directory])
            self.assertEqual(status, 1)
            run.assert_not_called()
            self.assertEqual(json.loads(report.read_text())['state'], 'failed')
            self.assertIn('Unknown mcuhw board', json.loads(report.read_text())['error'])
            self.assertFalse((artifacts / 'firmware.identity.json').exists())

    def test_bare_stale_elf_cannot_be_reported_as_a_successful_action(self):
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / 'firmware.elf').write_bytes(b'old unrelated image')
            with patch.object(verify.subprocess, 'run'):
                status = verify.main(['--board', 'arduino/uno_r4_minima', '--example', 'example/app/blink',
                                      '--build', directory])
            self.assertEqual(status, 1)
            result = json.loads((Path(directory) / 'artifact/cross-build.json').read_text())
            self.assertEqual(result['state'], 'failed')
            self.assertIn('No completed build record', result['error'])

    def test_declared_non_project_example_cannot_silently_disappear(self):
        from mcuhwlib import boards
        board = boards.load(ROOT, 'arduino/uno_r4_minima')
        board['examples'] = ['example/fragment.cpp']
        with patch('mcuhwlib.verification.board_settings.load', return_value=board):
            with self.assertRaisesRegex(ValueError, 'must name an example'):
                board_actions(ROOT, [board['id']])

    def test_esp_sdk_must_match_lock_before_any_build_command(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch.dict(verify.os.environ, {'IDF_PATH': '/fixture/sdk'}), \
                    patch.object(verify.environment, 'inspect_tool', side_effect=ValueError('unexpected SDK revision')), \
                    patch.object(verify, 'board_actions', return_value=[{
                        'board': 'waveshare/esp32p4_nano', 'revision': 'development-v1.0',
                        'example': 'example/app/blink', 'environment': 'esp-idf', 'key': 'fixture'}]), \
                    patch.object(verify.subprocess, 'run') as run:
                status = verify.main(['--board', 'waveshare/esp32p4_nano', '--example', 'example/app/blink',
                                      '--build', directory])
            self.assertEqual(status, 1)
            run.assert_not_called()
            result = json.loads((Path(directory) / 'artifact/cross-build.json').read_text())
            self.assertEqual(result['state'], 'failed')
            self.assertEqual(result['error'], 'unexpected SDK revision')

    def test_malformed_build_record_preserves_failure_report(self):
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / 'firmware.build.json').write_text(json.dumps({
                'schema_version': 1, 'board': 'arduino/uno_r4_minima', 'revision': 'r4',
                'startup': 'supplied'}))
            with patch.object(verify.subprocess, 'run'):
                status = verify.main(['--board', 'arduino/uno_r4_minima', '--example', 'example/app/blink',
                                      '--build', directory])
            self.assertEqual(status, 1)
            result = json.loads((Path(directory) / 'artifact/cross-build.json').read_text())
            self.assertEqual(result['state'], 'failed')
            self.assertIn('artifacts', result['error'])
