// @req-file REQ-0058
// Explicit reset-test variant; the regular system demo keeps feeding the WDT.
#define MCUBSP_SYSTEM_WATCHDOG_TIMEOUT 1
#include "../../system/others/promicro_nrf52840.cpp"
