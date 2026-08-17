#!/usr/bin/env python3
"""PE battle-data precovery oracle.

Recomputes NYPD / m0005i / CE2=14 / M0367I data contracts from the
SHA-1-exact EXE and Disc 1 PE.IMG. Writes or verifies:

  docs/evidence/pe-battle-data-precovery/
    ACTOR_RESOURCES.csv
    BATTLE_SCRIPTS.csv
    RESOURCE_PUBLICATION.csv
    PEIMG_PACKAGES.csv
    UNKNOWN_DEPENDENCIES.csv
    REPORT.md

Does not import production C. Does not modify UE5. Does not name
actors from appearance.
"""
from __future__ import annotations

import csv
import hashlib
import os
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
HDR = 0x800
PE_IMG_LBA = 1013
SECTOR_RAW = 2352
FORM1_OFF = 24
FORM1_USER = 2048

M0005I_TOKEN = 0xA80002C8
M0005I_REL = 0x266A
M0005I_PACKED = 0x0600A921
M0005I_CHUNK2 = "01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b"
M0005I_FULL = "fc48530a84811c31ebf4cde06bb12c9c7bcc815348db8dd8c2bb7dacd3410724"
M0367I_TOKEN = 0xA80663C8
M0367I_REL = 0x15050
M0367I_PACKED = 0x04E0AA21
M0367I_CHUNK2 = "ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e"
CE214_SHA = "db785a5eea1f78f945284e57955605326f5856adda1ebc83d0a95d7a0142b1b2"
CLIP4_SHA = "6207fbca2fe44a3549bf0b7fbcf1ce3e979a0a12e8b130e606ea4a985b1885e4"
WIN_6B35C = "1106cb2a94fa877af5a067e36a3c24d08f186694b112c2139d2869f7cf341f24"
WIN_B0E70 = "f2f5b2382224b409acea599d5bdf1572a082fe30d76d19731ff712b4aef0e2d0"
WIN_6C118 = "59c4bd179f4f37cbc881c9d847d80e469f4311f69c5efa904b2c583169cf70f2"
WIN_362B8 = "c1d9429303ec09b5ebea35272182d10d64b061ede1dee1c34a637581827db78d"
WIN_WRITER_A = (
    0x90C5000B,
    0x90C40007,
    0x8CC30004,
    0x00051040,
    0x00451021,
    0x00021180,
    0x00561021,
    0x244201C0,
    0x00042080,
    0x00822021,
    0x00671824,
    0x02831821,
    0xAC830000,
)
LIST_OFF = 0x202A4
DESC_OFF = 0x25014
KNOWN_OPS = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
    0x0D, 0x0E, 0x11, 0x12, 0x14, 0x1A, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x22, 0x24, 0x2A, 0x2E, 0x2F, 0x30, 0x31, 0x3F, 0x40, 0x41, 0x43,
    0x4B, 0x4E, 0x52, 0x53, 0x54, 0x59, 0x5A, 0x5E, 0x64, 0x65, 0x6A,
    0x6B, 0x6F, 0x70, 0x77, 0x79, 0x82, 0x84, 0x85, 0x86, 0x87, 0x88,
    0x89, 0x8B, 0x94, 0x9B, 0x9C, 0xA6, 0xAA, 0xAB, 0xAD, 0xB7, 0xB8,
    0xC7, 0xCE, 0xD9, 0xDC, 0xE1, 0xEA, 0xED,
}
YIELD_OPS = {0x01, 0x02, 0x1F, 0x20, 0x30, 0x64, 0x9C}
BRANCH_OPS = {0x00, 0x05, 0x12}
ROOT = pathlib.Path(__file__).resolve().parents[2]
EVIDENCE = ROOT / "docs" / "evidence" / "pe-battle-data-precovery"
CSV_NAMES = (
    "ACTOR_RESOURCES.csv",
    "BATTLE_SCRIPTS.csv",
    "RESOURCE_PUBLICATION.csv",
    "PEIMG_PACKAGES.csv",
    "UNKNOWN_DEPENDENCIES.csv",
)


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


def exe_off(va: int) -> int:
    return va - TADDR + HDR


def load_u32(blob: bytes, va: int) -> int:
    return struct.unpack_from("<I", blob, exe_off(va))[0]


def load_u16(blob: bytes, va: int) -> int:
    return struct.unpack_from("<H", blob, exe_off(va))[0]


def window_sha(blob: bytes, start: int, end: int) -> str:
    return hashlib.sha256(blob[exe_off(start) : exe_off(end)]).hexdigest()


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def find_disc() -> pathlib.Path:
    pointer = ROOT / "local" / "pe_disc1.path"
    if pointer.is_file():
        path = pathlib.Path(pointer.read_text().strip().splitlines()[0].strip())
        if path.is_file():
            return path
    env = os.environ.get("PE_DISC1_BIN", "").strip()
    if env:
        path = pathlib.Path(env)
        if path.is_file():
            return path
    raise SystemExit("FAIL: missing Disc 1 (local/pe_disc1.path or PE_DISC1_BIN)")


def read_form1(disc: pathlib.Path, lba: int, nsec: int) -> bytes:
    out = bytearray()
    with disc.open("rb") as fh:
        for i in range(nsec):
            fh.seek((lba + i) * SECTOR_RAW + FORM1_OFF)
            chunk = fh.read(FORM1_USER)
            require(len(chunk) == FORM1_USER, f"disc read {lba + i}")
            out += chunk
    return bytes(out)


def decode_token(exe: bytes, token: int) -> str:
    out = []
    for i in range(6):
        idx = (token >> ((5 - i) * 5 + 2)) & 0x1F
        ch = exe[exe_off(0x800930B4) + idx]
        if 97 <= ch < 123:
            ch -= 32
        out.append(chr(ch))
    return "".join(out)


