"""Exercise configurable board workspaces with real CMake targets and firmware.

@req REQ-0047 REQ-0048 REQ-0049 REQ-0052
"""
import json
from pathlib import Path
import shutil
import tempfile
import unittest

from test_targets import ROOT, query, run, targets


@unittest.skipUnless(shutil.which("arm-none-eabi-g++"), "Arm GNU compiler unavailable")
class ActiveBoardWorkspace(unittest.TestCase):
    requirements = ("REQ-0047", "REQ-0048", "REQ-0049", "REQ-0052")

    def configure(self, build, selection=None, root=ROOT, success=True, *extra):
        args = ["cmake", "--preset", "arm-cortex-m4", "-B", build,
                "-DMCUHW_WORKSPACE=ON", "-DMCUHW_BOARD="]
        if selection is not None:
            args.append(f"-DMCUHW_ACTIVE_BOARDS={selection}")
        return run(*args, *extra, cwd=root, success=success)

    def test_selection_persists_and_removing_board_removes_its_targets(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw active boards ") as directory:
            build = Path(directory)
            query(build)
            self.configure(build, "arduino/uno_r4_minima@r4")
            model = targets(build)
            self.assertEqual(set(model), {"examples", "arduino.uno_r4_minima.examples",
                "arduino.uno_r4_minima.tests"} |
                {f"arduino.uno_r4_minima.examples.{name}" for name in
                 ("blink", "serial_echo", "spi_dma", "tmp102", "w5100")})
            self.configure(build)  # Applying the shared preset must not reset the list.
            self.assertEqual(set(targets(build)), set(model))
            self.configure(build, "")
            self.assertEqual(set(targets(build)), {"examples"})
            self.assertNotIn("MCUHW_CONFIGURED_BOARD:", (build / "CMakeCache.txt").read_text())

    def test_invalid_board_revision_architecture_and_duplicate_are_rejected(self):
        for selection, message in [
            ("missing/board", "Unknown mcuhw board"),
            ("../uno_r4_minima", "Invalid active board"),
            ("arduino/uno_r4_minima@unknown", "Unsupported revision"),
            ("st/nucleo_h753zi", "requires architecture 'cortex-m7'"),
            ("arduino/uno_r4_minima;arduino/uno_r4_minima@r4", "listed more than once"),
        ]:
            with self.subTest(selection=selection), tempfile.TemporaryDirectory() as directory:
                self.assertIn(message, self.configure(Path(directory), selection, success=False))

    def test_individual_and_aggregate_builds_are_isolated_from_test_dependencies(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw workspace examples ") as directory:
            build = Path(directory)
            self.configure(build, "arduino/uno_r4_minima", ROOT, True,
                           "-DMCUHW_MCUTEST_SOURCE_DIR=/missing-mcutest",
                           "-DMCUHW_MCUIP_SOURCE_DIR=/missing-mcuip")
            board = build / "board/arduino/uno_r4_minima/r4"
            prefix = "arduino.uno_r4_minima.examples"
            run("cmake", "--build", build, "--target", prefix + ".blink")
            self.assertTrue((board / f"example/blink/{prefix}.blink.elf").is_file())
            self.assertFalse((board / f"example/tmp102/{prefix}.tmp102.elf").exists())
            run("cmake", "--build", build, "--target", prefix)
            self.assertTrue((board / f"example/tmp102/{prefix}.tmp102.elf").is_file())
            for example in ("blink", "tmp102"):
                report = json.loads((board / f"example/{example}/{prefix}.{example}.identity.json").read_text())
                self.assertIn(f"source:board/arduino/uno_r4_minima/example/{example}.cpp", {p["path"] for p in report["files"]})
            self.assertFalse((board / "test/CMakeCache.txt").exists())
            output = run("cmake", "--build", build, "--target", "arduino.uno_r4_minima.tests", success=False)
            self.assertIn("mcutest headers not found", output)

    def test_user_preset_selection_and_all_compatible_boards(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw workspace presets ") as directory:
            root = Path(directory) / "source"
            self.copy_workspace(root)
            (root / "CMakeUserPresets.json").write_text(json.dumps({
                "version": 6,
                "configurePresets": [{"name": "my-m4", "inherits": "arm-cortex-m4",
                    "binaryDir": "${sourceDir}/build/my-m4",
                    "cacheVariables": {"MCUHW_WORKSPACE": "ON", "MCUHW_BOARD": "",
                                       "MCUHW_ACTIVE_BOARDS": "arduino/uno_r4_minima"}}],
                "buildPresets": [{"name": "my-m4", "inherits": "arm-cortex-m4", "configurePreset": "my-m4"}],
            }))
            build = root / "build/my-m4"
            query(build)
            run("cmake", "--preset", "my-m4", cwd=root)
            run("cmake", "--build", "--preset", "my-m4", "--target", "examples", cwd=root)
            model = set(targets(build))
            self.configure(build, "all", root)
            self.assertEqual(set(targets(build)), model)
            self.assertFalse(any("nucleo" in t or "esp32p4" in t for t in model))

    @staticmethod
    def copy_workspace(root):
        root.mkdir(parents=True)
        for name in ("include", "cmake", "board", "tool"):
            shutil.copytree(ROOT / name, root / name,
                            ignore=shutil.ignore_patterns("build", "__pycache__"))
        for name in ("CMakeLists.txt", "CMakePresets.json"):
            shutil.copy2(ROOT / name, root / name)

    def test_two_active_boards_and_hundred_unselected_boards_do_not_leak(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw workspace growth ") as directory:
            root, build = Path(directory) / "source", Path(directory) / "build"
            self.copy_workspace(root)
            path = root / "board/fixture/other/board.cmake"
            path.parent.mkdir(parents=True)
            path.write_text('include("${MCUHW_ROOT}/board/arduino/uno_r4_minima/board.cmake")\n'
                            'set(MCUHW_BOARD_EXAMPLES "")\n'
                            'set(MCUHW_BOARD_IO_EXAMPLES "")\n')
            query(build)
            selected = "arduino/uno_r4_minima;fixture/other"
            self.configure(build, selected, root)
            before = set(targets(build))
            self.assertFalse(any(name.startswith("fixture.other.examples") for name in before))
            run("cmake", "--build", build, "--target", "examples")
            output = build / "board/arduino/uno_r4_minima/r4/example/blink/arduino.uno_r4_minima.examples.blink.identity.json"
            report = json.loads(output.read_text())
            entries = {p["path"] for p in report["files"] if p["path"].endswith("board.cmake")}
            self.assertEqual(entries, {"source:board/arduino/uno_r4_minima/board.cmake"})
            commands = (build / "compile_commands.json").read_text()
            for index in range(100):
                path = root / f"board/unselected/board_{index}/board.cmake"
                path.parent.mkdir(parents=True)
                path.write_text("not even parsed by a focused workspace")
            self.configure(build, selected, root)
            self.assertEqual(set(targets(build)), before)
            self.assertEqual((build / "compile_commands.json").read_text(), commands)
            self.configure(build, "fixture/other", root)
            self.assertEqual(set(targets(build)), {"examples"})

    def test_single_board_cache_switch_and_preset_restores_uno(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw board cache switch ") as directory:
            root, build = Path(directory) / "source", Path(directory) / "build"
            self.copy_workspace(root)
            entry = root / "board/fixture/alternate/board.cmake"
            entry.parent.mkdir(parents=True)
            entry.write_text('include("${MCUHW_ROOT}/board/arduino/uno_r4_minima/board.cmake")\n'
                             'set(MCUHW_BOARD_EXAMPLES "")\n'
                             'set(MCUHW_BOARD_IO_EXAMPLES "")\n')
            query(build)
            run("cmake", "--preset", "arm-cortex-m4", "-B", build,
                "-DMCUHW_BOARD=arduino/uno_r4_minima", cwd=root)
            self.assertIn("example.blink", targets(build))
            self.assertIn("example.tmp102", targets(build))
            run("cmake", "--preset", "arm-cortex-m4", "-B", build,
                "-DMCUHW_BOARD=fixture/alternate", cwd=root)
            model = targets(build)
            self.assertIn("test.backend", model)
            self.assertNotIn("examples", model)
            self.assertNotIn("firmware", model)
            self.assertFalse(any(name.startswith("example.") for name in model))
            run("cmake", "--preset", "arm-cortex-m4", "-B", build, cwd=root)
            self.assertIn("example.blink", targets(build))
            self.assertIn("example.tmp102", targets(build))
