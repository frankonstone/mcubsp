#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)
. "$project_dir/tool/hardware-common.sh"
elf=${1:-"$project_dir/build/nucleo-h753zi/st.nucleo_h753zi.test.ethernet.elf"}
build_dir=$(dirname -- "$elf")
nm=$(hardware tool --name arm-nm --command arm-none-eabi-nm --override-env ARM_NM)

if [ ! -f "$elf" ]; then
    echo "missing firmware: $elf" >&2
    exit 2
fi

symbol_address() {
    "$nm" -n "$elf" | awk -v symbol="$1" \
        '$3 == symbol { print "0x" $1; found = 1; exit } END { if (!found) exit 1 }'
}

status_address=$(symbol_address mcuhw_nucleo_ethernet_status)
phy_id_address=$(symbol_address mcuhw_nucleo_ethernet_phy_id)
phy_status_address=$(symbol_address mcuhw_nucleo_ethernet_phy_status)
rx_address=$(symbol_address mcuhw_nucleo_ethernet_rx_frames)
replies_address=$(symbol_address mcuhw_nucleo_ethernet_replies)
rx_min_address=$(symbol_address mcuhw_nucleo_ethernet_rx_min)
rx_max_address=$(symbol_address mcuhw_nucleo_ethernet_rx_max)
link_transitions_address=$(symbol_address mcuhw_nucleo_ethernet_link_transitions)
control_address=$(symbol_address mcuhw_nucleo_ethernet_evidence_control)
evidence_address=$(symbol_address mcuhw_nucleo_ethernet_evidence_buffer)
coverage_address=$(symbol_address mcuhw_nucleo_ethernet_coverage_buffer)
control_complete_address=$(printf '0x%08x' $((control_address + 4)))
evidence_size_address=$(printf '0x%08x' $((control_address + 8)))
coverage_size_address=$(printf '0x%08x' $((control_address + 12)))
test_count_address=$(printf '0x%08x' $((control_address + 16)))
failure_count_address=$(printf '0x%08x' $((control_address + 20)))
build_id_address=$(printf '0x%08x' $((control_address + 24)))

run_dir=$(mktemp -d "${TMPDIR:-/tmp}/mcuhw-nucleo-h753zi-ethernet.XXXXXX")
log="$run_dir/openocd.log"
ping_log="$run_dir/ping.log"
evidence_raw="$run_dir/evidence-buffer.bin"
coverage_raw="$run_dir/coverage-buffer.bin"
artifact_dir="$(hardware artifact-dir --elf "$elf")"
evidence_file="$artifact_dir/st.nucleo_h753zi.test.ethernet.json"
coverage_capture="$artifact_dir/st.nucleo_h753zi.test.ethernet.mcuv"
coverage_report="$artifact_dir/st.nucleo_h753zi.test.ethernet.mcucov.json"
coverage_html="$artifact_dir/st.nucleo_h753zi.test.ethernet.mcucov.html"
coverage_lcov="$artifact_dir/ethernet-coverage.lcov"
manifest_dir="$run_dir/manifests"

# The host interface must already own an address in 192.168.75.0/24. The
# runner deliberately does not make privileged network-configuration changes.
if command -v ip >/dev/null 2>&1; then
    host_ipv4=$(ip -4 -o address show | awk '{ print $4 }')
else
    host_ipv4=$(ifconfig | awk '$1 == "inet" { print $2 }')
fi
case " $host_ipv4 " in
    *" 192.168.75."*) ;;
    *)
        echo "no host interface has an address in 192.168.75.0/24" >&2
        echo "configure the Ethernet peer before running this acceptance test" >&2
        exit 2
        ;;
esac

(sleep 2; ping -c 5 -s 0 192.168.75.30 >"$ping_log" 2>&1 || true; \
 ping -c 5 -s 1472 192.168.75.30 >>"$ping_log" 2>&1 || true; \
 ping -c 10 192.168.75.30 >>"$ping_log" 2>&1 || true) &
ping_pid=$!

if ! hardware flash --board st/nucleo_h753zi --elf "$elf" --tool-environment OPENOCD_EXE \
    --commands "reset halt; resume; sleep 20000; halt; echo [capture \"mdw $status_address 1\"]; echo [capture \"mdw $phy_id_address 1\"]; echo [capture \"mdw $phy_status_address 1\"]; echo [capture \"mdw $rx_address 1\"]; echo [capture \"mdw $replies_address 1\"]; echo [capture \"mdw $rx_min_address 1\"]; echo [capture \"mdw $rx_max_address 1\"]; echo [capture \"mdw $link_transitions_address 1\"]; echo [capture \"mdw $control_address 1\"]; echo [capture \"mdw $control_complete_address 1\"]; echo [capture \"mdw $evidence_size_address 1\"]; echo [capture \"mdw $coverage_size_address 1\"]; echo [capture \"mdw $test_count_address 1\"]; echo [capture \"mdw $failure_count_address 1\"]; echo [capture \"mdw $build_id_address 1\"]; dump_image $evidence_raw $evidence_address 8192; dump_image $coverage_raw $coverage_address 32768; resume; shutdown" \
    >"$log" 2>&1; then
    wait "$ping_pid" || true
    cat "$log"
    exit 1
