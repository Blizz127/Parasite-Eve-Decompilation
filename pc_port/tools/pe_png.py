#!/usr/bin/env python3
"""Dependency-free PNG writer for oracle heatmaps and frame views."""
import struct
import zlib


def write_png(path, width, height, rows):
    """rows: iterable of `height` byte strings, each width*3 RGB bytes."""
    raw = b"".join(b"\x00" + bytes(r) for r in rows)

    def chunk(tag, data):
        return (struct.pack(">I", len(data)) + tag + data +
                struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF))

    with open(path, "wb") as handle:
        handle.write(b"\x89PNG\r\n\x1a\n")
        handle.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)))
        handle.write(chunk(b"IDAT", zlib.compress(raw, 9)))
        handle.write(chunk(b"IEND", b""))


def vram_display24(vram, y0, height=240, width_px=320):
    """24-bit display rows from a raw 1024x512x16 VRAM dump."""
    rows = []
    for y in range(height):
        off = (y0 + y) * 1024 * 2
        rows.append(vram[off:off + width_px * 3])
    return rows
