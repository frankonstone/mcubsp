#!/usr/bin/env python3
# @req-file REQ-0055 REQ-0073
"""Generate selected Cortex-M4F register descriptions from pinned Arm sources.

No vendor header is included by consumers. This imports declarative CMSIS facts
with Armv7-M/ETM-M4 semantic overlays and nRF52840 instance limits.
"""
import argparse
import ast
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
REF = Path('doc/reference/nrf52840')
BASES = {'NVIC': 0xE000E100, 'SCB': 0xE000ED00, 'SCnSCB': 0xE000E000,
         'SysTick': 0xE000E010, 'ITM': 0xE0000000, 'DWT': 0xE0001000,
         'TPI': 0xE0040000, 'MPU': 0xE000ED90, 'FPU': 0xE000EF30,
         'CoreDebug': 0xE000EDF0, 'FPB': 0xE0002000, 'ETM': 0xE0041000}
NAMES = {'SCnSCB': 'scnscb', 'SysTick': 'systick', 'CoreDebug': 'debug'}
EXCLUDED_IRQS = {30, 31, 43, 44, 46}


def ident(name):
    return name.lower()


def expression(source, names):
    source = re.sub(r'/\*.*?\*/', '', source)
    source = re.sub(r'(?<=[0-9A-Fa-f])[uUlL]+\b', '', source)
    node = ast.parse(source.strip(), mode='eval').body
    def evaluate(n):
        if isinstance(n, ast.Constant) and isinstance(n.value, int):
            return n.value
        if isinstance(n, ast.Name):
            return names[n.id]
        if isinstance(n, ast.BinOp):
            a, b = evaluate(n.left), evaluate(n.right)
            operations = {ast.LShift: lambda: a << b, ast.RShift: lambda: a >> b,
                          ast.BitOr: lambda: a | b, ast.BitAnd: lambda: a & b,
                          ast.Add: lambda: a + b, ast.Sub: lambda: a - b}
            return operations[type(n.op)]()
        if isinstance(n, ast.UnaryOp) and isinstance(n.op, ast.Invert):
            return ~evaluate(n.operand)
        raise ValueError('unsupported macro expression: ' + source)
    return evaluate(node)


def field(name, position, width, access=None):
    return dict(name=name, position=position, width=width, access=access)


def register(name, offset, access, fields=(), mask=None, reset=0, known=0, required=0, width=32):
    fs = list(fields)
    if mask is None:
        mask = 0
        if access != 'read_only':
            for f in fs:
                if f.get('access') not in ('read_only', 'read_clear'):
                    mask |= ((1 << f['width']) - 1) << f['position']
    return dict(name=name, offset=offset, access=access, fields=fs, writable_mask=mask,
                reset=reset, known=known, required=required, width=width)