def packed_secs(packed: int) -> tuple[int, int, int]:
    return packed & 0xFF, (packed >> 8) & 0xFFF, packed >> 20


def rel24(blob: bytes, packed: int) -> int:
    return struct.unpack_from("<I", blob, (packed & 0x3FFFFF) + 4)[0] & 0x00FFFFFF


def walk12(blob: bytes, packed: int) -> list[dict]:
    count = packed >> 22
    rec = packed & 0x3FFFFF
    rows = []
    for i in range(count):
        off = rec + i * 12
        size, ptrw, w8 = struct.unpack_from("<III", blob, off)
        ptr = ptrw & 0xFFFFFF
        payload = blob[ptr : ptr + size] if ptr + size <= len(blob) else b""
        rows.append(
            {
                "i": i,
                "off": off,
                "size": size,
                "ptr": ptr,
                "idb": blob[off + 7],
                "ida": blob[off + 0xB],
                "sha256": hashlib.sha256(payload).hexdigest() if payload else "",
                "b0": blob[ptr] if ptr < len(blob) else None,
                "b1": blob[ptr + 1] if ptr + 1 < len(blob) else None,
                "b2": blob[ptr + 2] if ptr + 2 < len(blob) else None,
            }
        )
    return rows


def walk_script(blob: bytes, base: int, limit: int = 32) -> list[dict]:
    """17018 fetch: word @pc, word2 @pc+4, imms @pc+8, span=8+argc*4."""
    rows = []
    pc = base
    for _ in range(limit):
        if pc + 8 > len(blob):
            break
        word = struct.unpack_from("<I", blob, pc)[0]
        op = word & 0x1FFF
        argc = (word >> 13) & 0xF
        kinds = word >> 17
        span = 8 + argc * 4
        if pc + span > len(blob):
            break
        imms = [
            struct.unpack_from("<I", blob, pc + 8 + i * 4)[0] for i in range(argc)
        ]
        rows.append(
            {
                "rel": pc - base,
                "word": word,
                "op": op,
                "argc": argc,
                "kinds": kinds,
                "imms": imms,
                "span": span,
            }
        )
        if op not in KNOWN_OPS:
            break
        pc += span
    return rows


