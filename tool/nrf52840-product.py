#!/usr/bin/env python3
# @req-file REQ-0055 REQ-0007 REQ-0008
"""Reconcile pinned PS v1.11 registers/fields and generate product descriptions.

Requires Poppler pdftotext. No network, firmware upload or MMIO is performed.
The import is declarative; revision/board acceptance and driver sequencing remain separate.
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
    value = re.sub('_+', '_', re.sub(r'[^a-z0-9_]', '_', name.lower())).strip('_')
    keywords = set('alignas alignof and and_eq asm auto bitand bitor bool break case catch char class compl concept const consteval constexpr constinit continue co_await co_return co_yield decltype default delete do double else enum explicit export extern false float for friend goto if inline int long mutable namespace new noexcept not not_eq nullptr operator or or_eq private protected public register requires return short signed sizeof static struct switch template this thread_local throw true try typedef typeid typename union unsigned using virtual void volatile wchar_t while xor xor_eq'.split())
    return 'reg_' + value if value[0].isdigit() or value in keywords else value


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


def register_details(rows):
    details = {}
    current, body, start, section = None, [], 0, None
    for page, line, words in rows:
        match = re.fullmatch(r'(\d+(?:\.\d+)+) (' + REGISTER_NAME + r')(?: \(.*?\))?', line)
        if re.match(r'^\d+(?:\.\d+)+\s', line):
            if current:
                details.setdefault(current, []).append({'page': start, 'rows': body, 'section': section})
            current, body, start, section = (match[2] if match else None), [], page, (match[1] if match else None)
        elif current:
            body.append((line, words))
    if current:
        details.setdefault(current, []).append({'page': start, 'rows': body, 'section': section})
    return details


def extract_fields(detail):
    fields, positions, reset, last = {}, {}, None, None
    for line, words in detail['rows']:
        if line.startswith('Bit number'):
            positions = {(lo + hi) / 2: int(t) for t, lo, hi in words if t.isdigit()}
            assert len(positions) == 32, ('bit columns', detail['page'])
        if line.startswith('ID ') and 'R/W' not in line:
            for token, lo, hi in words:
                if not re.fullmatch('[A-Za-z]', token):
                    continue
                centre = (lo + hi) / 2
                column = min(positions, key=lambda x: abs(x - centre))
                assert abs(column - centre) < 4.4, ('ambiguous bit column', detail['page'], token)
                fields.setdefault(token, {'bits': []})['bits'].append(positions[column])
        match = re.match(r'^Reset (0x[0-9A-Fa-f]+)', line)
        if match:
            value = int(match[1], 16)
            assert reset is None or reset == value, ('inconsistent repeated reset', detail['page'])
            reset = value
        # IDs above Z are lower-case a..f. A trailing '-' is a table marker.
        # An ID diagram beginning 'ID R Q' is not a field row named ID.
        match = None if line.startswith('ID ') else re.match(
            r'^([A-Za-z])-? (RW1|W1|RW|R|W) ([A-Za-z][A-Za-z0-9_]*(?:\[\d+\])?)(?=\s|$)', line)
        if match:
            last = fields.setdefault(match[1], {'bits': []})
            if 'name' in last:
                assert last['name'] == match[3], ('repeated ID changes name', detail['page'])
            else:
                last.update(name=match[3], access=match[2], enums=[])
        elif last:
            if line == 'W1C':
                last['access'] = 'RW_W1C'
            match = re.match(r'^([A-Za-z0-9_]+) (0x[0-9A-Fa-f]+|\d+)\b', line)
            if match and 140 < words[0][1] < 240:
                value = {'name': match[1], 'value': int(match[2], 0)}
                if value not in last['enums']:
                    last['enums'].append(value)
    assert reset is not None, ('missing reset', detail['page'])
    normalized = []
    for field in fields.values():
        bits = sorted(set(field.pop('bits')))
        assert bits and 'name' in field, ('unnamed or unplaced field', detail['page'], field)
        assert bits == list(range(min(bits), max(bits) + 1)), ('noncontiguous field', detail['page'], field)
        field.update(position=min(bits), width=len(bits))
        assert all(e['value'] < 1 << len(bits) for e in field['enums'])
        normalized.append(field)
    return reset, normalized


def product_fields(bbox, blocks):
    details = register_details(geometry_rows(bbox))
    for name, block in blocks.items():
        for register in block['registers']:
            if name == 'UICR' and register['name'].startswith('UNUSED'):
                register['classification'] = 'reserved_not_a_usable_register'
                continue
            candidates = [d for d in details.get(register['name'], [])
                          if d['page'] >= register['page'] and any(
                              line.startswith('Address offset:') and int(line.split(':')[1], 0) == register['offset']
                              for line, _ in d['rows'])]
            assert candidates, ('missing detailed table', name, register['name'])
            detail = candidates[0]
            reset, fields = extract_fields(detail)
            assert fields, ('missing usable fields', name, register['name'])
            register.update(fields=fields, reset=reset, detail_page=detail['page'], section=detail['section'])
    # PS 5.4.3.25 explicitly says STATUS is read-only despite the RW table cell.
    status = next(f for r in blocks['CLOCK']['registers'] if r['name'] == 'LFRCMODE'
                  for f in r['fields'] if f['name'] == 'STATUS')
    status.update(source_access=status['access'], access='R',
                  review='PS 5.4.3.25 p.170: STATUS is read-only; the RW column is inconsistent.')


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
    ('CC_DIN', 'SRC_MEM_SIZE'): 'command', # writing SIZE starts this DMA path, PS 6.6.13.2.1.4
    ('CC_RNG', 'RNG_ICR'): 'clear_alias',
    ('CC_HOST_RGF', 'ICR'): 'clear_alias',
    ('CC_RNG', 'EHR_DATA[5]'): 'read_clear',
    ('CC_PKA', 'PKA_SRAM_RDATA'): 'read_clear',
    ('CC_RNG_SRAM', 'SRAM_DATA'): 'read_write_side_effect',
}


def svd_registers(svd):
    root = ET.parse(svd).getroot()
    peripherals = {p.findtext('name'): p for p in root.find('peripherals')}

    def expand(parent, prefix=''):
        for node in parent:
            if node.tag not in ('cluster', 'register'):
                continue
            for index in range(int(node.findtext('dim', '1'), 0)):
                name = prefix + node.findtext('name').replace('[%s]', f'[{index}]').replace('%s', str(index))
                if node.tag == 'cluster':
                    yield from expand(node, name + '.')
                else:
                    yield name, node
    return root, {name: dict(expand(peripherals[p.get('derivedFrom', name)].find('registers')))
                  for name, p in peripherals.items()}


def field_bits(field):
    if field.findtext('bitOffset') is not None:
        return int(field.findtext('bitOffset'), 0), int(field.findtext('bitWidth'), 0)
    if field.findtext('lsb') is not None:
        lo = int(field.findtext('lsb'), 0)
        return lo, int(field.findtext('msb'), 0) - lo + 1
    hi, lo = map(int, re.fullmatch(r'\[(\d+):(\d+)\]', field.findtext('bitRange')).groups())
    return lo, hi - lo + 1


def reconcile_fields(blocks, svd):
    root, sources = svd_registers(svd)
    access_names = {'read-only': 'R', 'write-only': 'W', 'read-write': 'RW',
                    'writeOnce': 'W1', 'read-writeonce': 'RW1'}
    normalize = lambda name: re.sub(r'[\[\]]', '', name).lower()
    matched, supplemental, changes, semantics = 0, 0, [], {}
    for block_name, block in blocks.items():
        raw_registers = sources.get('P0' if block_name == 'GPIO' else block_name, {})
        for register in block['registers']:
            if 'fields' not in register:
                continue
            raw = raw_registers.get(register['name'])
            if raw is None:
                supplemental += len(register['fields'])
                continue
            register['source_read_action'] = raw.findtext('readAction')
            register['source_write_action'] = raw.findtext('modifiedWriteValues')
            raw_fields = {normalize(f.findtext('name')): f for f in raw.findall('fields/field')}
            assert len(raw_fields) == len(register['fields']), ('field inventory differs', block_name, register['name'])
            assert int(raw.findtext('resetValue', root.findtext('resetValue')), 0) == register['reset']
            for field in register['fields']:
                rf = raw_fields[normalize(field['name'])]
                assert field_bits(rf) == (field['position'], field['width']), ('field shape differs', block_name, register['name'], field['name'])
                access = access_names[rf.findtext('access', raw.findtext('access', 'read-write'))]
                modification = rf.findtext('modifiedWriteValues', raw.findtext('modifiedWriteValues'))
                if modification == 'oneToClear' and access == 'RW':
                    access = 'RW_W1C'
                effective = field['access']
                if register_semantics(block_name, register) in ('write_one_to_clear', 'clear_alias'):
                    if effective == 'RW':
                        effective = 'RW_W1C'
                    if access == 'RW':
                        access = 'RW_W1C'
                if access == effective:
                    matched += 1
                else:
                    changes.append({'block': block_name, 'register': register['name'],
                                    'field': field['name'], 'mdk_access': access,
                                    'product_access': effective, 'page': register['detail_page'],
                                    'section': register['section'],
                                    'resolution': field.get('review', 'Use current product table; retain legacy MDK provenance separately.')})
    return {'document': '4413_417 v1.11', 'scope': 'All detailed peripheral field tables; not Arm core or revision/board qualification.',
            'matched_mdk_fields': matched, 'supplemental_fields': supplemental,
            'reviewed_access_differences': changes, 'unresolved_field_shape_or_reset_differences': []}


def register_semantics(block_name, register):
    name = register['name']
    access = {field['access'] for field in register['fields']}
    readable = bool(access & {'R', 'RW', 'RW1', 'RW_W1C'})
    writable = bool(access & {'W', 'W1', 'RW', 'RW1', 'RW_W1C'})
    ordinary = 'read_write' if readable and writable else 'read_only' if readable else 'write_only'
    if (block_name, name) in SEMANTICS:
        return SEMANTICS[(block_name, name)]
    if name.startswith('TASKS_'):
        return 'command'
    if name.startswith('EVENTS_'):
        return 'write_zero_to_clear'
    if name in ('INTENSET', 'CHENSET', 'EVTENSET', 'OUTSET', 'DIRSET') or name.endswith('.POWERSET'):
        return 'set_alias'
    if name in ('INTENCLR', 'CHENCLR', 'EVTENCLR', 'OUTCLR', 'DIRCLR') or name.endswith('.POWERCLR'):
        return 'clear_alias'
    if name in ('RESETREAS', 'LATCH', 'ERRORSRC') or register.get('source_write_action') == 'oneToClear':
        return 'write_one_to_clear'
    if 'RW_W1C' in access:
        assert not (access & {'W', 'W1', 'RW', 'RW1'}), ('mixed write semantics need explicit views', block_name, name)
        return 'write_one_to_clear'
    if register.get('source_read_action') in ('modifyExternal', 'clear'):
        if ordinary == 'read_only':
            return 'read_clear'
        if ordinary == 'read_write':
            return 'read_write_side_effect'
    return ordinary


def generate_products(blocks, instance_audit):
    outputs, includes, catalogue, crypto_catalogue, instances = {}, [], [], [], {}
    for name, block in blocks.items():
        namespace = ident(name)
        includes.append(f'#include <mcubsp/mcu/nrf52840/product/{namespace}.hpp>')
        lines = ['#pragma once', '// @req-file REQ-0055',
                 '// Nordic PS 4413_417 v1.11; generated by tool/nrf52840-product.py.',
                 '// Declarative product facts; sequencing/protection/errata remain driver obligations.',
                 '#include <mcubsp/core/register.hpp>', f'namespace mcubsp::mcu::nrf52840::product::{namespace} {{']
        if name == 'CTRL_AP':
            lines += ['// This is a debugger access port, not CPU MMIO.',
                      'inline constexpr unsigned access_port = 1;']
        else:
            lines += [f'inline constexpr std::uintptr_t base = 0x{block["instances"][0]["base"]:08X}U;']
        for instance in block['instances']:
            if instance['name'] in instances:
                assert instances[instance['name']] == instance['base']
            instances[instance['name']] = instance['base']
        register_count, field_count = 0, 0
        for register in block['registers']:
            if 'fields' not in register:
                continue
            register_count += 1
            field_count += len(register['fields'])
            reg, access = ident(register['name']), {f['access'] for f in register['fields']}
            semantics = register_semantics(name, register)
            mask, used = 0, 0
            for field in register['fields']:
                field_mask = ((1 << field['width']) - 1) << field['position']
                assert not used & field_mask, ('overlapping fields', name, register['name'])
                used |= field_mask
                reserved_size_control = name in ('CC_DIN', 'CC_DOUT') and register['name'] in (
                    'SRC_MEM_SIZE', 'DST_MEM_SIZE') and field['name'] in ('FIRST', 'LAST')
                if field['access'] != 'R' and not reserved_size_control:
                    mask |= field_mask
            once = bool(access & {'W1', 'RW1'})
            if name in ('CC_DIN', 'CC_DOUT') and register['name'] in ('SRC_MEM_SIZE', 'DST_MEM_SIZE'):
                lines.append('// Detailed prose reserves FIRST/LAST; only the 30-bit byte size is writable.')
            lines += [f'// PS {register["section"]}, p.{register["detail_page"]}',
                      f'struct {reg} : core::register_descriptor<std::uint32_t, 0x{register["offset"]:X}U,',
                      f'    core::access_semantics::{semantics}, 0x{mask:08X}U, false,',
                      f'    core::reserved_bit_policy::write_zero, 0U, 0x{register["reset"]:08X}U, 0xFFFFFFFFU> {{',
                      f'    static constexpr bool write_once = {str(once).lower()};', '};']
            if semantics in ('command', 'set_alias', 'clear_alias') and access & {'R', 'RW', 'RW1', 'RW_W1C'}:
                lines += ['// Explicit read view of an otherwise command-only descriptor.',
                          f'using {reg}_status = core::register_descriptor<std::uint32_t, 0x{register["offset"]:X}U,',
                          f'    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,',
                          f'    0U, 0x{register["reset"]:08X}U, 0xFFFFFFFFU, {reg}>;']
            lines.append(f'namespace {reg}_fields {{')
            names = set()
            for field in register['fields']:
                fn = ident(field['name'])
                assert fn not in names, ('duplicate field identifier', name, register['name'], fn)
                names.add(fn)
                field_access = 'read_only' if field['access'] == 'R' else semantics
                lines.append(f'using {fn} = core::field_descriptor<{reg}, {field["position"]}U, {field["width"]}U, core::access_semantics::{field_access}>;')
                enums = {}
                for enum in field['enums']:
                    en = re.sub(r'[^a-z0-9_]', '_', enum['name'].lower()).rstrip('_')
                    assert en not in enums or enums[en] == enum['value'], ('enum identity differs', name, reg, fn, en)
                    enums[en] = enum['value']
                for en, value in enums.items():
                    lines.append(f'inline constexpr std::uint32_t {fn}_{en} = 0x{value:X}U;')
                if enums:
                    lines.append(f'enum class {fn}_encoding : std::uint32_t {{')
                    for en, value in enums.items():
                        lines.append(f'    value_{en} = 0x{value:X}U,')
                    lines.append('};')
                    choices = ', '.join(f'{fn}_encoding::value_{en}' for en in enums)
                    lines.append(f'using {fn}_values = core::enum_field_descriptor<{fn}, {fn}_encoding, {choices}>;')
            lines.append('}')
        lines += ['}', '']
        outputs[Path(f'include/mcubsp/mcu/nrf52840/product/{namespace}.hpp')] = '\n'.join(lines)
        catalogue.append(f'    product_block{{"{name}", {register_count}U, {field_count}U, {str(name == "CTRL_AP").lower()}}},')
        if name.startswith('CC_'):
            crypto_catalogue.append(f'    crypto_block{{"{name}", 0x{block["instances"][0]["base"]:08X}U, {register_count}U, {field_count}U}},')
    lines = ['#pragma once', '// @req-file REQ-0055', '#include <array>', '#include <cstdint>', '#include <string_view>',
             *includes, '#include <mcubsp/mcu/nrf52840/product/pins.hpp>', 'namespace mcubsp::mcu::nrf52840::product {',
             'struct product_block { std::string_view name; unsigned registers, fields; bool debug_access_port; };',
             'inline constexpr std::array block_inventory{', *catalogue, '};',
             'struct crypto_block { std::string_view name; std::uintptr_t base; unsigned registers, fields; };',
             'inline constexpr std::array crypto_inventory{', *crypto_catalogue, '};', 'namespace instances {']
    for item in instance_audit['instances']:
        lines += [f'struct {ident(item["name"])} {{ static constexpr std::uintptr_t base = 0x{item["base"]:08X}U; static constexpr int irq = {item["irq"]}; }};']
    lines += ['}', '}', '']
    outputs[Path('include/mcubsp/mcu/nrf52840/product.hpp')] = '\n'.join(lines)
    return outputs

def instance_facts(text, blocks, svd):
    families = {item['name']: name for name, block in blocks.items() for item in block['instances']}
    root = ET.parse(svd).getroot()
    source = {p.findtext('name'): p for p in root.find('peripherals')}
    facts, active = [], False
    for page, content in enumerate(text.split('\f'), 1):
        for line in content.splitlines():
            line = line.strip()
            if line == '4.2.4 Instantiation':
                active = True
                continue
            if active and line.startswith('Table 3:'):
                active = False
            match = re.match(r'^(\d+|N/A)\s+(0x[0-9A-Fa-f]{8})\s+([A-Z][A-Z0-9_]*)\s+', line)
            if not active or not match:
                continue
            identifier, base, name = match.groups()
            family = families.get(name)
            assert family is not None or name == 'FPU', ('unaccounted instance', name)
            raw = source.get('P0' if name == 'GPIO' else name)
            interrupt = raw.find('interrupt') if raw is not None else None
            if interrupt is None and raw is not None:
                interrupt = source[raw.get('derivedFrom', raw.findtext('name'))].find('interrupt')
            irq = int(interrupt.findtext('value')) if interrupt is not None else -1
            if family is not None:
                assert any(i['name'] == name and i['base'] == int(base, 16) for i in blocks[family]['instances'])
            role = 'interrupt_only' if name == 'FPU' else 'software_interrupt_alias' if name.startswith('SWI') else 'registers'
            facts.append({'name': name, 'base': int(base, 16), 'fabric_id': -1 if identifier == 'N/A' else int(identifier),
                          'irq': irq, 'family': family, 'role': role, 'page': page})
    assert len(facts) == 85, ('instantiation denominator changed', len(facts))
    assert len({item['name'] for item in facts}) == len(facts)
    return {'document': '4413_417 v1.11', 'section': '4.2.4',
            'notes': ['Fabric IDs are not necessarily CPU IRQs.',
                      'FPU at 0x40026000 names IRQ 38; its MDK UNUSED placeholder is not a usable peripheral register.',
                      'GPIO is a deprecated alias of P0; SWI aliases use the corresponding EGU interrupt.',
                      'Shared base addresses alone do not establish exclusive ownership.'],
            'instances': facts}


def package_facts(text):
    packages, current, pending = {}, None, None
    for page, content in enumerate(text.split('\f'), 1):
        for line in content.splitlines():
            stripped = line.strip()
            heading = re.match(r'^7\.1\.([123]) ', stripped)
            if heading:
                current = {'1': 'aqfn73', '2': 'qfn48', '3': 'wlcsp94'}[heading[1]]
                packages[current], pending = [], None
            elif re.match(r'^7\.2\s', stripped):
                current, pending = None, None
            match = re.match(r'^([A-Z]{0,2}\d+)\s+P([01])\.(\d{2})\s+', stripped)
            if current and match:
                pending = {'ball': match[1], 'port': int(match[2]), 'pin': int(match[3]), 'page': page, 'lines': []}
                packages[current].append(pending)
            if pending is not None:
                pending['lines'].append(stripped)
    for name, pins in packages.items():
        assert len(pins) == {'aqfn73': 48, 'qfn48': 30, 'wlcsp94': 48}[name]
        assert len({(p['port'], p['pin']) for p in pins}) == len(pins)
        for pin in pins:
            body = ' '.join(pin.pop('lines'))
            pin['low_frequency_only'] = 'Standard drive, low frequency I/O' in body
            pin['functions'] = sorted(set(re.findall(r'\b(?:AIN[0-7]|XL[12]|NFC[12]|TRACECLK|TRACEDATA[0-3]|SWO|nRESET)\b', body)))
    electrical = {}
    for pins in packages.values():
        for pin in pins:
            key = pin['port'], pin['pin']
            assert key not in electrical or electrical[key] == pin['low_frequency_only']
            electrical[key] = pin['low_frequency_only']
    return {'document': '4413_417 v1.11', 'section': '7.1, tables 61-63',
            'low_frequency_limit_hz': 10000,
            'scope': 'Silicon package bonds and fixed functions; no clone-board header exposure claim.',
            'packages': packages}


def generate_pin_metadata(facts):
    lines = ['#pragma once', '// @req-file REQ-0055', '// PS 4413_417 v1.11 7.1, tables 61-63; generated product facts.',
             '#include <array>', '#include <cstdint>', '#include <optional>', '#include <mcubsp/core/metadata.hpp>',
             'namespace mcubsp::mcu::nrf52840::product {',
             'enum class package : unsigned { aqfn73, qfn48, wlcsp94 };',
             'inline constexpr std::array<std::array<std::uint32_t, 2>, 3> package_gpio_masks{{']
    for pins in facts['packages'].values():
        masks = [sum(1 << p['pin'] for p in pins if p['port'] == port) for port in (0, 1)]
        lines.append(f'    {{0x{masks[0]:08X}U, 0x{masks[1]:08X}U}},')
    lines += ['}};', 'inline constexpr std::array<std::array<std::uint32_t, 2>, 3> low_frequency_masks{{']
    for pins in facts['packages'].values():
        masks = [sum(1 << p['pin'] for p in pins if p['port'] == port and p['low_frequency_only']) for port in (0, 1)]
        lines.append(f'    {{0x{masks[0]:08X}U, 0x{masks[1]:08X}U}},')
    lines += ['}};',
              '[[nodiscard]] constexpr bool bonded(package kind, core::pin_id pin) noexcept {',
              '    auto p = static_cast<unsigned>(kind);',
              '    return p < 3 && pin.port < 2 && pin.index < 32 && (package_gpio_masks[p][pin.port] & (1U << pin.index));', '}',
              '[[nodiscard]] constexpr bool low_frequency_only(package kind, core::pin_id pin) noexcept {',
              '    return bonded(kind, pin) && (low_frequency_masks[static_cast<unsigned>(kind)][pin.port] & (1U << pin.index));', '}',
              'inline constexpr std::uint32_t low_frequency_limit_hz = 10000;',
              '// Electrical restrictions are the same for a die pin in every package.',
              '[[nodiscard]] constexpr bool silicon_low_frequency_only(core::pin_id pin) noexcept {',
              '    return pin.port < 2 && pin.index < 32 && ((low_frequency_masks[0][pin.port] | low_frequency_masks[1][pin.port] | low_frequency_masks[2][pin.port]) & (1U << pin.index));', '}',
              '[[nodiscard]] constexpr bool digital_route_supported(package kind, core::pin_id pin, std::uint32_t hz, bool high_drive = false) noexcept {',
              '    return bonded(kind, pin) && (!low_frequency_only(kind, pin) || (!high_drive && hz <= low_frequency_limit_hz));', '}',
              'template <package Kind, core::pin_id Pin> requires(bonded(Kind, Pin))',
              'struct package_pin { static constexpr core::pin_id id = Pin; };',
              '// FICR INFO.PACKAGE, PS 4.4.1.16.3. Unknown is not a largest-package fallback.',
              '[[nodiscard]] constexpr std::optional<package> package_from_ficr(std::uint32_t code) noexcept {',
              '    switch (code) { case 0x2004: return package::aqfn73; case 0x2000: return package::qfn48;',
              '        case 0x2005: return package::wlcsp94; default: return std::nullopt; }', '}', '}', '']
    return '\n'.join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--pdf', required=True, type=Path)
    parser.add_argument('--source-dir', required=True, type=Path)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    if not __debug__:
        parser.error('run this audited importer without Python optimization; assertions are required')
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
        plain = text.read_text()
        blocks = overviews(plain)
        instances = instance_facts(plain, blocks, svd)
        pins = package_facts(plain)
        product_fields(bbox, blocks)
    audit = reconcile(blocks, svd)
    field_audit = reconcile_fields(blocks, svd)
    outputs = generate_products(blocks, instances)
    outputs[REFERENCE / 'product-instance-audit.json'] = json.dumps(instances, indent=2) + '\n'
    outputs[REFERENCE / 'package-pins.json'] = json.dumps(pins, indent=2) + '\n'
    outputs[Path('include/mcubsp/mcu/nrf52840/product/pins.hpp')] = generate_pin_metadata(pins)
    outputs[REFERENCE / 'product-field-audit.json'] = json.dumps(field_audit, indent=2) + '\n'
    outputs[REFERENCE / 'product-facts.json'] = json.dumps(blocks, indent=2) + '\n'
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
