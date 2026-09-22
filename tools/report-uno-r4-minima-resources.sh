#!/bin/sh
set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_dir="$project_dir/build/uno-r4-minima"
output=${1:-"$build_dir/artifacts/mcuhw_uno_r4_minima_resources.json"}
size_tool=${ARM_SIZE:-arm-none-eabi-size}
nm_tool=${ARM_NM:-arm-none-eabi-nm}

mkdir -p "$(dirname -- "$output")"
temporary=$(mktemp "${TMPDIR:-/tmp}/mcuhw-resources.XXXXXX")
identity_temporary=$(mktemp "${TMPDIR:-/tmp}/mcuhw-source-identity.XXXXXX")
identity_output=${output%.json}.source-identity.txt
trap 'rm -f "$temporary" "$identity_temporary"' EXIT INT TERM

for path in CMakeLists.txt CMakePresets.json cmake include \
            board/uno_r4_minima tests tools; do
    if [ -d "$project_dir/$path" ]; then
        find "$project_dir/$path" -type f -print
    elif [ -f "$project_dir/$path" ]; then
        printf '%s\n' "$project_dir/$path"
    fi
done | sort | while IFS= read -r source; do
    relative=${source#"$project_dir/"}
    digest=$(shasum -a 256 "$source" | awk '{print $1}')
    printf '%s  %s\n' "$digest" "$relative"
done > "$identity_temporary"
source_identity=$(shasum -a 256 "$identity_temporary" | awk '{print $1}')

first=true
{
    printf '{"format":"mcuhw-resource-report","version":1,'
    printf '"board":"Arduino UNO R4 Minima",'
    printf '"mcu":"Renesas RA4M1 R7FA4M1AB",'
    printf '"toolchain":"arm-none-eabi-gcc 15.2.1",'
    printf '"source_identity_sha256":"%s",' "$source_identity"
    printf '"source_identity_manifest":"%s",' \
        "$(basename -- "$identity_output")"
    printf '"images":['
    for suite in startup_gpio gpio_loopback pwm uart i2c spi adc dmac release_metrics; do
        elf="$build_dir/mcuhw_uno_r4_minima_${suite}"
        case "$suite" in
            release_metrics) ;;
            *) elf="${elf}_tests" ;;
        esac
        elf="${elf}.elf"
        [ -f "$elf" ] || {
            echo "missing ELF: $elf" >&2
            exit 2
        }
        flash=$($size_tool -A "$elf" | awk '
            $1 == ".vectors" || $1 == ".text" || $1 == ".ARM.extab" ||
            $1 == ".ARM.exidx" || $1 == ".init_array" ||
            $1 == "mcucov_modules" || $1 == ".data" { total += $2 }
            END { print total + 0 }')
        static_ram=$($size_tool -A "$elf" | awk '
            $1 == ".data" || $1 == ".bss" || $1 == ".noinit" {
                total += $2
            }
            END { print total + 0 }')
        image_end=$($nm_tool -n "$elf" | awk '$3 == "__image_ram_end" {print $1}')
        stack_limit=$($nm_tool -n "$elf" | awk '$3 == "__stack_limit" {print $1}')
        stack_top=$($nm_tool -n "$elf" | awk '$3 == "__stack" {print $1}')
        ram_headroom=$((0x$stack_limit - 0x$image_end))
        stack_reserve=$((0x$stack_top - 0x$stack_limit))
        digest=$(shasum -a 256 "$elf" | awk '{print $1}')
        $first || printf ','
        first=false
        instrumented=true
        [ "$suite" = release_metrics ] && instrumented=false
        printf '{"suite":"%s","elf":"%s","sha256":"%s",' \
            "$suite" "$(basename -- "$elf")" "$digest"
        printf '"mcucov_instrumented":%s,"flash_bytes":%s,' \
            "$instrumented" "$flash"
        printf '"static_ram_bytes":%s,"ram_headroom_before_stack":%s,' \
            "$static_ram" "$ram_headroom"
        printf '"stack_reserve_bytes":%s}' "$stack_reserve"
    done
    printf ']}'
} > "$temporary"
mv "$temporary" "$output"
mv "$identity_temporary" "$identity_output"
trap - EXIT INT TERM
printf '%s\n' "$output"
printf '%s\n' "$identity_output"
