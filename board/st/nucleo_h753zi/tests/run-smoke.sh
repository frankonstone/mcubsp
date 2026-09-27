#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)
. "$project_dir/tools/hardware-common.sh"
elf=${1:-"$project_dir/build/nucleo-h753zi/mcuhw_nucleo_h753zi_startup_gpio_smoke.elf"}
build_dir=$(dirname -- "$elf")
nm=$(hardware tool --name arm-nm --command arm-none-eabi-nm --override-env ARM_NM)

if [ ! -f "$elf" ]; then
    echo "missing firmware: $elf" >&2
    echo "build it with: cmake --preset nucleo-h753zi && cmake --build --preset nucleo-h753zi" >&2
    exit 2
fi

symbol_address() {
    "$nm" -n "$elf" | awk -v symbol="$1" \
        '$3 == symbol { print "0x" $1; found = 1; exit } END { if (!found) exit 1 }'
}

status_address=$(symbol_address mcuhw_nucleo_smoke_status)
clock_address=$(symbol_address mcuhw_nucleo_clock_status)
vector_address=$(symbol_address mcuhw_nucleo_vector_table)
moder_address=$(symbol_address mcuhw_nucleo_gpio_moder)
odr_address=$(symbol_address mcuhw_nucleo_gpio_odr)
run_dir=$(mktemp -d "${TMPDIR:-/tmp}/mcuhw-nucleo-h753zi.XXXXXX")
log="$run_dir/openocd.log"

if ! hardware flash --board st/nucleo_h753zi --elf "$elf" --tool-environment OPENOCD_EXE \
    --commands "reset halt; resume; sleep 250; halt; echo [capture \"mdw $status_address 1\"]; echo [capture \"mdw $clock_address 1\"]; echo [capture \"mdw $vector_address 1\"]; echo [capture \"mdw $moder_address 1\"]; echo [capture \"mdw $odr_address 1\"]; resume; shutdown" \
    > "$log" 2>&1; then
    cat "$log"
    exit 1
fi
cat "$log"

value_at() {
    awk -v address="$1" \
        'tolower($1) == tolower(address ":") { print tolower($2); found = 1; exit } END { if (!found) exit 1 }' \
        "$log"
}

status=$(value_at "$status_address")
clock=$(value_at "$clock_address")
vector=$(value_at "$vector_address")
moder=$(value_at "$moder_address")
odr=$(value_at "$odr_address")

[ "$status" = "48570001" ] || { echo "smoke status failed: $status" >&2; exit 1; }
[ "$clock" = "00000000" ] || { echo "clock verification failed: $clock" >&2; exit 1; }
[ "$vector" = "08000000" ] || { echo "unexpected vector table: $vector" >&2; exit 1; }
[ $((0x$moder & 0xC)) -eq 4 ] || { echo "PE1 is not an output: $moder" >&2; exit 1; }
[ $((0x$odr & 0x2)) -eq 2 ] || { echo "PE1 output latch is not high: $odr" >&2; exit 1; }

echo "NUCLEO-H753ZI startup/GPIO smoke test passed"
echo "Yellow user LED LD2 is held on; artifacts: $run_dir"
