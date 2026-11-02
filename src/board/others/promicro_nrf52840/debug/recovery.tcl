# @req-file REQ-0072
# Nordic PS v1.11 4.8.3; OpenOCD 0.12 target/nrf52.cfg apreg contract.
# Loading this file performs no target/probe operation.
proc mcubsp_ctrlap_status {{dap nrf52.dap} {ap 1}} {
    if {![string is integer -strict $ap] || $ap < 0 || $ap > 255} {
        error "invalid CTRL-AP index"
    }
    set id [$dap apreg $ap 0xFC]
    if {$id != 0x02880000} { error "unreviewed CTRL-AP identity" }
    return [list id $id erase_busy [$dap apreg $ap 8] protected [$dap apreg $ap 12]]
}
proc mcubsp_ctrlap_recover {acknowledgement resources_relinquished {budget_ms 5000} {attempts 2} {poll_was_enabled 1} {dap nrf52.dap} {ap 1}} {
    if {$acknowledgement ne "ERASE_ALL_FLASH_UICR_RAM_AND_BOOTLOADER"} {
        error "explicit whole-device erase acknowledgement is required"
    }
    if {!$resources_relinquished} { error "exclusive debugger/reset ownership is required" }
    if {![string is integer -strict $budget_ms] || $budget_ms < 10 || $budget_ms > 30000 ||
        ![string is integer -strict $attempts] || $attempts < 1 || $attempts > 3} {
        error "invalid bounded recovery interval or retry count"
    }
    array set status [mcubsp_ctrlap_status $dap $ap]
    if {$status(erase_busy) != 0} { error "CTRL-AP erase is already busy; no command was issued" }
    poll off
    set result [catch {
        set success 0
        set polls [expr {$budget_ms / 10}]
        for {set attempt 0} {$attempt < $attempts && !$success} {incr attempt} {
            $dap apreg $ap 4 0
            $dap apreg $ap 4 1
            for {set n 0} {$n < $polls} {incr n} {
                if {[$dap apreg $ap 8] != 0} { error "CTRL-AP erase remains busy; no retry or reset was issued" }
            if {[$dap apreg $ap 8] == 0} { break }
                sleep 10
            }
            if {[$dap apreg $ap 8] != 0} { error "CTRL-AP erase remains busy; no retry or reset was issued" }
            if {[$dap apreg $ap 8] == 0} {
                $dap apreg $ap 0 1
                $dap apreg $ap 0 0
                $dap apreg $ap 4 0
                set success [expr {[$dap apreg $ap 12] == 1}]
            }
            # Anomaly 176: retries are explicit and bounded, never infinite.
        }
        if {!$success} { error "CTRL-AP recovery did not complete/unlock; device contents may be partially erased" }
    } message]
    if {$poll_was_enabled} { poll on } else { poll off }
    if {$result} { error $message }
    return "erased; reset/re-examine and explicitly restore the compatible bootloader/firmware"
}
