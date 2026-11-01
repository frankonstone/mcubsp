#!/usr/bin/env python3
# @req REQ-0055
"""Reproduce the pinned Nordic MDK register-facts import, without network access.

This imports source facts, not a completed product-specification/errata audit.
Use --check to compare all outputs without changing the checkout.
"""
from pathlib import Path
import xml.etree.ElementTree as E
import json, re, hashlib, argparse, tempfile, sys
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--source-dir', type=Path, required=True, help='directory containing the source files listed in the pinned manifest')
parser.add_argument('--check', action='store_true')
args = parser.parse_args()
repository = Path(__file__).resolve().parents[1]
source = args.source_dir.resolve()
expected_path = repository / 'doc/reference/nrf52840/source-manifest.json'
expected = json.loads(expected_path.read_text())
for entry in expected['sources']:
    path = source / entry['file']
    if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != entry['sha256']:
        parser.error(f'missing or changed pinned source: {path}')
temporary = tempfile.TemporaryDirectory(prefix='mcubsp-nordic-import-') if args.check else None
root = Path(temporary.name) if temporary else repository
svd = E.parse(source / 'nrf52840.svd').getroot()
ps = {p.findtext('name'): p for p in svd.find('peripherals')}
out = root / 'include/mcubsp/mcu/nrf52840/registers'
out.mkdir(parents=True, exist_ok=True)
license = (svd.findtext('licenseText') or '').replace('\\n', '').strip()
notice = root / 'doc/reference/nrf52840'
notice.mkdir(parents=True, exist_ok=True)
(notice / 'NORDIC-LICENSE.txt').write_text(license + '\n')
keywords = {'break', 'continue', 'export', 'friend', 'operator', 'requires', 'concept', 'co_await', 'co_return', 'co_yield', 'using', 'void', 'bool', 'double', 'float', 'static', 'extern', 'thread_local', 'mutable', 'virtual', 'inline', 'explicit', 'alignas', 'alignof', 'sizeof', 'noexcept', 'typeid', 'throw', 'try', 'catch', 'this', 'while', 'for', 'else', 'goto', 'asm', 'switch', 'in', 'out', 'register', 'default', 'namespace', 'class', 'short', 'long', 'char', 'int', 'volatile', 'const', 'public', 'private', 'protected', 'delete', 'new', 'auto', 'enum', 'do', 'if', 'switch', 'case', 'return', 'and', 'or', 'not', 'xor', 'true', 'false', 'signed', 'unsigned', 'struct', 'union', 'typename', 'template'}

def ident(s):
    s = re.sub('[^a-zA-Z0-9_]', '_', s).lower()
    if s[0].isdigit() or s in keywords:
        s = 'reg_' + s
    return s

def num(t):
    if t.startswith('#'):
        return int(t[1:], 2)
    return int(t, 0)

def field_bits(f):
    if f.findtext('bitOffset') is not None:
        return (num(f.findtext('bitOffset')), num(f.findtext('bitWidth')))
    if f.findtext('lsb') is not None:
        p = num(f.findtext('lsb'))
        return (p, num(f.findtext('msb')) - p + 1)
    m = re.match('\\[(\\d+):(\\d+)\\]', f.findtext('bitRange'))
    return (int(m[2]), int(m[1]) - int(m[2]) + 1)

def field_access(block, register, field, inherited):
    # PS 4413_417 v1.11 5.4.3.25 p.170 explicitly makes STATUS read-only.
    # The SVD and table's RW cell disagree with that statement; prefer the
    # restrictive access, as in product/clock.hpp and product-field-audit.json.
    if (block, register, field.findtext('name')) == ('CLOCK', 'LFRCMODE', 'STATUS'):
        return 'read-only'
    return field.findtext('access', inherited)

def flatten(parent, prefix='', offset=0):
    for x in parent:
        if x.tag not in ['register', 'cluster']:
            continue
        n = x.findtext('name')
        dim = num(x.findtext('dim', '1'))
        stride = num(x.findtext('dimIncrement', '0'))
        for i in range(dim):
            name = n.replace('[%s]', str(i)).replace('%s', str(i))
            name = prefix + name
            addr = offset + num(x.findtext('addressOffset')) + i * stride
            if x.tag == 'cluster':
                yield from flatten(x, name + '_', addr)
            else:
                yield (name, addr, x)
