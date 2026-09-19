#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
selection=${1:-startup-gpio}
if [ "$selection" = all ]; then
    for suite in startup-gpio gpio-loopback pwm uart i2c spi adc; do
        "$0" "$suite"
    done
    evidence_dir="$project_dir/build/uno-r4-minima/artifacts"
    mcucov_exe=${MCUCOV_EXE:-}
    if [ -z "$mcucov_exe" ]; then
        mcucov_exe=$(find "$project_dir/../mcucov/output/install" \
            -type f -path '*/bin/mcucov' | sort | sed -n '1p')
    fi
    if [ -z "$mcucov_exe" ] || [ ! -x "$mcucov_exe" ]; then
        echo "mcucov host tool not found; set MCUCOV_EXE" >&2
        exit 2
    fi
    aggregate_capture="$evidence_dir/mcuhw_uno_r4_minima_all_tests.mcuv"
    aggregate_json="$evidence_dir/mcuhw_uno_r4_minima_all_tests.mcucov.json"
    aggregate_html="$evidence_dir/mcuhw_uno_r4_minima_all_tests.mcucov.html"
    aggregate_lcov="$evidence_dir/coverage.lcov"
    "$mcucov_exe" merge \
        "$evidence_dir/mcuhw_uno_r4_minima_startup_gpio_tests.mcuv" \
        "$evidence_dir/mcuhw_uno_r4_minima_gpio_loopback_tests.mcuv" \
        "$evidence_dir/mcuhw_uno_r4_minima_pwm_tests.mcuv" \
        "$evidence_dir/mcuhw_uno_r4_minima_uart_tests.mcuv" \
        "$evidence_dir/mcuhw_uno_r4_minima_i2c_tests.mcuv" \
        "$evidence_dir/mcuhw_uno_r4_minima_spi_tests.mcuv" \
        "$evidence_dir/mcuhw_uno_r4_minima_adc_tests.mcuv" \
        --output "$aggregate_capture"
    set -- \
        --manifest "$project_dir"/build/uno-r4-minima/mcucov/mcuhw_uno_r4_minima_startup_gpio_tests/startup_gpio-*.mcucov.json \
        --manifest "$project_dir"/build/uno-r4-minima/mcucov/mcuhw_uno_r4_minima_gpio_loopback_tests/gpio_loopback-*.mcucov.json \
        --manifest "$project_dir"/build/uno-r4-minima/mcucov/mcuhw_uno_r4_minima_pwm_tests/pwm-*.mcucov.json \
        --manifest "$project_dir"/build/uno-r4-minima/mcucov/mcuhw_uno_r4_minima_uart_tests/uart-*.mcucov.json \
        --manifest "$project_dir"/build/uno-r4-minima/mcucov/mcuhw_uno_r4_minima_i2c_tests/i2c-*.mcucov.json \
        --manifest "$project_dir"/build/uno-r4-minima/mcucov/mcuhw_uno_r4_minima_spi_tests/spi-*.mcucov.json \
        --manifest "$project_dir"/build/uno-r4-minima/mcucov/mcuhw_uno_r4_minima_adc_tests/adc-*.mcucov.json
    "$mcucov_exe" report --input "$aggregate_capture" "$@" \
        --source-root "$project_dir" --format json --output "$aggregate_json"
    "$mcucov_exe" report --input "$aggregate_capture" "$@" \
        --source-root "$project_dir" --format html --output "$aggregate_html"
    "$mcucov_exe" report --input "$aggregate_capture" "$@" \
        --source-root "$project_dir" --format lcov --output "$aggregate_lcov"
    echo "aggregate mcucov capture: $aggregate_capture"
    echo "aggregate mcucov JSON: $aggregate_json"
    echo "aggregate mcucov HTML: $aggregate_html"
    echo "aggregate mcucov LCOV: $aggregate_lcov"
    exit 0
fi

