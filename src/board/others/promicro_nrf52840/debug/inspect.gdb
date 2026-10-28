# @req-file REQ-0055 REQ-0073
# Source after connecting to OpenOCD. No reset, programming, or auto-commands.
define mcubsp_identity
  printf "FICR INFO: PART, VARIANT, PACKAGE, RAM KiB, FLASH KiB\n"
  x/5wx 0x10000100
  printf "Nordic errata predicates: ROM part bits 7:0; ROM revision bits 7:4\n"
  x/wx 0xf0000fe0
  x/wx 0xf0000fe8
end
define mcubsp_debug_units
  printf "FP_CTRL (instruction count bits 7:4/14:12, literal count bits 11:8)\n"
  x/wx 0xe0002000
  printf "DWT_CTRL (comparator count bits 31:28); CYCCNT\n"
  x/2wx 0xe0001000
  printf "ETMIDR; TPIU supported parallel port widths\n"
  x/wx 0xe00411e4
  x/wx 0xe0040000
end
define mcubsp_faults
  printf "SCB CFSR, HFSR, DFSR, MMFAR, BFAR, AFSR\n"
  x/6wx 0xe000ed28
end
