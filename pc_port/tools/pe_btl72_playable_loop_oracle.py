#!/usr/bin/env python3
"""PE-BTL72 playable-loop TRACE oracle (integration, not a new leaf).

Proves 3F3C4 still jals 3EB04, then checks btl72_playable_loop.csv
through actors_captured. Does not import production C.
"""

from __future__ import annotations

import csv
import hashlib
import os
import pathlib
import struct
import subprocess
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path(__file__).resolve().parents[2]
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR = 0x80010000
HDR = 0x800

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
    "field",
    "mailbox_3",
    "m0005i_enter",
    "mode6_consumed",
    "hp_copied",
    "input_held",
    "actors_captured",
    "attack_available",
]
FORBIDDEN_STATES = [
    "hp_mutated",
    "encounter_complete",
    "post_return",
    "wait_mode7",
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def main() -> int:
    require(EXE.is_file(), f"missing {EXE}")
    blob = EXE.read_bytes()
    require(hashlib.sha1(blob).hexdigest() == SHA1, "EXE SHA-1")
    require(jal_target(load_u32(blob, 0x8003F40C)) == 0x8003EB04,
            "3F3C4 jal 3EB04")
    require(load_u32(blob, 0x8003EB04) != 0, "3EB04 present")

    contract = (
        ROOT / "docs" / "evidence" / "pe-btl0-field-battle-handoff"
        / "TRACE_CONTRACT.md"
    ).read_text()
    for col in COLS:
        require(col in contract, f"contract missing column {col}")
    for state in REQUIRED_STATES + FORBIDDEN_STATES:
        require(state in contract, state)

    csv_path = ROOT / "pc_port" / "build" / "btl72_playable_loop.csv"
    native = ROOT / "pc_port" / "build" / "pe-native-tests"
    if native.is_file():
        env = os.environ.copy()
        env["PE_TEST_FILTER"] = "BTL72"
        rc = subprocess.run([str(native)], cwd=str(ROOT), env=env, check=False)
        require(rc.returncode == 0, "pe-native-tests BTL72")
    require(csv_path.is_file(), f"missing {csv_path} (run pe-native-tests)")

    with csv_path.open(newline="") as fh:
        reader = csv.DictReader(fh)
        require(reader.fieldnames == COLS, f"header {reader.fieldnames}")
        rows = list(reader)
    got = [r["transition_state"] for r in rows]
    require(got == REQUIRED_STATES, f"row order {got}")
    for state in FORBIDDEN_STATES:
        require(state not in got, f"unexpected {state}")
    for row in rows:
        require(row["battle_mode"] == "0", "mode stays 0")
        require(row["encounter_id"] == "m0005i_mod6_350C", "first request id")
    hp = [r for r in rows if r["transition_state"] == "hp_copied"]
    require(len(hp) == 1, "one hp_copied row")
    require(len(hp[0]["player_state_hash"]) == 64, "sha256 hex")
    held = [r for r in rows if r["transition_state"] == "input_held"]
    captured = [r for r in rows if r["transition_state"] == "actors_captured"]
    require(held[0]["player_state_hash"] == hp[0]["player_state_hash"],
            "HP hash stable through input")
    require(captured[0]["player_state_hash"] == hp[0]["player_state_hash"],
            "HP hash stable through actors")
    attack = [r for r in rows if r["transition_state"] == "attack_available"]
    require(attack[0]["player_state_hash"] == hp[0]["player_state_hash"],
            "HP hash stable through 0x85 hit")
    print(
        "PASS: BTL72 TRACE field → mailbox_3 → m0005i_enter → "
        "mode6_consumed → hp_copied → input_held → actors_captured → "
        "attack_available; no hp_mutated/complete"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
