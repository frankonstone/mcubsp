#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../../../../.." && pwd)
. "$project_dir/tool/hardware-common.sh"
dry_run=0
if [ "${1:-}" = --dry-run ]; then dry_run=1; shift; fi
selection=${1:-startup-gpio}
build_dir=${MCUBSP_BUILD_DIR:-"$project_dir/output/build/arduino-uno-r4-minima/src/board/arduino/uno_r4_minima/test"}

aggregate_evidence() {
    evidence_dir=$(hardware artifact-dir --elf "$build_dir/arduino.uno_r4_minima.test.startup_gpio.elf")
    combined_tests="$evidence_dir/mcutest-hardware.json"
    jq -s '{
        format: "mcutest-results",
        version: 1,
        run: {
            board: "Arduino UNO R4 Minima",
            suite: "all",
            aggregation: "per-suite J-Link RTT artifacts"
        },
        summary: {
            tests: (map(.summary.tests) | add),
            failures: (map(.summary.failures) | add),
            passed: (map(.summary.passed) | add),
            registry_overflow: (map(.summary.registry_overflow) | add)
        },
        tests: (map(.tests) | add),
        runs: map({
            run: .run,
            summary: .summary,
            hardware_observations: .hardware_observations
        })
    }' \
        "$evidence_dir/arduino.uno_r4_minima.test.startup_gpio.json" \
        "$evidence_dir/arduino.uno_r4_minima.test.gpio_loopback.json" \
        "$evidence_dir/arduino.uno_r4_minima.test.pwm.json" \
        "$evidence_dir/arduino.uno_r4_minima.test.uart.json" \
        "$evidence_dir/arduino.uno_r4_minima.test.i2c.json" \
        "$evidence_dir/arduino.uno_r4_minima.test.spi.json" \
        "$evidence_dir/arduino.uno_r4_minima.test.adc.json" \
        "$evidence_dir/arduino.uno_r4_minima.test.dmac.json" \
        "$evidence_dir/arduino.uno_r4_minima.test.release_metrics.json" \
        > "$combined_tests.tmp"
    mv "$combined_tests.tmp" "$combined_tests"
    echo "combined mcutest evidence: $combined_tests"
    mcucov_exe=$(hardware tool --name mcucov --command mcucov --override-env MCUCOV_EXE)
    aggregate_capture="$evidence_dir/mcubsp_uno_r4_minima_all_tests.mcuv"
    aggregate_json="$evidence_dir/mcubsp_uno_r4_minima_all_tests.mcucov.json"
    aggregate_html="$evidence_dir/mcubsp_uno_r4_minima_all_tests.mcucov.html"
    aggregate_lcov="$evidence_dir/coverage.lcov"
    "$mcucov_exe" merge \
        "$evidence_dir/arduino.uno_r4_minima.test.startup_gpio.mcuv" \
        "$evidence_dir/arduino.uno_r4_minima.test.gpio_loopback.mcuv" \
        "$evidence_dir/arduino.uno_r4_minima.test.pwm.mcuv" \
        "$evidence_dir/arduino.uno_r4_minima.test.uart.mcuv" \
        "$evidence_dir/arduino.uno_r4_minima.test.i2c.mcuv" \
        "$evidence_dir/arduino.uno_r4_minima.test.spi.mcuv" \
        "$evidence_dir/arduino.uno_r4_minima.test.adc.mcuv" \
        "$evidence_dir/arduino.uno_r4_minima.test.dmac.mcuv" \
        --output "$aggregate_capture"
    aggregate_manifests=$(mktemp -d "${TMPDIR:-/tmp}/mcubsp-uno-manifests.XXXXXX")
    "${MCUBSP_PYTHON:-python3}" "$project_dir/tool/coverage-manifests.py" \
        --output "$aggregate_manifests" \
        --input "$build_dir/mcucov/startup_gpio" \
        --input "$build_dir/mcucov/gpio_loopback" \
        --input "$build_dir/mcucov/pwm" --input "$build_dir/mcucov/uart" \
        --input "$build_dir/mcucov/i2c" --input "$build_dir/mcucov/spi" \
        --input "$build_dir/mcucov/adc" --input "$build_dir/mcucov/dmac"
    set -- --manifest "$aggregate_manifests"
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
}