def csv_write(path: pathlib.Path, fieldnames: list[str], rows: list[dict]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="\n", encoding="utf-8") as fh:
        writer = csv.DictWriter(fh, fieldnames=fieldnames, lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow({k: row.get(k, "") for k in fieldnames})


def csv_read(path: pathlib.Path) -> tuple[list[str], list[dict]]:
    with path.open("r", encoding="utf-8", newline="") as fh:
        reader = csv.DictReader(fh)
        rows = list(reader)
        return list(reader.fieldnames or []), rows


def hex_list(values: list[int], width: int = 2) -> str:
    return "|".join(f"0x{v:0{width}X}" for v in values)


class World:
    def __init__(self) -> None:
        exe_path = ROOT / "build" / "disc1.candidate.exe"
        require(exe_path.is_file(), f"missing {exe_path}")
        self.exe = exe_path.read_bytes()
        require(hashlib.sha1(self.exe).hexdigest() == SHA1, "EXE SHA-1")
        require(struct.unpack_from("<I", self.exe, 0x1C)[0] == 0x1EE000, "tsize")
        self.disc = find_disc()
        s0, s1, s2 = packed_secs(M0005I_PACKED)
        self.m5_c0 = read_form1(self.disc, PE_IMG_LBA + M0005I_REL, s0)
        self.m5_c1 = read_form1(self.disc, PE_IMG_LBA + M0005I_REL + s0, s1)
        self.m5_c2 = read_form1(self.disc, PE_IMG_LBA + M0005I_REL + s0 + s1, s2)
        self.m5_full = self.m5_c0 + self.m5_c1 + self.m5_c2
        require(hashlib.sha256(self.m5_c2).hexdigest() == M0005I_CHUNK2, "m0005i chunk2")
        require(hashlib.sha256(self.m5_full).hexdigest() == M0005I_FULL, "m0005i full")
        self.ce214 = read_form1(self.disc, PE_IMG_LBA + 396, 32)
        require(hashlib.sha256(self.ce214).hexdigest() == CE214_SHA, "CE2=14 bank")
        t0, t1, t2 = packed_secs(M0367I_PACKED)
        self.m367_c0 = read_form1(self.disc, PE_IMG_LBA + M0367I_REL, t0)
        self.m367_c1 = read_form1(self.disc, PE_IMG_LBA + M0367I_REL + t0, t1)
        self.m367_c2 = read_form1(self.disc, PE_IMG_LBA + M0367I_REL + t0 + t1, t2)
        self.m367_full = self.m367_c0 + self.m367_c1 + self.m367_c2
        require(hashlib.sha256(self.m367_c2).hexdigest() == M0367I_CHUNK2, "M0367I chunk2")
        self.m5_hdr = struct.unpack_from("<I", self.m5_c2, 4)[0] & 0x3FFFFF
        self.ce_hdr = struct.unpack_from("<I", self.ce214, 4)[0] & 0x3FFFFF
        self.m367_hdr = struct.unpack_from("<I", self.m367_c2, 4)[0] & 0x3FFFFF

    def pin_exe(self) -> None:
        require(decode_token(self.exe, M0005I_TOKEN) == "M0005I", "M0005I token")
        require(decode_token(self.exe, M0367I_TOKEN) == "M0367I", "M0367I token")
        require(load_u32(self.exe, 0x80093378 + 4 * 8) == M0005I_REL, "table[4] rel")
        require(load_u32(self.exe, 0x80093378 + 4 * 8 + 4) == M0005I_PACKED, "table[4] packed")
        require(load_u32(self.exe, 0x80093378 + 366 * 8) == M0367I_REL, "table[366] rel")
        require(load_u32(self.exe, 0x80093378 + 366 * 8 + 4) == M0367I_PACKED, "table[366] packed")
        require(load_u16(self.exe, 0x800930D8 + 22 * 2) == 396, "CE2=14 start")
        require(load_u16(self.exe, 0x800930D8 + 23 * 2) == 428, "CE2=14 end")
        require(jal_target(load_u32(self.exe, 0x8003F07C)) == 0x8006B35C, "3F074 jal 6B35C")
        require(jal_target(load_u32(self.exe, 0x8003F088)) == 0x8006B4F8, "3F074 jal 6B4F8")
        require((0x8006B4F8 - 0x8006B35C) // 4 == 103, "6B35C 103w")
        require(window_sha(self.exe, 0x8006B35C, 0x8006B4F8) == WIN_6B35C, "6B35C sha")
        require(load_u32(self.exe, 0x8006B378) == 0xAC400198, "6B35C sw B0E70 via +0x198")
        require(load_u32(self.exe, 0x8006B3F0) == 0xAC400948, "6B35C zero +0x948")
        require(load_u32(self.exe, 0x8006B7C8) == 0x8EA2000C, "hdr+0x0C lw")
        require(load_u32(self.exe, 0x8006B804) == 0xAC620198, "hdr+0x0C sw B0E70[idB]")
        require(window_sha(self.exe, 0x8006B7C8, 0x8006B820) == WIN_B0E70, "hdr+0x0C window")
        for i, word in enumerate(WIN_WRITER_A):
            require(load_u32(self.exe, 0x8006B84C + i * 4) == word, f"Writer A[{i}]")
        require(load_u32(self.exe, 0x8006C118) == 0xAE820198, "6C118 sw B0E70[0]")
        require(load_u32(self.exe, 0x8006C158) == 0xAC6201C0, "Writer B sw +0x1C0")
        require(window_sha(self.exe, 0x8006C0E4, 0x8006C174) == WIN_6C118, "6C118 window")
        require(window_sha(self.exe, 0x800362B8, 0x800363F4) == WIN_362B8, "362B8 sha")
        require(load_u32(self.exe, 0x800351A8) == 0x8C220E70, "35038 lw B0E70[type]")
        require(load_u32(self.exe, 0x8001A6DC) == 0xAE2201B0, "1A680 sw +0x1B0")
        require(load_u32(self.exe, 0x800910A0 + 0xCE * 4) == 0x800181CC, "table[0xCE]")
        require(load_u32(self.exe, 0x800910A0 + 0x9B * 4) == 0x80015240, "table[0x9B]")
        require(load_u32(self.exe, 0x800910A0 + 0x08 * 4) == 0x8001735C, "table[0x08]")
        require(load_u32(self.exe, 0x8006B8C4) == 0xAEC20944, "sw +0x944")
        require(load_u32(self.exe, 0x8006B8E8) == 0xAEC20948, "sw +0x948")
        require(load_u32(self.exe, 0x8006B90C) == 0xAEC2094C, "sw +0x94C")
        require(load_u32(self.exe, 0x8006B94C) == 0xAC620950, "sw +0x950")

    def packages(self) -> list[dict]:
        s0, s1, s2 = packed_secs(M0005I_PACKED)
        t0, t1, t2 = packed_secs(M0367I_PACKED)
        rows = [
            {
                "package_id": "m0005i",
                "token": f"0x{M0005I_TOKEN:08X}",
                "decoded_name": "M0005I",
                "table_index": 4,
                "rel": f"0x{M0005I_REL:X}",
                "packed": f"0x{M0005I_PACKED:08X}",
                "sectors": f"{s0}+{s1}+{s2}",
                "peimg_lba_range": f"[{PE_IMG_LBA + M0005I_REL},{PE_IMG_LBA + M0005I_REL + s0 + s1 + s2})",
                "size": len(self.m5_full),
                "sha256": hashlib.sha256(self.m5_full).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "3-chunk field package; chunk2 hdr@+4",
                "consumer": "func_8006B4F8",
                "writer": "PE.IMG table D_80093378[4]",
                "confidence": "PROVEN",
                "notes": "NYPD/Eve-intro room; first 0x89 at module 6 +0x350C",
            },
            {
                "package_id": "m0005i_chunk0",
                "token": f"0x{M0005I_TOKEN:08X}",
                "decoded_name": "M0005I",
                "table_index": 4,
                "rel": f"0x{M0005I_REL:X}",
                "packed": f"0x{M0005I_PACKED:08X}",
                "sectors": str(s0),
                "peimg_lba_range": f"[{PE_IMG_LBA + M0005I_REL},{PE_IMG_LBA + M0005I_REL + s0})",
                "size": len(self.m5_c0),
                "sha256": hashlib.sha256(self.m5_c0).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "6B4F8 dest overlay+0x194",
                "consumer": "func_8006B4F8 / 6E6A8",
                "writer": "PE.IMG",
                "confidence": "PROVEN",
                "notes": "",
            },
            {
                "package_id": "m0005i_chunk1",
                "token": f"0x{M0005I_TOKEN:08X}",
                "decoded_name": "M0005I",
                "table_index": 4,
                "rel": f"0x{M0005I_REL:X}",
                "packed": f"0x{M0005I_PACKED:08X}",
                "sectors": str(s1),
                "peimg_lba_range": f"[{PE_IMG_LBA + M0005I_REL + s0},{PE_IMG_LBA + M0005I_REL + s0 + s1})",
                "size": len(self.m5_c1),
                "sha256": hashlib.sha256(self.m5_c1).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "6B4F8 dest overlay+0x168",
                "consumer": "func_8006B4F8 / 6E6A8",
                "writer": "PE.IMG",
                "confidence": "PROVEN",
                "notes": "",
            },
            {
                "package_id": "m0005i_chunk2",
                "token": f"0x{M0005I_TOKEN:08X}",
                "decoded_name": "M0005I",
                "table_index": 4,
                "rel": f"0x{M0005I_REL:X}",
                "packed": f"0x{M0005I_PACKED:08X}",
                "sectors": str(s2),
                "peimg_lba_range": f"[{PE_IMG_LBA + M0005I_REL + s0 + s1},{PE_IMG_LBA + M0005I_REL + s0 + s1 + s2})",
                "size": len(self.m5_c2),
                "sha256": hashlib.sha256(self.m5_c2).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "scripts+Writer A+B0E70+12574; dest overlay+0x18C",
                "consumer": "func_8006B4F8 publish cut",
                "writer": "PE.IMG",
                "confidence": "PROVEN",
                "notes": f"hdr@{self.m5_hdr:#x}",
            },
            {
                "package_id": "ce2_14_clip_bank",
                "token": "CE2=14",
                "decoded_name": "",
                "table_index": "D_800930D8[22..23]",
                "rel": "396",
                "packed": "",
                "sectors": "32",
                "peimg_lba_range": f"[{PE_IMG_LBA + 396},{PE_IMG_LBA + 428})",
                "size": len(self.ce214),
                "sha256": hashlib.sha256(self.ce214).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "Writer B 12-byte dir; type-0 commands",
                "consumer": "func_8001A680 via D_800B0E98",
                "writer": "func_8006BECC state6 / 6C140",
                "confidence": "PROVEN",
                "notes": "6C1CC(a0=1) sets CE2=14; 144FC/29810 do not jal it",
            },
            {
                "package_id": "m0367i",
                "token": f"0x{M0367I_TOKEN:08X}",
                "decoded_name": "M0367I",
                "table_index": 366,
                "rel": f"0x{M0367I_REL:X}",
                "packed": f"0x{M0367I_PACKED:08X}",
                "sectors": f"{t0}+{t1}+{t2}",
                "peimg_lba_range": f"[{PE_IMG_LBA + M0367I_REL},{PE_IMG_LBA + M0367I_REL + t0 + t1 + t2})",
                "size": len(self.m367_full),
                "sha256": hashlib.sha256(self.m367_full).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": "3-chunk dest package; 6B4F8 reload",
                "consumer": "func_8006B4F8",
                "writer": "0x31 store D_8009D280",
                "confidence": "PROVEN",
                "notes": "watch-arm dest after m0005i; do not infer story meaning",
            },
            {
                "package_id": "m0367i_chunk2",
                "token": f"0x{M0367I_TOKEN:08X}",
                "decoded_name": "M0367I",
                "table_index": 366,
                "rel": f"0x{M0367I_REL:X}",
                "packed": f"0x{M0367I_PACKED:08X}",
                "sectors": str(t2),
                "peimg_lba_range": f"[{PE_IMG_LBA + M0367I_REL + t0 + t1},{PE_IMG_LBA + M0367I_REL + t0 + t1 + t2})",
                "size": len(self.m367_c2),
                "sha256": hashlib.sha256(self.m367_c2).hexdigest(),
                "compression": "none_form1_user",
                "decoded_structure": f"hdr@{self.m367_hdr:#x}; Writer A count 36; hdr+0x0C count 3",
                "consumer": "func_8006B4F8 publish cut",
                "writer": "PE.IMG",
                "confidence": "PROVEN",
                "notes": "",
            },
        ]
        seen: set[str] = set()
        ce_dir = walk12(
            self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x10)[0]
        )
        for rec in ce_dir:
            if rec["sha256"] in seen:
                continue
            seen.add(rec["sha256"])
            idbs = [r["idb"] for r in ce_dir if r["sha256"] == rec["sha256"]]
            rows.append(
                {
                    "package_id": f"ce2_14_clip_ptr_{rec['ptr']:X}",
                    "token": "CE2=14",
                    "decoded_name": "",
                    "table_index": f"idB={','.join(str(x) for x in idbs)}",
                    "rel": f"0x{rec['ptr']:X}",
                    "packed": "",
                    "sectors": "",
                    "peimg_lba_range": f"[{PE_IMG_LBA + 396},{PE_IMG_LBA + 428})",
                    "size": rec["size"],
                    "sha256": rec["sha256"],
                    "compression": "none",
                    "decoded_structure": (
                        f"clip enc={rec['b0']} bones-1={rec['b1']} frames={rec['b2']}"
                    ),
                    "consumer": "func_8001A680 +0x1B0",
                    "writer": "func_8006C140 Writer B",
                    "confidence": "PROVEN",
                    "notes": "type-0 command bank; do not name clip",
                }
            )
        m5_dir = walk12(
            self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x10)[0]
        )
        seen.clear()
        for rec in m5_dir:
            if rec["sha256"] in seen:
                continue
            seen.add(rec["sha256"])
            keys = [f"t{r['ida']}_c{r['idb']:02X}" for r in m5_dir if r["sha256"] == rec["sha256"]]
            rows.append(
                {
                    "package_id": f"m0005i_writera_ptr_{rec['ptr']:X}",
                    "token": f"0x{M0005I_TOKEN:08X}",
                    "decoded_name": "M0005I",
                    "table_index": ",".join(keys),
                    "rel": f"0x{rec['ptr']:X}",
                    "packed": "",
                    "sectors": "",
                    "peimg_lba_range": f"m0005i_chunk2+0x{rec['ptr']:X}",
                    "size": rec["size"],
                    "sha256": rec["sha256"],
                    "compression": "none",
                    "decoded_structure": (
                        f"Writer A payload b0={rec['b0']} b1={rec['b1']} b2={rec['b2']}"
                    ),
                    "consumer": "func_8001A680 +0x1B0",
                    "writer": "func_8006B84C Writer A",
                    "confidence": "PROVEN",
                    "notes": "room package clips; type-0 has no cmd 4 here",
                }
            )
        return rows

    def publication(self) -> list[dict]:
        m5_0c = walk12(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x0C)[0])
        m5_10 = walk12(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x10)[0])
        ce_0c = walk12(self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x0C)[0])
        ce_10 = walk12(self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x10)[0])
        obj948 = rel24(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x18)[0])
        obj94c = rel24(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x1C)[0])
        rows = [
            {
                "object": "D_800B0E70[0..9]",
                "field_or_global": "overlay+0x198",
                "writer": "func_8006B35C",
                "writer_va": "0x8006B378",
                "source_data": "zero",
                "source_offset": "",
                "lifetime": "dest_enter_clear",
                "publication_order": 1,
                "consumer": "func_80035038 +0x1AC",
                "consumer_va": "0x800351A8",
                "actor_type": "0-9",
                "confidence": "PROVEN",
                "notes": "10-word countdown from overlay+0x1BC to +0x198",
            },
            {
                "object": "D_800B0E98 type rows",
                "field_or_global": "overlay+0x1C0",
                "writer": "func_8006B35C",
                "writer_va": "0x8006B398",
                "source_data": "zero",
                "source_offset": "",
                "lifetime": "dest_enter_clear",
                "publication_order": 2,
                "consumer": "func_8001A680 +0x1B0",
                "consumer_va": "0x8001A6C8",
                "actor_type": "0-9",
                "confidence": "PROVEN",
                "notes": "10 rows x 48 words; stride 192",
            },
            {
                "object": "D_800B161C/20/24/28",
                "field_or_global": "overlay+0x944..+0x958",
                "writer": "func_8006B35C",
                "writer_va": "0x8006B3C0",
                "source_data": "zero",
                "source_offset": "",
                "lifetime": "dest_enter_clear",
                "publication_order": 3,
                "consumer": "12574/1A918/371B0",
                "consumer_va": "0x80012574",
                "actor_type": "",
                "confidence": "PROVEN",
                "notes": "zeroer of 6B4F8 publish slots",
            },
        ]
        for rec in m5_0c:
            rows.append(
                {
                    "object": f"D_800B0E70[{rec['idb']}]",
                    "field_or_global": "actor+0x1AC",
                    "writer": "func_8006B4F8 hdr+0x0C @ 0x8006B804",
                    "writer_va": "0x8006B804",
                    "source_data": "m0005i_chunk2",
                    "source_offset": f"0x{rec['ptr']:X}",
                    "lifetime": "until_next_6B35C",
                    "publication_order": 4,
                    "consumer": "func_80035038",
                    "consumer_va": "0x800351A8",
                    "actor_type": str(rec["idb"]),
                    "confidence": "PROVEN",
                    "notes": f"idA={rec['ida']} size={rec['size']} sha256={rec['sha256']}",
                }
            )
        rows.append(
            {
                "object": "D_800B0E98[type*192+cmd*4]",
                "field_or_global": "actor+0x1B0",
                "writer": "func_8006B4F8 hdr+0x10 Writer A @ 0x8006B84C",
                "writer_va": "0x8006B84C",
                "source_data": "m0005i_chunk2",
                "source_offset": f"0x{m5_10[0]['off']:X}" if m5_10 else "",
                "lifetime": "until_next_6B35C",
                "publication_order": 5,
                "consumer": "func_8001A680",
                "consumer_va": "0x8001A6C8",
                "actor_type": "mixed",
                "confidence": "PROVEN",
                "notes": f"21 records; type2 cmd=0x17 ptr=0x1F3A8 byte2=51",
            }
        )
        rows.append(
            {
                "object": "D_800B161C",
                "field_or_global": "overlay+0x944",
                "writer": "func_8006B4F8 hdr+0x14 / 12574",
                "writer_va": "0x8006B8C4",
                "source_data": "m0005i_chunk2",
                "source_offset": f"0x{LIST_OFF:X}",
                "lifetime": "until_next_6B35C",
                "publication_order": 6,
                "consumer": "func_800125E0",
                "consumer_va": "0x800125E0",
                "actor_type": "list",
                "confidence": "PROVEN",
                "notes": "count=7 type entries; 125E0 desc is type1+type6 only",
            }
        )
        rows.append(
            {
                "object": "D_800B1620",
                "field_or_global": "overlay+0x948",
                "writer": "func_8006B4F8 hdr+0x18",
                "writer_va": "0x8006B8E8",
                "source_data": "m0005i_chunk2",
                "source_offset": f"0x{obj948:X}",
                "lifetime": "until_next_6B35C",
                "publication_order": 7,
                "consumer": "func_8001A918",
                "consumer_va": "0x8001A918",
                "actor_type": "",
                "confidence": "PROVEN",
                "notes": f"lhu+2=1 +0x20=0x760; sha256={hashlib.sha256(self.m5_c2[obj948:obj948+0x800]).hexdigest()}",
            }
        )
        rows.append(
            {
                "object": "D_800B1624",
                "field_or_global": "overlay+0x94C",
                "writer": "func_8006B4F8 hdr+0x1C",
                "writer_va": "0x8006B90C",
                "source_data": "m0005i_chunk2",
                "source_offset": f"0x{obj94c:X}",
                "lifetime": "until_next_6B35C",
                "publication_order": 8,
                "consumer": "func_800371B0 / view apply",
                "consumer_va": "0x800371B0",
                "actor_type": "",
                "confidence": "PROVEN",
                "notes": f"sha256={hashlib.sha256(self.m5_c2[obj94c:obj94c+0x422C]).hexdigest() if obj94c+0x422C<=len(self.m5_c2) else ''}",
            }
        )
        require(len(ce_0c) == 1, "CE2=14 hdr+0x0C count")
        rows.append(
            {
                "object": "D_800B0E70[0]",
                "field_or_global": "actor+0x1AC type0",
                "writer": "func_8006C118 inside 6BECC",
                "writer_va": "0x8006C118",
                "source_data": "ce2_14_clip_bank",
                "source_offset": f"0x{ce_0c[0]['ptr']:X}",
                "lifetime": "until_next_6B35C",
                "publication_order": 9,
                "consumer": "func_80035038",
                "consumer_va": "0x800351A8",
                "actor_type": "0",
                "confidence": "PROVEN",
                "notes": (
                    f"rel24 of hdr+0x0C; size={ce_0c[0]['size']} "
                    f"sha256={ce_0c[0]['sha256']}; not m0005i hdr+0x0C"
                ),
            }
        )
        clip4 = next(r for r in ce_10 if r["ida"] == 0 and r["idb"] == 4)
        require(clip4["sha256"] == CLIP4_SHA, "clip4 sha")
        rows.append(
            {
                "object": "D_800B0E98[4]",
                "field_or_global": "actor+0x1B0 type0 cmd4",
                "writer": "func_8006C140 Writer B",
                "writer_va": "0x8006C158",
                "source_data": "ce2_14_clip_bank",
                "source_offset": "0x6C14",
                "lifetime": "until_next_6B35C",
                "publication_order": 10,
                "consumer": "func_8001A680 / 29810",
                "consumer_va": "0x8001A6DC",
                "actor_type": "0",
                "confidence": "PROVEN",
                "notes": f"1700B enc=1 bones-1=30 frames=18 sha256={CLIP4_SHA}",
            }
        )
        rows.append(
            {
                "object": "actor+0x1B4 dest",
                "field_or_global": "actor+0x1B4",
                "writer": "in-actor dest object; not a table pointer",
                "writer_va": "",
                "source_data": "actor body",
                "source_offset": "+0x1B4",
                "lifetime": "actor",
                "publication_order": 11,
                "consumer": "func_80015240 / func_8006CC68",
                "consumer_va": "0x80015240",
                "actor_type": "0",
                "confidence": "PROVEN",
                "notes": "15240 dest=actor+0x1B4; 6CC68 publishes D254+0x1B4 to D_800B0D10; 362B8/3D050 init only if +0x1AC!=0",
            }
        )
        return rows

    def scripts(self) -> list[dict]:
        n = struct.unpack_from("<I", self.m5_c2, LIST_OFF + 4)[0]
        require(n == 7, "12574 list count")
        rows = []
        for typ in range(n):
            rel = struct.unpack_from("<I", self.m5_c2, LIST_OFF + 8 + typ * 4)[0]
            base = LIST_OFF + rel
            insns = walk_script(self.m5_c2, base, 24)
            require(insns, f"type {typ} empty")
            window = self.m5_c2[base : base + sum(x["span"] for x in insns)]
            ops = [x["op"] for x in insns]
            known = [f"0x{op:02X}" for op in ops if op in KNOWN_OPS]
            unknown = [f"0x{op:02X}" for op in ops if op not in KNOWN_OPS]
            branches = []
            yields = []
            for insn in insns:
                if insn["op"] in BRANCH_OPS and insn["imms"]:
                    branches.append(f"+{insn['rel']:X}->+{(insn['imms'][0] << 1) & 0xFFFFFFFF:X}")
                if insn["op"] in YIELD_OPS:
                    yields.append(f"+{insn['rel']:X}/0x{insn['op']:02X}")
            rows.append(
                {
                    "actor_type": typ,
                    "scene": "m0005i",
                    "source": "m0005i_chunk2",
                    "base_offset": f"0x{base:X}",
                    "window_size": len(window),
                    "sha256": hashlib.sha256(window).hexdigest(),
                    "first_word": f"0x{insns[0]['word']:08X}",
                    "first_opcode": f"0x{insns[0]['op']:02X}",
                    "opcode_sequence": hex_list(ops),
                    "branch_targets": ";".join(branches),
                    "yield_points": ";".join(yields),
                    "known_opcodes": "|".join(known),
                    "unknown_opcodes": "|".join(unknown),
                    "confidence": "PROVEN",
                    "notes": (
                        "17018 word@pc word2@pc+4 imms@pc+8; "
                        + (
                            "125E0-spawned"
                            if typ in (1, 6)
                            else "0x08-spawned"
                            if typ in (0, 3, 5)
                            else "listed_not_125E0_desc"
                        )
                    ),
                }
            )
        return rows

    def actors(self) -> list[dict]:
        m5_0c = {r["idb"]: r for r in walk12(self.m5_c2, struct.unpack_from("<I", self.m5_c2, self.m5_hdr + 0x0C)[0])}
        ce_0c = walk12(self.ce214, struct.unpack_from("<I", self.ce214, self.ce_hdr + 0x0C)[0])[0]
        desc = self.m5_c2[DESC_OFF : DESC_OFF + 8]
        require(desc[0] == 2 and desc[1] == 1 and desc[3] == 6, "125E0 desc")
        scripts = {int(r["actor_type"]): r for r in self.scripts()}
        vtable = {0: "func_80035C84", **{t: "func_80035E04" for t in range(1, 10)}}
        spawn = {
            0: "type1_0x08_second",
            1: "125E0_desc[0]",
            2: "listed_only",
            3: "type1_0x08_first",
            4: "listed_only",
            5: "type1_0x08_third",
            6: "125E0_desc[1]_D20C_head",
        }
        rows = []
        for typ in range(7):
            model = ""
            model_off = ""
            model_size = ""
            model_sha = ""
            model_src = ""
            if typ == 0:
                model_src = "ce2_14_clip_bank"
                model_off = f"0x{ce_0c['ptr']:X}"
                model_size = ce_0c["size"]
                model_sha = ce_0c["sha256"]
                model = "B0E70[0] via 6C118"
            elif typ in m5_0c:
                rec = m5_0c[typ]
                model_src = "m0005i_chunk2"
                model_off = f"0x{rec['ptr']:X}"
                model_size = rec["size"]
                model_sha = rec["sha256"]
                model = f"B0E70[{typ}] via 6B804"
            else:
                model = "B0E70 empty on this path"
            anim = "D_800B0E98 Writer A+B"
            if typ == 0:
                anim = "CE2=14 Writer B + m0005i Writer A cmds 0x18/0x1D-0x20"
            elif typ == 2:
                anim = "m0005i Writer A (cmd 0x17 byte2=51)"
            elif typ == 5:
                anim = "m0005i Writer A cmds 0-3 (60/88B payloads)"
            rows.append(
                {
                    "actor_type": typ,
                    "scene": "m0005i",
                    "descriptor_source": "m0005i_chunk2+0x25014" if typ in (1, 6) else "type1_0x08_desc",
                    "descriptor_offset": "0x25014" if typ in (1, 6) else "runtime",
                    "script_source": scripts[typ]["source"],
                    "script_offset": scripts[typ]["base_offset"],
                    "script_sha256": scripts[typ]["sha256"],
                    "first_vm_pc": scripts[typ]["base_offset"],
                    "first_opcode": scripts[typ]["first_opcode"],
                    "model_resource_source": model_src,
                    "model_offset": model_off,
                    "model_size": model_size,
                    "model_sha256": model_sha,
                    "animation_bank": anim,
                    "publication_global": "D_800B0E70+D_800B0E98+D_800B161C",
                    "resource_object_writer": "6B35C; 6B4F8 6B804/6B84C; 6C118",
                    "update_function": vtable[typ],
                    "known_field_offsets": "+0x0C type; +0x0D idB; +0x1AC B0E70; +0x1B0 B0E98; +0x1B4 dest; +0x98 flags; +0x190 jalr",
                    "spawn_path": spawn[typ],
                    "confidence": "PROVEN" if typ in (0, 1, 3, 5, 6) else "PROVEN_LISTED_NOT_125E0",
                    "notes": model,
                }
            )
        return rows

    def unknowns(self) -> list[dict]:
        return [
            {
                "id": "B0E70_type0_body",
                "needed_by": "35038 +0x1AC!=0 tail / 362B8 / 3D050",
                "surface": "CE2=14 +0x8 size 23864",
                "what_is_known": "writer 6C118; hash pinned; 35038 copies to +0x1AC",
                "what_is_missing": "decoded TMD/skeleton fields; 3D050 tail still gated",
                "blocker": "do_not_publish_into_runtime_until_3D050_tail",
                "confidence": "SOURCE_PROVEN_STRUCTURE_PARTIAL",
                "notes": "not named; type-0 resource object only",
            },
            {
                "id": "actor_plus_1B4_init",
                "needed_by": "15240 / 6CC68 / 3A088",
                "surface": "actor+0x1B4 dest object",
                "what_is_known": "in-actor dest; 15240 dest=actor+0x1B4; empty when +0x1AC==0",
                "what_is_missing": "35038 +0x1AC!=0 stores that fill dest+0 / dest+0x24",
                "blocker": "362B8/3D050 not this lane",
                "confidence": "CONSUMER_PROVEN_WRITER_PARTIAL",
                "notes": "do not invent host-side dest",
            },
            {
                "id": "type1_type3_type6_B0E70",
                "needed_by": "1AA78 / 362B8",
                "surface": "D_800B0E70[1,3,6]",
                "what_is_known": "m0005i hdr+0x0C writes idB 2 and 5 only; 6C118 writes [0] only",
                "what_is_missing": "any later writer for types 1/3/6 on this encounter",
                "blocker": "empty_+0x1AC_is_authentic",
                "confidence": "PROVEN_ABSENT_ON_THIS_PATH",
                "notes": "35038 ORs +0x98 0xE0; 1AA78 0x80 no-op",
            },
            {
                "id": "slot_ids_1332_1333_1334",
                "needed_by": "m0005i module2 0x5A",
                "surface": "tags 50/51/52",
                "what_is_known": "numeric IDs only; written by 2FF78",
                "what_is_missing": "table that maps 1332/1333/1334 to a model/script",
                "blocker": "do_not_name_from_appearance",
                "confidence": "NUMERIC_ONLY",
                "notes": "see pe-btl0-field-battle-handoff",
            },
            {
                "id": "ce2_14_clip_semantics",
                "needed_by": "1A680 / 1A4AC ticker",
                "surface": "CE2=14 idB 0-23,28",
                "what_is_known": "sizes/frames/hashes; cmd4 is first 29810 bind",
                "what_is_missing": "which later cmds the type-0 stream issues",
                "blocker": "index_type0_stream_0x2E/0x2F",
                "confidence": "BANK_PROVEN",
                "notes": "idB 0/1/2/3/7/22/23/28 share one 3796B payload",
            },
            {
                "id": "m0367i_spawn_list",
                "needed_by": "next dest after m0005i 0x31",
                "surface": "M0367I chunk2 hdr+0x14",
                "what_is_known": "token/table/chunk2 hash; Writer A 36 rows; hdr+0x0C idB 2/3/4",
                "what_is_missing": "125E0 desc types and script heads",
                "blocker": "next_checkpoint",
                "confidence": "PACKAGE_PROVEN_LIST_PENDING",
                "notes": "pre-recover before runtime hops",
            },
        ]


