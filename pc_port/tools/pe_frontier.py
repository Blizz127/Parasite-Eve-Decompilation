#!/usr/bin/env python3
"""Read the strict-frontier names from pc_port/configs/frontier.json.

Oracles must not hardcode cut names.  They call strict_frontier(root, mode)
and compare the returned cut/caller against the strict-mode diagnostic.
This module imports no production code.
"""

from __future__ import annotations

import json
import pathlib


def load_frontier(root: pathlib.Path) -> dict:
    path = pathlib.Path(root) / "pc_port" / "configs" / "frontier.json"
    with path.open() as handle:
        return json.load(handle)


def strict_frontier(root: pathlib.Path, mode: str = "default") -> dict:
    """Return {'cut': str, 'caller': str} for the named strict mode."""
    entry = load_frontier(root)["strict_frontier"][mode]
    if not entry.get("cut") or not entry.get("caller"):
        raise SystemExit(f"frontier.json: incomplete entry for {mode!r}")
    return entry


def frontier_reached(strict_output: str, root: pathlib.Path,
                     mode: str = "default") -> bool:
    entry = strict_frontier(root, mode)
    return (entry["cut"] in strict_output and
            f"called from: {entry['caller']}" in strict_output)


if __name__ == "__main__":
    import sys
    root = pathlib.Path(__file__).resolve().parents[2]
    mode = sys.argv[1] if len(sys.argv) > 1 else "default"
    entry = strict_frontier(root, mode)
    print(f"{entry['cut']} called from: {entry['caller']}")