if [ "$selection" = all ]; then
    for suite in startup-gpio gpio-loopback pwm uart i2c spi adc dmac release-metrics; do
        if [ "$dry_run" -eq 1 ]; then sh "$0" --dry-run "$suite"; else sh "$0" "$suite"; fi
    done
    if [ "$dry_run" -eq 0 ]; then aggregate_evidence; fi
    exit 0
fi

if [ "$selection" = aggregate ]; then
    aggregate_evidence
    exit 0
fi

case "$selection" in
    startup-gpio)
        target=arduino.uno_r4_minima.test.startup_gpio
        manifest_stem=startup_gpio ;;
    gpio-loopback)
        target=arduino.uno_r4_minima.test.gpio_loopback
        manifest_stem=gpio_loopback ;;
    pwm)
        target=arduino.uno_r4_minima.test.pwm
        manifest_stem=pwm ;;
    uart)
        target=arduino.uno_r4_minima.test.uart
        manifest_stem=uart ;;
    i2c)
        target=arduino.uno_r4_minima.test.i2c
        manifest_stem=i2c ;;
    spi)
        target=arduino.uno_r4_minima.test.spi
        manifest_stem=spi ;;
    adc)
        target=arduino.uno_r4_minima.test.adc
        manifest_stem=adc ;;
    dmac)
        target=arduino.uno_r4_minima.test.dmac
        manifest_stem=dmac ;;
    release-metrics)
        target=arduino.uno_r4_minima.test.release_metrics
        manifest_stem= ;;
    *.elf)
        elf=$selection
        target=$(basename "$elf" .elf)
        case "$target" in
            arduino.uno_r4_minima.test.startup_gpio) manifest_stem=startup_gpio ;;
            arduino.uno_r4_minima.test.gpio_loopback) manifest_stem=gpio_loopback ;;
            arduino.uno_r4_minima.test.pwm) manifest_stem=pwm ;;
            arduino.uno_r4_minima.test.uart) manifest_stem=uart ;;
            arduino.uno_r4_minima.test.i2c) manifest_stem=i2c ;;
            arduino.uno_r4_minima.test.spi) manifest_stem=spi ;;
            arduino.uno_r4_minima.test.adc) manifest_stem=adc ;;
            arduino.uno_r4_minima.test.dmac) manifest_stem=dmac ;;
            arduino.uno_r4_minima.test.release_metrics) manifest_stem= ;;
            *) manifest_stem= ;;
        esac ;;
    *)
        echo "unknown suite: $selection" >&2
        echo "use startup-gpio, gpio-loopback, pwm, uart, i2c, spi, adc, dmac, release-metrics, aggregate, all, or an ELF path" >&2
        exit 2 ;;
esac
elf=${elf:-"$build_dir/$target.elf"}
build_dir=$(dirname -- "$elf")
timeout_seconds=${MCUBSP_BOARD_TEST_TIMEOUT:-20}
if [ "$dry_run" -eq 1 ]; then
    hardware flash --board arduino/uno_r4_minima --elf "$elf" --tool-environment JLINK_EXE --dry-run
    exit
fi
rtt_logger=$(hardware tool --name jlink-rtt --command JLinkRTTLogger --override-env JLINK_RTT_LOGGER)
nm=$(hardware tool --name arm-nm --command arm-none-eabi-nm --override-env ARM_NM)
device=$(hardware runner-field --board arduino/uno_r4_minima --field device)

if [ ! -f "$elf" ]; then
    echo "missing firmware: $elf" >&2
    echo "build it with: cmake --preset arduino-uno-r4-minima -DMCUBSP_BUILD_BOARD_TESTS=ON -DMCUBSP_COVERAGE=ON && cmake --build --preset arduino-uno-r4-minima" >&2
    exit 2
