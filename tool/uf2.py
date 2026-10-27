#!/usr/bin/env python3
"""Convert a linked Arm ELF to UF2 inside an explicit application partition.

No probe, serial port, mount or network access. Format: microsoft/uf2 README.
"""
# @req REQ-0053
import argparse
from pathlib import Path
import struct


def encode(data, *, family, start, size):
    if not 0 < family <= 0xFFFFFFFF:
        raise ValueError('UF2 requires an explicit 32-bit family ID')
    if start < 0 or start % 256 or size <= 0 or size % 256 or start + size > 2**32:
        raise ValueError('application partition must fit 32 bits and use 256-byte boundaries')
    if len(data) < 52 or data[:7] != b'\x7fELF\x01\x01\x01':
        raise ValueError('expected a little-endian ELF32 image')
    header = struct.unpack_from('<HHIIIIIHHHHHH', data, 16)
    if header[0:3] != (2, 40, 1) or header[7] != 52:
        raise ValueError('expected a linked Arm ELF executable')
    offset, stride, count = header[4], header[8], header[9]
    if stride < 32 or offset < 52 or offset + stride * count > len(data):
        raise ValueError('truncated ELF program headers')
    pages, used = {}, set()
    for index in range(count):
        kind, source, virtual, address, length, memory, _, _ = struct.unpack_from(
            '<IIIIIIII', data, offset + index * stride)
        if kind != 1 or length == 0:
            continue
        if length > memory or source + length > len(data):
            raise ValueError('invalid ELF load segment')
        if address < start or address + length > start + size:
            raise ValueError('ELF load segment is outside application partition')
        for position, value in enumerate(data[source:source + length], address):
            if position in used:
                raise ValueError('overlapping ELF load segments')
            used.add(position)
            page = position & ~255
            if page not in pages:
                pages[page] = bytearray(b'\xff' * 256)
            pages[page][position - page] = value
    if not pages or min(pages) != start:
        raise ValueError('application must have load data at the partition start')
    blocks = []
    for index, (address, payload) in enumerate(sorted(pages.items())):
        header = struct.pack('<8I', 0x0A324655, 0x9E5D5157, 0x2000,
                             address, 256, index, len(pages), family)
        blocks.append(header + payload + bytes(220) + struct.pack('<I', 0x0AB16F30))
    return b''.join(blocks)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--elf', type=Path, required=True)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--family', type=lambda value: int(value, 0), required=True)
    parser.add_argument('--start', type=lambda value: int(value, 0), required=True)
    parser.add_argument('--size', type=lambda value: int(value, 0), required=True)
    args = parser.parse_args()
    if args.output.resolve() == args.elf.resolve():
        parser.error('output must not overwrite the ELF')
    try:
        result = encode(args.elf.read_bytes(), family=args.family, start=args.start, size=args.size)
        args.output.write_bytes(result)
    except (OSError, ValueError) as error:
        parser.error(str(error))


if __name__ == '__main__':
    main()
