"""Build real selected targets before inspecting compiler dependency evidence."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from mcuhwlib.provenance import flash_bundle, identity


def run(*args):
    result = subprocess.run(list(map(str, args)), text=True, capture_output=True)
    if result.returncode:
        raise AssertionError(result.stdout + result.stderr)
    return result.stdout


@unittest.skipUnless(shutil.which("cmake") and shutil.which("ninja"),
                     "CMake and Ninja are required")
class ProvenanceTests(unittest.TestCase):
    requirements = ("REQ-0049", "REQ-0052")

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory(prefix="mcuhw provenance ")
        self.addCleanup(self.temporary.cleanup)
        self.source = Path(self.temporary.name) / "source"
        self.source.mkdir()
        self.build = Path(self.temporary.name) / "build"
        (self.source / "main.cpp").write_text(
            '#include "selected.hpp"\n#include "configuration.hpp"\n'
            'int main() { return selected_value == configured_value ? 0 : 1; }\n')
        (self.source / "selected.hpp").write_text(
            '#include "nested.hpp"\nconstexpr int selected_value = nested_value;\n')
        (self.source / "nested.hpp").write_text('constexpr int nested_value = 7;\n')
        (self.source / "configuration.hpp.in").write_text(
            'constexpr int configured_value = @SELECTED_VALUE@;\n')
        (self.source / "unrelated.cpp").write_text('int unrelated() { return 17; }\n')
        (self.source / "unrelated.hpp").write_text('#error not part of selected target\n')
        (self.source / "linker-input.txt").write_text('link configuration version 1\n')
        (self.source / "settings.txt").write_text(json.dumps({"revision": "r1"}))
        (self.source / "CMakeLists.txt").write_text('''
cmake_minimum_required(VERSION 3.25)
project(selected_provenance LANGUAGES CXX)
set(SELECTED_VALUE 7 CACHE STRING "Application configuration")
configure_file(configuration.hpp.in configuration.hpp @ONLY)
add_executable(selected main.cpp)
set_target_properties(selected PROPERTIES SUFFIX ".elf")
target_include_directories(selected PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
set_property(TARGET selected APPEND PROPERTY LINK_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/linker-input.txt")
add_library(unrelated STATIC EXCLUDE_FROM_ALL unrelated.cpp)
''')

    def configure_and_build(self, *arguments):
        run("cmake", "-S", self.source, "-B", self.build, "-G", "Ninja", *arguments)
        run("cmake", "--build", self.build, "--target", "selected")

    def report(self):
        return identity(self.source, self.build, "selected.elf",
                        extra=(self.source / "settings.txt",))

    def test_selected_transitive_headers_are_recorded(self):
        self.configure_and_build()
        result = self.report()
        paths = {entry["path"] for entry in result["files"]}
        self.assertIn("source:main.cpp", paths)
        self.assertIn("source:selected.hpp", paths)
        self.assertIn("source:nested.hpp", paths)
        self.assertIn("build:configuration.hpp", paths)
        self.assertIn("source:linker-input.txt", paths)
        self.assertNotIn("source:unrelated.cpp", paths)
        self.assertNotIn("source:unrelated.hpp", paths)
        self.assertEqual(result["object_count"], 1)
        self.assertEqual(result["toolchains"]["CXX"]["target"],
                         run(os.environ.get("CXX") or shutil.which("c++") or "c++",
                             "-dumpmachine").strip())

    def test_unrelated_device_and_source_additions_keep_identity(self):
        self.configure_and_build()
        before = self.report()
        directory = self.source / "device/vendor/new_sensor"
        directory.mkdir(parents=True)
        (directory / "settings.txt").write_text('{"kind":"device"}\n')
        (directory / "driver.hpp").write_text('#error unrelated new device\n')
        (self.source / "unrelated.cpp").write_text('int unrelated() { return 1000; }\n')
        self.configure_and_build()
        after = self.report()
        self.assertEqual(before["identity"], after["identity"])
        self.assertEqual(before["source_count"], after["source_count"])
        self.assertEqual(before["object_count"], after["object_count"])

    def test_used_header_invalidates_identity(self):
        self.configure_and_build()
        before = self.report()["identity"]
        (self.source / "nested.hpp").write_text('constexpr int nested_value = 8;\n')
        run("cmake", "--build", self.build, "--target", "selected")
        self.assertNotEqual(before, self.report()["identity"])

    def test_prebuilt_object_bytes_invalidate_identity_without_dependency_records(self):
        compiler = os.environ.get("CXX") or shutil.which("c++")
        if not compiler:
            self.skipTest("Native C++ compiler unavailable")
        library_source = self.source / "prebuilt.cpp"
        library_object = self.source / "prebuilt.o"
        project = self.source / "CMakeLists.txt"
        project.write_text(project.read_text() + '''
target_sources(selected PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/prebuilt.o")
''')
        main = self.source / "main.cpp"
        main.write_text('int prebuilt_value();\nint main() { return prebuilt_value(); }\n')

        def compile_prebuilt(value):
            library_source.write_text(f'int prebuilt_value() {{ return {value}; }}\n')
            run(compiler, "-c", library_source, "-o", library_object)

        compile_prebuilt(1)
        self.configure_and_build()
        before = self.report()
        records = {entry["path"]: entry["sha256"] for entry in before["files"]}
        self.assertIn("source:prebuilt.o", records)
        self.assertNotIn("source:prebuilt.cpp", records)  # No invented source provenance.
        compile_prebuilt(2)
        run("cmake", "--build", self.build, "--target", "selected")
        after = self.report()
        self.assertEqual(before["commands"], after["commands"])
        self.assertNotEqual(before["identity"], after["identity"])
        self.assertNotEqual(records["source:prebuilt.o"],
                            next(entry["sha256"] for entry in after["files"]
                                 if entry["path"] == "source:prebuilt.o"))

    @unittest.skipUnless(shutil.which("git"), "Git is required for revision fixture")
    def test_repository_revision_is_separate_from_selected_identity(self):
        self.configure_and_build()
        git = ("git", "-C", self.source, "-c", "user.name=mcuhw test",
               "-c", "user.email=mcuhw-test@example.invalid")
        run(*git, "init", "--quiet")
        run(*git, "add", ".")
        run(*git, "-c", "commit.gpgsign=false", "commit", "--quiet", "-m", "fixture")
        before = self.report()
        (self.source / "unrelated.hpp").write_text('// Unrelated documentation change\n')
        run(*git, "add", "unrelated.hpp")
        run(*git, "-c", "commit.gpgsign=false", "commit", "--quiet", "-m", "unrelated change")
        after = self.report()
        self.assertNotEqual(before["repository_revision"], after["repository_revision"])
        self.assertEqual(before["identity"], after["identity"])

    def test_selected_link_configuration_and_manifest_invalidate_identity(self):
        self.configure_and_build()
        before = self.report()["identity"]
        (self.source / "linker-input.txt").write_text('link configuration version 2\n')
        run("cmake", "--build", self.build, "--target", "selected")
        linked = self.report()["identity"]
        self.assertNotEqual(before, linked)
        (self.source / "settings.txt").write_text(json.dumps({"revision": "r2"}))
        self.assertNotEqual(linked, self.report()["identity"])

    def test_application_configuration_invalidates_identity(self):
        self.configure_and_build()
        before = self.report()["identity"]
        self.configure_and_build("-DSELECTED_VALUE=9")
        self.assertNotEqual(before, self.report()["identity"])

    @unittest.skipIf(os.name == "nt", "Executable compiler wrapper fixture uses POSIX shebang")
    def test_toolchain_version_change_at_same_path_invalidates_identity(self):
        compiler = shutil.which("c++")
        if not compiler:
            self.skipTest("Native C++ compiler unavailable")
        version_file = self.source / "toolchain-version.txt"
        version_file.write_text("fixture-1")
        wrapper = self.source / "compiler"
        wrapper.write_text(f'''#!{sys.executable}
import os
from pathlib import Path
import subprocess
import sys
compiler = {compiler!r}
if sys.argv[1:] == ["--version"]:
    subprocess.run([compiler, "--version"], check=True)
    print("Compiler identity fixture: " + Path({str(version_file)!r}).read_text())
else:
    os.execv(compiler, [compiler, *sys.argv[1:]])
''')
        wrapper.chmod(0o755)
        self.configure_and_build(f"-DCMAKE_CXX_COMPILER={wrapper}")
        before = self.report()
        version_file.write_text("fixture-2")
        after = self.report()
        self.assertEqual(before["files"], after["files"])
        self.assertEqual(before["commands"], after["commands"])
        self.assertNotEqual(before["toolchains"], after["toolchains"])
        self.assertNotEqual(before["identity"], after["identity"])

    def test_unbuilt_target_has_no_claimed_identity(self):
        run("cmake", "-S", self.source, "-B", self.build, "-G", "Ninja")
        with self.assertRaisesRegex(ValueError, "No compiled dependency graph"):
            self.report()


class FlashBundleTests(unittest.TestCase):
    requirements = ("REQ-0049", "REQ-0052")

    def test_flash_identity_covers_bootloader_partition_application_and_offsets(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw flash identity ") as directory:
            build = Path(directory)
            files = {"0x2000": "bootloader.bin", "0x8000": "partition-table.bin",
                     "0x10000": "firmware.bin"}
            for filename in files.values():
                (build / filename).write_bytes(b"synthetic flash fixture")
            manifest = build / "flasher_args.json"
            settings = {"flash_files": files, "flash_settings": {"flash_mode": "dio"}}
            manifest.write_text(json.dumps(settings))
            previous = flash_bundle(build, manifest)
            self.assertEqual(previous["image_count"], 3)
            for filename in files.values():
                (build / filename).write_bytes(b"updated synthetic flash fixture")
                current = flash_bundle(build, manifest)
                self.assertNotEqual(previous["identity"], current["identity"])
                previous = current
            (build / "unrelated.bin").write_bytes(b"not flashed")
            self.assertEqual(previous["identity"], flash_bundle(build, manifest)["identity"])
            settings["flash_files"]["0x20000"] = settings["flash_files"].pop("0x10000")
            manifest.write_text(json.dumps(settings))
            self.assertNotEqual(previous["identity"], flash_bundle(build, manifest)["identity"])

    def test_flash_identity_rejects_images_outside_selected_build(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw flash identity ") as directory:
            build = Path(directory) / "build"
            build.mkdir()
            (build.parent / "outside.bin").write_bytes(b"outside fixture")
            manifest = build / "flasher_args.json"
            manifest.write_text(json.dumps({"flash_files": {"0x2000": "../outside.bin"}}))
            with self.assertRaisesRegex(ValueError, "Invalid flash image"):
                flash_bundle(build, manifest)


if __name__ == "__main__":
    unittest.main()
