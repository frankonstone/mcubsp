# Shared by board-owned physical procedures. Importing this file touches no device.
# @req REQ-0049 REQ-0050
hardware() {
    "${MCUBSP_PYTHON:-python3}" "$project_dir/tool/hardware.py" "$@"
}
