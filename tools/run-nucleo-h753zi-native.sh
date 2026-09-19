#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
elf=${1:-"$project_dir/build/nucleo-h753zi/mcuhw_nucleo_h753zi_native_peripherals.elf"}
openocd=${OPENOCD_EXE:-openocd}
nm=${ARM_NM:-arm-none-eabi-nm}

if [ ! -f "$elf" ]; then
    echo "missing firmware: $elf" >&2
    echo "build it with: cmake --preset nucleo-h753zi && cmake --build build/nucleo-h753zi" >&2
    exit 2
fi

symbol_address() {
    "$nm" -n "$elf" | awk -v symbol="$1" \
        '$3 == symbol { print "0x" $1; found = 1; exit } END { if (!found) exit 1 }'
}

status_address=$(symbol_address mcuhw_nucleo_native_status)
arr_address=$(symbol_address mcuhw_nucleo_pwm_arr)
ccr_address=$(symbol_address mcuhw_nucleo_pwm_ccr)
adc_address=$(symbol_address mcuhw_nucleo_adc_sample)
brr_address=$(symbol_address mcuhw_nucleo_uart_brr)
cache_address=$(symbol_address mcuhw_nucleo_cache_buffer)
control_address=$(symbol_address mcuhw_nucleo_evidence_control)
evidence_address=$(symbol_address mcuhw_nucleo_evidence_buffer)
coverage_address=$(symbol_address mcuhw_nucleo_coverage_buffer)
control_complete_address=$(printf '0x%08x' $((control_address + 4)))
evidence_size_address=$(printf '0x%08x' $((control_address + 8)))
coverage_size_address=$(printf '0x%08x' $((control_address + 12)))
test_count_address=$(printf '0x%08x' $((control_address + 16)))
failure_count_address=$(printf '0x%08x' $((control_address + 20)))
build_id_address=$(printf '0x%08x' $((control_address + 24)))
run_dir=$(mktemp -d "${TMPDIR:-/tmp}/mcuhw-nucleo-h753zi-native.XXXXXX")
log="$run_dir/openocd.log"
evidence_raw="$run_dir/evidence-buffer.bin"
coverage_raw="$run_dir/coverage-buffer.bin"
evidence_dir="$project_dir/build/nucleo-h753zi/artifacts"
evidence_file="$evidence_dir/mcuhw_nucleo_h753zi_native_peripherals.json"
coverage_capture="$evidence_dir/mcuhw_nucleo_h753zi_native_peripherals.mcuv"
coverage_report="$evidence_dir/mcuhw_nucleo_h753zi_native_peripherals.mcucov.json"
coverage_html="$evidence_dir/mcuhw_nucleo_h753zi_native_peripherals.mcucov.html"
coverage_lcov="$evidence_dir/coverage.lcov"
coverage_manifest_filtered_dir="$run_dir/nucleo-target-manifests"

if ! "$openocd" -f interface/stlink.cfg -f target/stm32h7x.cfg \
    -c "adapter speed 4000; init; reset halt; program $elf verify; reset halt; resume; sleep 2500; halt; echo [capture \"mdw $status_address 1\"]; echo [capture \"mdw $arr_address 1\"]; echo [capture \"mdw $ccr_address 1\"]; echo [capture \"mdw $adc_address 1\"]; echo [capture \"mdw $brr_address 1\"]; echo [capture \"mdw $cache_address 4\"]; echo [capture \"mdw $control_address 1\"]; echo [capture \"mdw $control_complete_address 1\"]; echo [capture \"mdw $evidence_size_address 1\"]; echo [capture \"mdw $coverage_size_address 1\"]; echo [capture \"mdw $test_count_address 1\"]; echo [capture \"mdw $failure_count_address 1\"]; echo [capture \"mdw $build_id_address 1\"]; dump_image $evidence_raw $evidence_address 8192; dump_image $coverage_raw $coverage_address 32768; resume; shutdown" \
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
arr=$(value_at "$arr_address")
ccr=$(value_at "$ccr_address")
adc=$(value_at "$adc_address")
brr=$(value_at "$brr_address")
cache_first=$(value_at "$cache_address")
control_magic=$(value_at "$control_address")
control_complete=$(value_at "$control_complete_address")
evidence_size_hex=$(value_at "$evidence_size_address")
coverage_size_hex=$(value_at "$coverage_size_address")
test_count_hex=$(value_at "$test_count_address")
failure_count=$(value_at "$failure_count_address")
coverage_build_id=$(value_at "$build_id_address")

[ "$status" = "000000ff" ] || { echo "native status failed: $status" >&2; exit 1; }
[ "$arr" = "00000c7f" ] || { echo "TIM1 ARR mismatch: $arr" >&2; exit 1; }
[ "$ccr" = "00000320" ] || { echo "TIM1 CCR1 mismatch: $ccr" >&2; exit 1; }
[ "$brr" = "0000022c" ] || { echo "USART3 BRR mismatch: $brr" >&2; exit 1; }
[ "$cache_first" = "cace0000" ] || { echo "cache clean/readback mismatch: $cache_first" >&2; exit 1; }
[ $((0x$adc)) -le 16383 ] || { echo "ADC1 result out of 14-bit range: $adc" >&2; exit 1; }
[ "$control_magic" = "4d435543" ] || { echo "evidence control magic mismatch: $control_magic" >&2; exit 1; }
[ "$control_complete" = "00000001" ] || { echo "on-board mcutest/mcucov capture failed" >&2; exit 1; }
[ "$failure_count" = "00000000" ] || { echo "on-board test failures: $failure_count" >&2; exit 1; }
[ "$coverage_build_id" = "4d487501" ] || { echo "coverage build ID mismatch: $coverage_build_id" >&2; exit 1; }

