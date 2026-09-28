"""Ensure split-suite evidence cannot incorporate stale or unselected results."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location("verify", Path(__file__).resolve().parents[2] / "tool/verify.py")
verify = importlib.util.module_from_spec(spec)
spec.loader.exec_module(verify)


class VerificationEvidenceTests(unittest.TestCase):
    requirements = ("REQ-0036", "REQ-0052")

    def test_registry_selects_only_requested_suites(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            current, stale = build / "current.json", build / "old.json"
            record = {"format":"mcutest-results", "version":1, "tests":[
                {"name":"current", "status":"passed", "requirements":["REQ-0052"]}]}
            current.write_text(json.dumps(record))
            stale.write_text("invalid stale output")
            paths = verify.evidence_paths({"tests":[{"command":["cmake", f"-DOUTPUT_FILE={current}"]}]}, build)
            output = build / "merged.json"
            self.assertEqual(verify.merge_evidence(paths, output), 1)
            self.assertEqual(json.loads(output.read_text())["tests"], record["tests"])

    def test_missing_current_result_cannot_reuse_old_aggregate(self):
        with tempfile.TemporaryDirectory() as directory:
            build = Path(directory)
            with self.assertRaisesRegex(ValueError, "did not write"):
                verify.merge_evidence([build / "missing.json"], build / "merged.json")
            self.assertFalse((build / "merged.json").exists())

    def test_registry_cannot_delete_external_evidence(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(ValueError, "escapes"):
                verify.evidence_paths({"tests":[{"command":["-DOUTPUT_FILE=/external/evidence.json"]}]}, Path(directory))
