"""Run real unittest cases and export their outcomes as mcutest evidence."""
import argparse
import json
from pathlib import Path
import sys
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tool"))


class EvidenceResult(unittest.TextTestResult):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.records = []

    def record(self, test, status):
        self.records.append({"name": test.id(), "status": status,
                             "requirements": list(getattr(test, "requirements", ()))})

    def addSuccess(self, test):
        super().addSuccess(test)
        self.record(test, "passed")

    def addFailure(self, test, error):
        super().addFailure(test, error)
        self.record(test, "failed")

    def addError(self, test, error):
        super().addError(test, error)
        self.record(test, "failed")

    def addSkip(self, test, reason):
        super().addSkip(test, reason)
        # Missing execution is not a passing test or fabricated evidence.
        self.record(test, "skipped")

    def addSubTest(self, test, subtest, error):
        super().addSubTest(test, subtest, error)
        if error is not None:
            self.records.append({"name": subtest.id(), "status": "failed",
                                 "requirements": list(getattr(test, "requirements", ()))})


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=ROOT / "build/host/artifact/mcutest-tooling.json")
    parser.add_argument("--pattern", default="test_*.py")
    args = parser.parse_args()
    suite = unittest.defaultTestLoader.discover(str(Path(__file__).parent), pattern=args.pattern)
    result = unittest.TextTestRunner(verbosity=2, resultclass=EvidenceResult).run(suite)
    report = {"format": "mcutest-results", "version": 1,
              "summary": {"tests": result.testsRun, "failures": len(result.failures) + len(result.errors)},
              "tests": result.records}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n")
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
