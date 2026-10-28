#!/usr/bin/env python3
# @req-file REQ-0055 REQ-0007 REQ-0008
"""Reconcile pinned PS v1.11 register tables and generate its CryptoCell supplement.

Requires Poppler pdftotext. No network, firmware upload or MMIO is performed.
The audit covers register names/absolute addresses, not the full mode/errata gate.
"""
import argparse
import hashlib
import json
import re
import subprocess
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REFERENCE = Path('doc/reference/nrf52840')
REGISTER_NAME = r'[A-Z][A-Z0-9_]*(?:\[\d+\])?(?:\.[A-Z][A-Z0-9_]*(?:\[\d+\])?)*'


def ident(name):
    return re.sub(r'[^a-z0-9_]', '_', name.lower()).rstrip('_')


def overviews(text):
    blocks = {}
    current = None
    overview = False
    in_instances = False
    instances = []
    for page, content in enumerate(text.split('\f'), 1):
        for line in content.splitlines():
            line = line.strip()
            if line == 'Instances':
                in_instances, overview, instances = True, False, []
                continue
            if in_instances:
                match = re.match(r'^([A-Z][A-Z0-9_]*)\s+(0x[0-9A-Fa-f]{8})\s+', line)
                if match:
                    name, address = match.groups()
                    instances.append({'name': name, 'base': int(address, 16)})
                    if len(instances) == 1:
                        current = name
                        blocks[current] = {'instances': [], 'registers': [], 'page': page}
                    blocks[current]['instances'] = instances.copy()
                if line in ('Configuration', 'Register overview'):
                    in_instances = False
            if line == 'Register overview':
                overview = True
                continue
            if re.match(r'^\d+(?:\.\d+)+\s', line) or line.startswith('Address offset:'):
                overview = False
            if overview and current:
                match = re.match(r'^(' + REGISTER_NAME + r')\s+(0x[0-9A-Fa-f]+)\s+', line)
                if match:
                    name, offset = match.groups()
                    # CTRL-AP has its own DAP register space, no CPU MMIO base.
                    if current == 'APPROTECT' and name == 'RESET':
                        current = 'CTRL_AP'
                        blocks[current] = {'instances': [], 'registers': [], 'page': page,
                                           'address_space': 'debug_access_port'}
                    blocks[current]['registers'].append({'name': name, 'offset': int(offset, 16),
                                                          'page': page})
    return blocks


def geometry_rows(bbox):
    ns = {'x': 'http://www.w3.org/1999/xhtml'}
    for number, page in enumerate(ET.parse(bbox).getroot().findall('.//x:page', ns), 1):
        groups = []
        for word in sorted(page.findall('.//x:word', ns), key=lambda w: float(w.attrib['yMin'])):
            top = float(word.attrib['yMin'])
            if not groups or top - groups[-1][0] > 1.5:
                groups.append((top, []))
            groups[-1][1].append(word)
        for _, words in groups:
            words.sort(key=lambda w: float(w.attrib['xMin']))
            yield number, ' '.join(w.text or '' for w in words), [
                (w.text, float(w.attrib['xMin']), float(w.attrib['xMax'])) for w in words]


def crypto_fields(bbox, blocks):
    details = {}
    current, body, start = None, [], 0
    for page, line, words in geometry_rows(bbox):
        match = re.fullmatch(r'6\.6\.(?:12|13)(?:\.\d+)+ (' + REGISTER_NAME + ')', line)
        if re.match(r'^\d+(?:\.\d+)+\s', line):
            if current:
                details[current] = {'page': start, 'rows': body}
            current, body, start = match[1] if match else None, [], page
        elif current:
            body.append((line, words))
    if current:
        details[current] = {'page': start, 'rows': body}
    for name, block in blocks.items():
        if not name.startswith('CC_'):
            continue
        for register in block['registers']:
            detail = details[register['name']]
            fields, positions, reset, last = {}, {}, None, None
            for line, words in detail['rows']:
                if line.startswith('Address offset:'):
                    assert int(line.split(':')[1], 0) == register['offset']
                if line.startswith('Bit number'):
                    positions = {(lo + hi) / 2: int(t) for t, lo, hi in words if t.isdigit()}
                    assert len(positions) == 32
                if line.startswith('ID ') and 'R/W' not in line:
                    for token, lo, hi in words:
                        if not re.fullmatch('[A-Z]', token):
                            continue
                        centre = (lo + hi) / 2
                        column = min(positions, key=lambda x: abs(x - centre))
                        # A half-cell displacement is ambiguous: fail closed.
                        assert abs(column - centre) < 4.4, (name, register['name'], token)
                        fields.setdefault(token, {'bits': []})['bits'].append(positions[column])
                match = re.match(r'^Reset (0x[0-9A-Fa-f]+)', line)
                if match:
                    reset = int(match[1], 16)
                match = re.match(r'^([A-Z]) (RW1|W1|RW|R|W) ([A-Za-z][A-Za-z0-9_]*)\b', line)
                if match:
                    last = fields.setdefault(match[1], {'bits': []})
                    last.update(name=match[3], access=match[2], enums=[])
                elif last:
                    match = re.match(r'^([A-Za-z0-9_]+) (0x[0-9A-Fa-f]+|\d+)\b', line)
                    if match and 140 < words[0][1] < 240:
                        last['enums'].append({'name': match[1], 'value': int(match[2], 0)})
            assert reset is not None, (name, register['name'], 'reset')
            normalized = []
            for field in fields.values():
                bits = sorted(set(field.pop('bits')))
                assert bits and 'name' in field, (name, register['name'], field)
                assert bits == list(range(min(bits), max(bits) + 1))
                field.update(position=min(bits), width=len(bits))
                assert all(e['value'] < 1 << len(bits) for e in field['enums'])
                normalized.append(field)
            register.update(fields=normalized, reset=reset, detail_page=detail['page'])