fi

rtt_symbol=_ZN6mcubsp7testing3rtt16terminal_controlE
if ! rtt_address=$("$nm" -n "$elf" | awk -v symbol="$rtt_symbol" \
    '$3 == symbol { print toupper($1); found = 1; exit } END { if (!found) exit 1 }'); then
    echo "RTT control block symbol not found in firmware: $elf" >&2
    exit 2
fi

run_dir=$(mktemp -d "${TMPDIR:-/tmp}/mcubsp-uno-r4.XXXXXX")
flash_log="$run_dir/jlink-flash.log"
rtt_log="$run_dir/mcutest-rtt.log"
evidence_dir=$(hardware artifact-dir --elf "$build_dir/arduino.uno_r4_minima.test.startup_gpio.elf")
evidence_file="$evidence_dir/$target.json"
coverage_capture="$evidence_dir/$target.mcuv"
coverage_report="$evidence_dir/$target.mcucov.json"
coverage_html="$evidence_dir/$target.mcucov.html"
coverage_manifest_dir="$build_dir/mcucov/$manifest_stem"
coverage_manifest=
if [ -n "$manifest_stem" ]; then
    coverage_manifest="$run_dir/manifests"
    "${MCUBSP_PYTHON:-python3}" "$project_dir/tool/coverage-manifests.py" \
        --input "$coverage_manifest_dir" --output "$coverage_manifest"
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
    MCUBSP_RTT_EXE=$rtt_logger \
    MCUBSP_RTT_DEVICE=$device \
    MCUBSP_RTT_PROBE=$MCUBSP_PROBE_SERIAL \
    MCUBSP_RTT_ADDRESS=$rtt_address \
    MCUBSP_RTT_LOG=$rtt_log \
    expect -c '
        set timeout -1
        spawn -noecho $env(MCUBSP_RTT_EXE) \
            -Device $env(MCUBSP_RTT_DEVICE) -If SWD -Speed 4000 -USB $env(MCUBSP_RTT_PROBE) \
            -RTTAddress $env(MCUBSP_RTT_ADDRESS) -RTTChannel 0 \
            $env(MCUBSP_RTT_LOG)
        expect eof
    ' > "$run_dir/jlink-rtt.log" 2>&1 &
    logger_pid=$!
}

if ! hardware flash --board arduino/uno_r4_minima --elf "$elf" \
    --tool-environment JLINK_EXE > "$flash_log" 2>&1; then
    cat "$flash_log" >&2
    exit 1
fi

# Commander releases the probe immediately after starting the core. Give the
# firmware time to initialize its RTT control block before Logger validates it.
sleep 1

start_logger


elapsed=0
while [ "$elapsed" -lt "$timeout_seconds" ]; do
    if [ -f "$rtt_log" ] && grep -q 'MCUBSP_RESULT:' "$rtt_log"; then
        cat "$rtt_log"
        if grep -q '^MCUBSP_JSON:' "$rtt_log"; then
            mkdir -p "$evidence_dir"
            sed -n 's/^MCUBSP_JSON://p' "$rtt_log" | tail -n 1 \
                > "$evidence_file.tmp"
            mv "$evidence_file.tmp" "$evidence_file"
            echo "mcutrace evidence: $evidence_file"
        fi
        if grep -q '^MCUBSP_COVERAGE_HEX:' "$rtt_log"; then
            mkdir -p "$evidence_dir"
            sed -n 's/^MCUBSP_COVERAGE_HEX://p' "$rtt_log" | tail -n 1 \
                | xxd -r -p > "$coverage_capture.tmp"
            mv "$coverage_capture.tmp" "$coverage_capture"
            mcucov_exe=$(hardware tool --name mcucov --command mcucov --override-env MCUCOV_EXE)
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
        if grep -q 'MCUBSP_RESULT:PASS' "$rtt_log"; then
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
