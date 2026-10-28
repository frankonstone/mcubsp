#!/usr/bin/env python3
"""Host Tcl trace sequencing tests. No probe, target, or USB device is accessed."""
# @req-file REQ-0073 REQ-0055
import json
from pathlib import Path
import shutil
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[2]
SCRIPT = ROOT / 'src/board/others/promicro_nrf52840/debug/trace.tcl'
PRELUDE = r'''
array set memory {
    0x10000100 0x52840 0xE000EDF0 0x20000 0x4000040C 0x10001
    0xE0041000 0x411 0xE0041010 2 0xE00411E4 0x4114F250
    0xE000EDFC 0x1000000 0xE0001000 1 0xE0001004 0x12345678
    0x4000055C 3 0xE0000E00 0x30 0xE0000E40 0 0xE0000E80 0
}
set writes {}
set respond 1
set fail_address 0
set reads 0
proc read_memory {address width count} {
    global memory reads
    incr reads
    if {[info exists memory($address)]} { return [list $memory($address)] }
    return [list 0]
}
proc mww {address value} {
    global memory writes respond fail_address
    if {$address >= 0xE0041004 && $address <= 0xE0041F00} {
        if {!($memory(0xE0041000) & 0x400) || !($memory(0xE0041010) & 2)} {
            error "ETM configuration write without programming acknowledgement"
        }
    }
    # Model an ambiguous failure after commit.
    lappend writes [list $address $value]
    set memory($address) $value
    if {$address == 0xE0041000 && $respond} {
        set memory(0xE0041010) [expr {($value & 0x400) ? 2 : 0}]
    }
    if {$address == 0xE0041010} {
        # PROGRAMMING status is read-only; writing SR must not clear it.
        set memory($address) [expr {($value & ~2) | (($memory(0xE0041000) & 0x400) ? 2 : 0)}]
    }
    if {$address == $fail_address} { error "injected write failure" }
}
proc tpiu {args} {}
proc mcubsp.tpiu {args} {
    global tpiu_calls
    lappend tpiu_calls $args
}
proc check {condition} {
    if {![uplevel 1 [list expr $condition]]} { error "assertion failed: $condition" }
}
'''


class DebugTrace(unittest.TestCase):
    def run_script(self, body):
        tcl = shutil.which('tclsh')
        self.assertIsNotNone(tcl, 'Tcl interpreter is required for debugger script verification')
        # Paths are Tcl data, never executable interpolation.
        encoded = str(SCRIPT).replace('\\', '\\\\').replace('}', '\\}')
        code = PRELUDE + f'\nsource {{{encoded}}}\n' + body
        code = 'if {[catch {\n' + code + '\n} failure]} {puts stderr $failure; exit 1}\n'
        result = subprocess.run([tcl], input=code, text=True, capture_output=True, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr + result.stdout)

    def test_loading_is_read_only_and_invalid_handoff_does_not_mutate(self):
        self.run_script(r'''
check {[llength $writes] == 0 && $reads == 0}
check {[catch {mcubsp_trace_start swo 0}]}
check {[catch {mcubsp_trace_start invalid 1}]}
check {[catch {mcubsp_trace_start swo 1 0}]}
check {[catch {mcubsp_trace_start swo 1 3000000}]}
check {[llength $writes] == 0 && !$mcubsp_trace_owned}
''')

    def test_swo_lifecycle_preserves_dwt_clock_and_restores_pin_mux(self):
        self.run_script(r'''
mcubsp_trace_start swo 1
check {$mcubsp_trace_owned && !$mcubsp_trace_etm}
check {$memory(0x4000055C) == 0x10000}
check {$memory(0xE0000E00) == 1}
check {[catch {mcubsp_trace_start parallel 1}]}
mcubsp_trace_stop
check {!$mcubsp_trace_owned}
check {$memory(0x4000055C) == 3 && $memory(0xE0000E00) == 0x30}
check {$memory(0xE0001000) == 1 && $memory(0xE0001004) == 0x12345678}
check {$memory(0xE000EDFC) & 0x1000000}
foreach w $writes { check {[lindex $w 0] != 0xE0001000 && [lindex $w 0] != 0xE0001004} }
''')

    def test_parallel_initializes_etmv3_registers_and_obeys_programming_handshake(self):
        self.run_script(r'''
mcubsp_trace_start parallel 1
check {$mcubsp_trace_owned && $mcubsp_trace_etm}
check {$memory(0x4000055C) == 0x20000}
check {$memory(0xE0041020) == 0x6F && $memory(0xE0041024) == 0}
check {$memory(0xE0041200) == 2 && !($memory(0xE0041000) & 0x401)}
set control 0x411
foreach w $writes {
    lassign $w address value
    if {$address == 0xE0041000} {
        # Powerdown clear and programming request change only their own bit.
        if {$control & 1} { check {($control ^ $value) == 1} }
        if {!($control & 0x400) && ($value & 0x400)} { check {($control ^ $value) == 0x400} }
        set control $value
    }
}
mcubsp_trace_stop
check {!$mcubsp_trace_owned && ($memory(0xE0041000) & 1)}
''')

    def test_foreign_owners_wrong_etm_and_unhalted_targets_are_rejected(self):
        self.run_script(r'''
set memory(0x5000071C) 0
foreach {address value} {0xE000EDF0 0 0xE0000E80 1 0x4000055C 0x10000 0xE00411E4 0x4114F450 0x4000040C 0} {
    set saved $memory($address)
    set memory($address) $value
    check {[catch {mcubsp_trace_start parallel 1}]}
    check {[llength $writes] == 0 && !$mcubsp_trace_owned}
    set memory($address) $saved
}
foreach invalid {0x600 0x10000} {
    set memory(0x5000071C) $invalid
    check {[catch {mcubsp_trace_start parallel 1}]}
    check {[llength $writes] == 0 && !$mcubsp_trace_owned}
}
''')

    def test_failed_start_and_stop_hold_ownership_until_explicit_recovery(self):
        self.run_script(r'''
set fail_address 0xE0000E80
check {[catch {mcubsp_trace_start swo 1}]}
check {$mcubsp_trace_owned}
check {[catch {mcubsp_trace_stop}]}
check {$mcubsp_trace_owned}
set fail_address 0
mcubsp_trace_stop
check {!$mcubsp_trace_owned}
''')

    def test_etm_timeout_is_bounded_and_allows_explicit_stop_retry(self):
        self.run_script(r'''
set respond 0
set memory(0xE0041010) 0
check {[catch {mcubsp_trace_start parallel 1}]}
check {$mcubsp_trace_owned && $reads < 1100}
set respond 1
mcubsp_trace_stop
check {!$mcubsp_trace_owned}
''')


if __name__ == '__main__':
    result = unittest.TextTestRunner(verbosity=2).run(unittest.defaultTestLoader.loadTestsFromTestCase(DebugTrace))
    failed = {test.id() for test, _ in result.failures + result.errors}
    print(json.dumps({'format': 'mcutest-results', 'version': 1,
        'summary': {'tests': result.testsRun, 'failures': len(failed)},
        'tests': [{'name': 'nrf_debug/' + name,
                   'status': 'failed' if f'__main__.DebugTrace.{name}' in failed else 'passed',
                   'requirements': ['REQ-0073']}
                  for name in unittest.defaultTestLoader.getTestCaseNames(DebugTrace)]}))
    raise SystemExit(not result.wasSuccessful())
