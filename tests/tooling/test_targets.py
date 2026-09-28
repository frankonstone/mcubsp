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


def board_targets(board):
    result = {"tests"}
    result.update(f"test.{group}" for group in ("board", "core", "driver", "io", "backend", "device", "mcu", "compile"))
    if board == "arduino/uno_r4_minima":
        result.update(("firmware", "examples", "example.app.blink", "example.app.tmp102"))
        result.update(f"example.io.{name}" for name in ("blink", "serial_echo", "spi_dma", "tmp102", "w5100"))
    return result


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
            # Board/header growth creates new compiler jobs, not new targets.
            (headers / "c.hpp").write_text("#pragma once\nstruct Another {};\n")
            run("cmake", "--build", build, "--target", "check-headers")
            self.assertEqual(list(targets(build)), ["check-headers"])
            self.assertEqual(len(json.loads((build / "compile_commands.json").read_text())), 3)


class FocusedTargets(unittest.TestCase):
    requirements = ("REQ-0038", "REQ-0047", "REQ-0052")

    def test_native_preset_preserves_explicit_suite_and_verification_settings(self):
        candidates = [Path(os.environ.get("MCUHW_MCUTEST_SOURCE_DIR", ROOT.parent / "mcutest")), ROOT / "_ci/mcutest"]
        mcutest = next((p for p in candidates if (p / "CMakeLists.txt").is_file()), None)
        if mcutest is None:
            self.skipTest("mcutest checkout required for real native suite build")
        with tempfile.TemporaryDirectory(prefix="mcuhw selected suite ") as directory:
            build = Path(directory)
            query(build)
            run("cmake", "-S", ROOT, "-B", build, "-G", "Ninja",
                "-DMCUHW_BUILD_PROBES=ON", "-DMCUHW_CHECK_HEADERS=ON")
            run("cmake", "--preset", "native", "-B", build,
                "-DMCUHW_BUILD_TESTS=ON", "-DMCUHW_BUILD_PROBES=OFF", "-DMCUHW_CHECK_HEADERS=OFF",
                "-DMCUHW_TEST_SUITES=device:ti/tmp102", f"-DMCUHW_MCUTEST_SOURCE_DIR={mcutest}")
            model = targets(build)
            self.assertEqual(set(model), {"native-tests", "mcuhw_test_device_ti_tmp102"})
            run("cmake", "--build", build, "--target", "native-tests")
            run("ctest", "--test-dir", build, "--output-on-failure")
            report = json.loads((build / "artifacts/native/device_ti_tmp102.json").read_text())
            self.assertGreater(report["summary"]["tests"], 0)
            self.assertEqual(report["summary"]["failures"], 0)

    def test_shipped_presets_default_to_uno_only_for_cortex_m4(self):
        run("cmake", "--list-presets=all")
        presets = json.loads((ROOT / "CMakePresets.json").read_text())
        self.assertEqual({p["name"] for p in presets["configurePresets"] if not p.get("hidden")},
                         {"native", "arm-cortex-m4", "arm-cortex-m7", "esp32p4-riscv"})
        configurations = {preset["name"]: preset for preset in presets["configurePresets"]}
        self.assertEqual(configurations["base"]["cacheVariables"]["MCUHW_BOARD"],
                         "arduino/uno_r4_minima")
        self.assertNotIn("MCUHW_BOARD", configurations["arm-cortex-m4"].get("cacheVariables", {}))
        for name in ("native", "arm-cortex-m7", "esp32p4-riscv"):
            self.assertIsNone(configurations[name]["cacheVariables"]["MCUHW_BOARD"])
        for preset in presets["buildPresets"]:
            self.assertNotIn("targets", preset)

    @unittest.skipUnless(shutil.which("arm-none-eabi-g++"), "Arm GNU compiler unavailable")
    def test_board_define_exposes_tasks_and_firmware_provenance(self):
        for preset, board, example in (("arm-cortex-m4", "arduino/uno_r4_minima", "blink"),
                                       ("arm-cortex-m7", "st/nucleo_h753zi", "")):
            with self.subTest(board=board), tempfile.TemporaryDirectory(prefix="mcuhw board tasks ") as directory:
                build = Path(directory)
                query(build)
                board_arg = [] if preset == "arm-cortex-m4" else [f"-DMCUHW_BOARD={board}"]
                output = run("cmake", "--preset", preset, "-B", build, *board_arg,
                    f"-DMCUHW_EXAMPLE={example}", "-DCMAKE_CXX_FLAGS=-DMCUHW_GLOBAL_FLAG=1",
                    "-DMCUHW_MCUTEST_SOURCE_DIR=/missing", "-DMCUHW_MCUIP_SOURCE_DIR=/missing")
                self.assertIn(f"Selected board: {board}@", output)
                self.assertEqual(set(targets(build)), board_targets(board))
                cache = (build / "CMakeCache.txt").read_text()
                self.assertIn(f"MCUHW_BOARD:STRING={board}", cache)
                if example:
                    run("cmake", "--build", build, "--target", "firmware", "examples")
                    self.assertTrue((build / "application/firmware.elf").is_file())
                    self.assertFalse(list(build.rglob("selected_board.hpp")))
                    identity = json.loads((build / "application/firmware.identity.json").read_text())
                    self.assertEqual(identity["target"], "application/firmware.elf")
                    paths = {p["path"] for p in identity["files"]}
                    self.assertIn(f"source:examples/apps/{example}/main.cpp", paths)
                    self.assertFalse(any("mcutest" in p or "header-checks" in p for p in paths))
                    commands = json.loads((build / "compile_commands.json").read_text())
                    self.assertTrue(all("-DMCUHW_GLOBAL_FLAG=1" in c["command"] for c in commands if c["file"].endswith(".cpp")))
                # Reapplying the preset retains the selected board and app.
                run("cmake", "--preset", preset, "-B", build)
                self.assertEqual(set(targets(build)), board_targets(board))
                failure = run("cmake", "--build", build, "--target", "test.board", success=False)
                self.assertIn("mcutest headers not found", failure)
                seed = next((build / "boards").rglob("tests-cache.cmake")).read_text()
                self.assertIn("-DMCUHW_GLOBAL_FLAG=1", seed)
                # Clearing the selection removes all board tasks.
                run("cmake", "--preset", preset, "-B", build, "-DMCUHW_BOARD=")
                self.assertEqual(set(targets(build)), set())

    @unittest.skipUnless(shutil.which("arm-none-eabi-g++"), "Arm GNU compiler unavailable")
    def test_board_define_rejects_wrong_architecture_and_undeclared_firmware(self):
        for board, example, message in (
            ("st/nucleo_h753zi", "", "requires architecture 'cortex-m7'"),
            ("missing/board", "", "Unknown mcuhw board"),
            ("arduino/uno_r4_minima", "missing", "not a declared application"),
        ):
            with self.subTest(board=board, example=example), tempfile.TemporaryDirectory() as directory:
                output = run("cmake", "--preset", "arm-cortex-m4", "-B", directory,
                             f"-DMCUHW_BOARD={board}", f"-DMCUHW_EXAMPLE={example}", success=False)
                self.assertIn(message, output)

    @unittest.skipUnless(shutil.which("arm-none-eabi-g++"), "Arm GNU compiler unavailable")
    def test_io_examples_configure_standalone(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw standalone io ") as directory:
            build = Path(directory)
            run("cmake", "-S", ROOT / "examples/io", "-B", build, "-G", "Ninja",
                f"-DMCUHW_SOURCE_DIR={ROOT}",
                f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/McuhwToolchain.cmake",
                "-DMCUHW_BOARD=arduino/uno_r4_minima", "-DMCUHW_IO_EXAMPLE=tmp102",
                "-DMCUHW_INSTALL=OFF")
            run("cmake", "--build", build)
            self.assertTrue((build / "firmware.elf").is_file())
            self.assertFalse(list(build.rglob("selected_board.hpp")))

    @unittest.skipUnless(shutil.which("arm-none-eabi-g++"), "Arm GNU compiler unavailable")
    def test_board_menu_native_categories_run_with_a_host_compiler(self):
        mcutest = Path(os.environ.get("MCUHW_MCUTEST_SOURCE_DIR", ROOT.parent / "mcutest"))
        if not (mcutest / "CMakeLists.txt").is_file():
            self.skipTest("mcutest checkout required")
        with tempfile.TemporaryDirectory(prefix="mcuhw board native groups ") as directory:
            build = Path(directory)
            query(build)
            run("cmake", "--preset", "arm-cortex-m4", "-B", build,
                "-DMCUHW_BOARD=arduino/uno_r4_minima", f"-DMCUHW_MCUTEST_SOURCE_DIR={mcutest}")
            self.assertEqual(set(targets(build)), board_targets("arduino/uno_r4_minima"))
            self.assertFalse(any((build / f"native-tests/{group}/CMakeCache.txt").exists()
                                 for group in ("backend", "device", "core")))
            device_build = build / "native-tests/device"
            query(device_build)
            run("cmake", "--build", build, "--target", "test.device")
            model = targets(device_build)
            self.assertTrue(any("device_ti_tmp102" in name for name in model))
            self.assertFalse(any("ip_" in name or "mcu_" in name for name in model))
            reports = list((device_build / "artifacts/native").glob("*.json"))
            self.assertTrue(reports)
            for report in reports:
                self.assertEqual(json.loads(report.read_text())["summary"]["failures"], 0)
            self.assertFalse((build / "native-tests/backend/CMakeCache.txt").exists())
            backend_build = build / "native-tests/backend"
            query(backend_build)
            run("cmake", "--build", build, "--target", "test.backend")
            self.assertEqual(set(targets(backend_build)), {"native-tests",
                "mcuhw_test_ip_renesas_ra4m1_native", "mcuhw_test_ip_st_stm32h753_native",
                "mcuhw_test_ip_espressif_esp32p4_native"})
            commands = json.loads((backend_build / "compile_commands.json").read_text())
            self.assertTrue(all("arm-none-eabi" not in c["command"] for c in commands))
            self.assertFalse(next((build / "boards").rglob("tests-cache.cmake")).parent.joinpath("tests/CMakeCache.txt").exists())
