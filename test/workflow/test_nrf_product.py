#!/usr/bin/env python3
"""Offline regressions for Nordic product-table extraction and source reconciliation."""
# @req-file REQ-0055
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('nrf_product', ROOT / 'tool/nrf52840-product.py')
product = importlib.util.module_from_spec(spec)
spec.loader.exec_module(product)
core_spec = importlib.util.spec_from_file_location('nrf_core', ROOT / 'tool/nrf52840-core.py')
arm_core = importlib.util.module_from_spec(core_spec)
core_spec.loader.exec_module(arm_core)


def words(labels):
    return [(text, 100 + (31 - bit) * 9, 104 + (31 - bit) * 9) for text, bit in labels]


def detail():
    # The printed table uses upper-case A..Z followed by lower-case a..f.
    ids = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef'
    rows = [('Bit number ' + ' '.join(str(x) for x in range(31, -1, -1)),
             words([(str(bit), bit) for bit in range(32)])),
            ('ID ' + ' '.join(reversed(ids)), words([(label, bit) for bit, label in enumerate(ids)])),
            ('Reset 0x00000000', [])]
    for bit, label in enumerate(ids):
        rows += [(f'{label}- RW TRIGGERED[{bit}]', []), ('W1C', [])]
    return {'page': 1, 'rows': rows, 'section': '6.1.1'}