def build() -> tuple[World, dict[str, tuple[list[str], list[dict]]]]:
    world = World()
    world.pin_exe()
    tables = {
        "ACTOR_RESOURCES.csv": (
            [
                "actor_type",
                "scene",
                "descriptor_source",
                "descriptor_offset",
                "script_source",
                "script_offset",
                "script_sha256",
                "first_vm_pc",
                "first_opcode",
                "model_resource_source",
                "model_offset",
                "model_size",
                "model_sha256",
                "animation_bank",
                "publication_global",
                "resource_object_writer",
                "update_function",
                "known_field_offsets",
                "spawn_path",
                "confidence",
                "notes",
            ],
            world.actors(),
        ),
        "BATTLE_SCRIPTS.csv": (
            [
                "actor_type",
                "scene",
                "source",
                "base_offset",
                "window_size",
                "sha256",
                "first_word",
                "first_opcode",
                "opcode_sequence",
                "branch_targets",
                "yield_points",
                "known_opcodes",
                "unknown_opcodes",
                "confidence",
                "notes",
            ],
            world.scripts(),
        ),
        "RESOURCE_PUBLICATION.csv": (
            [
                "object",
                "field_or_global",
                "writer",
                "writer_va",
                "source_data",
                "source_offset",
                "lifetime",
                "publication_order",
                "consumer",
                "consumer_va",
                "actor_type",
                "confidence",
                "notes",
            ],
            world.publication(),
        ),
        "PEIMG_PACKAGES.csv": (
            [
                "package_id",
                "token",
                "decoded_name",
                "table_index",
                "rel",
                "packed",
                "sectors",
                "peimg_lba_range",
                "size",
                "sha256",
                "compression",
                "decoded_structure",
                "consumer",
                "writer",
                "confidence",
                "notes",
            ],
            world.packages(),
        ),
        "UNKNOWN_DEPENDENCIES.csv": (
            [
                "id",
                "needed_by",
                "surface",
                "what_is_known",
                "what_is_missing",
                "blocker",
                "confidence",
                "notes",
            ],
            world.unknowns(),
        ),
    }
    return world, tables


