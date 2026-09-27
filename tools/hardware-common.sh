# Shared host entry point for board-local physical fixtures.
# The fixture sets project_dir before sourcing this file.
hardware() {
    "${MCUHW_PYTHON:-python3}" "$project_dir/tools/hardware.py" "$@"
}
