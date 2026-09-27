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
        args = ["cmake", "--preset", "arm-cortex-m4", "-B", build, "-DMCUHW_WORKSPACE=ON"]
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
                "arduino.uno_r4_minima.examples.blink", "arduino.uno_r4_minima.examples.tmp102",
                "arduino.uno_r4_minima.tests"})
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
            board = build / "boards/arduino/uno_r4_minima/r4"
            prefix = "arduino.uno_r4_minima.examples"
            run("cmake", "--build", build, "--target", prefix + ".blink")
            self.assertTrue((board / f"examples/blink/{prefix}.blink.elf").is_file())
            self.assertFalse((board / f"examples/tmp102/{prefix}.tmp102.elf").exists())
            run("cmake", "--build", build, "--target", prefix)
            self.assertTrue((board / f"examples/tmp102/{prefix}.tmp102.elf").is_file())
            for example in ("blink", "tmp102"):
                report = json.loads((board / f"examples/{example}/{prefix}.{example}.identity.json").read_text())
                self.assertIn(f"source:examples/apps/{example}/main.cpp", {p["path"] for p in report["files"]})
            self.assertFalse((board / "tests/CMakeCache.txt").exists())
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
                    "cacheVariables": {"MCUHW_WORKSPACE": "ON", "MCUHW_ACTIVE_BOARDS": "arduino/uno_r4_minima"}}],
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
        for name in ("include", "cmake", "board", "examples", "tools"):
            shutil.copytree(ROOT / name, root / name,
                            ignore=shutil.ignore_patterns("build", "__pycache__"))
        for name in ("CMakeLists.txt", "CMakePresets.json"):
            shutil.copy2(ROOT / name, root / name)

    def test_two_active_boards_and_hundred_unselected_boards_do_not_leak(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw workspace growth ") as directory:
            root, build = Path(directory) / "source", Path(directory) / "build"
            self.copy_workspace(root)
            original = json.loads((root / "board/arduino/uno_r4_minima/component.json").read_text())
            # A synthetic second board reuses an existing platform, has its own
            # identity and only one example. It makes no hardware support claim.
            second = {**original, "id": "fixture/other", "examples": ["examples/apps/blink/CMakeLists.txt"], "evidence": []}
            path = root / "board/fixture/other/component.json"
            path.parent.mkdir(parents=True)
            path.write_text(json.dumps(second))
            query(build)
            selected = "arduino/uno_r4_minima;fixture/other"
            self.configure(build, selected, root)
            before = set(targets(build))
            self.assertIn("fixture.other.examples.blink", before)
            self.assertNotIn("fixture.other.examples.tmp102", before)
            run("cmake", "--build", build, "--target", "examples")
            for board, revision, prefix in [("arduino/uno_r4_minima", "r4", "arduino.uno_r4_minima"),
                                            ("fixture/other", "r4", "fixture.other")]:
                output = build / f"boards/{board}/{revision}/examples/blink/{prefix}.examples.blink.identity.json"
                report = json.loads(output.read_text())
                manifests = {p["path"] for p in report["files"] if p["path"].endswith("component.json")}
                self.assertEqual(manifests, {f"source:board/{board}/component.json"})
            commands = (build / "compile_commands.json").read_text()
            for index in range(100):
                path = root / f"board/unselected/board_{index}/component.json"
                path.parent.mkdir(parents=True)
                path.write_text("not even parsed by a focused workspace")
            self.configure(build, selected, root)
            self.assertEqual(set(targets(build)), before)
            self.assertEqual((build / "compile_commands.json").read_text(), commands)
            self.configure(build, "fixture/other", root)
            self.assertEqual(set(targets(build)), {"examples", "fixture.other.examples", "fixture.other.examples.blink"})

    def test_single_board_cache_switch_replaces_examples_without_a_board_preset(self):
        with tempfile.TemporaryDirectory(prefix="mcuhw board cache switch ") as directory:
            root, build = Path(directory) / "source", Path(directory) / "build"
            self.copy_workspace(root)
            manifest = json.loads((root / "board/arduino/uno_r4_minima/component.json").read_text())
            manifest.update(id="fixture/alternate", examples=["examples/apps/blink/CMakeLists.txt"], evidence=[])
            manifest["build"]["entry"] = "board/fixture/alternate/board.cmake"
            entry = root / manifest["build"]["entry"]
            entry.parent.mkdir(parents=True)
            entry.write_text('include("${MCUHW_ROOT}/board/arduino/uno_r4_minima/board.cmake")\n'
                             'set(MCUHW_BOARD_IO_EXAMPLES "")\n')
            (entry.parent / "component.json").write_text(json.dumps(manifest))
            query(build)
            run("cmake", "--preset", "arm-cortex-m4", "-B", build,
                "-DMCUHW_BOARD=arduino/uno_r4_minima", cwd=root)
            self.assertIn("example.io.blink", targets(build))
            self.assertIn("example.app.tmp102", targets(build))
            run("cmake", "--preset", "arm-cortex-m4", "-B", build,
                "-DMCUHW_BOARD=fixture/alternate", cwd=root)
            model = targets(build)
            self.assertIn("example.app.blink", model)
            self.assertIn("test.backend", model)
            self.assertNotIn("example.app.tmp102", model)
            self.assertFalse(any(name.startswith("example.io.") for name in model))
            run("cmake", "--build", build, "--target", "example.app.blink", cwd=root)
            report = json.loads((build / "application/firmware.identity.json").read_text())
            manifests = {f["path"] for f in report["files"] if f["path"].endswith("component.json")}
            self.assertEqual(manifests, {"source:board/fixture/alternate/component.json"})
            run("cmake", "--preset", "arm-cortex-m4", "-B", build, cwd=root)
            self.assertEqual(set(targets(build)), set(model))