fi
wait "$ping_pid" || true
cat "$log"
cat "$ping_log"

value_at() {
    awk -v address="$1" \
        'tolower($1) == tolower(address ":") { print tolower($2); found = 1; exit } END { if (!found) exit 1 }' \
        "$log"
}

status=$(value_at "$status_address")
phy_id=$(value_at "$phy_id_address")
phy_status=$(value_at "$phy_status_address")
rx=$(value_at "$rx_address")
replies=$(value_at "$replies_address")
rx_min=$(value_at "$rx_min_address")
rx_max=$(value_at "$rx_max_address")
link_transitions=$(value_at "$link_transitions_address")
control_magic=$(value_at "$control_address")
control_complete=$(value_at "$control_complete_address")
evidence_size_hex=$(value_at "$evidence_size_address")
coverage_size_hex=$(value_at "$coverage_size_address")
test_count_hex=$(value_at "$test_count_address")
failure_count=$(value_at "$failure_count_address")
build_id=$(value_at "$build_id_address")

[ "$status" = "0000000f" ] || { echo "Ethernet status failed: $status" >&2; exit 1; }
[ $(((0x$phy_id >> 16) & 0xffff)) -eq 7 ] || { echo "LAN8742 ID1 mismatch: $phy_id" >&2; exit 1; }
[ $((0x$phy_id & 0xfff0)) -eq $((0xc130)) ] || { echo "LAN8742 ID2 mismatch: $phy_id" >&2; exit 1; }
[ $((0x$phy_status & 4)) -eq 4 ] || { echo "Ethernet link is down: $phy_status" >&2; exit 1; }
[ $((0x$rx)) -gt 0 ] || { echo "no Ethernet frame received" >&2; exit 1; }
[ $((0x$replies)) -gt 0 ] || { echo "MCUIP produced no ARP/ICMP reply" >&2; exit 1; }
[ $((0x$rx_min)) -le 64 ] || { echo "minimum Ethernet frame not observed: $rx_min" >&2; exit 1; }
[ $((0x$rx_max)) -ge 1514 ] || { echo "maximum-MTU Ethernet frame not observed: $rx_max" >&2; exit 1; }
[ "$control_magic" = "4d435543" ] || { echo "evidence magic mismatch" >&2; exit 1; }
[ "$control_complete" = "00000001" ] || { echo "evidence capture incomplete" >&2; exit 1; }
[ "$failure_count" = "00000000" ] || { echo "on-board test failures: $failure_count" >&2; exit 1; }
[ "$build_id" = "4d487502" ] || { echo "coverage build ID mismatch: $build_id" >&2; exit 1; }

evidence_size=$((0x$evidence_size_hex))
coverage_size=$((0x$coverage_size_hex))
test_count=$((0x$test_count_hex))
[ "$evidence_size" -gt 0 ] && [ "$evidence_size" -le 8192 ] || exit 1
[ "$coverage_size" -gt 0 ] && [ "$coverage_size" -le 32768 ] || exit 1
[ "$test_count" -eq 3 ] || { echo "unexpected test count: $test_count" >&2; exit 1; }

mkdir -p "$artifact_dir" "$manifest_dir"
dd if="$evidence_raw" of="$evidence_file.tmp" bs=1 count="$evidence_size" 2>/dev/null
mv "$evidence_file.tmp" "$evidence_file"
python3 -m json.tool "$evidence_file" >/dev/null
dd if="$coverage_raw" of="$coverage_capture.tmp" bs=1 count="$coverage_size" 2>/dev/null
mv "$coverage_capture.tmp" "$coverage_capture"

find "$build_dir/mcucov" -type f -name '*.mcucov.json' \
    | while IFS= read -r manifest; do
        case "$manifest" in
            */st.nucleo_h753zi.test.ethernet/*|*/st.nucleo_h753zi.early_init_coverage/*|*/st.nucleo_h753zi.test.ethernet_coverage_manifest/*)
                cp "$manifest" "$manifest_dir/$(basename "$(dirname "$manifest")").json"
                ;;
        esac
    done

mcucov_exe=$(hardware tool --name mcucov --command mcucov --override-env MCUCOV_EXE)
"$mcucov_exe" report --input "$coverage_capture" --manifest "$manifest_dir" \
    --source-root "$project_dir" --format json --output "$coverage_report"
"$mcucov_exe" report --input "$coverage_capture" --manifest "$manifest_dir" \
    --source-root "$project_dir" --format html --output "$coverage_html"
"$mcucov_exe" report --input "$coverage_capture" --manifest "$manifest_dir" \
    --source-root "$project_dir" --format lcov --output "$coverage_lcov"

echo "NUCLEO-H753ZI Ethernet + MCUIP test passed"
echo "PHY=$phy_id status=$phy_status link-observations=$((0x$link_transitions)) RX=$((0x$rx)) replies=$((0x$replies)) sizes=$((0x$rx_min))..$((0x$rx_max))"
echo "evidence: $evidence_file"
echo "coverage: $coverage_report"