counts = {}
inventory = []
types = []
for name, p in ps.items():
    canonical = p.get('derivedFrom', name)
    orig = ps[canonical]
    regs = list(flatten(orig.find('registers')))
    if name == canonical:
        namespace = ident(name)
        types.append(namespace)
        lines = ['#pragma once', '', '// @req-file REQ-0055', '// Nordic Semiconductor BSD-3-Clause derived register facts.', '// Source: nrfx v3.2.0 nrf52840.svd; license: doc/reference/nrf52840/NORDIC-LICENSE.txt.', '// Raw source facts; see system.hpp and doc/reference/nrf52840/README.md for limits.', '#include <mcubsp/core/register.hpp>', '', f'namespace mcubsp::mcu::nrf52840::registers::{namespace} {{']
        for rn, addr, r in regs:
            reg = ident(rn)
            access = r.findtext('access', orig.findtext('access', 'read-write'))
            fields_node = r.find('fields')
            fs = list(fields_node) if fields_node is not None else []
            writable = 0
            for f in fs:
                fa = field_access(name, rn, f, access)
                pos, width = field_bits(f)
                assert 0 <= pos < 32 and 0 < width <= 32 - pos
                if fa != 'read-only':
                    writable |= (1 << width) - 1 << pos
            if not fs and access != 'read-only':
                writable = 4294967295
            a = {'read-only': 'read_only', 'write-only': 'write_only', 'read-write': 'read_write', 'read-writeonce': 'read_write', 'writeOnce': 'write_only'}[access]
            modification = r.findtext('modifiedWriteValues')
            if modification == 'oneToClear':
                a = 'write_one_to_clear'
            elif modification == 'oneToSet':
                a = 'set_alias'
            read_action = r.findtext('readAction')
            if read_action == 'modifyExternal' and access == 'read-only':
                a = 'read_clear'
            elif read_action == 'modifyExternal' and access.startswith('read-write'):
                raise ValueError('read/write side-effect register needs semantic review')
            if rn.startswith('TASKS_'):
                a = 'command'
            elif rn.startswith('EVENTS_'):
                a = 'write_zero_to_clear'
            elif rn in ['INTENSET', 'CHENSET', 'EVTENSET', 'OUTSET', 'DIRSET'] or rn.endswith('_POWERSET'):
                a = 'set_alias'
            elif rn in ['INTENCLR', 'CHENCLR', 'EVTENCLR', 'OUTCLR', 'DIRCLR'] or rn.endswith('_POWERCLR'):
                a = 'clear_alias'
            elif rn in ['RESETREAS', 'LATCH', 'ERRORSRC']:
                a = 'write_one_to_clear'
            reset = num(r.findtext('resetValue', svd.findtext('resetValue')))
            mask = num(r.findtext('resetMask', svd.findtext('resetMask')))
            lines += [f'struct {reg} : core::register_descriptor<std::uint32_t, 0x{addr:X}U,', f'    core::access_semantics::{a}, 0x{writable:08X}U, false,', f"    core::reserved_bit_policy::write_zero, 0U, 0x{reset:08X}U, 0x{mask:08X}U> {{ static constexpr bool write_once = {str(access in ['read-writeonce', 'writeOnce']).lower()}; }};"]
            if fs:
                lines += [f'namespace {reg}_fields {{']
                for f in fs:
                    fn = ident(f.findtext('name'))
                    pos, width = field_bits(f)
                    assert 0 <= pos < 32 and 0 < width <= 32 - pos
                    fa = field_access(name, rn, f, access)
                    field_semantics = 'read_only' if fa == 'read-only' and access != 'read-only' else a
                    extra = f', core::access_semantics::{field_semantics}' if field_semantics != a else ''
                    lines += [f'using {fn} = core::field_descriptor<{reg}, {pos}U, {width}U{extra}>;']
                    for evs in f.findall('enumeratedValues'):
                        for ev in evs.findall('enumeratedValue'):
                            try:
                                v = num(ev.findtext('value'))
                            except (TypeError, ValueError):
                                continue
                            en = ident(ev.findtext('name'))
                            lines += [f'inline constexpr std::uint32_t {fn}_{en} = 0x{v:X}U;']
                lines += ['}']
        lines += ['}', '']
        (out / (namespace + '.hpp')).write_text('\n'.join(lines))
    counts[name] = {'registers': len(regs), 'fields': sum((len(r.findall('fields/field')) for _, _, r in regs))}
    irq = p.find('interrupt')
    irq = irq if irq is not None else orig.find('interrupt')
    inventory.append({'name': name, 'type': canonical, 'base': num(p.findtext('baseAddress')), 'irq': int(irq.findtext('value')) if irq is not None else -1, **counts[name]})
