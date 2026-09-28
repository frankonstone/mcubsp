"""Component-owned cross-build actions shared by local verification and CI.

Build actions never flash hardware. A missing tool is a failure, not a skipped
acceptance check. Infrastructure fixture tests remain a separate suite.
"""
from __future__ import annotations

import hashlib
from pathlib import Path, PurePosixPath

from . import boards as board_settings


def board_actions(root, boards):
    actions = []
    for identifier in sorted(set(boards)):
        board = board_settings.load(root, identifier)
        tools = set(board["build"].get("tools", []))
        if "esp-idf" in tools:
            environment = "esp-idf"
        elif "arm-gcc" in tools:
            environment = "arm-gcc"
        else:
            raise ValueError(f"board:{identifier}: no cross-build environment for build.tools")
        for example in sorted(board["examples"]):
            path = PurePosixPath(example)
            example_root = f'board/{identifier}/example/'
            if (not example.startswith(example_root) or path.parent.as_posix() != example_root.rstrip('/') or
                    path.suffix != '.cpp' or not path.stem.replace('_', '').isalnum() or
                    path.is_absolute() or '..' in path.parts or '\\' in example):
                raise ValueError(f"board:{identifier}: example {example!r} must name an example source under {example_root}")
            project = str(path.with_suffix(''))
            for revision in board["revisions"]:
                key = hashlib.sha256(f"{identifier}@{revision}:{project}".encode()).hexdigest()[:12]
                actions.append({"board": identifier, "revision": revision,
                                "example": project, "environment": environment, "key": key})
    return actions


def matrices(root, boards):
    actions = board_actions(root, boards)
    return {environment: {"include": [item for item in actions if item["environment"] == environment]}
            for environment in ("arm-gcc", "esp-idf")}