def flatten(parent, prefix='', offset=0):
    for node in parent:
        if node.tag not in ('register', 'cluster'):
            continue
        for index in range(int(node.findtext('dim', '1'), 0)):
            name = node.findtext('name').replace('[%s]', f'[{index}]').replace('%s', str(index))
            address = offset + int(node.findtext('addressOffset'), 0) + index * int(node.findtext('dimIncrement', '0'), 0)
            if node.tag == 'cluster':
                yield from flatten(node, prefix + name + '.', address)
            else:
                yield prefix + name, address


def reconcile(blocks, svd):
    peripherals = {p.findtext('name'): p for p in ET.parse(svd).getroot().find('peripherals')}
    result = []
    for name, block in blocks.items():
        canonical = 'P0' if name == 'GPIO' else name
        peripheral = peripherals.get(canonical)
        raw, base = {}, 0
        if peripheral is not None:
            base = int(peripheral.findtext('baseAddress'), 0)
            family = peripherals[peripheral.get('derivedFrom', canonical)]
            raw = dict(flatten(family.find('registers')))
        for register in block['registers']:
            entry = {'block': name, 'name': register['name'], 'page': register['page'],
                     'offset': register['offset']}
            if name == 'CTRL_AP':
                status = 'debug_access_port_not_cpu_mmio'
            elif name == 'UICR' and register['name'].startswith('UNUSED'):
                status = 'reserved_not_a_usable_register'
            else:
                address = block['instances'][0]['base'] + register['offset']
                entry['address'] = address
                if register['name'] in raw and base + raw[register['name']] == address:
                    status = 'mdk_absolute_address_matches'
                    if base != block['instances'][0]['base']:
                        entry.update(mdk_base=base, mdk_offset=raw[register['name']])
                elif name.startswith('CC_'):
                    assert register['name'] not in raw, entry
                    status = 'product_supplement'
                else:
                    raise ValueError(f'unexplained product register: {entry}')
            entry['status'] = status
            result.append(entry)
    return {'document': '4413_417 v1.11', 'scope': 'register overview names and absolute addresses; field/mode/errata gate remains separate',
            'counts': {status: sum(r['status'] == status for r in result) for status in sorted(set(r['status'] for r in result))},
            'registers': result}


# Semantic overrides from the detailed register text, not just the R/W column.
SEMANTICS = {
    ('CC_RNG', 'RNG_ICR'): 'clear_alias',
    ('CC_HOST_RGF', 'ICR'): 'clear_alias',
    ('CC_RNG', 'EHR_DATA[5]'): 'read_clear',
    ('CC_PKA', 'PKA_SRAM_RDATA'): 'read_clear',
    ('CC_RNG_SRAM', 'SRAM_DATA'): 'read_write_side_effect',
}


