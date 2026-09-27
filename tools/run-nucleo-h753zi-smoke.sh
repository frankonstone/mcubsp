#!/bin/sh
# Compatibility command; the board owns its physical fixture.
set -eu
project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
exec sh "$project_dir/board/st/nucleo_h753zi/tests/run-smoke.sh" "$@"
