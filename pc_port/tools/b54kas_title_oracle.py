#!/usr/bin/env python3
"""Independent B54K-AS oracle for the overlay title system.

1. Authenticates every retail span the translation claims (PE.IMG bytes).
2. Rebuilds the steady-state title frame from the overlay's own image data
   with a ByteModel written from the retail compositor semantics (packed
   dirty region, lighten blend dest = max(dest, src*alpha>>8), LoadImage
   placement), digests it, and requires both native display buffers to be
   byte-identical to the model after a real-disc run with PE_PORT_SKIP_FMV=1.
3. Checks the strict frontier from configs/frontier.json and the focused
   native tests.  Imports no production code."""

from __future__ import annotations

import argparse
import hashlib
import os
import pathlib
import struct
import subprocess
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from pe_frontier import strict_frontier  # noqa: E402

PEIMG_SHA1 = "146c0ce7308bf9fdc2ba5a84230e198db0663f3b"
BASE, LOAD = 0x03D2 * 0x800, 0x8018EFF0
SPANS = {
    "func_8018F2F4": (0x8018F2F4, 0x8018F468, "98797b654954055f64afe2c8dbd73b6e6656bf8750e60dbd9edc455886ed68d3"),
    "func_8018F468": (0x8018F468, 0x8018F7F0, "6f722cf555386eeed56ca586b89307f58145fc709761359a2ff0bc4a2e24a11b"),
    "func_8018F7F0": (0x8018F7F0, 0x8018F958, "e4f0123a8c35eacbf219ed9726830a339e904e798508ce4532cfedea19d8ab85"),
    "func_8018F958": (0x8018F958, 0x8018FBC0, "ac36c7979f939adcb2b298fb3833c258a6aab84e9254f10e5383bf21304b3c8a"),
    "func_8018FBC0": (0x8018FBC0, 0x8018FD04, "fe21ba8d190c86c040c75a805ba9a154aee145324b045ea1e3c3c31b5208e9ab"),
    "func_8018FD04": (0x8018FD04, 0x8018FE1C, "4dab35eb88099d67f7f3fbb0c3580df34c644b5cdca6aa417c621306f486c9b5"),
    "func_8018FE1C": (0x8018FE1C, 0x80190064, "e70cfcb9b2d4327776ee96a9a415c850684e8eed0762a4ff51bb85817e0181a5"),
    "func_80190064": (0x80190064, 0x80190660, "9234e8564f458e8c177093ddffc94e7de900c7523e9603c85707114b68b8c217"),
    "func_801909B4": (0x801909B4, 0x801918F8, "9072713338b26c335c1a31964282105dcd5f14951950c2d24585fa5948554d30"),
    "func_80192CE8": (0x80192CE8, 0x80192F98, "ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7"),
    "leaf_80192F98": (0x80192F98, 0x80192FE8, "e339ece19be959aaf7538119ef943b76bc1810afed05949f5123120b2f8191d4"),
    "leaf_80192FE8": (0x80192FE8, 0x80193084, "1b641bed974038f0ed4e35ed060d2827d9e4b7c44af35cd7e21b7f7a2471c74d"),
    "leaf_80193084": (0x80193084, 0x801930D8, "6150cf62f0fb7f42a1e51682ce4c253edaee314f82624bb14c6391be9e7bfadd"),
    "leaf_801930D8": (0x801930D8, 0x8019316C, "db86e9eaffeddab4d80032c1addb93a36031e8cf0ea641eaa5b5925682642cda"),
    "leaf_8019316C": (0x8019316C, 0x8019319C, "ab38ab30a1ccb884a587944369d287f26dd9dbc970fb8d6cf509b08f84123e0b"),
    "leaf_8019319C": (0x8019319C, 0x801931BC, "35d5b997c39024fb277cd3e709dddc260c16995db976f305f7316e1952a7115b"),
    "leaf_801931BC": (0x801931BC, 0x80193200, "7dee274602c8252507abb86d170fb5d85d0a5857e8163422422efb59f0fe7487"),
    "leaf_80193200": (0x80193200, 0x80193254, "39d79d014444c540ce5ab7e6c69e680282cb89793bffa7c19300a7726a2e4774"),
    "exit_80191410": (0x80191410, 0x80191548, "56760df8649a14491528bf9ee04633aac889fc90c9d3f9da18be0f53bae85cb4"),
    "exit_80191724": (0x80191724, 0x801918F8, "4293951edcd9e30490754a1c5809fc6039c5dce353cfc33a1feb75b37d557adf"),
}
IMAGE_BASE, IMAGE_TABLE, KIND_PARAMS = 0x80193254, 0x80193258, 0x801D0D5C
W, H = 320, 240