def generate_crypto(blocks):
    outputs = {}
    includes = []
    catalogue = []
    for name, block in blocks.items():
        if not name.startswith('CC_'):
            continue
        namespace = ident(name)
        includes.append(f'#include <mcubsp/mcu/nrf52840/product/{namespace}.hpp>')
        base = block['instances'][0]['base']
        lines = ['#pragma once', '// @req-file REQ-0055', '// Nordic PS 4413_417 v1.11; generated by tool/nrf52840-product.py.',
                 '// Register facts only; sequencing/protection/errata remain driver obligations.',
                 '#include <mcubsp/core/register.hpp>', f'namespace mcubsp::mcu::nrf52840::product::{namespace} {{',
                 f'inline constexpr std::uintptr_t base = 0x{base:08X}U;']
        for register in block['registers']:
            rn = register['name']
            reg = ident(rn)
            access = {f['access'] for f in register['fields']}
            readable = bool(access & {'R', 'RW', 'RW1'})
            writable = bool(access & {'W', 'W1', 'RW', 'RW1'})
            semantics = 'read_write' if readable and writable else 'read_only' if readable else 'write_only'
            semantics = SEMANTICS.get((name, rn), semantics)
            mask = sum(((1 << f['width']) - 1) << f['position'] for f in register['fields'] if f['access'] != 'R')
            once = bool(access & {'W1', 'RW1'})
            lines += [f'// PS p.{register["detail_page"]}',
                      f'struct {reg} : core::register_descriptor<std::uint32_t, 0x{register["offset"]:X}U,',
                      f'    core::access_semantics::{semantics}, 0x{mask:08X}U, false,',
                      f'    core::reserved_bit_policy::write_zero, 0U, 0x{register["reset"]:08X}U, 0xFFFFFFFFU> {{',
                      f'    static constexpr bool write_once = {str(once).lower()};', '};', f'namespace {reg}_fields {{']
            for f in register['fields']:
                fn = ident(f['name'])
                field_access = 'read_only' if f['access'] == 'R' else semantics
                lines.append(f'using {fn} = core::field_descriptor<{reg}, {f["position"]}U, {f["width"]}U, core::access_semantics::{field_access}>;')
                for enum in f['enums']:
                    lines.append(f'inline constexpr std::uint32_t {fn}_{ident(enum["name"])} = 0x{enum["value"]:X}U;')
            lines.append('}')
        lines += ['}', '']
        outputs[Path(f'include/mcubsp/mcu/nrf52840/product/{namespace}.hpp')] = '\n'.join(lines)
        catalogue.append(f'    crypto_block{{"{name}", 0x{base:08X}U, {len(block["registers"])}U, {sum(len(r["fields"]) for r in block["registers"])}U}},')
    outputs[Path('include/mcubsp/mcu/nrf52840/product.hpp')] = '\n'.join([
        '#pragma once', '// @req-file REQ-0055', '#include <array>', '#include <cstdint>', '#include <string_view>',
        *includes, 'namespace mcubsp::mcu::nrf52840::product {',
        'struct crypto_block { std::string_view name; std::uintptr_t base; unsigned registers, fields; };',
        'inline constexpr std::array crypto_inventory{', *catalogue, '};', '}', ''])
    return outputs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pdf', required=True, type=Path)
    parser.add_argument('--source-dir', required=True, type=Path)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    identity = json.loads((ROOT / REFERENCE / 'product-specification.json').read_text())
    if hashlib.sha256(args.pdf.read_bytes()).hexdigest() != identity['sha256']:
        parser.error('product PDF does not match pinned content identity')
    manifest = json.loads((ROOT / REFERENCE / 'source-manifest.json').read_text())
    svd = args.source_dir / 'nrf52840.svd'
    expected = next(e['sha256'] for e in manifest['sources'] if e['file'] == svd.name)
    if hashlib.sha256(svd.read_bytes()).hexdigest() != expected:
        parser.error('SVD does not match pinned content identity')
    with tempfile.TemporaryDirectory(prefix='mcubsp-nrf-product-') as directory:
        text, bbox = Path(directory) / 'product.txt', Path(directory) / 'product.html'
        subprocess.run(['pdftotext', '-layout', str(args.pdf), str(text)], check=True)
        subprocess.run(['pdftotext', '-bbox-layout', str(args.pdf), str(bbox)], check=True)
        blocks = overviews(text.read_text())
        crypto_fields(bbox, blocks)
    audit = reconcile(blocks, svd)
    outputs = generate_crypto(blocks)
    outputs[REFERENCE / 'product-register-audit.json'] = json.dumps(audit, indent=2) + '\n'
    facts = {name: block for name, block in blocks.items() if name.startswith('CC_')}
    outputs[REFERENCE / 'cryptocell-facts.json'] = json.dumps(facts, indent=2) + '\n'
    changed = []
    for relative, content in outputs.items():
        path = ROOT / relative
        if args.check:
            if not path.is_file() or path.read_text() != content:
                changed.append(str(relative))
        else:
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content)
    if changed:
        parser.error('generated outputs differ: ' + ', '.join(changed))
    print(json.dumps({'register_audit': audit['counts'], 'crypto_blocks': len(facts),
                      'crypto_registers': sum(len(b['registers']) for b in facts.values()),
                      'crypto_fields': sum(len(r['fields']) for b in facts.values() for r in b['registers']),
                      'checked': args.check}))


if __name__ == '__main__':
    main()