def cmsis_blocks(text):
    definitions = {}
    for match in re.finditer(r'^#define\s+(\w+)\s+([^\n]+)', text, re.M):
        name, body = match.groups()
        if name.endswith(('_Pos', '_Msk')):
            try:
                definitions[name] = expression(body.split('/*!')[0], definitions)
            except (ValueError, KeyError, SyntaxError, TypeError):
                raise ValueError('unparsed core field macro: ' + name)
    # CMSIS 5.9 comments out the required shift for this read-only field.
    # Cortex-M4 TRM 11.3.12 defines SubType at bits[7:4], MajorType at [3:0].
    assert definitions['TPI_DEVTYPE_SubType_Pos'] == 4 and definitions['TPI_DEVTYPE_SubType_Msk'] == 15
    definitions['TPI_DEVTYPE_SubType_Msk'] = 0xF0
    blocks = {}
    for match in re.finditer(r'typedef struct\s*\{(.*?)^\}\s*(\w+)_Type;', text, re.M | re.S):
        body, block = match.groups()
        regs = []
        for row in body.splitlines():
            item = re.search(r'__(IM|OM|IOM)\s+(uint\d+_t)\s+(\w+)(?:\[(\d+)U?\])?;.*Offset:\s*(0x[0-9A-Fa-f]+)', row)
            if not item:
                continue
            permission, ctype, name, count, offset = item.groups()
            width = int(ctype[4:-2])
            if block == 'ITM' and name in ('u8', 'u16', 'u32'):
                continue
            if block == 'FPU' and name == 'MVFR2':
                continue # CMSIS superset; absent from selected M4 r0p1 register summary.
            count = int(count or '1')
            if block == 'NVIC' and name in ('ISER', 'ICER', 'ISPR', 'ICPR', 'IABR'):
                count = 2
            if block == 'NVIC' and name == 'IP':
                count = 48
            for index in range(count):
                if block == 'NVIC' and name == 'IP' and index in EXCLUDED_IRQS:
                    continue
                if block == 'SCB' and name == 'SHP' and index not in (0, 1, 2, 7, 8, 10, 11):
                    continue
                rn = name + str(index) if count > 1 else name
                stem = re.sub(r'\d+$', '', name) if block == 'DWT' else name
                if block == 'MPU':
                    stem = re.sub('_A[123]$', '', stem)
                prefix = f'{block}_{stem}_'
                fs = []
                for macro, mask in definitions.items():
                    if not macro.startswith(prefix) or not macro.endswith('_Msk'):
                        continue
                    fn = macro[len(prefix):-4]
                    position = definitions[macro[:-4] + '_Pos']
                    value = mask >> position
                    assert value > 0 and value & (value + 1) == 0, (macro, mask)
                    fs.append(field(fn, position, value.bit_length()))
                access = {'IM': 'read_only', 'OM': 'write_only', 'IOM': 'read_write'}[permission]
                if not fs:
                    fs = [field('VALUE', 0, width)]
                regs.append(register(rn, int(offset, 16) + index * (width // 8), access, fs, width=width))
        blocks[block] = regs
    assert set(blocks) == set(BASES) - {'FPB', 'ETM'}
    return blocks


def semantic_overlays(blocks):
    for block, registers in blocks.items():
        for r in registers:
            name, fields = r['name'], r['fields']
            if block == 'NVIC':
                if name.startswith(('ISER', 'ISPR')):
                    r['access'] = 'set_alias'
                if name.startswith(('ICER', 'ICPR')):
                    r['access'] = 'clear_alias'
                if name.startswith('IABR'):
                    r['access'] = 'read_only'
                if re.fullmatch(r'(?:ISER|ISPR|ICER|ICPR|IABR)\d', name):
                    word = int(name[-1])
                    r['writable_mask'] = 0 if name.startswith('IABR') else sum(
                        1 << bit for bit in range(32) if word * 32 + bit < 48 and word * 32 + bit not in EXCLUDED_IRQS)
                if name.startswith('IP'):
                    r['writable_mask'] = 0xE0
                    r['fields'] = [field('PRIORITY', 5, 3)]
                if name == 'STIR':
                    r['access'] = 'command'
            elif block == 'SCB':
                if name == 'CPACR':
                    r.update(fields=[field('CP10', 20, 2), field('CP11', 22, 2)], writable_mask=0xF00000)
                if name == 'ICSR':
                    r['access'], r['writable_mask'] = 'command', 0x9E000000
                    for f in fields:
                        if not f['name'].endswith(('SET', 'CLR')):
                            f['access'] = 'read_only'
                if name == 'AIRCR':
                    r.update(access='command', writable_mask=0x707, required=0x05FA0000)
                    for f in fields:
                        if f['name'] in ('VECTKEYSTAT', 'ENDIANESS'):
                            f['access'] = 'read_only'
                if name.startswith('SHP'):
                    r['writable_mask'] = 0xE0
                    r['fields'] = [field('PRIORITY', 5, 3)]
                if name in ('CFSR', 'HFSR', 'DFSR'):
                    r.update(access='write_one_to_clear', writable_mask={
                        'CFSR': 0x030FBFBB, 'HFSR': 0xC0000002, 'DFSR': 0x1F}[name])
            elif block == 'SysTick':
                if name == 'CTRL':
                    r.update(access='read_write_side_effect', writable_mask=7)
                    for f in fields:
                        if f['name'] == 'COUNTFLAG':
                            f['access'] = 'read_clear'
                if name == 'VAL':
                    r['access'] = 'command'
            elif block == 'DWT':
                if name == 'CTRL':
                    for f in fields:
                        if f['name'] in ('NUMCOMP', 'NOTRCPKT', 'NOEXTTRIG', 'NOCYCCNT', 'NOPRFCNT'):
                            f['access'] = 'read_only'
                if name.startswith('FUNCTION'):
                    r['access'] = 'read_write_side_effect'
                    for f in fields:
                        if f['name'] == 'MATCHED':
                            f['access'] = 'read_clear'
                        if f['name'] == 'LNK1ENA':
                            f['access'] = 'read_only'
                    # M4 TRM 9.2: cycle matching only comparator 0, data matching
                    # only comparator 1. Keep unavailable superset fields out.
                    fields[:] = [f for f in fields if not (
                        (f['name'] == 'CYCMATCH' and name != 'FUNCTION0') or
                        (f['name'] in ('DATAVMATCH', 'DATAVSIZE', 'DATAVADDR0', 'DATAVADDR1', 'LNK1ENA')
                         and name != 'FUNCTION1'))]
            elif block == 'ITM':
                if name == 'TCR':
                    for f in fields:
                        if f['name'] == 'BUSY':
                            f['access'] = 'read_only'
                if name == 'TPR':
                    r['writable_mask'] = 15
                    r['fields'] = [field('PRIVMASK', 0, 4)]
                if name == 'LAR':
                    r['access'] = 'command'
            elif block == 'TPI':
                if name == 'FFCR':
                    # M4 TRM 11.3.4: TrigIn is RAO, not a writable control.
                    r['writable_mask'] = 2
                    for f in fields:
                        if f['name'] == 'TrigIn':
                            f['access'] = 'read_only'
                if name in ('CLAIMSET', 'CLAIMCLR'):
                    r['access'] = 'set_alias' if name == 'CLAIMSET' else 'clear_alias'
                    r['writable_mask'] = 15
                    r['fields'] = [field('TAG', 0, 4)]
                if name == 'ITCTRL':
                    r['writable_mask'] = 1
                    r['fields'] = [field('IME', 0, 1)]
            elif block == 'CoreDebug':
                if name == 'DHCSR':
                    r.update(access='read_write_side_effect', writable_mask=0x2F, required=0xA05F0000)
                    for f in fields:
                        if f['name'].startswith('S_'):
                            f['access'] = 'read_clear' if f['name'] in ('S_RESET_ST', 'S_RETIRE_ST') else 'read_only'
                if name == 'DCRSR':
                    r.update(access='command', writable_mask=0x1007F)
                    next(f for f in fields if f['name'] == 'REGSEL')['width'] = 7
            # Field-status masks take precedence over broad CMSIS register masks.
            if (block == 'DWT' and name in ('CTRL', 'FUNCTION0', 'FUNCTION1', 'FUNCTION2', 'FUNCTION3')) or (block == 'ITM' and name == 'TCR'):
                r['writable_mask'] = 0
                for f in fields:
                    if f.get('access') not in ('read_only', 'read_clear'):
                        r['writable_mask'] |= ((1 << f['width']) - 1) << f['position']
    # ITM stimulus writes have independent 8/16/32-bit views; reads report FIFO
    # readiness, not previously transmitted payload. Never introduce an RMW view.
    for port in range(32):
        for width in (8, 16, 32):
            blocks['ITM'].append(register(f'PORT{port}_U{width}', port * 4, 'write_only',
                [field('DATA', 0, width)], mask=(1 << width) - 1, width=width))
        blocks['ITM'].append(register(f'PORT{port}_STATUS', port * 4, 'read_only', [field('READY', 0, 1)]))


def component_ids(blocks, text):
    # CMSIS omits the DWT/TPI identification registers listed by the M4 TRM.
    for block in ('DWT', 'TPI'):
        prefix = BASES[block]
        for match in re.finditer(r'^\s*(0xE00[0-9A-Fa-f]{5})\s+([PC]ID[0-7])\s+RO\s+(0x[0-9A-Fa-f]+)\b', text, re.M):
            address, name, reset = match.groups()
            offset = int(address, 16) - prefix
            if 0xFD0 <= offset <= 0xFFC:
                assert not any(r['offset'] == offset for r in blocks[block])
                blocks[block].append(register(name, offset, 'read_only',
                    [field('ID', 0, 8)], reset=int(reset, 16), known=0xFFFFFFFF))
        assert len([r for r in blocks[block] if r['offset'] >= 0xFD0]) == 12


def architecture_features(blocks):
    # DDI0403E.b B4.2--B4.6. MMFR1/MMFR3 contain only reserved bits in v7-M.
    names = {
        'PFR0': [('STATE0', 0), ('STATE1', 4)],
        'PFR1': [('M_PROFILE', 8)], 'DFR': [('M_PROFILE_DEBUG', 20)],
        'ADR': [(f'IMPLEMENTATION_{i}', i * 4) for i in range(4)],
        'MMFR0': [('PMSA', 4), ('OUTERMOST_SHAREABILITY', 8), ('SHAREABILITY_LEVELS', 12),
                  ('TCM', 16), ('AUXILIARY_REGISTERS', 20)],
        'MMFR1': [], 'MMFR2': [('WFI_STALL', 24)], 'MMFR3': [],
        'ISAR0': [('BIT_COUNT', 4), ('BITFIELD', 8), ('COMPARE_BRANCH', 12),
                  ('COPROCESSOR', 16), ('DEBUG', 20), ('DIVIDE', 24)],
        'ISAR1': [('EXTEND', 12), ('IF_THEN', 16), ('IMMEDIATE', 20), ('INTERWORK', 24)],
        'ISAR2': [('LOAD_STORE', 0), ('MEMORY_HINT', 4), ('MULTI_ACCESS_INTERRUPT', 8),
                  ('MULTIPLY', 12), ('MULTIPLY_SIGNED', 16), ('MULTIPLY_UNSIGNED', 20), ('REVERSAL', 28)],
        'ISAR3': [('SATURATE', 0), ('SIMD', 4), ('SVC', 8), ('SYNCHRONIZATION', 12),
                  ('TABLE_BRANCH', 16), ('THUMB_COPY', 20), ('TRUE_NOP', 24)],
        'ISAR4': [('UNPRIVILEGED', 0), ('WITH_SHIFTS', 4), ('WRITEBACK', 8), ('BARRIER', 16),
                  ('SYNCHRONIZATION_FRACTION', 20), ('PSR_M', 24)],
    }
    for r in blocks['SCB']:
        if r['name'] in names:
            r['fields'] = [field(name, position, 4) for name, position in names[r['name']]]


def fpb_registers():
    # Armv7-M C1.11 and Cortex-M4 TRM 8.3: six instruction, two literal comparators.
    result = [register('CTRL', 0, 'read_write', [field('REV', 28, 4, 'read_only'),
        field('NUM_CODE_HIGH', 12, 3, 'read_only'), field('NUM_LIT', 8, 4, 'read_only'),
        field('NUM_CODE_LOW', 4, 4, 'read_only'), field('KEY', 1, 1, 'write_only'),
        field('ENABLE', 0, 1)], mask=1, required=2, reset=0x260, known=0xFFFFFFFF),
        register('REMAP', 4, 'read_write', [field('RMPSPT', 29, 1, 'read_only'), field('REMAP', 5, 24)], mask=0x1FFFFFE0)]
    for index in range(8):
        fs = [field('COMP', 2, 27), field('ENABLE', 0, 1)]
        if index < 6:
            fs.insert(0, field('REPLACE', 30, 2))
        result.append(register(f'COMP{index}', 8 + index * 4, 'read_write', fs, known=1))
    for name, offset in [('PID4', 0xFD0), ('PID5', 0xFD4), ('PID6', 0xFD8), ('PID7', 0xFDC),
                         ('PID0', 0xFE0), ('PID1', 0xFE4), ('PID2', 0xFE8), ('PID3', 0xFEC),
                         ('CID0', 0xFF0), ('CID1', 0xFF4), ('CID2', 0xFF8), ('CID3', 0xFFC)]:
        result.append(register(name, offset, 'read_only', [field('ID', 0, 8)]))
    return result


def etm_registers(text):
    # M4-specific summary, not every ETMv3 architecture option. Presence is pinned
    # by this table; unknown reset values remain unknown rather than becoming zero.
    result = []
    for match in re.finditer(r'^\s*(0xE0041[0-9A-Fa-f]{3})\s+(\w+)\s+(0x[0-9A-Fa-f]+|-)\s+(RW|RO|WO)\b', text, re.M):
        address, name, reset, access = match.groups()
        r = register(name, int(address, 16) - BASES['ETM'],
                     {'RW': 'read_write', 'RO': 'read_only', 'WO': 'write_only'}[access],
                     [field('VALUE', 0, 32)], reset=int(reset, 16) if reset != '-' else 0,
                     known=0xFFFFFFFF if reset != '-' else 0)
        result.append(r)
    assert len(result) == 40, ('ETM-M4 register summary changed', len(result))
    by_name = {r['name']: r for r in result}
    event_fields = [field('FUNCTION', 14, 3), field('RESOURCE_B', 7, 7), field('RESOURCE_A', 0, 7)]
    for name in ('ETMTRIGGER', 'ETMTEEVR', 'ETMTSEVR'):
        by_name[name]['fields'] = [dict(f) for f in event_fields]
        by_name[name]['writable_mask'] = 0x1FFFF
    by_name['ETMCR']['fields'] = [field(name, pos, width) for name, pos, width in [
        ('TIMESTAMP_ENABLE', 28, 1), ('PORT_SIZE_HIGH', 21, 1), ('PORT_MODE_LOW', 16, 2),
        ('PORT_MODE_HIGH', 13, 1), ('PORT_SELECT', 11, 1), ('PROGRAMMING', 10, 1),
        ('DEBUG_REQUEST', 9, 1), ('BRANCH_OUTPUT', 8, 1), ('STALL', 7, 1),
        ('PORT_SIZE_LOW', 4, 3), ('POWERDOWN', 0, 1)]]
    by_name['ETMCR']['writable_mask'] = sum(((1 << f['width']) - 1) << f['position'] for f in by_name['ETMCR']['fields'])
    by_name['ETMSR'].update(fields=[field('TRIGGER', 3, 1), field('START_STOP', 2, 1),
                                  field('PROGRAMMING', 1, 1, 'read_only'),
                                  field('OVERFLOW', 0, 1, 'read_only')], writable_mask=12)
    by_name['ETMTECR1'].update(fields=[field('START_STOP_ENABLE', 25, 1)], writable_mask=1 << 25)
    by_name['ETMFFLR'].update(fields=[field('LEVEL', 0, 8)], writable_mask=255)
    by_name['ETMCNTRLDVR1'].update(fields=[field('RELOAD', 0, 16)], writable_mask=0xFFFF)
    by_name['ETMTESSEICR'].update(fields=[field('STOP', 16, 4), field('START', 0, 4)], writable_mask=0xF000F)
    descriptions = {
        'ETMCCR': [('ID_PRESENT', 31, 1), ('MEMORY_ACCESS', 27, 1), ('START_STOP_PRESENT', 26, 1),
                   ('CONTEXT_COMPARATORS', 24, 2), ('FIFO_FULL_PRESENT', 23, 1), ('EXTERNAL_OUTPUTS', 20, 3),
                   ('EXTERNAL_INPUTS', 17, 3), ('SEQUENCER', 16, 1), ('COUNTERS', 13, 3),
                   ('MEMORY_DECODERS', 8, 5), ('DATA_COMPARATORS', 4, 4), ('ADDRESS_PAIRS', 0, 4)],
        'ETMSCR': [('NO_FETCH', 17, 1), ('PROCESSORS_MINUS_ONE', 12, 3), ('PORT_MODE_SUPPORTED', 11, 1),
                   ('PORT_SIZE_SUPPORTED', 10, 1), ('MAX_PORT_HIGH', 9, 1), ('FIFO_FULL_SUPPORTED', 8, 1),
                   ('MAX_PORT_LOW', 0, 3)],
        'ETMIDR': [('IMPLEMENTER', 24, 8), ('BRANCH_ENCODING', 20, 1), ('SECURITY_EXTENSIONS', 19, 1),
                   ('THUMB32', 18, 1), ('LOAD_PC_FIRST', 16, 1), ('PROCESSOR_FAMILY', 12, 4),
                   ('MAJOR', 8, 4), ('MINOR', 4, 4), ('REVISION', 0, 4)],
        'ETMCCER': [('TIMESTAMP_SIZE', 29, 1), ('TIMESTAMP_BINARY', 28, 1), ('REDUCED_COUNTER', 27, 1),
                    ('TIMESTAMP_IMPLEMENTED', 22, 1), ('EMBEDDED_ICE_BEHAVIOR', 21, 1),
                    ('START_STOP_EMBEDDED_ICE', 20, 1), ('WATCHPOINT_INPUTS', 16, 4),
                    ('INSTRUMENTATION_RESOURCES', 13, 3), ('NO_DATA_ADDRESS_COMPARE', 12, 1),
                    ('READABLE_REGISTERS', 11, 1), ('EXTENDED_INPUT_BUS', 3, 8), ('EXTENDED_SELECTORS', 0, 3)],
        'ETMSYNCFR': [('FREQUENCY', 0, 12)],
        'ETMIDR2': [('SWP_ORDER', 1, 1), ('RFE_ORDER', 0, 1)],
        'ETMPDSR': [('POWERED_UP', 0, 1)],
        'ITMISCIN': [('COREHALT', 4, 1), ('EXTIN', 0, 2)],
        'ITTRIGOUT': [('TRIGGER', 0, 1)],
        'ETM_ITATBCTR2': [('ATREADY', 0, 1)],
        'ETM_ITATBCTR0': [('ATVALID', 0, 1)],
        'ETMLAR': [('KEY', 0, 32)],
        'ETMLSR': [('ACCESS_8BIT', 2, 1), ('LOCKED', 1, 1), ('PRESENT', 0, 1)],
        'ETMAUTHSTATUS': [('SECURE_NONINVASIVE', 6, 2), ('NONSECURE_NONINVASIVE', 2, 2)],
        'ETMDEVTYPE': [('SUBTYPE', 4, 4), ('MAJOR', 0, 4)],
    }
    for name, fs in descriptions.items():
        r = by_name[name]
        r['fields'] = [field(*f) for f in fs]
        if r['access'] != 'read_only':
            r['writable_mask'] = sum(((1 << f['width']) - 1) << f['position'] for f in r['fields'])
    for name, r in by_name.items():
        if re.fullmatch('ETM[PC]IDR[0-7]', name):
            r['fields'] = [field('ID', 0, 8)]
    # CCR external inputs depend on integration, rather than the reset-table
    # example (the detail text explicitly permits zero through two inputs).
    by_name['ETMCCR']['known'] &= ~(7 << 17)
    by_name['ETMTRACEIDR'].update(fields=[field('TRACE_ID', 0, 7)], writable_mask=127)
    by_name['ETMITCTRL'].update(fields=[field('INTEGRATION', 0, 1)], writable_mask=1)
    by_name['ETMLAR']['access'] = 'command'
    for name in ('ETMCLAIMSET', 'ETMCLAIMCLR'):
        by_name[name].update(access='set_alias' if name.endswith('SET') else 'clear_alias',
                             fields=[field('TAG', 0, 4)], writable_mask=15)
    return result


def generate(blocks):
    outputs, includes, inventory = {}, [], []
    for block, regs in blocks.items():
        namespace = NAMES.get(block, block.lower())
        includes.append(f'#include <mcubsp/mcu/nrf52840/core_registers/{namespace}.hpp>')
        lines = ['#pragma once', '// @req-file REQ-0055 REQ-0073',
                 '// CMSIS-derived facts: Copyright (c) 2009-2020 Arm Limited. Apache-2.0.',
                 '// Source identities and semantic review: doc/reference/nrf52840/core-facts.json.',
                 '// See ARM-LICENSE.txt. Selected nRF52840 Cortex-M4F limits apply.',
                 '#include <mcubsp/core/register.hpp>',
                 f'namespace mcubsp::mcu::nrf52840::core_registers::{namespace} {{',
                 f'inline constexpr std::uintptr_t base = 0x{BASES[block]:08X}U;']
        for r in regs:
            name = ident(r['name'])
            lines += [f'struct {name} : core::register_descriptor<std::uint{r["width"]}_t, 0x{r["offset"]:X}U,',
                      f'    core::access_semantics::{r["access"]}, 0x{r["writable_mask"]:X}U, false,',
                      f'    core::reserved_bit_policy::write_zero, 0x{r["required"]:X}U, 0x{r["reset"]:X}U, 0x{r["known"]:X}U> {{}};']
            if r['access'] in ('command', 'set_alias', 'clear_alias') and r['name'] not in ('STIR', 'DCRSR', 'LAR', 'ETMLAR'):
                lines += [f'using {name}_status = core::register_descriptor<std::uint{r["width"]}_t, 0x{r["offset"]:X}U,',
                          f'    core::access_semantics::read_only, 0U, false, core::reserved_bit_policy::write_zero,',
                          f'    0U, 0x{r["reset"]:X}U, 0x{r["known"]:X}U, {name}>;']
            lines += [f'namespace {name}_fields {{']
            for f in r['fields']:
                access = f.get('access') or r['access']
                lines += [f'using {ident(f["name"])} = core::field_descriptor<{name}, {f["position"]}U, {f["width"]}U, core::access_semantics::{access}>;']
            lines += ['}']
        lines += ['}', '']
        outputs[Path(f'include/mcubsp/mcu/nrf52840/core_registers/{namespace}.hpp')] = '\n'.join(lines)
        inventory.append(f'    block{{"{block}", 0x{BASES[block]:08X}U, {len(regs)}U}},')
    outputs[Path('include/mcubsp/mcu/nrf52840/core_registers.hpp')] = '\n'.join([
        '#pragma once', '// @req-file REQ-0055 REQ-0073', '#include <array>', '#include <string_view>',
        *includes, 'namespace mcubsp::mcu::nrf52840::core_registers {',
        'struct block { std::string_view name; std::uintptr_t base; unsigned registers; };',
        'inline constexpr std::array inventory{', *inventory, '};',
        'inline constexpr unsigned priority_bits = 3, external_irq_slots = 48, mpu_regions = 8;',
        'inline constexpr unsigned fpb_instruction_comparators = 6, fpb_literal_comparators = 2, dwt_comparators = 4;',
        '[[nodiscard]] constexpr bool core_register_selector(unsigned value) noexcept {',
        '    return value <= 18 || value == 20 || value == 33 || (value >= 64 && value <= 95);',
        '}', '}', ''])
    return outputs


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-dir', type=Path, required=True)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    if not __debug__:
        parser.error('assertions must be enabled')
    arm = json.loads((ROOT / REF / 'arm-references.json').read_text())
    sources = json.loads((ROOT / REF / 'source-manifest.json').read_text())['sources']
    cmsis = next(s for s in sources if s['file'] == 'core_cm4.h')
    for entry in [cmsis, *arm['sources']]:
        path = args.source_dir / entry['file']
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != entry['sha256']:
            parser.error('missing or changed pinned Arm source: ' + str(path))
    blocks = cmsis_blocks((args.source_dir / 'core_cm4.h').read_text())
    semantic_overlays(blocks)
    architecture_features(blocks)
    blocks['FPB'] = fpb_registers()
    # Text extraction is a maintainer prerequisite; no download/build dependency.
    import subprocess
    import tempfile
    with tempfile.TemporaryDirectory(prefix='mcubsp-arm-core-') as directory:
        core_text = Path(directory) / 'core.txt'
        subprocess.run(['pdftotext', '-layout', str(args.source_dir / 'arm-cortex-m4.pdf'), str(core_text)], check=True)
        component_ids(blocks, core_text.read_text())
        text = Path(directory) / 'etm.txt'
        subprocess.run(['pdftotext', '-layout', str(args.source_dir / 'arm-etm-m4.pdf'), str(text)], check=True)
        blocks['ETM'] = etm_registers(text.read_text())
    outputs = generate(blocks)
    facts = {'sources': ['nrfx v3.2.0 pinned CMSIS 5.9 core_cm4.h', *[s['document'] for s in arm['sources']]],
             'scope': 'Selected nRF52840 Cortex-M4F; debugger/provisioning/driver state remains explicitly owned.',
             'review': ['NVIC limits: 48 slots, reserved IRQ holes, three priority bits; IABR is read-only (DUI0553B 4.2.5).',
                        'DCRSR REGSEL is seven bits with FP extension (DDI0403E.b C1.6.3); CMSIS five-bit mask covers non-FP only.',
                        'TPI DEVTYPE SubType mask uses bits 7:4 (M4 TRM 11.3.12); CMSIS 5.9 omits its shift.',
                        'DWT LNK1ENA is read-only; cycle/data match fields are limited to comparators 0/1 (M4 TRM 9.2).',
                        'SysTick CTRL and DWT MATCHED/DHCSR sticky status reads have explicit side-effect access.',
                        'AIRCR, DHCSR and FP_CTRL key values are required; generic RMW is disabled.',
                        'FPU MVFR2 is a CMSIS superset entry absent from the selected M4 r0p1 TRM and is excluded.',
                        'FPB version-1 instruction/literal comparators differ; only ENABLE has a defined comparator reset bit.',
                        'ETM presence comes from the ETM-M4 summary, not the larger optional architecture inventory.'],
             'blocks': blocks}
    facts['limitations'] = [
                           'Register masks do not enforce cross-field programming sequences, runtime capabilities or debugger ownership.',
                           'ETM-M4 presence and integration-dependent reset values require silicon/trace-access confirmation.']
    outputs[REF / 'core-facts.json'] = json.dumps(facts, indent=2) + '\n'
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
        parser.error('core outputs differ: ' + ', '.join(changed))
    print(json.dumps({'blocks': len(blocks), 'registers': sum(map(len, blocks.values())),
                      'fields': sum(len(r['fields']) for regs in blocks.values() for r in regs), 'checked': args.check}))


if __name__ == '__main__':
    main()