case "$selection" in
    startup-gpio)
        target=mcuhw_uno_r4_minima_startup_gpio_tests
        manifest_stem=startup_gpio ;;
    gpio-loopback)
        target=mcuhw_uno_r4_minima_gpio_loopback_tests
        manifest_stem=gpio_loopback ;;
    pwm)
        target=mcuhw_uno_r4_minima_pwm_tests
        manifest_stem=pwm ;;
    uart)
        target=mcuhw_uno_r4_minima_uart_tests
        manifest_stem=uart ;;
    i2c)
        target=mcuhw_uno_r4_minima_i2c_tests
        manifest_stem=i2c ;;
    spi)
        target=mcuhw_uno_r4_minima_spi_tests
        manifest_stem=spi ;;
    adc)
        target=mcuhw_uno_r4_minima_adc_tests
        manifest_stem=adc ;;
    *.elf)
        elf=$selection
        target=$(basename "$elf" .elf)
        case "$target" in
            mcuhw_uno_r4_minima_startup_gpio_tests) manifest_stem=startup_gpio ;;
            mcuhw_uno_r4_minima_gpio_loopback_tests) manifest_stem=gpio_loopback ;;
            mcuhw_uno_r4_minima_pwm_tests) manifest_stem=pwm ;;
            mcuhw_uno_r4_minima_uart_tests) manifest_stem=uart ;;
            mcuhw_uno_r4_minima_i2c_tests) manifest_stem=i2c ;;
            mcuhw_uno_r4_minima_spi_tests) manifest_stem=spi ;;
            mcuhw_uno_r4_minima_adc_tests) manifest_stem=adc ;;
            *) manifest_stem= ;;
        esac ;;
    *)
        echo "unknown suite: $selection" >&2
        echo "use startup-gpio, gpio-loopback, pwm, uart, i2c, spi, adc, all, or an ELF path" >&2
        exit 2 ;;
esac
elf=${elf:-"$project_dir/build/uno-r4-minima/$target.elf"}
timeout_seconds=${MCUHW_BOARD_TEST_TIMEOUT:-20}
jlink=${JLINK_EXE:-JLinkExe}
rtt_logger=${JLINK_RTT_LOGGER:-JLinkRTTLogger}
nm=${ARM_NM:-arm-none-eabi-nm}
device=${MCUHW_JLINK_DEVICE:-R7FA4M1AB}

if [ ! -f "$elf" ]; then
    echo "missing firmware: $elf" >&2
    echo "build it with: cmake --preset uno-r4-minima && cmake --build --preset uno-r4-minima" >&2
    exit 2
fi

rtt_symbol=_ZN5mcuhw7testing3rtt16terminal_controlE
if ! rtt_address=$("$nm" -n "$elf" | awk -v symbol="$rtt_symbol" \
    '$3 == symbol { print toupper($1); found = 1; exit } END { if (!found) exit 1 }'); then
    echo "RTT control block symbol not found in firmware: $elf" >&2
    exit 2
fi

run_dir=$(mktemp -d "${TMPDIR:-/tmp}/mcuhw-uno-r4.XXXXXX")
command_file="$run_dir/flash.jlink"
flash_log="$run_dir/jlink-flash.log"
rtt_log="$run_dir/mcutest-rtt.log"
evidence_dir="$project_dir/build/uno-r4-minima/artifacts"
evidence_file="$evidence_dir/$target.json"
coverage_capture="$evidence_dir/$target.mcuv"
coverage_report="$evidence_dir/$target.mcucov.json"
coverage_html="$evidence_dir/$target.mcucov.html"
coverage_manifest_dir="$project_dir/build/uno-r4-minima/mcucov/$target"
coverage_manifest=
if [ -n "$manifest_stem" ]; then
    coverage_manifest=$(find "$coverage_manifest_dir" -maxdepth 1 \
        -type f -name "$manifest_stem-*.mcucov.json" \
        | sort | sed -n '1p')
fi
logger_pid=

