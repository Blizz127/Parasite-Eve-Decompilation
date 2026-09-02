#!/usr/bin/env python3
"""Independent B54K-AQ oracle: retail movie-player return contract and the
flag-gated native bypass.  Imports no production code; the frontier names
come from pc_port/configs/frontier.json."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from pe_frontier import load_frontier, strict_frontier  # noqa: E402

PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
BASE = 0x03D2 * 0x800
LOAD = 0x8018EFF0
PLAYER, PLAYER_END = 0x801924F8, 0x80192934
PLAYER_SHA = "ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a"
CALLER, CALLER_END = 0x80192CE8, 0x80192F98
CALLER_SHA = "ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7"


def need(ok: bool, msg: str) -> None:
    if not ok:
        raise SystemExit(f"FAIL: {msg}")


def u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, BASE + addr - LOAD)[0]


def run(argv, expected, env=None):
    r = subprocess.run(argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                       text=True, timeout=300, check=False, env=env)
    need(r.returncode == expected,
         f"exit {r.returncode}, expected {expected}\n{r.stdout[-2500:]}")
    return r.stdout


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("peimg")
    ap.add_argument("--port")
    ap.add_argument("--tests")
    ap.add_argument("--disc")
    a = ap.parse_args()
    root = pathlib.Path(__file__).resolve().parents[2]
    img = pathlib.Path(a.peimg).read_bytes()
    need(hashlib.sha1(img).hexdigest() == PEIMG_SHA1, "PE.IMG identity")

    body = img[BASE + PLAYER - LOAD:BASE + PLAYER_END - LOAD]
    need(hashlib.sha256(body).hexdigest() == PLAYER_SHA, "player identity")
    caller = img[BASE + CALLER - LOAD:BASE + CALLER_END - LOAD]
    need(hashlib.sha256(caller).hexdigest() == CALLER_SHA, "caller identity")

    # Retail return contract: bne v0,-1 -> success tail with v0 = 1 in the
    # delay slot; the tail never rewrites $v0 before `jr $ra`.
    need(u32(img, 0x80192870) == 0x1443001E and u32(img, 0x80192874) == 0x24020001,
         "success branch / delay-slot return value 1")
    tail = [u32(img, addr) for addr in range(0x801928EC, 0x80192934, 4)]
    need(not any(((w >> 16) & 0x1F) == 2 and (w >> 26) in (0x08, 0x09, 0x0F, 0x23, 0x24)
                 for w in tail) and
         not any((w >> 26) == 0 and ((w >> 11) & 0x1F) == 2 for w in tail),
         "success tail rewrites $v0")
    need(u32(img, 0x8019292C) == 0x03E00008, "tail jr $ra")
    # Early guard returns 0 for index >= 47.
    need(u32(img, 0x80192508) == 0x2E02002F and u32(img, 0x80192518) == 0x14400003 and
         u32(img, 0x80192520) == 0x08064A45 and u32(img, 0x80192524) == 0x00001021,
         "guard sltiu 47 / bnez / early return 0")
    # Sole caller ignores $v0: the next reads are D_800B0DBA then D_800B0DBC.
    need(u32(img, 0x80192E00) == (0x0C000000 | ((PLAYER >> 2) & 0x03FFFFFF)),
         "jal func_801924F8 at 0x80192E00")
    need(u32(img, 0x80192E08) == 0x3C03800B and u32(img, 0x80192E0C) == 0x24630DBA and
         u32(img, 0x80192E10) == 0x90620000 and u32(img, 0x80192E18) == 0x10400051 and
         u32(img, 0x80192E24) == 0x86020000 and u32(img, 0x80192E2C) == 0x1840004C,
         "caller reads D_800B0DBA/D_800B0DBC, not $v0")
    jal = 0x0C000000 | ((PLAYER >> 2) & 0x03FFFFFF)
    overlay = img[BASE:BASE + (0x0457 - 0x03D2) * 0x800]
    sites = [LOAD + i for i in range(0, len(overlay) - 3, 4)
             if struct.unpack_from("<I", overlay, i)[0] == jal]
    need(sites == [0x80192E00], "exactly one player call site")
    print("  OK retail: normal return 1, ignored by the sole caller; guard first")

    cfg = load_frontier(root)
    fb = cfg["fmv_bypass"]
    need(fb["return_value"] == 1 and fb["flag_env"] == "PE_PORT_SKIP_FMV" and
         fb["order_log_entry"] == "func_801924F8_fmv_bypass", "frontier.json bypass contract")
    default = strict_frontier(root, "default")
    skip = strict_frontier(root, "skip_fmv")
    need(default["cut"] != skip["cut"], "both frontiers must differ")

    src = (root / "pc_port/game/boot/func_801924F8_port.c").read_text()
    guard = src.index("record_index >= 47u")
    bypass = src.index("if (PE_Port_SkipFmv())")
    first_store = src.index("PE_StoreU8(0x800B0DBFu")
    need(guard < bypass < first_store, "bypass must follow the guard and precede every store")
    block = src[bypass:src.index("return 1;", bypass)]
    need("PE_Store" not in block and "func_" not in block.replace("func_801924F8_fmv_bypass", ""),
         "bypass block writes memory or calls retail code")
    need('Stub_Record("func_801924F8_fmv_bypass"' in src, "order-log entry name")
    for name in ("func_80081314_port.c", "func_80080D5C_port.c"):
        path = root / "pc_port/game/boot" / name
        if path.exists():
            need("SkipFmv" not in path.read_text(), f"{name} must not know the flag")
    for name in ("pe_libcd.c", "pe_stream.c"):
        need("SkipFmv" not in (root / "pc_port/platform" / name).read_text(),
             f"{name} must not know the flag")
    print("  OK source: single gate at the player entry; no CD shortcut")

    tests = pathlib.Path(a.tests) if a.tests else root / "pc_port/build/pe-native-tests"
    port = pathlib.Path(a.port) if a.port else root / "pc_port/build/parasite-eve-port"
    disc = pathlib.Path(a.disc) if a.disc else pathlib.Path(
        os.environ.get("PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KAQ"
    env.pop("PE_PORT_SKIP_FMV", None)
    out = run([str(tests)], 0, env)
    # stderr stub lines interleave with the TEST line, so count results.
    need("2 passed, 0 failed" in out and "flag_off... PASS" in out and
         "FAIL" not in out, "focused controls")

    off_env = os.environ.copy()
    off_env.pop("PE_PORT_SKIP_FMV", None)
    strict_off = run([str(port), "--headless", "--disc-image", str(disc), "--strict-stubs"],
                     1, off_env)
    need(default["cut"] in strict_off and
         f"called from: {default['caller']}" in strict_off and
         "func_801924F8_fmv_bypass" not in strict_off,
         "flag off: strict frontier moved or bypass fired")

    on_env = os.environ.copy()
    on_env["PE_PORT_SKIP_FMV"] = "1"
    trace = root / "pc_port/build/b54kaq_skip_fmv.trace"
    if trace.exists():
        trace.unlink()
    strict_on = run([str(port), "--headless", "--disc-image", str(disc), "--strict-stubs",
                     "--trace", str(trace)], 1, on_env)
    need(skip["cut"] in strict_on and f"called from: {skip['caller']}" in strict_on,
         "flag on: strict frontier is not the configured skip_fmv frontier")
    need(default["cut"] not in strict_on, "flag on: CdlReadS cut still reached")
    need("[STUB:FMV_BYPASS] func_801924F8_fmv_bypass" in strict_on, "flag on: bypass not logged")
    tr = trace.read_text()
    need(tr.count("func_801924F8_fmv_bypass") == 1, "flag on: trace lacks exactly one bypass event")
    need(tr.index("func_801924F8_fmv_bypass") > tr.index("call_func_8001220C"),
         "flag on: bypass event precedes boot")
    trace.unlink()
    print("  OK native: flag off keeps the CdlReadS frontier; flag on reaches", skip["cut"])
    print("\nB54K-AQ movie-bypass oracle: PASS.")


if __name__ == "__main__":
    main()