def write_tables(tables: dict[str, tuple[list[str], list[dict]]]) -> None:
    EVIDENCE.mkdir(parents=True, exist_ok=True)
    for name, (fields, rows) in tables.items():
        csv_write(EVIDENCE / name, fields, rows)


def verify_tables(tables: dict[str, tuple[list[str], list[dict]]]) -> None:
    for name, (fields, rows) in tables.items():
        path = EVIDENCE / name
        require(path.is_file(), f"missing {path}")
        got_fields, got_rows = csv_read(path)
        require(got_fields == fields, f"{name} columns")
        require(len(got_rows) == len(rows), f"{name} row count {len(got_rows)}!={len(rows)}")
        for i, (want, got) in enumerate(zip(rows, got_rows)):
            for key in fields:
                require(str(want[key]) == got[key], f"{name}[{i}].{key}")


def verify_report(tables: dict[str, tuple[list[str], list[dict]]]) -> None:
    path = EVIDENCE / "REPORT.md"
    require(path.is_file(), "missing REPORT.md")
    text = path.read_text(encoding="utf-8")
    for needle in (
        "PE battle-data precovery",
        SHA1,
        M0005I_CHUNK2,
        M0005I_FULL,
        CE214_SHA,
        CLIP4_SHA,
        M0367I_CHUNK2,
        "D_800B0E70",
        "D_800B0E98",
        "func_8006B35C",
        "func_8006C118",
        "HUMAN_VERIFY",
    ):
        require(needle in text, f"REPORT missing {needle}")
    pkgs = {r["package_id"]: r for r in tables["PEIMG_PACKAGES.csv"][1]}
    require(pkgs["m0005i"]["sha256"] in text, "REPORT m0005i sha")
    require(pkgs["ce2_14_clip_bank"]["sha256"] in text, "REPORT CE2=14 sha")


def main() -> int:
    write = "--write" in sys.argv
    world, tables = build()
    type0 = next(r for r in tables["BATTLE_SCRIPTS.csv"][1] if r["actor_type"] == 0)
    type6 = next(r for r in tables["BATTLE_SCRIPTS.csv"][1] if r["actor_type"] == 6)
    require(type0["first_opcode"] == "0x9B", "type0 first 0x9B")
    require(type6["first_opcode"] == "0xCE", "type6 first 0xCE")
    require(type6["first_word"] == "0x000080CE", "type6 word")
    if write:
        write_tables(tables)
    verify_tables(tables)
    verify_report(tables)
    print(
        "PASS: battle-data precovery; "
        f"m0005i {len(world.m5_full)}B; CE2=14 25 clips; "
        f"B0E70 idB {sorted(r['idb'] for r in walk12(world.m5_c2, struct.unpack_from('<I', world.m5_c2, world.m5_hdr + 0x0C)[0]))}; "
        "scripts 0-6; 6B35C/6B804/6C118 pinned"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
