#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
smoke_dir="$project_dir/board/esp32p4_nano/baremetal_smoke"
port=${1:-${MCUHW_ESP32P4_PORT:-/dev/cu.usbmodem5A7A0368561}}
timeout_seconds=${MCUHW_BOARD_TEST_TIMEOUT:-20}

if [ -z "${IDF_PATH:-}" ] || [ ! -f "$IDF_PATH/tools/idf.py" ]; then
    echo "activate ESP-IDF v6.1 before running this script" >&2
    exit 2
fi
if [ ! -e "$port" ]; then
    echo "ESP32-P4 serial port not found: $port" >&2
    exit 2
fi

python_exe=$(command -v python)
idf_py="$IDF_PATH/tools/idf.py"
mcucov_exe=${MCUCOV_EXE:-}
if [ -z "$mcucov_exe" ]; then
    mcucov_exe=$(find "$project_dir/../mcucov/output/install" \
        -type f -path '*/bin/mcucov' | sort | sed -n '1p')
fi
if [ -z "$mcucov_exe" ] || [ ! -x "$mcucov_exe" ]; then
    echo "mcucov host tool not found; set MCUCOV_EXE" >&2
    exit 2
fi
if ! command -v jq >/dev/null 2>&1; then
    echo "jq is required to normalize MCUCOV manifests" >&2
    exit 2
fi

"$python_exe" "$idf_py" -C "$smoke_dir" build
"$python_exe" "$idf_py" -C "$smoke_dir" -p "$port" flash

run_dir=$(mktemp -d "${TMPDIR:-/tmp}/mcuhw-esp32p4.XXXXXX")
trap 'rm -rf "$run_dir"' EXIT INT TERM
monitor_log="$run_dir/monitor.log"
normalized_log="$run_dir/monitor.txt"

MCUHW_MONITOR_LOG=$monitor_log \
MCUHW_MONITOR_PYTHON=$python_exe \
MCUHW_MONITOR_PORT=$port \
MCUHW_MONITOR_ELF="$smoke_dir/build/mcuhw_esp32p4_baremetal_smoke.elf" \
MCUHW_MONITOR_TIMEOUT=$timeout_seconds \
expect <<'EOF'
    set timeout $env(MCUHW_MONITOR_TIMEOUT)
    log_file -noappend $env(MCUHW_MONITOR_LOG)
    spawn -noecho $env(MCUHW_MONITOR_PYTHON) -m esp_idf_monitor \
        -p $env(MCUHW_MONITOR_PORT) -b 115200 --target esp32p4 \
        --revision 100 --disable-address-decoding $env(MCUHW_MONITOR_ELF)
    expect {
        -re {MCUHW_CONTEXT board=ESP32-P4-NANO .*qualification=development} {}
        timeout { exit 124 }
        eof { exit 125 }
    }
    expect {
        -re {MCUHW_RESULT status=(pass|fail) suite=esp32p4_native tests=([0-9]+) failures=([0-9]+) registry_overflow=([0-9]+) coverage=(captured|failed)} {}
        timeout { exit 124 }
        eof { exit 125 }
    }
    send "\035"
    expect eof
EOF

tr -d '\r' < "$monitor_log" > "$normalized_log"
cat "$normalized_log"

result=$(sed -n 's/^MCUHW_RESULT //p' "$normalized_log" | tail -n 1)
boot=$(sed -n 's/^MCUHW_BOOT //p' "$normalized_log" | tail -n 1)
coverage_hex=$(sed -n 's/^MCUHW_COVERAGE_HEX://p' "$normalized_log" | tail -n 1)
case "$boot" in
    status=load\ policy=factory\ offset=0x00010000\ size=0x00100000) ;;
    *)
        echo "ESP32-P4 custom bootloader handoff was not observed: ${boot:-missing boot record}" >&2
        exit 1 ;;
esac
if ! grep -q 'SPI Flash Size : 16MB' "$normalized_log"; then
    echo "ESP32-P4 image did not report the expected 16 MiB flash size" >&2
    exit 1
fi
if grep -q 'Starting scheduler' "$normalized_log"; then
    echo "ESP32-P4 smoke application unexpectedly started the scheduler" >&2
    exit 1
fi
if [ "$result" != "status=pass suite=esp32p4_native tests=5 failures=0 registry_overflow=0 coverage=captured" ]; then
    echo "ESP32-P4 smoke test did not pass: ${result:-missing result}" >&2
    exit 1
fi
if ! grep -q 'MCUHW_UART backend=esp32p4 uart=0 baud=115191 completion=tx_done' "$normalized_log"; then
    echo "ESP32-P4 UART0 transmission marker was not observed" >&2
    exit 1
fi
if ! grep -q 'MCUHW_I2C backend=esp32p4 i2c=0 address=0x18 device=ES8311 chip_id=0x8311 .*completion=repeated_start_read' "$normalized_log"; then
    echo "ESP32-P4 ES8311 identity transaction was not observed" >&2
    exit 1
