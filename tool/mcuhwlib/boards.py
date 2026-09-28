"""Read a board's build settings from its local board.cmake entry."""
from __future__ import annotations

import json
from pathlib import Path
import subprocess
import tempfile

_run_cmake = subprocess.run


def load(root, identifier, revision=None):
    root = Path(root).resolve()
    with tempfile.TemporaryDirectory(prefix="mcuhw-board-") as temporary:
        output = Path(temporary) / "board.json"
        result = _run_cmake([
            "cmake", f"-DMCUHW_ROOT={root}", f"-DMCUHW_BOARD={identifier}",
            f"-DMCUHW_BOARD_INFO_OUTPUT={output}", "-P",
            str(root / "cmake/McuhwBoardInfo.cmake")], capture_output=True, text=True)
        if result.returncode:
            raise ValueError(result.stderr.strip())
        board = json.loads(output.read_text())
    if revision is not None and revision not in board["revisions"]:
        raise ValueError(f"Unsupported revision {revision!r} for {identifier}; supported: {', '.join(board['revisions'])}")
    return board


def discover(root):
    root = Path(root)
    return [load(root, f"{path.parent.parent.name}/{path.parent.name}")
            for path in sorted((root / "board").glob("*/*/board.cmake"))]
