#!/usr/bin/env python3
"""PE-CH1 BTL1 TRACE_CONTRACT oracle (integration, not a new leaf).

Checks:
  1. Consume-cut EXE words via pe_ch1_299cc_oracle.py
  2. TRACE_CONTRACT column names and required BTL1 transition states
  3. Persist bank span D_800A77F0..+0x800
  4. If pc_port/build/btl1_trace.csv exists: row sequence, persist_hash
     unchanged on the 0x89 (mode6_request) row, consume 6→0
"""

from __future__ import annotations

import csv
import hashlib
import pathlib
import subprocess
import sys

COLS = [
    "field_tick",
    "field_scene",
    "field_script_pc",
    "encounter_id",
    "transition_state",
    "battle_mode",
    "formation_id",
    "player_state_hash",
    "persist_hash",
    "rng_state",
    "battle_tick",
]

REQUIRED_STATES = [
    "mailbox_3",
    "m0005i_enter",
    "rng_selected",
    "slots_ready",
    "mode6_request",
    "mode6_consumed",
]

PERSIST = 0x800A77F0
PERSIST_LEN = 0x800
RNG_SEED = [
    0x40, 0x10,
    2584, 1597, 987, 610, 377, 233, 144, 89, 55, 34,
    21, 13, 8, 5, 3, 2, 1,
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def main() -> int:
    root = pathlib.Path(__file__).resolve().parents[2]
    cut = subprocess.run(
        [sys.executable, str(root / "pc_port" / "tools" / "pe_ch1_299cc_oracle.py")],
        check=False,
    )
    require(cut.returncode == 0, "pe_ch1_299cc_oracle.py")
    hop = subprocess.run(
        [sys.executable, str(root / "pc_port" / "tools" / "pe_ch1_17bb4_oracle.py")],
        check=False,
    )
    require(hop.returncode == 0, "pe_ch1_17bb4_oracle.py")

    contract = (root / "docs" / "evidence" / "pe-btl0-field-battle-handoff"
                / "TRACE_CONTRACT.md").read_text()
    for col in COLS:
        require(col in contract, f"contract missing column {col}")
    require("mode6_consumed" in contract, "mode6_consumed")
    require("D_800A77F0" in contract, "persist bank")

    csv_path = root / "pc_port" / "build" / "btl1_trace.csv"
    require(csv_path.is_file(), f"missing {csv_path} (run pe-native-tests first)")

    with csv_path.open(newline="") as fh:
        rows = list(csv.DictReader(fh))
    require(rows, "empty CSV")
    require(list(rows[0].keys()) == COLS, f"columns {list(rows[0].keys())}")

    states = [r["transition_state"] for r in rows]
    for want in REQUIRED_STATES:
        require(want in states, f"missing state {want} in {states}")

    req = next(r for r in rows if r["transition_state"] == "mode6_request")
    cons = next(r for r in rows if r["transition_state"] == "mode6_consumed")
    enter = next(r for r in rows if r["transition_state"] == "m0005i_enter")
    rng = next(r for r in rows if r["transition_state"] == "rng_selected")
    require(enter["field_script_pc"] == "mod4+0x31", "real 0x31 trace row")
    rng_words = [int(word, 16) for word in rng["rng_state"].split(".")]
    require(rng_words == RNG_SEED, f"pre-0x1A RNG image {rng_words}")
    require(rng["formation_id"] == "49", "seeded 0x1A variant 49")
    require(req["battle_mode"] == "6", "0x89 row battle_mode")
    require(cons["battle_mode"] == "0", "consume row battle_mode")
    # persist_hash must match the row immediately before 0x89 as well.
    idx = states.index("mode6_request")
    require(idx > 0, "0x89 row must not be first")
    prev = rows[idx - 1]
    require(len(req["persist_hash"]) == 64, "sha256 hex")
    require(req["persist_hash"] == prev["persist_hash"],
            "persist_hash changed on 0x89 row")
    require(req["persist_hash"] == cons["persist_hash"],
            "persist_hash drifted after consume")
    require(cons["field_scene"] == "m0005i", "consume scene")
    require(cons["formation_id"].startswith("49;"), "selected variant carried")
    require("1332" in cons["formation_id"], "formation 1332")
    require("1333" in cons["formation_id"], "formation 1333")
    require("1334" in cons["formation_id"], "formation 1334")

    persist_bin = root / "pc_port" / "build" / "btl1_persist.bin"
    if persist_bin.is_file():
        blob = persist_bin.read_bytes()
        require(len(blob) == PERSIST_LEN, f"persist dump {len(blob)}")
        digest = hashlib.sha256(blob).hexdigest()
        require(digest == req["persist_hash"], "CSV persist_hash vs dump")

    csv4_path = root / "pc_port" / "build" / "btl1_trace_mailbox4.csv"
    require(csv4_path.is_file(), f"missing {csv4_path}")
    with csv4_path.open(newline="") as fh:
        rows4 = list(csv.DictReader(fh))
    require(rows4 and list(rows4[0].keys()) == COLS, "mailbox 4 columns")
    states4 = [r["transition_state"] for r in rows4]
    require("mailbox_4" in states4, "mailbox 4 trace row")
    for want in ["m0005i_enter", "rng_selected", "slots_ready",
                 "mode6_request", "mode6_consumed"]:
        require(want in states4, f"mailbox 4 missing {want}")
    enter4 = next(r for r in rows4 if r["transition_state"] == "m0005i_enter")
    rng4 = next(r for r in rows4 if r["transition_state"] == "rng_selected")
    req4 = next(r for r in rows4 if r["transition_state"] == "mode6_request")
    cons4 = next(r for r in rows4 if r["transition_state"] == "mode6_consumed")
    require(enter4["field_script_pc"] == "mod4+0x31", "mailbox 4 real 0x31")
    require([int(word, 16) for word in rng4["rng_state"].split(".")]
            == RNG_SEED, "mailbox 4 RNG image")
    require(req4["battle_mode"] == "6" and cons4["battle_mode"] == "0",
            "mailbox 4 consume 6->0")
    require(req4["persist_hash"] == cons4["persist_hash"],
            "mailbox 4 persist drift")
    require(cons4["formation_id"].startswith("49;"),
            "mailbox 4 selected variant")

    print("PASS: BTL1 mailbox 3+4 TRACE through mode6_consumed + "
          "persist_hash unchanged on 0x89")
    return 0


if __name__ == "__main__":
    sys.exit(main())
