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
sys.path.insert(0, str(ROOT / "tools"))
from mcuhwlib.impact import impact
from mcuhwlib.verification import board_actions, matrices

spec = importlib.util.spec_from_file_location("verify_boards", ROOT / "tools/verify-boards.py")
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)


class BoardVerificationTests(unittest.TestCase):
    requirements = ("REQ-0048", "REQ-0052")

    def test_codec_change_selects_codec_and_its_mounted_board(self):
        selected = impact(ROOT, ["tests/device/test_es8311.cpp"])
        self.assertEqual(selected["mode"], "affected")
        self.assertEqual(selected["boards"], ["waveshare/esp32p4_nano"])
        self.assertIn("device:everest/es8311", selected["suites"])
        actions = matrices(ROOT, selected["boards"])
        self.assertEqual(actions["arm-gcc"]["include"], [])
        self.assertEqual(len(actions["esp-idf"]["include"]), 2)

    def test_new_local_board_and_revision_need_no_ci_registry(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "catalog").mkdir()
            shutil.copy(ROOT / "catalog/schema.json", root / "catalog/schema.json")
            value = json.loads((ROOT / "board/arduino/uno_r4_minima/component.json").read_text())
            value.update(id="fixture/local", revisions=["v1", "v2"], default_revision="v1", evidence=[])
            value["examples"] = ["examples/new-app/CMakeLists.txt"]
            path = root / "board/fixture/local/component.json"
            path.parent.mkdir(parents=True)
            path.write_text(json.dumps(value))
            actions = board_actions(root, ["fixture/local"])
            self.assertEqual([a["revision"] for a in actions], ["v1", "v2"])
            self.assertEqual({a["example"] for a in actions}, {"examples/new-app"})
            self.assertEqual(len({a["key"] for a in actions}), 2)

    def test_ra_controller_tests_have_one_backend_owner(self):
        selected = impact(ROOT, ["tests/backend/ra4m1/test_uart.cpp"])
        self.assertEqual(selected["mode"], "affected")
        self.assertEqual(selected["boards"], ["arduino/uno_r4_minima"])
        self.assertIn("ip:renesas/ra4m1_native", selected["suites"])
        self.assertNotIn("ip:espressif/esp32p4_native", selected["suites"])

    def test_missing_tool_fails_instead_of_skipping_cross_build(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch.object(verify.subprocess, "run", side_effect=FileNotFoundError("cmake missing")):
                status = verify.main(["--board", "arduino/uno_r4_minima", "--example", "examples/apps/blink",
                                      "--build", directory])
            self.assertEqual(status, 1)
            result = json.loads((Path(directory) / "artifacts/cross-build.json").read_text())
            self.assertEqual(result["state"], "failed")
            self.assertEqual(result["stage"], "cross-build")

    def test_success_requires_firmware_artifact(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch.object(verify.subprocess, "run"):
                status = verify.main(["--board", "arduino/uno_r4_minima", "--example", "examples/apps/blink",
                                      "--build", directory])
            self.assertEqual(status, 1)
            self.assertEqual(json.loads((Path(directory) / "artifacts/cross-build.json").read_text())["state"], "failed")

    def test_invalid_selection_replaces_previous_success_and_its_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            artifacts = Path(directory) / 'artifacts'
            artifacts.mkdir()
            report = artifacts / 'cross-build.json'
            report.write_text(json.dumps({'state': 'passed'}))
            (artifacts / 'firmware.identity.json').write_text('old passing identity')
            with patch.object(verify.subprocess, 'run') as run:
                status = verify.main(['--board', 'missing/board', '--example', 'examples/apps/blink',
                                      '--build', directory])
            self.assertEqual(status, 1)
            run.assert_not_called()
            self.assertEqual(json.loads(report.read_text())['state'], 'failed')
            self.assertIn('unknown board', json.loads(report.read_text())['error'])
            self.assertFalse((artifacts / 'firmware.identity.json').exists())

    def test_bare_stale_elf_cannot_be_reported_as_a_successful_action(self):
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / 'firmware.elf').write_bytes(b'old unrelated image')
            with patch.object(verify.subprocess, 'run'):
                status = verify.main(['--board', 'arduino/uno_r4_minima', '--example', 'examples/apps/blink',
                                      '--build', directory])
            self.assertEqual(status, 1)
            result = json.loads((Path(directory) / 'artifacts/cross-build.json').read_text())
            self.assertEqual(result['state'], 'failed')
            self.assertIn('No completed build record', result['error'])

    def test_declared_non_project_example_cannot_silently_disappear(self):
        from mcuhwlib import catalog
        board = catalog.load(ROOT, 'board', 'arduino/uno_r4_minima')
        board['examples'] = ['examples/fragment.cpp']
        with patch('mcuhwlib.verification.catalog.load', return_value=board):
            with self.assertRaisesRegex(catalog.CatalogError, 'must name an examples'):
                board_actions(ROOT, [board['id']])

    def test_esp_sdk_must_match_lock_before_any_build_command(self):
        with tempfile.TemporaryDirectory() as directory:
            with patch.dict(verify.os.environ, {'IDF_PATH': '/fixture/sdk'}), \
                    patch.object(verify.environment, 'inspect_tool', side_effect=ValueError('unexpected SDK revision')), \
                    patch.object(verify.subprocess, 'run') as run:
                status = verify.main(['--board', 'waveshare/esp32p4_nano', '--example', 'examples/apps/blink',
                                      '--build', directory])
            self.assertEqual(status, 1)
            run.assert_not_called()
            result = json.loads((Path(directory) / 'artifacts/cross-build.json').read_text())
            self.assertEqual(result['state'], 'failed')
            self.assertEqual(result['error'], 'unexpected SDK revision')

    def test_malformed_build_record_preserves_failure_report(self):
        with tempfile.TemporaryDirectory() as directory:
            (Path(directory) / 'firmware.build.json').write_text(json.dumps({
                'schema_version': 1, 'board': 'arduino/uno_r4_minima', 'revision': 'r4',
                'startup': 'supplied'}))
            with patch.object(verify.subprocess, 'run'):
                status = verify.main(['--board', 'arduino/uno_r4_minima', '--example', 'examples/apps/blink',
                                      '--build', directory])
            self.assertEqual(status, 1)
            result = json.loads((Path(directory) / 'artifacts/cross-build.json').read_text())
            self.assertEqual(result['state'], 'failed')
            self.assertIn('artifacts', result['error'])
