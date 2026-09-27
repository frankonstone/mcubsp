"""Selected-board configuration and real linked external applications."""
# @req REQ-0049
import os
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


def run(*args, success=True, **kwargs):
    result = subprocess.run([str(arg) for arg in args], text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            **kwargs)
    if success and result.returncode:
        raise AssertionError(result.stdout)
    if not success and not result.returncode:
        raise AssertionError("Unexpected success: " + result.stdout)
    return result.stdout


class SelectionTests(unittest.TestCase):
    requirements = ('REQ-0049',)
    requirements = ("REQ-0049",)
    def test_unknown_board(self):
        output = run("cmake", "-DMCUHW_BOARD=missing/board", "-P",
                     ROOT / "cmake/McuhwToolchain.cmake", success=False)
        self.assertIn("Unknown mcuhw board", output)

    def test_unsupported_revision(self):
        output = run("cmake", "-DMCUHW_BOARD=arduino/uno_r4_minima",
                     "-DMCUHW_BOARD_REVISION=unknown", "-P",
                     ROOT / "cmake/McuhwToolchain.cmake", success=False)
        self.assertIn("Unsupported revision", output)

    def test_identity_path_traversal(self):
        output = run("cmake", "-DMCUHW_BOARD=../uno_r4_minima", "-P",
                     ROOT / "cmake/McuhwToolchain.cmake", success=False)
        self.assertIn("exact vendor/name", output)

    def test_incompatible_host_compiler(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw board host ") as directory:
            output = run("cmake", "-S", ROOT / "examples/apps/blink",
                         "-B", directory, "-G", "Ninja",
                         f"-DMCUHW_SOURCE_DIR={ROOT}",
                         "-DMCUHW_BOARD=arduino/uno_r4_minima", success=False)
            self.assertIn("Incompatible compiler", output)

    def test_supplied_startup_rejects_custom_linker_script(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw startup ownership ") as directory:
            source = Path(directory)
            (source / "CMakeLists.txt").write_text('''
cmake_minimum_required(VERSION 3.25)
project(startup_ownership LANGUAGES CXX)
add_subdirectory("${MCUHW_SOURCE_DIR}" mcuhw)
mcuhw_add_firmware(firmware SOURCES main.cpp STARTUP supplied LINKER_SCRIPT custom.ld)
''')
            output = run("cmake", "-S", source, "-B", source / "build", "-G", "Ninja",
                         f"-DMCUHW_SOURCE_DIR={ROOT}", success=False)
            self.assertIn("LINKER_SCRIPT requires STARTUP external", " ".join(output.split()))


@unittest.skipUnless(shutil.which("arm-none-eabi-g++"), "Arm GNU compiler unavailable")
class ArmApplicationTests(unittest.TestCase):
    requirements = ('REQ-0049',)
    requirements = ("REQ-0049", "REQ-0052")
    def test_supplied_startup_from_cxx_only_project_and_artifact_pointer(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw cxx startup ") as directory:
            base = Path(directory)
            (base / "main.cpp").write_text('int main() { for (;;) __asm__("wfi"); }\n')
            (base / "CMakeLists.txt").write_text('''
cmake_minimum_required(VERSION 3.25)
project(cxx_startup LANGUAGES CXX)
add_subdirectory("${MCUHW_SOURCE_DIR}" mcuhw)
mcuhw_add_firmware(firmware SOURCES main.cpp STARTUP supplied)
''')
            build, pointer = base / "build", base / "last-build.json"
            run("cmake", "-S", base, "-B", build, "-G", "Ninja",
                f"-DMCUHW_SOURCE_DIR={ROOT}", "-DMCUHW_BOARD=arduino/uno_r4_minima",
                f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/McuhwToolchain.cmake",
                f"-DMCUHW_ARTIFACT_POINTER={pointer}")
            run("cmake", "--build", build)
            symbols = run("arm-none-eabi-nm", build / "firmware.elf")
            self.assertRegex(symbols, r"[0-9a-f]+ T Reset_Handler")
            self.assertIn("__isr_vector", symbols)
            artifact = json.loads((build / "firmware.build.json").read_text())
            self.assertEqual(artifact["board"], "arduino/uno_r4_minima")
            self.assertEqual(Path(artifact["artifacts"]["elf"]), (build / "firmware.elf").resolve())
            self.assertEqual(Path(json.loads(pointer.read_text())["manifest"]),
                             (build / "firmware.build.json").resolve())

    def test_legacy_io_examples_link_real_reset_vectors(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw legacy io ") as directory:
            build = Path(directory)
            run("cmake", "-S", ROOT, "-B", build, "-G", "Ninja",
                f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/toolchain-arm-cortex-m4.cmake",
                "-DMCUHW_BUILD_IO_EXAMPLES=ON", "-DMCUHW_INSTALL=OFF")
            examples = ["blink", "serial_echo", "spi_dma", "tmp102", "w5100"]
            run("cmake", "--build", build, "--target", *[f"mcuhw_io_{name}" for name in examples])
            for example in examples:
                with self.subTest(example=example):
                    symbols = run("arm-none-eabi-nm", build / f"mcuhw_io_{example}.elf")
                    self.assertRegex(symbols, r"[0-9a-f]+ T Reset_Handler")
                    self.assertIn("__isr_vector", symbols)

    def test_board_manifest_must_match_selected_architecture(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw wrong architecture ") as directory:
            output = run("cmake", "-S", ROOT / "examples/apps/blink", "-B", directory,
                         "-G", "Ninja", f"-DMCUHW_SOURCE_DIR={ROOT}",
                         "-DMCUHW_BOARD=st/nucleo_h753zi",
                         f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/toolchain-arm-cortex-m4.cmake",
                         success=False)
            self.assertIn("requires architecture 'cortex-m7'", " ".join(output.split()))

    def test_external_bsp_preserves_conventional_custom_toolchain(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw custom toolchain ") as directory:
            base = Path(directory)
            compiler = Path(shutil.which("arm-none-eabi-g++")).as_posix()
            (base / "toolchain.cmake").write_text(f'''set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_CXX_COMPILER "{compiler}")
''')
            (base / "main.cpp").write_text('int main() { for (;;) __asm__("wfi"); }\n')
            (base / "CMakeLists.txt").write_text('''cmake_minimum_required(VERSION 3.25)
project(custom_bsp LANGUAGES CXX)
add_subdirectory("${MCUHW_SOURCE_DIR}" mcuhw)
mcuhw_add_firmware(firmware SOURCES main.cpp STARTUP external)
target_link_options(firmware PRIVATE -nostdlib -Wl,--entry=main)
''')
            build = base / "build"
            arguments = ("cmake", "-S", base, "-B", build, "-G", "Ninja",
                         f"-DMCUHW_SOURCE_DIR={ROOT}", "-DMCUHW_BOARD=arduino/uno_r4_minima",
                         f"-DCMAKE_TOOLCHAIN_FILE={base}/toolchain.cmake")
            run(*arguments)
            run("cmake", "--build", build)
            self.assertTrue((build / "firmware.elf").is_file())
            self.assertIn("MCUHW_ARCH:STRING=cortex-m4", (build / "CMakeCache.txt").read_text())
            output = run(*arguments, "-DMCUHW_ARCH=cortex-m7", success=False)
            self.assertIn("requires architecture 'cortex-m4'", " ".join(output.split()))

    def test_external_blink_and_tmp102_all_arm_boards(self):
        for board in ("arduino/uno_r4_minima", "st/nucleo_h753zi"):
            for example in ("blink", "tmp102"):
                with self.subTest(board=board, example=example):
                    with tempfile.TemporaryDirectory(prefix="mcuhw external app ") as directory:
                        base = Path(directory)
                        source, build = base / "source", base / "build"
                        shutil.copytree(ROOT / "examples/apps" / example, source)
                        run("cmake", "-S", source, "-B", build, "-G", "Ninja",
                            f"-DMCUHW_SOURCE_DIR={ROOT}", f"-DMCUHW_BOARD={board}",
                            f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/McuhwToolchain.cmake")
                        run("cmake", "--build", build)
                        self.assertTrue((build / "firmware.elf").is_file())
                        self.assertTrue((build / "firmware.bin").is_file())
                        report = json.loads((build / "firmware.identity.json").read_text())
                        paths = {entry["path"] for entry in report["files"]}
                        self.assertIn(f"source:board/{board}/component.json", paths)
                        sensor = "source:include/mcuhw/device/sensor/tmp102.hpp"
                        self.assertEqual(sensor in paths, example == "tmp102")
                        self.assertTrue(report["toolchains"].get("CXX"))
                        ninja = (build / "build.ninja").read_text()
                        unrelated = "nucleo_h753zi" if "arduino" in board else "uno_r4_minima"
                        self.assertNotIn(f"board/{unrelated}/startup.S", ninja)
                        self.assertNotIn("mcutest", ninja)
                        self.assertNotIn("header-checks", ninja)
                        self.assertNotIn("architecture_probes", ninja)

    def test_firmware_from_relocated_installed_bundle_without_source(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw installed board ") as directory:
            base = Path(directory)
            checkout = base / "checkout"
            checkout.mkdir()
            for name in ("include", "cmake", "board", "device", "mcu", "ip", "catalog",
                         "examples", "tools", "doc", "tests"):
                shutil.copytree(ROOT / name, checkout / name,
                                ignore=shutil.ignore_patterns("build", "__pycache__",
                                                              "_vendor", "_generated"))
            for name in ("CMakeLists.txt", "README.md", "requirements.md", "implementation-plan.md"):
                shutil.copy2(ROOT / name, checkout / name)
            package_build = base / "package build"
            prefix = base / "prefix"
            run("cmake", "-S", checkout, "-B", package_build, "-G", "Ninja",
                f"-DCMAKE_INSTALL_PREFIX={prefix}")
            run("cmake", "--install", package_build)
            relocated = base / "relocated prefix"
            prefix.rename(relocated)
            shutil.rmtree(checkout)
            shutil.rmtree(package_build)
            bundle = relocated / "share/mcuhw"
            application = base / "application"
            shutil.copytree(bundle / "examples/apps/blink", application)
            build = base / "firmware build"
            run("cmake", "-S", application, "-B", build, "-G", "Ninja",
                f"-DMCUHW_SOURCE_DIR={bundle}", "-DMCUHW_BOARD=arduino/uno_r4_minima",
                f"-DCMAKE_TOOLCHAIN_FILE={bundle}/cmake/McuhwToolchain.cmake")
            run("cmake", "--build", build)
            self.assertTrue((build / "firmware.elf").is_file())
            ninja = (build / "build.ninja").read_text()
            self.assertNotIn(str(ROOT), ninja)
            self.assertNotIn(str(checkout), ninja)

    def test_external_startup_is_the_only_vector_owner(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw external bsp ") as directory:
            base = Path(directory)
            (base / "startup.cpp").write_text('''
#include <cstdint>
extern "C" [[noreturn]] void Reset_Handler() { for (;;) __asm__("wfi"); }
extern "C" [[gnu::section(".vectors"), gnu::used]]
const std::uintptr_t __isr_vector[]{0x20008000, reinterpret_cast<std::uintptr_t>(&Reset_Handler)};
''')
            shutil.copy(ROOT / "board/arduino/uno_r4_minima/bsp/linker.ld", base / "bsp.ld")
            (base / "CMakeLists.txt").write_text('''
cmake_minimum_required(VERSION 3.25)
project(external_bsp LANGUAGES CXX)
add_subdirectory("${MCUHW_SOURCE_DIR}" mcuhw)
mcuhw_add_firmware(firmware SOURCES startup.cpp STARTUP external
    LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/bsp.ld")
target_link_options(firmware PRIVATE -nostdlib)
''')
            build = base / "build"
            run("cmake", "-S", base, "-B", build, "-G", "Ninja",
                f"-DMCUHW_SOURCE_DIR={ROOT}", "-DMCUHW_BOARD=arduino/uno_r4_minima",
                f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/McuhwToolchain.cmake")
            run("cmake", "--build", build)
            symbols = run("arm-none-eabi-nm", build / "firmware.elf")
            self.assertEqual(symbols.count("__isr_vector"), 1)
            self.assertEqual(symbols.count("Reset_Handler"), 1)
            self.assertNotIn("mcuhw_uno_early_init", symbols)
            ninja = (build / "build.ninja").read_text()
            self.assertNotIn("board/arduino/uno_r4_minima/bsp/startup.S", ninja)

    def test_reject_reusing_build_directory_for_another_board(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw selection ") as directory:
            arguments = ("cmake", "-S", ROOT / "examples/apps/blink", "-B", directory,
                         "-G", "Ninja", f"-DMCUHW_SOURCE_DIR={ROOT}",
                         f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/McuhwToolchain.cmake")
            run(*arguments, "-DMCUHW_BOARD=arduino/uno_r4_minima")
            output = run(*arguments, "-DMCUHW_BOARD=st/nucleo_h753zi",
                         "-DMCUHW_BOARD_REVISION=mb1364-rev-e", success=False)
            self.assertIn("separate build directory", " ".join(output.split()))

    def test_manifest_change_reconfigures_selected_board(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw manifest rebuild ") as directory:
            base = Path(directory)
            resources = base / "resources"
            board = resources / "board/arduino/uno_r4_minima"
            board.mkdir(parents=True)
            (resources / "cmake").mkdir()
            for filename in ("component.json", "board.cmake"):
                shutil.copy2(ROOT / "board/arduino/uno_r4_minima" / filename, board / filename)
            shutil.copy2(ROOT / "cmake/toolchain-arm-cortex-m4.cmake", resources / "cmake")
            shutil.copytree(ROOT / "cmake/architectures", resources / "cmake/architectures")
            shutil.copytree(ROOT / "include", resources / "include")
            source = base / "application"
            source.mkdir()
            (source / "main.cpp").write_text('int main() { for (;;) __asm__("wfi"); }\n')
            (source / "CMakeLists.txt").write_text('''
cmake_minimum_required(VERSION 3.25)
project(manifest_rebuild LANGUAGES CXX)
add_subdirectory("${MCUHW_SOURCE_DIR}" mcuhw)
mcuhw_add_firmware(firmware SOURCES main.cpp STARTUP external)
target_link_options(firmware PRIVATE -nostdlib -Wl,--entry=main)
''')
            build = base / "build"
            run("cmake", "-S", source, "-B", build, "-G", "Ninja",
                f"-DMCUHW_SOURCE_DIR={ROOT}", f"-DMCUHW_ROOT={resources}",
                "-DMCUHW_BOARD=arduino/uno_r4_minima",
                f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/McuhwToolchain.cmake")
            run("cmake", "--build", build)
            (board / "changed.cmake").write_text(
                'message(FATAL_ERROR "Selected manifest change was reconfigured")\n')
            manifest = board / "component.json"
            data = json.loads(manifest.read_text())
            data["build"]["entry"] = "board/arduino/uno_r4_minima/changed.cmake"
            manifest.write_text(json.dumps(data))
            output = run("cmake", "--build", build, success=False)
            self.assertIn("Selected manifest change was reconfigured", output)


@unittest.skipUnless(os.environ.get("MCUHW_TEST_ESP_IDF"),
                     "Set MCUHW_TEST_ESP_IDF to the pinned ESP-IDF SDK for SDK cross-builds")
class EspApplicationTests(unittest.TestCase):
    requirements = ('REQ-0049',)
    requirements = ("REQ-0049", "REQ-0052")
    def test_external_blink_and_tmp102(self):
        for example in ("blink", "tmp102"):
            with self.subTest(example=example):
                with tempfile.TemporaryDirectory(prefix="mcuhw esp app ") as directory:
                    base = Path(directory)
                    source, build = base / "source", base / "build"
                    shutil.copytree(ROOT / "examples/apps" / example, source)
                    command = ["cmake", "-S", source, "-B", build, "-G", "Ninja",
                               f"-DMCUHW_SOURCE_DIR={ROOT}",
                               "-DMCUHW_BOARD=waveshare/esp32p4_nano",
                               f"-DCMAKE_TOOLCHAIN_FILE={ROOT}/cmake/McuhwToolchain.cmake",
                               f"-DMCUHW_IDF_PATH={os.environ['MCUHW_TEST_ESP_IDF']}"]
                    if os.environ.get("MCUHW_TEST_IDF_PYTHON"):
                        command.append(f"-DPYTHON={os.environ['MCUHW_TEST_IDF_PYTHON']}")
                    run(*command)
                    run("cmake", "--build", build)
                    self.assertTrue((build / "firmware.elf").is_file())
                    self.assertTrue((build / "firmware.bin").is_file())
                    self.assertTrue((build / "esp-idf-build/flasher_args.json").is_file())
                    report = json.loads((build / "firmware.identity.json").read_text())
                    self.assertTrue(report["toolchains"].get("CXX"))
                    boot_report = json.loads((build / "firmware.bootloader.identity.json").read_text())
                    self.assertTrue(boot_report["toolchains"].get("CXX"))
                    self.assertTrue(any(entry["path"].endswith("bootloader_start.cpp")
                                        for entry in boot_report["files"]))
                    flash_report = json.loads((build / "firmware.flash.identity.json").read_text())
                    self.assertEqual(flash_report["scope"], "flash-bundle")
                    self.assertEqual(flash_report["image_count"], 3)
                    ninja = (build / "esp-idf-build/build.ninja").read_text()
                    self.assertNotIn("mcutest", ninja)
                    self.assertNotIn("mcucov", ninja)
                    self.assertNotIn("board/uno_r4_minima", ninja)


if __name__ == "__main__":
    unittest.main()
