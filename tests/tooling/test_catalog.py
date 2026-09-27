"""Catalog rejection, local contribution and evidence-boundary checks.

@req REQ-0048 REQ-0051
"""
import copy
import hashlib
import json
from pathlib import Path
import shutil
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tools"))
from mcuhwlib import catalog


class CatalogFixture(unittest.TestCase):
    requirements = ('REQ-0048', 'REQ-0051')
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        (self.root / "catalog").mkdir()
        shutil.copy(ROOT / "catalog/schema.json", self.root / "catalog/schema.json")
        (self.root / "shared.hpp").write_text("i2c spi led variant_one\n")
        (self.root / "board.cmake").write_text("# local rules\n")
        (self.root / "toolchain.cmake").write_text("# shared toolchain\n")
        (self.root / "README.md").write_text("Fixture only, no real hardware support.\n")

    def component(self, kind="device", identifier="fixture/sensor", provides=None, requires=None):
        provides = provides or []
        return dict(schema_version=2, kind=kind, id=identifier, name="Fixture", summary="Synthetic catalog test only",
                    category="temperature" if kind == "device" else "development-board", revisions=["v1"], default_revision="v1",
                    depends=[], provides=provides, requires=requires or [], headers=["shared.hpp"], sources=[],
                    docs=["README.md"], tests=[], examples=[], ownership={"maintainers":["test fixture"]},
                    capability_sources={cap:{"path":"shared.hpp", "contains":cap} for cap in provides},
                    evidence=[], limitations=["Not hardware support."])

    def write(self, value, destination=None):
        path = self.root / (destination or f"{value['kind']}/{value['id']}/component.json")
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(value))
        return path

    def board(self, identifier="fixture/board", provides=None):
        value = self.component("board", identifier, provides or ["i2c", "led"])
        value["build"] = {"entry":"board.cmake", "toolchain":"toolchain.cmake", "profile_header":"shared.hpp", "profile_type":"fixture::board", "architecture":"fixture"}
        return value

    def assertInvalid(self, value, text):
        self.write(value)
        self.assertIn(text, "\n".join(catalog.validate(self.root)))

    def test_local_components_need_no_registry(self):
        board = self.board()
        chip = self.component(requires=["i2c"])
        self.write(board)
        self.write(chip)
        self.assertEqual([], catalog.validate(self.root))
        self.assertEqual(2, len(catalog.discover(self.root)))
        self.assertEqual([board, chip], catalog.compose(self.root, board["id"], [chip["id"]]))
        second = self.board("another/board")
        self.write(second)
        self.assertEqual(3, len(catalog.discover(self.root)))
        self.assertEqual([], catalog.validate(self.root))

    def test_direct_selection_does_not_enumerate(self):
        board = self.board()
        ip = self.component("ip", "fixture/native", ["i2c"])
        board["depends"] = ["ip:fixture/native"]
        self.write(board); self.write(ip)
        with patch.object(Path, "glob", side_effect=AssertionError("catalog scan during selection")), \
             patch.object(Path, "rglob", side_effect=AssertionError("recursive scan during selection")):
            self.assertEqual(board, catalog.load(self.root, "board", board["id"]))
            self.assertEqual([board, ip], catalog.closure(self.root, "board", board["id"]))

    def test_dependency_diamond_is_unique(self):
        for ident, depends in [("fixture/a",["ip:fixture/b", "ip:fixture/c"]),
                               ("fixture/b",["ip:fixture/d"]), ("fixture/c",["ip:fixture/d"]), ("fixture/d",[])]:
            value = self.component("ip",ident)
            value["depends"] = depends
            self.write(value)
        result = catalog.closure(self.root, "ip", "fixture/a")
        self.assertEqual(["fixture/a", "fixture/b", "fixture/d", "fixture/c"], [c["id"] for c in result])

    def test_unsupported_schema(self):
        value = self.component(); value["schema_version"] = 3
        self.assertInvalid(value, "schema_version: unsupported")

    def test_boolean_is_not_integer_schema_version(self):
        value = self.component(); value["schema_version"] = True
        self.assertInvalid(value, "schema_version: expected integer")

    def test_unknown_fields_rejected(self):
        value = self.component(); value["dependz"] = []
        self.assertInvalid(value, "unknown field dependz")

    def test_duplicate_ids_rejected(self):
        value = self.component()
        self.write(value)
        self.write(value, "device/duplicate/entry/component.json")
        self.assertIn("duplicate component ID device:fixture/sensor", "\n".join(catalog.validate(self.root)))

    def test_duplicate_json_keys_rejected(self):
        path = self.write(self.component())
        path.write_text(path.read_text().replace('"schema_version": 2', '"schema_version": 2, "schema_version": 2'))
        self.assertIn("duplicate JSON key", "\n".join(catalog.validate(self.root)))

    def test_missing_dependency_rejected(self):
        value = self.component(); value["depends"] = ["ip:fixture/missing"]
        self.assertInvalid(value, "unknown ip 'fixture/missing'")

    def test_cycle_rejected(self):
        a = self.component("ip", "fixture/a"); a["depends"] = ["ip:fixture/b"]
        b = self.component("ip", "fixture/b"); b["depends"] = ["ip:fixture/a"]
        self.write(b)
        self.assertInvalid(a, "dependency cycle")

    def test_mounted_device_capabilities_checked(self):
        board = self.board()
        board["depends"] = ["device:fixture/sensor"]
        self.write(self.component(requires=["spi"]))
        self.assertInvalid(board, "mounted device:fixture/sensor requires unavailable capabilities: spi")

    def test_duplicate_mcu_variants_rejected(self):
        value = self.component("mcu", "fixture/mcu")
        variant = dict(id="one", family="family", part="part", package="package", silicon="v1",
                       source={"path":"shared.hpp", "contains":"variant_one"})
        value["variants"] = [variant, dict(variant)]
        self.assertInvalid(value, "duplicate MCU variant IDs")

    def test_invalid_default_revision(self):
        value = self.component(); value["default_revision"] = "v2"
        self.assertInvalid(value, "default_revision is not listed")

    def test_unknown_revision_selection(self):
        self.write(self.component())
        with self.assertRaisesRegex(catalog.CatalogError, "unsupported revision.*choose v1"):
            catalog.load(self.root,"device","fixture/sensor","v2")

    def test_invalid_dependency_revision(self):
        self.write(self.component("ip","fixture/native"))
        value = self.component(); value["depends"] = ["ip:fixture/native@v2"]
        self.assertInvalid(value, "unsupported revision")

    def test_conflicting_dependency_revisions(self):
        a=self.component("ip","fixture/a"); a["depends"]=["ip:fixture/b", "ip:fixture/c"]
        b=self.component("ip","fixture/b"); b["depends"]=["ip:fixture/d@v1"]
        c=self.component("ip","fixture/c"); c["depends"]=["ip:fixture/d@v2"]
        d=self.component("ip","fixture/d"); d["revisions"].append("v2")
        for value in [a,b,c,d]: self.write(value)
        with self.assertRaisesRegex(catalog.CatalogError,"conflicting revisions"):
            catalog.closure(self.root,"ip","fixture/a")

    def test_unsupported_capability_selection(self):
        self.write(self.board())
        with self.assertRaisesRegex(catalog.CatalogError,"unsupported capabilities: ethernet"):
            catalog.closure(self.root,"board","fixture/board",capabilities=["ethernet"])

    def test_device_requires_board_exposed_capability(self):
        board=self.board(); board["depends"]=["ip:fixture/native"]
        self.write(board)
        self.write(self.component("ip","fixture/native",["spi"]))
        self.write(self.component(requires=["spi"]))
        with self.assertRaisesRegex(catalog.CatalogError,"requires spi, unavailable"):
            catalog.compose(self.root,"fixture/board",["fixture/sensor"])

    def test_capability_summary_needs_source_anchor(self):
        value=self.component(provides=["i2c"]); value["capability_sources"]={}
        self.assertInvalid(value,"must cover exactly")

    def test_changed_source_anchor_rejected(self):
        value=self.component(provides=["i2c"])
        (self.root/"shared.hpp").write_text("SPI only\n")
        self.assertInvalid(value,"source summary anchor absent")

    def test_missing_source_file_rejected(self):
        value=self.component(); value["headers"]=["missing.hpp"]
        self.assertInvalid(value,"missing referenced file")

    def test_path_traversal_rejected(self):
        value=self.component(); value["headers"]=["../shared.hpp"]
        self.assertInvalid(value,"invalid repository-relative path")
        with self.assertRaises(catalog.CatalogError):
            catalog.load(self.root,"board","../board")

    def test_symlink_escape_rejected(self):
        (self.root/"outside.hpp").symlink_to(ROOT/"include/mcuhw/io.hpp")
        value=self.component(); value["headers"]=["outside.hpp"]
        self.assertInvalid(value,"path escapes repository")

    def test_mcu_requires_explicit_variants(self):
        self.assertInvalid(self.component("mcu","fixture/mcu"),"MCU requires explicit variants")

    def test_board_requires_authoritative_build_identity(self):
        for field in ("entry", "toolchain", "profile_header", "profile_type", "architecture"):
            with self.subTest(field=field):
                value = self.board()
                del value["build"][field]
                self.assertInvalid(value, f"board requires build.{field}")

    def test_runner_requires_adapter_fields_and_safe_flash_regions(self):
        value = self.board()
        runner = dict(type="jlink", tool="jlink", device="fixture", interface="SWD", speed_khz=4000,
                      artifact="firmware.elf", recovery_doc="README.md",
                      flash_regions=[dict(start="0x4000", size="0x1000")])
        value["build"]["runner"] = runner
        self.write(value)
        self.assertEqual([], catalog.validate(self.root))
        del runner["device"]
        self.assertInvalid(value, "jlink runner requires device")
        runner["device"] = "fixture"
        runner["flash_regions"].append(dict(start="0x4800", size="0x1000"))
        self.assertInvalid(value, "nonoverlapping")
        runner["flash_regions"] = [dict(start="0x4000", size="0x1000")]
        runner["artifact"] = "../firmware.elf"
        self.assertInvalid(value, "invalid repository-relative path")

    def test_duplicate_test_ownership_is_rejected(self):
        first, second = self.component(), self.component(identifier="fixture/second")
        first["tests"] = second["tests"] = ["shared.hpp"]
        self.write(first)
        self.assertInvalid(second, "already owned by")

    def test_evidence_states_remain_separate(self):
        path=self.root/"result.json"; path.write_text('{"result":"PASS"}\n')
        evidence=dict(stage="physical",state="passed",revision="v1",scope="Exact fixture only",path="result.json",
                      sha256=hashlib.sha256(path.read_bytes()).hexdigest())
        self.assertEqual("passed",catalog.evidence_state(self.root,evidence))
        path.write_text('{"result":"FAIL"}\n')
        self.assertEqual("stale",catalog.evidence_state(self.root,evidence))
        self.assertEqual("failed",catalog.evidence_state(self.root,dict(evidence,state="failed")))
        path.unlink()
        self.assertEqual("missing",catalog.evidence_state(self.root,evidence))
        self.assertEqual("stale",catalog.evidence_state(self.root,dict(evidence,state="stale")))

    def test_search_evidence_filters_match_one_record_and_revision(self):
        value=self.component(requires=["i2c"]); value["revisions"].append("v2")
        value["evidence"]=[dict(stage="physical",state="failed",scope="v1 fixture",revision="v1",path="README.md"),
                           dict(stage="host-test",state="passed",scope="host only",revision="v2",path="README.md")]
        self.write(value)
        self.assertEqual([value],catalog.search(self.root,"temperature",capability="i2c"))
        self.assertEqual([],catalog.search(self.root,stage="physical",state="passed"))
        self.assertEqual([],catalog.search(self.root,stage="physical",revision="v2"))
        self.assertEqual([value],catalog.search(self.root,stage="physical",state="failed",revision="v1"))

    def test_generated_index_is_deterministic_and_scoped(self):
        value=self.component(); value["evidence"]=[dict(stage="physical",state="missing",scope="No fixture",revision="v1")]
        self.write(value)
        self.assertEqual(catalog.render_index(self.root),catalog.render_index(self.root))
        self.assertIn('"effective_state": "missing"',catalog.render_index(self.root))
        text=catalog.render_markdown(self.root)
        self.assertIn("physical **missing**",text)
        self.assertIn("No fixture",text)
        self.assertIn("do not imply a physically tested composition",text)

    def test_board_and_device_templates_are_local_only(self):
        for kind in ("board", "device"):
            value=json.loads((ROOT/f"catalog/templates/{kind}.json").read_text())
            value["id"]="fixture/"+kind
            value["headers"]=["shared.hpp"]
            value["docs"]=["README.md"]
            value["sources"]=[]; value["tests"]=[]; value["examples"]=[]
            value["provides"]=[]; value["capability_sources"]={}; value["requires"]=[]
            if kind=="board":
                value["depends"]=["ip:fixture/native"]
                value["build"]={"entry":"board.cmake","toolchain":"toolchain.cmake", "profile_header":"shared.hpp", "profile_type":"fixture::board", "architecture":"fixture"}
                self.write(self.component("ip","fixture/native"))
            self.write(value)
        self.assertEqual([],catalog.validate(self.root))
        self.assertEqual(3,len(catalog.discover(self.root)))


class RepositoryCatalog(unittest.TestCase):
    requirements = ('REQ-0048', 'REQ-0051')
    def test_real_catalog_validates(self):
        self.assertEqual([],catalog.validate(ROOT))

    def test_i2c_temperature_ethernet_and_flash_discovery(self):
        self.assertEqual(["ti/tmp102"],[c["id"] for c in catalog.search(ROOT,kind="device",category="temperature",capability="i2c")])
        self.assertEqual(["st/nucleo_h753zi","waveshare/esp32p4_nano"],
                         [c["id"] for c in catalog.search(ROOT,kind="board",capability="ethernet")])
        self.assertEqual(["arduino/uno_r4_minima"],[c["id"] for c in catalog.search(ROOT,kind="board",runner="jlink")])
        self.assertEqual([],catalog.search(ROOT,kind="device",category="temperature",stage="physical",state="passed"))


if __name__ == "__main__":
    unittest.main()
