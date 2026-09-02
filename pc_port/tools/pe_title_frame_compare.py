#!/usr/bin/env python3
"""B54K-AT frame comparison: native VRAM display buffer vs a PCSX-Redux
24-bit screenshot (320x240x3 raw from PCSX.GPU.takeScreenShot).

PCSX-Redux frames the CRT output: with the retail DispEnv (screen y 0,
240 lines) its capture shows 16 blank lines at the top and drops the last 16
display rows.  --retail-row-offset 16 compares native row y with retail row
y+16, requires the retail rows above the offset to be black and the native
rows that fell off the capture to be black, and otherwise demands exact
equality.  Prints the exact byte/pixel diff, writes a heatmap PNG, and exits
0 only on exact equality.  --negative <other.raw> additionally proves the comparison
rejects a deliberately wrong frame.  Imports no production code."""

from __future__ import annotations

import argparse
import hashlib
import pathlib
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
from pe_png import write_png, vram_display24  # noqa: E402

W, H = 320, 240


def load_port(path: pathlib.Path, buffer: int) -> bytes:
    vram = path.read_bytes()
    if len(vram) != 1024 * 512 * 2:
        raise SystemExit("port dump must be 1024x512x16")
    return b"".join(vram_display24(vram, 240 * buffer))


def load_redux(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    if len(data) != W * H * 3:
        raise SystemExit(f"redux frame must be {W}x{H}x3 (got {len(data)} bytes)")
    return data


def shift_retail(retail: bytes, offset: int) -> tuple[bytes, int, int]:
    """Realign a Redux capture to native display rows.  Returns the realigned
    frame plus the count of non-black bytes in the retail rows above the
    offset (must be 0)."""
    if offset <= 0:
        return retail, 0, 0
    head = retail[:offset * W * 3]
    body = retail[offset * W * 3:]
    padded = body + b"\0" * (offset * W * 3)
    return padded, sum(1 for v in head if v), offset


def compare(a: bytes, b: bytes, heatmap: pathlib.Path | None,
            rows_compared: int = H) -> tuple[int, int]:
    diff_bytes = 0
    diff_pixels = 0
    rows = []
    for y in range(H):
        row = bytearray()
        for x in range(W):
            i = (y * W + x) * 3
            pa, pb = a[i:i + 3], b[i:i + 3]
            if y >= rows_compared:
                pb = pa if not any(pa) else b"\xff\xff\xff"
            if pa != pb:
                diff_pixels += 1
                diff_bytes += sum(1 for u, v in zip(pa, pb) if u != v)
                mag = max(abs(u - v) for u, v in zip(pa, pb))
                row += bytes((255, 255 - mag, 0))
            else:
                g = pa[0] // 4
                row += bytes((g, g, g))
        rows.append(bytes(row))
    if heatmap:
        write_png(str(heatmap), W, H, rows)
    return diff_pixels, diff_bytes


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("port_vram")
    ap.add_argument("redux_raw")
    ap.add_argument("--buffer", type=int, default=0, help="display half 0 or 1")
    ap.add_argument("--heatmap")
    ap.add_argument("--negative", help="a different retail frame that must NOT match")
    ap.add_argument("--negative-heatmap")
    ap.add_argument("--retail-row-offset", type=int, default=0)
    a = ap.parse_args()
    port = load_port(pathlib.Path(a.port_vram), a.buffer)
    redux_raw = load_redux(pathlib.Path(a.redux_raw))
    print(f"port   SHA-256 {hashlib.sha256(port).hexdigest()}")
    print(f"retail SHA-256 {hashlib.sha256(redux_raw).hexdigest()} (capture, before realignment)")
    redux, head_nonblack, off = shift_retail(redux_raw, a.retail_row_offset)
    rows_compared = H - off
    if off:
        print(f"retail rows 0..{off-1} non-black bytes: {head_nonblack} (must be 0); "
              f"native rows {rows_compared}..{H-1} must be black")
    px, by = compare(port, redux, pathlib.Path(a.heatmap) if a.heatmap else None, rows_compared)
    print(f"diff: {px} pixels / {by} bytes of {W*H} pixels (rows compared: {rows_compared})")
    ok = px == 0 and head_nonblack == 0
    if a.negative:
        neg, _, _ = shift_retail(load_redux(pathlib.Path(a.negative)), a.retail_row_offset)
        npx, nby = compare(port, neg, pathlib.Path(a.negative_heatmap) if a.negative_heatmap else None, rows_compared)
        print(f"negative control diff: {npx} pixels / {nby} bytes (must be > 0)")
        ok = ok and npx > 0
    print("RESULT:", "EXACT" if px == 0 else "MISMATCH", "| negative control",
          ("REJECTS" if a.negative and npx > 0 else ("ACCEPTS(!)" if a.negative else "n/a")))
    raise SystemExit(0 if ok else 1)


if __name__ == "__main__":
    main()
