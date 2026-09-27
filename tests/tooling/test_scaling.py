"""Affected-suite safety and real consumer isolation at catalog scale.

@req REQ-0052
"""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/"tools"))
from mcuhwlib import catalog
from mcuhwlib.impact import impact

spec=importlib.util.spec_from_file_location("mcuhw_check_scale",ROOT/"tools/check-scale.py")
scale=importlib.util.module_from_spec(spec)
spec.loader.exec_module(scale)


class AffectedComponents(unittest.TestCase):
    requirements=("REQ-0052",)

    def test_device_change_selects_its_contracts_without_unrelated_backends(self):
        result=impact(ROOT,["include/mcuhw/device/sensor/tmp102.hpp"])
        self.assertEqual("affected",result["mode"])
        self.assertEqual(["device:ti/tmp102"],result["components"])
        self.assertEqual(["compile","device:ti/tmp102","io"],result["suites"])
        self.assertNotIn("all",result["suites"])

    def test_ip_change_includes_mcu_and_transitive_board_dependents(self):
        result=impact(ROOT,["include/mcuhw/backend/esp32p4/i2c.hpp"])
        self.assertEqual(["board:waveshare/esp32p4_nano","ip:espressif/esp32p4_native","mcu:espressif/esp32p4"],result["components"])
        self.assertEqual(["waveshare/esp32p4_nano"],result["boards"])
        self.assertIn("descriptions",result["suites"])
        self.assertNotIn("mcu:renesas/ra4m1",result["suites"])

    def test_board_change_selects_native_dependency_contracts_and_cross_build(self):
        result=impact(ROOT,["board/st/nucleo_h753zi/component.json"])
        self.assertEqual(["board:st/nucleo_h753zi"],result["components"])
        self.assertEqual(["st/nucleo_h753zi"],result["boards"])
        self.assertIn("mcu:st/stm32h753",result["suites"])
        self.assertIn("ip:st/stm32h753_native",result["suites"])
        self.assertNotIn("board:st/nucleo_h753zi",result["suites"])

    def test_core_shared_deleted_and_unknown_inputs_select_all(self):
        for path in ["include/mcuhw/core/metadata.hpp","include/mcuhw/driver/i2c.hpp",
                     "examples/apps/blink/CMakeLists.txt","include/mcuhw/deleted_header.hpp","README.md"]:
            with self.subTest(path=path):
                result=impact(ROOT,[path])
                self.assertEqual("all",result["mode"])
                self.assertEqual(["all"],result["suites"])
                self.assertEqual(3,len(result["boards"]))
                self.assertTrue(result["reasons"])

    def test_absolute_owned_path_and_duplicate_inputs_are_normalized(self):
        relative="include/mcuhw/device/sensor/tmp102.hpp"
        result=impact(ROOT,[ROOT/relative,relative])
        self.assertEqual([relative],result["changed_paths"])
        self.assertEqual("affected",result["mode"])
        self.assertEqual(result,impact(ROOT,[relative]))

    def test_empty_diff_is_empty(self):
        result=impact(ROOT,[])
        self.assertEqual([],result["components"])
        self.assertEqual([],result["suites"])
        self.assertEqual([],result["cmake_args"])

    def test_outside_or_uncertain_paths_select_all(self):
        for path in ["../outside.hpp",Path(tempfile.gettempdir())/"unrelated.hpp"]:
            self.assertEqual("all",impact(ROOT,[path])["mode"])

    def test_catalog_contains_no_scaling_fixtures(self):
        self.assertFalse(any(value["id"].startswith("scale_fixture/") for value in catalog.discover(ROOT)))


class CatalogScale(unittest.TestCase):
    requirements=("REQ-0052",)

    def test_real_native_consumer_isolated_from_3300_unrelated_entries(self):
        if not shutil.which("cmake") or not shutil.which("ninja"):
            self.skipTest("CMake and Ninja required for measured scaling")
        compiler=os.environ.get("CXX") or shutil.which("g++-15") or shutil.which("g++-14")
        if not compiler:
            self.skipTest("Set CXX to a tested C++23 compiler to exercise native scaling")
        with tempfile.TemporaryDirectory(prefix="mcuhw-scale-report-") as temporary:
            output=Path(temporary)/"scaling.json"
            report=scale.benchmark(ROOT,output,mode="host-device",compiler=compiler)
            saved=json.loads(output.read_text())
        self.assertEqual("passed",report["state"])
        self.assertEqual(300,report["expanded"]["board_count"]-report["baseline"]["board_count"])
        self.assertEqual(3000,report["expanded"]["device_count"]-report["baseline"]["device_count"])
        self.assertEqual(1,report["expanded"]["build"]["target_count"])
        self.assertEqual(1,report["expanded"]["build"]["compile_count"])
        self.assertTrue(all(report["unchanged"].values()))
        self.assertEqual("host-device",saved["mode"])
        self.assertGreater(saved["expanded"]["discovery_seconds"],0)
        self.assertTrue(report["expanded"]["build"]["identity"]["files"])
        self.assertFalse(any("scale_fixture" in item["path"] for item in report["expanded"]["build"]["identity"]["files"]))


if __name__=="__main__":
    unittest.main()