lines = ['#pragma once', '', '// @req-file REQ-0055', '#include <array>', '#include <cstdint>', '#include <string_view>']
lines += [f'#include <mcubsp/mcu/nrf52840/registers/{n}.hpp>' for n in types]
lines += ['', 'namespace mcubsp::mcu::nrf52840 {', 'struct instance_description { std::string_view name; std::uintptr_t base; int irq; unsigned registers; unsigned fields; };', 'inline constexpr std::array instance_inventory{']
for x in inventory:
    lines.append(f'''    instance_description{{"{x['name']}", 0x{x['base']:08X}U, {x['irq']}, {x['registers']}U, {x['fields']}U}},''')
lines += ['};', 'namespace instances {']
for x in inventory:
    n = ident(x['name'])
    lines += [f'struct {n} {{', f"    static constexpr std::uintptr_t base = 0x{x['base']:08X}U;", f"    static constexpr int irq = {x['irq']};", '};']
lines += ['}', '}', '']
(root / 'include/mcubsp/mcu/nrf52840/catalogue.hpp').write_text('\n'.join(lines))
manifest = {'schema_version': 1, 'review_date': '2026-10-27', 'baseline': 'nrfx v3.2.0', 'commit': '7ef620bedd3fd41828e0f81523a1d08a986b8a0e', 'sources': [], 'instances': inventory, 'open_gaps': ['This legacy SVD lacks 192 product-specification CryptoCell registers. product.hpp supplies the current definitions; product-register-audit.json reconciles the seven older CC_HOST_RGF absolute addresses. Current product/core tables and package bonding are separate generated inventories. Complete instance/mode/errata sequencing and actual board/stepping qualification remain open.', 'Actual connected silicon build/package and revision-specific observations remain unavailable: no SWD probe or application USB readback is present.', 'Not all documented modes and routes have independent product-specification review.']}
for f in (source / e['file'] for e in expected['sources']):
    if f.is_file() and f.suffix in ['.h', '.c', '.svd', '.dtsi']:
        manifest['sources'].append({'file': f.name, 'bytes': f.stat().st_size, 'sha256': hashlib.sha256(f.read_bytes()).hexdigest()})
manifest['semantic_overlays'] = [{'block': 'CLOCK', 'register': 'LFRCMODE', 'field': 'STATUS',
    'access': 'read-only', 'source': '4413_417 v1.11 5.4.3.25 p.170',
    'source_identity': 'product-specification.json', 'review_date': '2026-10-28'}]
(notice / 'source-manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
print(len(types), 'register families;', len(inventory), 'instances;', sum((x['registers'] for x in inventory)), 'expanded registers;', sum((x['fields'] for x in inventory)), 'fields')
if args.check:
    failed = []
    for path in sorted(root.rglob('*')):
        if path.is_file():
            relative = path.relative_to(root)
            existing = repository / relative
            if not existing.is_file() or existing.read_bytes() != path.read_bytes():
                failed.append(str(relative))
    if failed:
        print('import outputs differ: ' + ', '.join(failed), file=sys.stderr)
        sys.exit(1)
    print('Pinned import outputs match the checkout.')
