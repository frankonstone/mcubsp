#!/bin/sh
# Compatibility command; the board owns its physical fixture.
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
exec sh "$project_dir/board/waveshare/esp32p4_nano/tests/run-smoke.sh" "$@"
