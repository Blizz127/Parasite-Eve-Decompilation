#!/usr/bin/env python3
"""Verify the evidence-correct m0003i/m0372i/m0004i camera trace."""

import csv
import json
import pathlib
import subprocess
import sys

COLS = [
    "field_scene", "field_script_pc", "opcode", "arg0", "arg1",
    "slot0_flags", "slot1_flags", "slot0_param", "slot1_param",
    "view_index", "view_h", "matrix_r11", "matrix_trx",
]
EXPECTED = [
    ("m0003i", "mod0+0x4D0", "0x7B", "0", "16384"),
    ("m0003i", "mod0+0x4E0", "0x7B", "1", "16384"),
    ("m0003i", "mod0+0x4F0", "0x75", "0", "1"),
    ("m0003i", "mod0+0x500", "0x75", "1", "1"),
    ("m0372i", "mod3+0xAE8", "0x82", "1", "unset"),
    ("m0004i", "mod0+0xF4", "0x82", "1", "unset"),
    ("m0005i", "mod0+0x6BC", "0x31", "0xA80002C8", "unset"),
]


def require(ok: bool, msg: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {msg}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    for script in ("pe_ch2_65954_oracle.py", "pe_ch2_659c8_oracle.py",
                   "pe_ch2_66800_oracle.py"):
        result = subprocess.run(
            [sys.executable, str(root / "pc_port" / "tools" / script)],
            check=False,
        )
        require(result.returncode == 0, script)

    census = json.loads((root / "local" / "btl0" / "census.json").read_text())
    by_opcode = {row["opcode"]: row for row in census}
    require("m0004i" not in by_opcode["0x75"]["scenes"], "0x75 not in m0004i")
    require("m0004i" not in by_opcode["0x7B"]["scenes"], "0x7B not in m0004i")
    require("m0004i" in by_opcode["0x82"]["scenes"], "0x82 in m0004i")
    samples75 = [(r["scene"], r["module"], r["pc"], r["args"])
                 for r in by_opcode["0x75"]["samples"]]
    samples7b = [(r["scene"], r["module"], r["pc"], r["args"])
                 for r in by_opcode["0x7B"]["samples"]]
    require(samples7b[:2] == [
        ("m0003i", 0, 0x4D0, [0, 16384]),
        ("m0003i", 0, 0x4E0, [1, 16384]),
    ], "m0003i 0x7B sequence")
    require(samples75[:2] == [
        ("m0003i", 0, 0x4F0, [0, 1]),
        ("m0003i", 0, 0x500, [1, 1]),
    ], "m0003i 0x75 sequence")

    path = root / "pc_port" / "build" / "ch2_prefix_camera_trace.csv"
    require(path.is_file(), f"missing {path}")
    with path.open(newline="") as fh:
        rows = list(csv.DictReader(fh))
    require(rows and list(rows[0]) == COLS, "trace columns")
    got = [tuple(row[key] for key in COLS[:5]) for row in rows]
    require(got == EXPECTED, f"trace route {got}")
    require(rows[1]["slot0_param"] == "64"
            and rows[1]["slot1_param"] == "64", "0x7B value>>8 stores")
    require(rows[3]["slot0_flags"] == "167"
            and rows[3]["slot1_flags"] == "254", "0x75 OR 6 stores")
    for row in rows[4:]:
        require(row["view_index"] == "1", "0x82 view index 1")
        require(row["view_h"] == "837", "synthetic view H")
        require(row["matrix_r11"] == "4352", "synthetic matrix R11")
        require(row["matrix_trx"] == "287454020", "synthetic matrix TRX")

    print("PASS: PE-CH2 m0003i 0x7B/0x75 + m0372i/m0004i 0x82 "
          "+ m0005i hop trace")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