cleanup() {
    if [ -n "$logger_pid" ]; then
        kill "$logger_pid" 2>/dev/null || true
        wait "$logger_pid" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

start_logger() {
    # The macOS RTT Logger build needs a terminal-backed session to initialize
    # the target connection reliably. expect(1) supplies a PTY without sending
    # the EOF that script(1) forwards in a background, non-interactive shell.
    MCUHW_RTT_EXE=$rtt_logger \
    MCUHW_RTT_DEVICE=$device \
    MCUHW_RTT_ADDRESS=$rtt_address \
    MCUHW_RTT_LOG=$rtt_log \
    expect -c '
        set timeout -1
        spawn -noecho $env(MCUHW_RTT_EXE) \
            -Device $env(MCUHW_RTT_DEVICE) -If SWD -Speed 4000 \
            -RTTAddress $env(MCUHW_RTT_ADDRESS) -RTTChannel 0 \
            $env(MCUHW_RTT_LOG)
        expect eof
    ' > "$run_dir/jlink-rtt.log" 2>&1 &
    logger_pid=$!
}

{
    echo "r"
    echo "h"
    echo "loadfile $elf"
    echo "r"
    echo "g"
    echo "exit"
} > "$command_file"

"$jlink" -Device "$device" -If SWD -Speed 4000 -AutoConnect 1 \
    -CommandFile "$command_file" > "$flash_log" 2>&1

# Commander releases the probe immediately after starting the core. Give the
# firmware time to initialize its RTT control block before Logger validates it.
sleep 1

start_logger

elapsed=0
while [ "$elapsed" -lt "$timeout_seconds" ]; do
    if [ -f "$rtt_log" ] && grep -q 'MCUHW_RESULT:' "$rtt_log"; then
        cat "$rtt_log"
        if grep -q '^MCUHW_JSON:' "$rtt_log"; then
            mkdir -p "$evidence_dir"
            sed -n 's/^MCUHW_JSON://p' "$rtt_log" | tail -n 1 \
                > "$evidence_file.tmp"
            mv "$evidence_file.tmp" "$evidence_file"
            echo "mcutrace evidence: $evidence_file"
        fi
        if grep -q '^MCUHW_COVERAGE_HEX:' "$rtt_log"; then
            mkdir -p "$evidence_dir"
            sed -n 's/^MCUHW_COVERAGE_HEX://p' "$rtt_log" | tail -n 1 \
                | xxd -r -p > "$coverage_capture.tmp"
            mv "$coverage_capture.tmp" "$coverage_capture"
            mcucov_exe=${MCUCOV_EXE:-}
            if [ -z "$mcucov_exe" ]; then
                mcucov_exe=$(find "$project_dir/../mcucov/output/install" \
                    -type f -path '*/bin/mcucov' | sort | sed -n '1p')
            fi
            if [ -z "$mcucov_exe" ] || [ ! -x "$mcucov_exe" ]; then
                echo "mcucov host tool not found; set MCUCOV_EXE" >&2
                exit 2
            fi
            "$mcucov_exe" report --input "$coverage_capture" \
                --manifest "$coverage_manifest" --source-root "$project_dir" \
                --format json --output "$coverage_report"
            "$mcucov_exe" report --input "$coverage_capture" \
                --manifest "$coverage_manifest" --source-root "$project_dir" \
                --format html --output "$coverage_html"
            echo "mcucov capture: $coverage_capture"
            echo "mcucov JSON: $coverage_report"
            echo "mcucov HTML: $coverage_html"
        elif [ -n "$coverage_manifest" ]; then
            echo "instrumented firmware produced no MCUCOV capture" >&2
            exit 1
        fi
        if grep -q 'MCUHW_RESULT:PASS' "$rtt_log"; then
            echo "artifacts: $run_dir"
            exit 0
        fi
        echo "artifacts: $run_dir" >&2
        exit 1
    fi
    if ! kill -0 "$logger_pid" 2>/dev/null; then
        wait "$logger_pid" 2>/dev/null || true
        logger_pid=
        sleep 1
        start_logger
    fi
    sleep 1
    elapsed=$((elapsed + 1))
done

cat "$flash_log" >&2
test ! -f "$rtt_log" || cat "$rtt_log" >&2
echo "timed out after ${timeout_seconds}s; artifacts: $run_dir" >&2
exit 3