fi
if ! grep -q 'MCUHW_AUDIO backend=esp32p4 i2s=0 codec=ES8311 rate=48000 format=philips16 capture_bytes=4032 mic_nonzero=[1-9][0-9]* .*playback_bytes=193536 tone_hz=1000 tone_peak=6000 duration_ms=1008 amp_gpio=53 verification=dma_codec_readback_no_acoustic_oracle' "$normalized_log"; then
    echo "ESP32-P4 ES8311 microphone/playback transaction was not observed" >&2
    exit 1
fi
if ! grep -q 'MCUHW_SPI backend=esp32p4 spi=2 rate=1000000 frames=4 segments=2 completion=trans_done target=none' "$normalized_log"; then
    echo "ESP32-P4 GPSPI2 transaction marker was not observed" >&2
    exit 1
fi
case "$coverage_hex" in
    ''|*[!0-9a-fA-F]*)
        echo "ESP32-P4 smoke test produced an invalid coverage frame" >&2
        exit 1 ;;
esac
if [ $((${#coverage_hex} % 2)) -ne 0 ]; then
    echo "ESP32-P4 coverage frame has an odd number of hex digits" >&2
    exit 1
fi

artifact_dir="$smoke_dir/build/artifacts"
mkdir -p "$artifact_dir"
printf '%s' "$coverage_hex" | xxd -r -p \
    > "$artifact_dir/esp32p4_native_smoke.mcuv.tmp"
mv "$artifact_dir/esp32p4_native_smoke.mcuv.tmp" \
    "$artifact_dir/esp32p4_native_smoke.mcuv"
cp "$normalized_log" "$artifact_dir/esp32p4_native_smoke.uart.txt"
jq -n --arg project_dir "$project_dir" '
    {
        format: "mcutest-results",
        version: 1,
        run: {
            board: "Waveshare ESP32-P4-NANO",
            mcu: "Espressif ESP32-P4NRW32 revision v1.0",
            suite: "esp32p4_native",
            firmware: "pln-0019-native-app-v9",
            fixture: "on-board CH343 USB-to-UART bridge, ES8311 codec, analog microphone and speaker; unconnected SPI header",
            interface: "direct USB UART0; I2C0 and I2S0 to onboard ES8311; GPSPI2 on GPIO20-GPIO23",
            toolchain: "riscv32-esp-elf GCC 15.2.0",
            qualification: "development",
            clocks_hz: {cpu: 360000000, apb: 90000000, uart_source: 40000000},
            coverage: {
                producer: "mcucov",
                profile: "full",
                build_id: 1296584713,
                transport_encoding: "UART hexadecimal frame"
            }
        },
        summary: {tests: 5, failures: 0, passed: 5, registry_overflow: 0},
        tests: [
            {
                name: "esp32p4_gpio/gpio45_initializes_toggles_and_releases",
                status: "passed",
                file: ($project_dir + "/board/esp32p4_nano/tests/gpio.cpp"),
                line: 35,
                requirements: ["REQ-0009", "REQ-0034", "REQ-0036"]
            },
            {
                name: "esp32p4_i2c/i2c0_reads_onboard_es8311_identity_with_repeated_start",
                status: "passed",
                file: ($project_dir + "/board/esp32p4_nano/tests/i2c.cpp"),
                line: 95,
                requirements: [
                    "REQ-0011", "REQ-0014", "REQ-0017", "REQ-0019",
                    "REQ-0024", "REQ-0034", "REQ-0036"
                ]
            },
            {
                name: "esp32p4_audio/captures_onboard_microphone_and_plays_audible_tone",
                status: "passed",
                file: ($project_dir + "/board/esp32p4_nano/tests/audio.cpp"),
                line: 186,
                requirements: [
                    "REQ-0009", "REQ-0010", "REQ-0011", "REQ-0013",
                    "REQ-0014", "REQ-0017", "REQ-0019", "REQ-0024",
                    "REQ-0034", "REQ-0036"
                ]
            },
            {
                name: "esp32p4_spi/gpspi2_executes_one_native_multisegment_command",
                status: "passed",
                file: ($project_dir + "/board/esp32p4_nano/tests/spi.cpp"),
                line: 49,
                requirements: [
                    "REQ-0010", "REQ-0011", "REQ-0014", "REQ-0017",
                    "REQ-0019", "REQ-0028", "REQ-0034", "REQ-0036"
                ]
            },
            {
                name: "esp32p4_uart/uart0_transmits_through_direct_usb_bridge",
                status: "passed",
                file: ($project_dir + "/board/esp32p4_nano/tests/uart.cpp"),
                line: 92,
                requirements: [
                    "REQ-0011", "REQ-0014", "REQ-0016", "REQ-0017",
                    "REQ-0019", "REQ-0027", "REQ-0034", "REQ-0036"
                ]
            }
        ],
        hardware_observations: {
            uart0_achieved_baud: 115191,
            uart0_completion: "tx_done",
            uart0_transport: "CH343 USB-to-UART",
            gpio: 45,
            i2c0: {
                sda_gpio: 7,
                scl_gpio: 8,
                requested_hz: 100000,
                target: "onboard ES8311 audio codec",
                address: "0x18",
                chip_id: "0x8311",
                registers: ["0xFD", "0xFE", "0xFF"],
                transaction: "register-select write, repeated START, one-byte read and STOP"
            },
            i2s0: {
                mclk_gpio: 13,
                bclk_gpio: 12,
                ws_gpio: 10,
                data_out_gpio: 9,
                data_in_gpio: 11,
                sample_rate_hz: 48000,
                format: "Philips I2S, stereo, 16-bit",
                target: "onboard ES8311 analog microphone and DAC",
                capture_bytes: 4032,
                playback_bytes: 193536,
                tone_hz: 1000,
                tone_peak: 6000,
                tone_duration_ms: 1008,
                amplifier_enable_gpio: 53,
                verification: "codec configuration plus completed RX/TX DMA; no independent acoustic oracle"
            },
            gpspi2: {
                miso_gpio: 20,
                mosi_gpio: 21,
                sck_gpio: 22,
                cs_gpio: 23,
                requested_hz: 1000000,
                frames: 4,
                segments: 2,
                completion: "TRANS_DONE with USR idle",
                target: "none; controller/register execution only"
            },
            board_changes: "none",
            deferred: "independent acoustic/logic-analyzer audio verification, SPI loopback/device and logic-analyzer capture, logic-analyzer I2C timing capture, UART RX and injected line errors"
        }
    }
' > "$artifact_dir/esp32p4_native_smoke.mcutest.json.tmp"
mv "$artifact_dir/esp32p4_native_smoke.mcutest.json.tmp" \
    "$artifact_dir/esp32p4_native_smoke.mcutest.json"

manifest_dir="$smoke_dir/build/mcucov/esp32p4_native_smoke"
if ! find "$manifest_dir" -maxdepth 1 -type f -name '*.mcucov.json' \
    | grep -q .; then
    echo "ESP32-P4 MCUCOV manifest not found" >&2
    exit 2
fi

# A FULL-profile translation unit can exceed the fixed JSON arena in the
# MCUCOV host reader. One manifest per source module is semantically identical
# when the report command merges the directory, and keeps every JSON document
# within that bound.
report_manifest_dir="$run_dir/manifests"
trace_manifest_dir="$run_dir/trace-manifests"
mkdir -p "$report_manifest_dir"
mkdir -p "$trace_manifest_dir"
for manifest in "$manifest_dir"/*.mcucov.json; do
    stem=${manifest##*/}
    stem=${stem%.mcucov.json}
    module_count=$(jq '.modules | length' "$manifest")
    if [ "$module_count" -eq 0 ]; then
        echo "ESP32-P4 MCUCOV manifest has no modules: $manifest" >&2
        exit 2
    fi
    module_index=0
    while [ "$module_index" -lt "$module_count" ]; do
        split_manifest="$report_manifest_dir/$stem-$module_index.mcucov.json"
        jq -c --argjson module_index "$module_index" \
            '{format, version, build_id, modules: [.modules[$module_index]]}' \
            "$manifest" > "$split_manifest"
        source_path=$(jq -r '.modules[0].path' "$split_manifest")
        case "$source_path" in
            board/esp32p4_nano/tests/*|include/mcuhw/backend/esp32p4/*)
                cp "$split_manifest" "$trace_manifest_dir/" ;;
        esac
        module_index=$((module_index + 1))
    done
done
"$mcucov_exe" report --input "$artifact_dir/esp32p4_native_smoke.mcuv" \
    --manifest "$report_manifest_dir" --source-root "$project_dir" --format json \
    --output "$artifact_dir/esp32p4_native_smoke.mcucov.json"
"$mcucov_exe" report --input "$artifact_dir/esp32p4_native_smoke.mcuv" \
    --manifest "$report_manifest_dir" --source-root "$project_dir" --format html \
    --output "$artifact_dir/esp32p4_native_smoke.mcucov.html"
"$mcucov_exe" report --input "$artifact_dir/esp32p4_native_smoke.mcuv" \
    --manifest "$trace_manifest_dir" --source-root "$project_dir" --format json \
    --output "$artifact_dir/esp32p4_native_smoke.trace.mcucov.json"

echo "mcucov capture: $artifact_dir/esp32p4_native_smoke.mcuv"
echo "mcutest evidence: $artifact_dir/esp32p4_native_smoke.mcutest.json"
echo "mcucov JSON: $artifact_dir/esp32p4_native_smoke.mcucov.json"
echo "mcucov HTML: $artifact_dir/esp32p4_native_smoke.mcucov.html"
echo "mcutrace MCUCOV JSON: $artifact_dir/esp32p4_native_smoke.trace.mcucov.json"
