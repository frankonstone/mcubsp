"""Exercise public consumption in clean, relocated and parent-owned builds."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class PackagingTests(unittest.TestCase):
    requirements = ("REQ-0047",)

    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix="mcuhw packaging ")
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name)

    def run_command(self, *args):
        result = subprocess.run(list(map(str, args)), capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        return result.stdout

    def consumer(self, source, setup):
        source.mkdir()
        (source / "main.cpp").write_text(
            "#include <mcuhw/device/sensor/tmp102.hpp>\n"
            "#include <mcuhw/core/register.hpp>\n"
            "int main() { return 0; }\n")
        (source / "CMakeLists.txt").write_text(
            "cmake_minimum_required(VERSION 3.25)\n"
            "project(consumer LANGUAGES CXX)\n"
            "include(CTest)\n" + setup + "\n"
            "if(NOT BUILD_TESTING)\nmessage(FATAL_ERROR \"Parent tests disabled\")\nendif()\n"
            "add_executable(app main.cpp)\n"
            "target_link_libraries(app PRIVATE mcuhw::device mcuhw::io)\n"
            "add_test(NAME parent.test COMMAND app)\n")

    def build_consumer(self, source, *options):
        build = source / "build"
        self.run_command("cmake", "-S", source, "-B", build, "-G", "Ninja",
                         "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", *options)
        self.run_command("cmake", "--build", build)
        listing = json.loads(self.run_command("ctest", "--test-dir", build, "--show-only=json-v1"))
        self.assertEqual([test["name"] for test in listing["tests"]], ["parent.test"])
        commands = json.loads((build / "compile_commands.json").read_text())
        self.assertEqual(len(commands), 1, commands)
        self.assertNotIn("-Werror", commands[0]["command"])
        self.run_command("ctest", "--test-dir", build, "--output-on-failure")

    def test_source_embedding_preserves_parent_and_has_no_probes(self):
        source = self.work / "consumer"
        self.consumer(source, f'add_subdirectory("{ROOT.as_posix()}" mcuhw)')
        self.build_consumer(source,
                            "-DMCUHW_MCUTEST_SOURCE_DIR=/missing",
                            "-DMCUHW_MCUIP_SOURCE_DIR=/missing")

    def test_installed_package_is_relocatable_and_self_contained(self):
        # Copy the shipping inputs, then remove them after installation. This
        # proves the consumer cannot quietly fall back to the original source.
        checkout = self.work / "checkout"
        checkout.mkdir()
        for name in ("include", "cmake", "board", "device", "mcu", "ip", "catalog", "examples", "tools", "doc", "tests"):
            if (ROOT / name).exists():
                shutil.copytree(ROOT / name, checkout / name,
                                ignore=shutil.ignore_patterns("build", "__pycache__", "_vendor", "_generated"))
        for name in ("CMakeLists.txt", "README.md", "requirements.md", "implementation-plan.md"):
            shutil.copy2(ROOT / name, checkout / name)
        sdk_project = checkout / "board/waveshare/esp32p4_nano/tests/firmware"
        (sdk_project / "sdkconfig").write_text("# Local generated configuration\n")
        (sdk_project / "sdkconfig.old").write_text("# Previous generated configuration\n")
        build = self.work / "package build"
        prefix = self.work / "prefix"
        self.run_command("cmake", "-S", checkout, "-B", build, "-G", "Ninja",
                         f"-DCMAKE_INSTALL_PREFIX={prefix}")
        self.run_command("cmake", "--build", build)
        self.run_command("cmake", "--install", build)
        relocated = self.work / "relocated prefix"
        prefix.rename(relocated)
        shutil.rmtree(checkout)
        shutil.rmtree(build)
        installed_sdk_project = relocated / "share/mcuhw/board/waveshare/esp32p4_nano/tests/firmware"
        self.assertFalse((installed_sdk_project / "sdkconfig").exists())
        self.assertFalse((installed_sdk_project / "sdkconfig.old").exists())
        self.assertTrue((installed_sdk_project / "sdkconfig.defaults").is_file())
        for exported in (relocated / "lib/cmake/mcuhw").glob("*.cmake"):
            content = exported.read_text()
            self.assertNotIn(str(ROOT), content)
            self.assertNotIn(str(checkout), content)
            self.assertNotIn(str(prefix), content)
        source = self.work / "installed consumer"
        self.consumer(source, "find_package(mcuhw 0.1 CONFIG REQUIRED)")
        self.build_consumer(source, f"-DCMAKE_PREFIX_PATH={relocated}")

    def test_direct_headers_need_no_cmake_or_board(self):
        compiler = os.environ.get("CXX") or shutil.which("c++") or shutil.which("clang++")
        self.assertIsNotNone(compiler, "Set CXX to a C++23 compiler")
        source = self.work / "header.cpp"
        source.write_text("#include <mcuhw/device/sensor/tmp102.hpp>\nint main() { return 0; }\n")
        self.run_command(compiler, "-std=c++23", "-I", ROOT / "include", "-fsyntax-only", source)


if __name__ == "__main__":
    unittest.main()
