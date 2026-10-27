#!/usr/bin/env python3
"""UF2 format and partition regressions; no devices are accessed."""
import importlib.util
import json
from pathlib import Path
import struct
import unittest

ROOT = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('mcubsp_uf2', ROOT / 'tool/uf2.py')
uf2 = importlib.util.module_from_spec(spec)
spec.loader.exec_module(uf2)
START, SIZE, FAMILY = 0x26000, 0xC6000, 0xADA52840


def elf(segments, machine=40):
    identity = b'\x7fELF\x01\x01\x01' + bytes(9)
    header = identity + struct.pack('<HHIIIIIHHHHHH', 2, machine, 1, START + 9,
                                    52, 0, 0, 52, 32, len(segments), 0, 0, 0)
    programs, contents = [], bytearray()
    base = 52 + 32 * len(segments)
    for address, payload in segments:
        programs.append(struct.pack('<IIIIIIII', 1, base + len(contents),
                                     address, address, len(payload), len(payload), 5, 4))
        contents.extend(payload)
    return header + b''.join(programs) + contents


def encode(data):
    return uf2.encode(data, family=FAMILY, start=START, size=SIZE)


class Uf2(unittest.TestCase):
    def test_payload_reconstruction_family_and_block_headers(self):
        data = bytes(range(256)) + b'last'
        image = encode(elf([(START, data)]))
        self.assertEqual(len(image), 1024)
        for index in range(2):
            block = image[index * 512:(index + 1) * 512]
            self.assertEqual(struct.unpack_from('<8I', block),
                (0x0A324655, 0x9E5D5157, 0x2000, START + index * 256,
                 256, index, 2, FAMILY))
            self.assertEqual(struct.unpack_from('<I', block, 508)[0], 0x0AB16F30)
        self.assertEqual(image[32:288], data[:256])
        self.assertEqual(image[544:548], b'last')
        self.assertEqual(image[548:800], b'\xff' * 252)

    def test_sparse_segments_share_pages_without_fabricating_gap_pages(self):
        data = elf([(START, b'first'), (START + 32, b'data'), (START + 1024, b'last')])
        image = encode(data)
        self.assertEqual(len(image), 1024)
        self.assertEqual(image[64:68], b'data')
        self.assertEqual(struct.unpack_from('<I', image, 512 + 12)[0], START + 1024)
        self.assertEqual(encode(data), image)

    def test_rejects_softdevice_storage_bootloader_and_uicr_writes(self):
        for address in (0, START - 1, START + SIZE, 0xF4000, 0x10001200):
            with self.subTest(address=hex(address)), self.assertRaisesRegex(ValueError, 'outside'):
                encode(elf([(START, b'vectors'), (address, b'bad')]))
        with self.assertRaisesRegex(ValueError, 'outside'):
            encode(elf([(START, b'vectors'), (START + SIZE - 1, b'overflow')]))

    def test_rejects_malformed_architecture_truncation_and_overlap(self):
        for data in (b'', bytes(52), elf([(START, b'a')], machine=62),
                     elf([(START, b'a')])[:-1], elf([(START, b'aa'), (START + 1, b'b')])):
            with self.subTest(data=data[:20]), self.assertRaises(ValueError):
                encode(data)

    def test_requires_the_application_origin_and_a_bounded_partition(self):
        for data in (elf([]), elf([(START + 256, b'wrong origin')])):
            with self.assertRaisesRegex(ValueError, 'partition start'):
                encode(data)
        for changes in ({'family': 0}, {'start': START + 1}, {'size': SIZE - 1},
                        {'size': 0}, {'start': 2**32}, {'family': 2**32}):
            with self.subTest(changes=changes), self.assertRaises(ValueError):
                uf2.encode(elf([(START, b'data')]),
                           **({'family': FAMILY, 'start': START, 'size': SIZE} | changes))

    def test_uses_flash_load_address_for_initialized_ram(self):
        data = bytearray(elf([(START, b'vectors'), (START + 256, b'RAM!')]))
        # Second PT_LOAD has a RAM VMA and flash LMA, as emitted by the linker.
        struct.pack_into('<I', data, 52 + 32 + 8, 0x20000000)
        image = encode(data)
        self.assertEqual(struct.unpack_from('<I', image, 512 + 12)[0], START + 256)
        self.assertEqual(image[544:548], b'RAM!')


if __name__ == '__main__':
    result = unittest.TextTestRunner(verbosity=2).run(
        unittest.defaultTestLoader.loadTestsFromTestCase(Uf2))
    failed = {test.id() for test, _ in result.failures + result.errors}
    print(json.dumps({'format': 'mcutest-results', 'version': 1,
        'summary': {'tests': result.testsRun, 'failures': len(failed)},
        'tests': [{'name': 'uf2/' + name,
            'status': 'failed' if f'__main__.Uf2.{name}' in failed else 'passed',
            'requirements': ['REQ-0053']}
            for name in unittest.defaultTestLoader.getTestCaseNames(Uf2)]}))
    raise SystemExit(not result.wasSuccessful())