class ProductImport(unittest.TestCase):
    def test_all_32_field_ids_array_names_and_split_access_annotations_survive(self):
        reset, fields = product.extract_fields(detail())
        self.assertEqual(reset, 0)
        self.assertEqual(len(fields), 32)
        by_position = {field['position']: field for field in fields}
        self.assertEqual(by_position[31]['name'], 'TRIGGERED[31]')
        self.assertEqual(by_position[26]['name'], 'TRIGGERED[26]')
        self.assertTrue(all(f['access'] == 'RW_W1C' for f in fields))

    def test_id_diagram_containing_r_is_not_a_field_definition(self):
        data = detail()
        # A previous parser interpreted 'ID R Q' as a field named Q with ID ID.
        data['rows'].insert(2, ('ID R Q P', words([('R', 17), ('Q', 16), ('P', 15)])))
        _, fields = product.extract_fields(data)
        self.assertEqual(len(fields), 32)
        self.assertFalse(any(field['name'] == 'Q' for field in fields))

    def test_truncated_or_disagreeing_tables_fail_closed(self):
        broken = detail()
        broken['rows'] = broken['rows'][:-2]
        with self.assertRaisesRegex(AssertionError, 'unnamed or unplaced'):
            product.extract_fields(broken)
        broken = detail()
        broken['rows'].append(('Reset 0x00000001', []))
        with self.assertRaisesRegex(AssertionError, 'inconsistent repeated reset'):
            product.extract_fields(broken)

    def test_cpp_identifiers_avoid_keywords_and_reserved_double_underscores(self):
        self.assertEqual(product.ident('BREAK'), 'reg_break')
        self.assertEqual(product.ident('PERREGION[0].SUBSTATWA'), 'perregion_0_substatwa')
        self.assertEqual(product.ident('32MHz'), 'reg_32mhz')

    def test_semantics_preserve_commands_clear_aliases_and_read_side_effects(self):
        def register(name, access='RW', **kw):
            return dict(name=name, fields=[{'access': access}], **kw)
        self.assertEqual(product.register_semantics('EGU0', register('TASKS_TRIGGER[0]', 'W')), 'command')
        self.assertEqual(product.register_semantics('PPI', register('CHENCLR')), 'clear_alias')
        self.assertEqual(product.register_semantics('MWU', register('STATUS', 'RW_W1C')), 'write_one_to_clear')
        self.assertEqual(product.register_semantics('UART0', register('RXD', 'R', source_read_action='modifyExternal')), 'read_clear')
        self.assertEqual(product.register_semantics('CC_RNG_SRAM', register('SRAM_DATA')), 'read_write_side_effect')
        self.assertEqual(product.register_semantics('CC_DIN', register('SRC_MEM_SIZE','W')), 'command')

    def test_svd_write_actions_are_compared_as_semantics_not_false_rw_conflicts(self):
        svd = '''<device><resetValue>0</resetValue><peripherals><peripheral>
        <name>TEST</name><registers><register><name>STATUS</name><addressOffset>0</addressOffset>
        <access>read-write</access><modifiedWriteValues>oneToClear</modifiedWriteValues>
        <fields><field><name>FLAG</name><bitOffset>0</bitOffset><bitWidth>1</bitWidth></field></fields>
        </register></registers></peripheral></peripherals></device>'''
        blocks = {'TEST': {'registers': [{'name': 'STATUS', 'reset': 0, 'detail_page': 1,
                    'section': '6.1', 'fields': [{'name': 'FLAG', 'position': 0, 'width': 1,
                                                'access': 'RW_W1C', 'enums': []}]}]}}
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'source.svd'
            path.write_text(svd)
            audit = product.reconcile_fields(blocks, path)
        self.assertEqual(audit['matched_mdk_fields'], 1)
        self.assertEqual(audit['reviewed_access_differences'], [])

    def test_wrong_pdf_is_rejected_before_poppler_or_svd_are_needed(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'wrong.pdf'
            path.write_bytes(b'not the pinned manual')
            result = subprocess.run([sys.executable, ROOT / 'tool/nrf52840-product.py',
                '--pdf', path, '--source-dir', directory], text=True, capture_output=True)
        self.assertEqual(result.returncode, 2)
        self.assertIn('does not match pinned content identity', result.stderr)

    def test_core_byte_priorities_and_documented_cmsis_mask_overlay_survive_import(self):
        source = '#define TPI_DEVTYPE_SubType_Pos 4U\n#define TPI_DEVTYPE_SubType_Msk (0xFUL /* << TPI_DEVTYPE_SubType_Pos */)\n'
        source += '#define TPI_DEVTYPE_MajorType_Pos 0U\n#define TPI_DEVTYPE_MajorType_Msk (0xFUL << TPI_DEVTYPE_MajorType_Pos)\n'
        for block in arm_core.BASES.keys() - {'FPB', 'ETM'}:
            member = '__IOM uint8_t IP[48U]; /* Offset: 0x300 */' if block == 'NVIC' else (
                '__IM uint32_t DEVTYPE; /* Offset: 0xFCC */' if block == 'TPI' else '')
            source += f'typedef struct\n{{\n{member}\n}} {block}_Type;\n'
        blocks = arm_core.cmsis_blocks(source)
        self.assertEqual(len(blocks['NVIC']), 43)
        self.assertTrue(all(r['width'] == 8 for r in blocks['NVIC']))
        self.assertEqual(blocks['NVIC'][1]['offset'], 0x301)
        self.assertEqual(blocks['TPI'][0]['fields'][0]['position'], 4)
        self.assertEqual(blocks['TPI'][0]['fields'][0]['width'], 4)

    def test_core_macro_evaluator_accepts_constants_without_executing_source(self):
        self.assertEqual(arm_core.expression('(0xFUL << POSITION)', {'POSITION': 4}), 0xF0)
        with self.assertRaises(ValueError):
            arm_core.expression('__import__("os").system("false")', {})


if __name__ == '__main__':
    result = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(ProductImport))
    failed = {test.id() for test, _ in result.failures + result.errors}
    print(json.dumps({'format': 'mcutest-results', 'version': 1,
        'summary': {'tests': result.testsRun, 'failures': len(failed)},
        'tests': [{'name': 'nrf_product/' + name,
                   'status': 'failed' if f'__main__.ProductImport.{name}' in failed else 'passed',
                   'requirements': ['REQ-0055']}
                  for name in unittest.defaultTestLoader.getTestCaseNames(ProductImport)]}))
    raise SystemExit(not result.wasSuccessful())
