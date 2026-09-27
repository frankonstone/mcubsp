"""Keep target menus scoped while retaining real independent compilation gates."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def run(*args, success=True, cwd=ROOT):
    result = subprocess.run(list(map(str, args)), cwd=cwd, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if success != (result.returncode == 0):
        raise AssertionError(result.stdout)
    return result.stdout


def query(build):
    path = build / ".cmake/api/v1/query"
    path.mkdir(parents=True, exist_ok=True)
    (path / "codemodel-v2").write_text("")


def targets(build):
    reply = build / ".cmake/api/v1/reply"
    index = json.loads(sorted(reply.glob("index-*.json"))[-1].read_text())
    entry = next(item for item in index["objects"] if item["kind"] == "codemodel")
    model = json.loads((reply / entry["jsonFile"]).read_text())
    return {item["name"]: json.loads((reply / item["jsonFile"]).read_text())
            for item in model["configurations"][0]["targets"]}


def preset_targets(name):
    data = json.loads((ROOT / "CMakePresets.json").read_text())
    return next(p["targets"] for p in data["buildPresets"] if p["name"] == name)


class HeaderTargets(unittest.TestCase):
    requirements = ("REQ-0003", "REQ-0038", "REQ-0047", "REQ-0052")

    def test_one_target_keeps_headers_independent_even_with_unity_and_pch(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw header targets ") as directory:
            source = Path(directory)
            headers = source / "include/mcuhw"
            headers.mkdir(parents=True)
            (headers / "a.hpp").write_text("#pragma once\nstruct Known {};\n")
            # Would compile incorrectly if a.hpp were injected by unity/PCH.
            (headers / "b.hpp").write_text("#pragma once\ninline Known value() { return {}; }\n")
            (source / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.25)
project(header_independence LANGUAGES CXX)
set(CMAKE_UNITY_BUILD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_library(mcuhw INTERFACE)
add_library(mcuhw::mcuhw ALIAS mcuhw)
target_include_directories(mcuhw INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
include("${MCUHW_SOURCE_DIR}/cmake/McuhwProjectOptions.cmake")
mcuhw_add_header_self_containment_checks(check-headers ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_precompile_headers(check-headers PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/mcuhw/a.hpp")
''')
            build = source / "build"
            query(build)
            run("cmake", "-S", source, "-B", build, "-G", "Ninja", f"-DMCUHW_SOURCE_DIR={ROOT}")
            failure = run("cmake", "--build", build, "--target", "check-headers", success=False)
            self.assertIn("b.hpp", failure)
            self.assertEqual(list(targets(build)), ["check-headers"])
            commands = json.loads((build / "compile_commands.json").read_text())
            self.assertEqual(len(commands), 2)
            self.assertFalse(any("Unity/" in c["file"] or "cmake_pch" in c["file"] for c in commands))
            (headers / "b.hpp").write_text('#pragma once\n#include <mcuhw/a.hpp>\ninline Known value() { return {}; }\n')
            run("cmake", "--build", build, "--target", "check-headers")
            # Catalog/header growth creates new compiler jobs, not new targets.
            (headers / "c.hpp").write_text("#pragma once\nstruct Another {};\n")
            run("cmake", "--build", build, "--target", "check-headers")
            self.assertEqual(list(targets(build)), ["check-headers"])
            self.assertEqual(len(json.loads((build / "compile_commands.json").read_text())), 3)


class FocusedTargets(unittest.TestCase):
    requirements = ("REQ-0038", "REQ-0047", "REQ-0052")

    def test_component_preset_clears_old_maintainer_options_and_builds_only_selection(self):
        candidates = [Path(os.environ.get("MCUHW_MCUTEST_SOURCE_DIR", ROOT.parent / "mcutest")), ROOT / "_ci/mcutest"]
        mcutest = next((p for p in candidates if (p / "CMakeLists.txt").is_file()), None)
        if mcutest is None:
            self.skipTest("mcutest checkout required for real native suite build")
        with tempfile.TemporaryDirectory(prefix="mcuhw selected suite ") as directory:
            build = Path(directory)
            query(build)
            run("cmake", "-S", ROOT, "-B", build, "-G", "Ninja",
                "-DMCUHW_BUILD_PROBES=ON", "-DMCUHW_CHECK_HEADERS=ON")
            run("cmake", "--preset", "component", "-B", build,
                "-DMCUHW_TEST_SUITES=device:ti/tmp102", f"-DMCUHW_MCUTEST_SOURCE_DIR={mcutest}")
            model = targets(build)
            self.assertEqual(set(model), {"native-tests", "mcuhw_test_device_ti_tmp102"})
            run("cmake", "--build", build, "--target", *preset_targets("component"))
            run("ctest", "--test-dir", build, "--output-on-failure")
            report = json.loads((build / "artifacts/native/device_ti_tmp102.json").read_text())
            self.assertGreater(report["summary"]["tests"], 0)
            self.assertEqual(report["summary"]["failures"], 0)

    def test_shipped_presets_have_explicit_build_actions(self):
        listing = run("cmake", "--list-presets=all")
        self.assertIn("verify-host", listing)
        presets = json.loads((ROOT / "CMakePresets.json").read_text())
        for preset in presets["buildPresets"]:
            self.assertTrue(preset["targets"], preset["name"])
        self.assertEqual(preset_targets("host"), ["native-tests", "check-headers", "check-compile"])

    @unittest.skipUnless(shutil.which("arm-none-eabi-g++"), "Arm GNU compiler unavailable")
    def test_application_presets_expose_only_selected_firmware_and_provenance(self):
        for preset, example in (("uno-r4-minima-app", "blink"), ("nucleo-h753zi-app", "tmp102")):
            with self.subTest(preset=preset), tempfile.TemporaryDirectory(prefix="mcuhw preset app ") as directory:
                build = Path(directory)
                query(build)
                run("cmake", "--preset", preset, "-B", build, f"-DMCUHW_EXAMPLE={example}",
                    "-DMCUHW_MCUTEST_SOURCE_DIR=/missing", "-DMCUHW_MCUIP_SOURCE_DIR=/missing")
                self.assertEqual(set(targets(build)), {"firmware"})
                run("cmake", "--build", build, "--target", *preset_targets(preset))
                self.assertTrue((build / "application/firmware.elf").is_file())
                identity = json.loads((build / "application/firmware.identity.json").read_text())
                self.assertEqual(identity["target"], "application/firmware.elf")
                paths = {p["path"] for p in identity["files"]}
                self.assertIn(f"source:examples/apps/{example}/main.cpp", paths)
                self.assertFalse(any("mcutest" in p or "header-checks" in p for p in paths))

    @unittest.skipUnless(shutil.which("arm-none-eabi-g++"), "Arm GNU compiler unavailable")
    def test_application_rejects_mixed_maintainer_scope(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw mixed scope ") as directory:
            output = run("cmake", "--preset", "uno-r4-minima-app", "-B", directory,
                         "-DMCUHW_CHECK_HEADERS=ON", success=False)
            self.assertIn("Application presets build one firmware", output)
