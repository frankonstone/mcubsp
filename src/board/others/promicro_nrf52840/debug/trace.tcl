# @req-file REQ-0073
# Load after OpenOCD target/nrf52.cfg. No target writes occur on loading.
# References: Nordic PS v1.11 4.8.6/5.4.3.24; Arm DDI 0440C 3.2-3.4;
# Cortex-M4 100166_0001_04_en ch.10/11; CMSIS 5.9 core_cm4.h.
# Caller explicitly relinquishes all trace units/pins while target is halted.
# FPB breakpoints and DWT watchpoints remain managed by GDB/OpenOCD.
tpiu create mcubsp.tpiu -dap nrf52.dap -ap-num 0 -baseaddr 0xE0040000
set mcubsp_trace_owned 0
set mcubsp_trace_etm 0

proc mcubsp_trace_read {address} {
    return [lindex [read_memory $address 32 1] 0]
}
proc mcubsp_trace_wait {address mask expected {budget 1024}} {
    for {set n 0} {$n < $budget} {incr n} {
        if {([mcubsp_trace_read $address] & $mask) == $expected} { return }
    }
    error [format "trace handshake timed out at 0x%08X" $address]
}
proc mcubsp_etm_program {} {
    mww 0xE0041FB0 0xC5ACCE55
    set control [mcubsp_trace_read 0xE0041000]
    # Clear POWERDOWN alone, then change only PROGRAMMING until SR acknowledges.
    if {$control & 1} {
        mww 0xE0041000 [expr {$control & ~1}]
        set control [mcubsp_trace_read 0xE0041000]
    }
    mww 0xE0041000 [expr {$control | 0x400}]
    mcubsp_trace_wait 0xE0041010 2 2
    return [mcubsp_trace_read 0xE0041000]
}
proc mcubsp_etm_start {} {
    if {([mcubsp_trace_read 0xE00411E4] & 0xFF0) != 0x250} {
        error "ETM is not the reviewed ETM-M4 v3.5 implementation"
    }
    set control [mcubsp_etm_program]
    # Initialize all configurable registers with no architected reset value.
    # Event 0x6F is TRUE; 0x406F is NOT(TRUE). No processor stalls or trigger IRQs.
    foreach {address value} {
        0xE0041008 0x406F 0xE0041010 0
        0xE0041020 0x6F   0xE0041024 0 0xE0041028 0
        0xE0041140 0      0xE00411F0 0 0xE00411F8 0x406F
        0xE0041200 2      0xE0041F00 0
    } { mww $address $value }
    # ETM ID 2, ITM ID 1. Keep PROGRAMMING set while changing other CR bits.
    set control [expr {($control & ~0x10000381) | 0xC00}]
    mww 0xE0041000 $control
    mww 0xE0041000 [expr {$control & ~0x400}]
    mcubsp_trace_wait 0xE0041010 2 0
}
proc mcubsp_trace_start {mode resources_relinquished {swo_hz 2000000}} {
    global mcubsp_trace_owned mcubsp_trace_etm mcubsp_trace_saved
    if {$mode ni {swo parallel}} { error "mode must be swo or parallel" }
    if {!$resources_relinquished} { error "explicit trace/pin handoff is required" }
    if {$mcubsp_trace_owned} { error "trace already owned; stop it first" }
    if {$mode eq "swo" && ($swo_hz <= 0 || 32000000 % $swo_hz || 32000000 / $swo_hz > 8192)} {
        error "SWO rate must exactly divide 32 MHz with a prescaler in 1..8192"
    }
    if {([mcubsp_trace_read 0x10000100] != 0x52840) ||
        !([mcubsp_trace_read 0xE000EDF0] & 0x20000)} {
        error "a halted nRF52840 target is required"
    }
    set mux [mcubsp_trace_read 0x4000055C]
    if {$mux & 0x30000} { error "trace pins already routed by another owner" }
    if {[mcubsp_trace_read 0xE0000E80] & 1} { error "ITM already enabled by another owner" }
    set etm [mcubsp_trace_read 0xE0041000]
    if {!($etm & 1) && !($etm & 0x400)} { error "ETM already running" }
    if {$mode eq "parallel" && ([mcubsp_trace_read 0xE00411E4] & 0xFF0) != 0x250} {
        error "ETM is not the reviewed ETM-M4 v3.5 implementation"
    }
    if {([mcubsp_trace_read 0x4000040C] & 0x10001) != 0x10001} {
        error "application must hold HFXO running for the capture"
    }
    set pins {0x50000A00}
    if {$mode eq "parallel"} { set pins {0x5000071C 0x50000A00 0x50000730 0x5000072C 0x50000A24} }
    foreach pin $pins {
        set configuration [mcubsp_trace_read $pin]
        set drive [expr {($configuration >> 8) & 7}]
        if {$drive ni {0 3} || ($configuration & 0x30000)} {
            error "trace pins require S0S1/H0H1 drive and disabled GPIO sensing"
        }
    }
    set mcubsp_trace_saved [list $mux [mcubsp_trace_read 0xE0000E00] [mcubsp_trace_read 0xE0000E40] [mcubsp_trace_read 0xE0000E80]]
    # Hold ownership before any potentially committed write. A failure requires stop/retry.
    set mcubsp_trace_owned 1
    set mcubsp_trace_etm [expr {$mode eq "parallel"}]
    mww 0xE000EDFC [expr {[mcubsp_trace_read 0xE000EDFC] | 0x1000000}]
    # TRCENA and CYCCNT are never disabled/reset: execution may borrow DWT.
    mww 0x4000055C [expr {($mux & ~0x30003) | ($mode eq "swo" ? 0x10000 : 0x20000)}]
    if {$mode eq "swo"} {
        mcubsp.tpiu configure -protocol uart -traceclk 32000000 -pin-freq $swo_hz -output external
    } else {
        mcubsp.tpiu configure -protocol sync -traceclk 32000000 -port-width 4 -formatter 1 -output external
    }
    mcubsp.tpiu enable
    mww 0xE0000FB0 0xC5ACCE55
    mww 0xE0000E40 0xF
    mww 0xE0000E00 1
    mww 0xE0000E80 [expr {0x1000D | ($mode eq "swo" ? 0x10 : 0)}]
    if {$mcubsp_trace_etm} { mcubsp_etm_start }
}
proc mcubsp_trace_stop {} {
    global mcubsp_trace_owned mcubsp_trace_etm mcubsp_trace_saved
    if {!$mcubsp_trace_owned} { return }
    if {!([mcubsp_trace_read 0xE000EDF0] & 0x20000)} { error "halt target before stopping trace" }
    mww 0xE0000FB0 0xC5ACCE55
    mww 0xE0000E00 0
    mww 0xE0000E80 [expr {[mcubsp_trace_read 0xE0000E80] & ~1}]
    mcubsp_trace_wait 0xE0000E80 0x800000 0
    if {$mcubsp_trace_etm} {
        set control [mcubsp_etm_program]
        mww 0xE0041000 [expr {$control & ~0x800}]
        mww 0xE0041000 [expr {($control & ~0x800) | 1}]
    }
    mcubsp.tpiu disable
    lassign $mcubsp_trace_saved mux ter tpr tcr
    mww 0xE0000E00 $ter
    mww 0xE0000E40 $tpr
    mww 0xE0000E80 $tcr
    mww 0x4000055C $mux
    set mcubsp_trace_owned 0
    set mcubsp_trace_etm 0
}