def need(ok, msg):
    if not ok:
        raise SystemExit(f"FAIL: {msg}")


def run(argv, expected, env=None):
    r = subprocess.run(argv, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                       text=True, timeout=600, check=False, env=env)
    need(r.returncode == expected, f"exit {r.returncode}, expected {expected}\n{r.stdout[-2500:]}")
    return r.stdout


class Overlay:
    def __init__(self, img: bytes):
        self.img = img

    def rd(self, addr, n):
        off = BASE + addr - LOAD
        return self.img[off:off + n]

    def u32(self, addr):
        return struct.unpack("<I", self.rd(addr, 4))[0]

    def s16(self, addr):
        return struct.unpack("<h", self.rd(addr, 2))[0]

    def s32(self, addr):
        return struct.unpack("<i", self.rd(addr, 4))[0]


def title_model(ov: Overlay) -> bytes:
    """Steady-state title frame (320x240x3) per the retail algorithm."""
    frame = bytearray(W * H * 3)                      # ClearImage black bars
    bg = IMAGE_BASE + ov.u32(IMAGE_TABLE)
    need(ov.s16(bg + 0x10) == 480 and ov.s16(bg + 0x12) == 204, "background geometry")
    frame[0x14 * 960:(0x14 + 204) * 960] = ov.rd(bg + 0x14, 204 * 960)   # func_8018F2F4

    # Task kinds present at steady state: 1 (spawned by the loop) and 2
    # (spawned by kind 1's handler), both at alpha 0x100.
    sprites = []
    for kind in (1, 2):
        hdr = IMAGE_BASE + ov.u32(IMAGE_TABLE + kind * 4)
        params = KIND_PARAMS + kind * 12
        x, y = ov.s32(params), ov.s32(params + 4)
        units, h = ov.s16(hdr + 0x10), ov.s16(hdr + 0x12)
        w = int((units * 2) / 3)                      # mult 0x55555556 idiom
        sprites.append((kind, hdr + 0x14, x, y, w, h))
    ux = min(s[2] for s in sprites)
    uy = min(s[3] for s in sprites)
    uw = max(s[2] + s[4] for s in sprites) - ux
    uh = max(s[3] + s[5] for s in sprites) - uy
    row_words = int(uw * 3 / 4)
    # packed region from the background (func_8018F468 copy loop)
    src_bytes = ((uy - 0x14) * 320 + ux) * 3
    src = bg + 0x14 + int(src_bytes / 4) * 4
    region = bytearray()
    for _ in range(uh):
        region += ov.rd(src, row_words * 4)
        src += row_words * 4 + (0xF0 - row_words) * 4
    # blit each sprite (func_8018F7F0 at alpha 0x100: dest = max(dest, src))
    for kind, pixels, x, y, w, h in sprites:
        words = int(w * 3 / 4)
        dest = int((((y - uy) * uw + (x - ux)) * 3) / 4) * 4
        skip = int(((uw - w) * 3) / 4)
        sp = pixels
        for _ in range(h):
            for _ in range(words):
                word = ov.rd(sp, 4)
                if word != b"\0\0\0\0":
                    for k in range(4):
                        region[dest + k] = max(region[dest + k], word[k])
                sp += 4
                dest += 4
            dest += skip * 4
    # LoadImage of the dirty rect: x -> (x*3)>>1 halfwords = x*3 bytes
    for r in range(uh):
        off = (uy + r) * 960 + ((ux * 3) >> 1) * 2
        frame[off:off + row_words * 4] = region[r * row_words * 4:(r + 1) * row_words * 4]
    return bytes(frame)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("peimg")
    ap.add_argument("--port")
    ap.add_argument("--tests")
    ap.add_argument("--disc")
    ap.add_argument("--keep-vram")
    a = ap.parse_args()
    root = pathlib.Path(__file__).resolve().parents[2]
    img = pathlib.Path(a.peimg).read_bytes()
    need(hashlib.sha1(img).hexdigest() == PEIMG_SHA1, "PE.IMG identity")
    ov = Overlay(img)
    for name, (s, e, sha) in SPANS.items():
        need(hashlib.sha256(ov.rd(s, e - s)).hexdigest() == sha, f"span {name}")
    # structural words the translation depends on
    need(ov.u32(0x80191140) == 0x2464016C and ov.u32(0x8019114C) == 0x24620034, "pool extent/stride")
    need(ov.u32(0x801911AC) == 0x2A8203E8 and ov.u32(0x80191404) == 0x284203E8, "attract bound 1000")
    need(ov.u32(0x801911A4) == 0x2463319C, "kind-1 handler identity")
    need(ov.u32(IMAGE_TABLE + 4) == 0x2FD3C and ov.u32(IMAGE_TABLE + 8) == 0x325D0, "kind 1/2 image offsets")
    need(ov.s32(KIND_PARAMS + 12) == 0x58 and ov.s32(KIND_PARAMS + 16) == 0xB4 and
         ov.s32(KIND_PARAMS + 24) == 0x68 and ov.s32(KIND_PARAMS + 28) == 0xC8, "kind 1/2 positions")
    print(f"  OK retail: {len(SPANS)} spans authenticated; pool, bound, handler, sprite tables")

    model = title_model(ov)
    model_sha = hashlib.sha256(model).hexdigest()
    print(f"  model title frame SHA-256 {model_sha}")

    skip = strict_frontier(root, "skip_fmv")
    default = strict_frontier(root, "default")
    src = (root / "pc_port/game/boot/overlay_title_port.c").read_text()
    need("m0360i" not in src and "0xA8066048" not in src and "persist" not in src, "no scheduler planting")
    helpers = (root / "pc_port/game/boot/title_exe_helpers_port.c").read_text()
    need('Bootstrap_ReturnVoid("func_800425DC"' in helpers, "memory-card poll stays an explicit boundary")

    tests = pathlib.Path(a.tests) if a.tests else root / "pc_port/build/pe-native-tests"
    port = pathlib.Path(a.port) if a.port else root / "pc_port/build/parasite-eve-port"
    disc = pathlib.Path(a.disc) if a.disc else pathlib.Path(
        os.environ.get("PE_DISC1_BIN", (root / "local/pe_disc1.path").read_text().strip()))
    env = os.environ.copy()
    env["PE_TEST_FILTER"] = "B54KAS"
    out = run([str(tests)], 0, env)
    need("4 passed, 0 failed" in out and "FAIL" not in out, "focused controls")

    on = os.environ.copy()
    on["PE_PORT_SKIP_FMV"] = "1"
    strict = run([str(port), "--headless", "--disc-image", str(disc), "--strict-stubs"], 1, on)
    need(skip["cut"] in strict and f"called from: {skip['caller']}" in strict, "skip_fmv strict frontier")
    off = os.environ.copy()
    off.pop("PE_PORT_SKIP_FMV", None)
    strict_off = run([str(port), "--headless", "--disc-image", str(disc), "--strict-stubs"], 1, off)
    need(default["cut"] in strict_off, "flag-off frontier moved")

    new_game = strict_frontier(root, "skip_fmv_new_game")
    driven = run([str(port), "--headless", "--disc-image", str(disc), "--max-frames", "900",
                  "--pad", "8@560-563", "--pad", "4000@700-703"], 0, on)
    need(f"[STUB:BOOTSTRAP_RET] {new_game['cut']}" in driven and
         "stop_reason=unresolved-boundary" in driven,
         "Start+Cross did not reach the configured New Game boundary")
    print(f"  OK input: Start then Cross exits the title loop at {new_game['cut']}")

    vram = pathlib.Path(a.keep_vram) if a.keep_vram else root / "pc_port/build/b54kas_title.vram"
    run([str(port), "--headless", "--disc-image", str(disc), "--max-frames", "620",
         "--vram-dump", str(vram)], 0, on)
    data = vram.read_bytes()
    need(len(data) == 1024 * 512 * 2, "vram dump size")
    for buf in (0, 1):
        frame = b"".join(data[((240 * buf + y) * 1024) * 2:((240 * buf + y) * 1024) * 2 + 960] for y in range(240))
        sha = hashlib.sha256(frame).hexdigest()
        need(sha == model_sha, f"display buffer {buf} differs from the model ({sha})")
    if not a.keep_vram:
        vram.unlink()
    print("  OK native: both display buffers byte-identical to the independent title model")
    print(f"  OK strict: skip_fmv stops at {skip['cut']}; default keeps {default['cut']}")
    print("\nB54K-AS title-system oracle: PASS.")


if __name__ == "__main__":
    main()