evidence_size=$((0x$evidence_size_hex))
coverage_size=$((0x$coverage_size_hex))
test_count=$((0x$test_count_hex))
[ "$evidence_size" -gt 0 ] && [ "$evidence_size" -le 8192 ] \
    || { echo "invalid evidence size: $evidence_size" >&2; exit 1; }
[ "$coverage_size" -gt 0 ] && [ "$coverage_size" -le 32768 ] \
    || { echo "invalid coverage size: $coverage_size" >&2; exit 1; }
[ "$test_count" -eq 8 ] || { echo "unexpected on-board test count: $test_count" >&2; exit 1; }

mkdir -p "$evidence_dir"
dd if="$evidence_raw" of="$evidence_file.tmp" bs=1 count="$evidence_size" 2>/dev/null
mv "$evidence_file.tmp" "$evidence_file"
python3 -m json.tool "$evidence_file" >/dev/null
dd if="$coverage_raw" of="$coverage_capture.tmp" bs=1 count="$coverage_size" 2>/dev/null
mv "$coverage_capture.tmp" "$coverage_capture"

coverage_manifest=$(find \
    "$project_dir/build/nucleo-h753zi/mcucov/mcuhw_nucleo_h753zi_native_peripherals" \
    -maxdepth 1 -type f -name 'native_peripherals-*.mcucov.json' \
    | sort | sed -n '1p')
[ -n "$coverage_manifest" ] || { echo "mcucov manifest not found" >&2; exit 2; }
# Consolidate the board manifest and analysis-only backend manifests. Shared
# core/driver modules already belong to the cross-board UNO aggregate, so keep
# this report target-specific and give mcutrace one coverage node per source.
python3 -c '
import json, pathlib, sys
root = pathlib.Path(sys.argv[1])
output = pathlib.Path(sys.argv[2])
output.mkdir(parents=True, exist_ok=True)
for path in root.glob("mcuhw_nucleo_h753zi_*/*.mcucov.json"):
    if not (path.parent.name in ("mcuhw_nucleo_h753zi_native_peripherals",
                                "mcuhw_nucleo_h753zi_early_init_coverage")
            or path.parent.name.endswith("_coverage_manifest")):
        continue
    with path.open(encoding="utf-8") as source:
        manifest = json.load(source)
    selected = []
    for module in manifest.get("modules", []):
        source_path = module.get("path", "")
        if not (source_path.startswith("board/nucleo_h753zi/")
                or source_path.startswith("include/mcuhw/backend/stm32h753/")):
            continue
        selected.append(module)
    manifest["modules"] = selected
    destination_path = output / (path.parent.name + ".mcucov.json")
    with destination_path.open("w", encoding="utf-8") as destination:
        json.dump(manifest, destination, separators=(",", ":"))
' "$project_dir/build/nucleo-h753zi/mcucov" "$coverage_manifest_filtered_dir"
mcucov_exe=${MCUCOV_EXE:-}
if [ -z "$mcucov_exe" ]; then
    mcucov_exe=$(find "$project_dir/../mcucov/output/install" \
        -type f -path '*/bin/mcucov' | sort | sed -n '1p')
fi
[ -n "$mcucov_exe" ] && [ -x "$mcucov_exe" ] \
    || { echo "mcucov host tool not found; set MCUCOV_EXE" >&2; exit 2; }
"$mcucov_exe" report --input "$coverage_capture" \
    --manifest "$coverage_manifest_filtered_dir" --source-root "$project_dir" \
    --format json --output "$coverage_report"
"$mcucov_exe" report --input "$coverage_capture" \
    --manifest "$coverage_manifest_filtered_dir" --source-root "$project_dir" \
    --format html --output "$coverage_html"
"$mcucov_exe" report --input "$coverage_capture" \
    --manifest "$coverage_manifest_filtered_dir" --source-root "$project_dir" \
    --format lcov --output "$coverage_lcov"

vcp=${NUCLEO_VCP:-}
if [ -z "$vcp" ]; then
    set -- /dev/cu.usbmodem*
    if [ -e "$1" ]; then
        vcp=$1
    else
        set -- /dev/ttyACM*
        [ -e "$1" ] && vcp=$1
    fi
fi
[ -n "$vcp" ] || { echo "ST-LINK VCP not found; set NUCLEO_VCP" >&2; exit 1; }

"$project_dir/tools/read-serial-banner.py" \
    "$vcp" "MCUHW H753 USART3 OK"

echo "NUCLEO-H753ZI native peripheral test passed"
echo "status=$status PWM(ARR=$arr CCR1=$ccr) ADC=$adc USART3(BRR=$brr)"
echo "on-board mcutest: $test_count passed, coverage bytes: $coverage_size"
echo "mcutest evidence: $evidence_file"
echo "mcucov JSON: $coverage_report"
echo "mcucov HTML: $coverage_html"
echo "mcucov LCOV: $coverage_lcov"
echo "VCP=$vcp banner received; artifacts: $run_dir"
