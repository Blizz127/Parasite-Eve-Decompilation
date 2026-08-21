#!/usr/bin/env python3
"""One-off PE-BTL0-BOSS-ID research scanner. Not production."""

from __future__ import annotations

import json
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from tools.research import pe_pst0_scan as pst0
from tools.research import pe_txt0_decode as txt0

DISC = Path(
    "/var/home/blizz/Projects/Parasite-Eve-Decompilation/rom/image/"
    "Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin"
)

LOAD = 0x80010000
EXE_HDR = 0x800


def va2off(va: int) -> int:
    return va - LOAD + EXE_HDR


def word_at(exe: bytes, va: int) -> int:
    return struct.unpack_from("<I", exe, va2off(va))[0]


def disasm_range(exe: bytes, start: int, end: int) -> list[str]:
    """Tiny MIPS-I decoder for the windows we care about."""
    lines = []
    last_lui: dict[int, int] = {}
    for va in range(start, end, 4):
        w = word_at(exe, va)
        op = w >> 26
        rs = (w >> 21) & 0x1F
        rt = (w >> 16) & 0x1F
        rd = (w >> 11) & 0x1F
        sa = (w >> 6) & 0x1F
        fn = w & 0x3F
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm
        target = ((w & 0x03FFFFFF) << 2) | (va & 0xF0000000)
        comment = ""
        if op == 0x0F:
            last_lui[rt] = (imm << 16) & 0xFFFFFFFF
            mnem = f"lui     ${rt}, 0x{imm:04X}"
            comment = f"-> 0x{last_lui[rt]:08X}"
        elif op == 0x09:
            mnem = f"addiu   ${rt}, ${rs}, {simm}"
            if rs in last_lui:
                comment = f"-> 0x{(last_lui[rs] + simm) & 0xFFFFFFFF:08X}"
        elif op == 0x0D:
            mnem = f"ori     ${rt}, ${rs}, 0x{imm:04X}"
            if rs in last_lui:
                comment = f"-> 0x{(last_lui[rs] | imm) & 0xFFFFFFFF:08X}"
        elif op == 0x0C:
            mnem = f"andi    ${rt}, ${rs}, 0x{imm:04X}"
        elif op == 0x08:
            mnem = f"addi    ${rt}, ${rs}, {simm}"
        elif op == 0x23:
            mnem = f"lw      ${rt}, {simm}(${rs})"
            if rs in last_lui:
                comment = f"-> 0x{(last_lui[rs] + simm) & 0xFFFFFFFF:08X}"
        elif op == 0x21:
            mnem = f"lh      ${rt}, {simm}(${rs})"
        elif op == 0x25:
            mnem = f"lhu     ${rt}, {simm}(${rs})"
        elif op == 0x20:
            mnem = f"lb      ${rt}, {simm}(${rs})"
        elif op == 0x24:
            mnem = f"lbu     ${rt}, {simm}(${rs})"
        elif op == 0x2B:
            mnem = f"sw      ${rt}, {simm}(${rs})"
            if rs in last_lui:
                comment = f"-> 0x{(last_lui[rs] + simm) & 0xFFFFFFFF:08X}"
        elif op == 0x29:
            mnem = f"sh      ${rt}, {simm}(${rs})"
        elif op == 0x28:
            mnem = f"sb      ${rt}, {simm}(${rs})"
        elif op == 0x03:
            mnem = f"jal     0x{target:08X}"
        elif op == 0x02:
            mnem = f"j       0x{target:08X}"
        elif op == 0x04:
            dest = va + 4 + (simm << 2)
            mnem = f"beq     ${rs}, ${rt}, 0x{dest:08X}"
        elif op == 0x05:
            dest = va + 4 + (simm << 2)
            mnem = f"bne     ${rs}, ${rt}, 0x{dest:08X}"
        elif op == 0x06:
            dest = va + 4 + (simm << 2)
            mnem = f"blez    ${rs}, 0x{dest:08X}"
        elif op == 0x07:
            dest = va + 4 + (simm << 2)
            mnem = f"bgtz    ${rs}, 0x{dest:08X}"
        elif op == 0x01:
            dest = va + 4 + (simm << 2)
            if rt == 0:
                mnem = f"bltz    ${rs}, 0x{dest:08X}"
            elif rt == 1:
                mnem = f"bgez    ${rs}, 0x{dest:08X}"
            else:
                mnem = f"regimm  ${rs} rt={rt} 0x{dest:08X}"
        elif op == 0x0A:
            mnem = f"slti    ${rt}, ${rs}, {simm}"
        elif op == 0x0B:
            mnem = f"sltiu   ${rt}, ${rs}, {simm}"
        elif op == 0x00:
            if fn == 0x08:
                mnem = f"jr      ${rs}"
            elif fn == 0x09:
                mnem = f"jalr    ${rd}, ${rs}"
            elif fn == 0x21:
                mnem = f"addu    ${rd}, ${rs}, ${rt}"
            elif fn == 0x23:
                mnem = f"subu    ${rd}, ${rs}, ${rt}"
            elif fn == 0x24:
                mnem = f"and     ${rd}, ${rs}, ${rt}"
            elif fn == 0x25:
                mnem = f"or      ${rd}, ${rs}, ${rt}"
            elif fn == 0x27:
                mnem = f"nor     ${rd}, ${rs}, ${rt}"
            elif fn == 0x2A:
                mnem = f"slt     ${rd}, ${rs}, ${rt}"
            elif fn == 0x2B:
                mnem = f"sltu    ${rd}, ${rs}, ${rt}"
            elif fn == 0x00:
                if w == 0:
                    mnem = "nop"
                else:
                    mnem = f"sll     ${rd}, ${rt}, {sa}"
            elif fn == 0x02:
                mnem = f"srl     ${rd}, ${rt}, {sa}"
            elif fn == 0x03:
                mnem = f"sra     ${rd}, ${rt}, {sa}"
            elif fn == 0x04:
                mnem = f"sllv    ${rd}, ${rt}, ${rs}"
            elif fn == 0x08:
                mnem = f"jr      ${rs}"
            elif fn == 0x20:
                mnem = f"add     ${rd}, ${rs}, ${rt}"
            elif fn == 0x22:
                mnem = f"sub     ${rd}, ${rs}, ${rt}"
            elif fn == 0x18:
                mnem = f"mult    ${rs}, ${rt}"
            elif fn == 0x19:
                mnem = f"multu   ${rs}, ${rt}"
            elif fn == 0x1A:
                mnem = f"div     ${rs}, ${rt}"
            elif fn == 0x1B:
                mnem = f"divu    ${rs}, ${rt}"
            elif fn == 0x10:
                mnem = f"mfhi    ${rd}"
            elif fn == 0x12:
                mnem = f"mflo    ${rd}"
            else:
                mnem = f"special fn=0x{fn:02X} rd=${rd} rs=${rs} rt=${rt}"
        elif op == 0x0E:
            mnem = f"xori    ${rt}, ${rs}, 0x{imm:04X}"
        elif op == 0x0F:
            mnem = f"lui     ${rt}, 0x{imm:04X}"
        else:
            mnem = f".word   0x{w:08X} op=0x{op:02X}"
        extra = f"  ; {comment}" if comment else ""
        lines.append(f"0x{va:08X}: {w:08X}  {mnem}{extra}")
    return lines


def find_func_end(exe: bytes, start: int, limit: int = 0x800) -> int:
    """End after first jr $ra whose delay slot is included, with a cap."""
    end = start + limit
    for va in range(start, start + limit, 4):
        w = word_at(exe, va)
        if w == 0x03E00008:
            return va + 8
    return end


def scan_jals(exe: bytes, start: int, end: int) -> list[int]:
    hits = []
    for va in range(start, end, 4):
        w = word_at(exe, va)
        if (w >> 26) == 3:
            dest = ((w & 0x03FFFFFF) << 2) | (va & 0xF0000000)
            hits.append(dest)
    return hits


def scan_abs_stores(exe: bytes, start: int, end: int) -> list[dict]:
    last_lui: dict[int, int] = {}
    hits = []
    for va in range(start, end, 4):
        w = word_at(exe, va)
        op = w >> 26
        rs = (w >> 21) & 0x1F
        rt = (w >> 16) & 0x1F
        imm = w & 0xFFFF
        simm = imm - 0x10000 if imm >= 0x8000 else imm
        if op == 0x0F:
            last_lui[rt] = (imm << 16) & 0xFFFFFFFF
        elif op in (0x2B, 0x29, 0x28, 0x23, 0x21, 0x25, 0x20, 0x24):
            if rs in last_lui:
                addr = (last_lui[rs] + simm) & 0xFFFFFFFF
                hits.append(
                    {
                        "pc": va,
                        "kind": {
                            0x2B: "sw",
                            0x29: "sh",
                            0x28: "sb",
                            0x23: "lw",
                            0x21: "lh",
                            0x25: "lhu",
                            0x20: "lb",
                            0x24: "lbu",
                        }[op],
                        "rt": rt,
                        "addr": addr,
                    }
                )
    return hits


def find_all_markers(blob: bytes) -> list[dict]:
    rows = []
    i = 0
    while i + 3 <= len(blob):
        if blob[i] in (0xFF, 0xF9) and blob[i + 1] == 0xFE:
            mid = blob[i + 2]
            rec = txt0.extract_record(blob, mid)
            # extract_record finds first occurrence of that id; skip if not this pos
            if rec and rec["marker_offset"] == i:
                tokens = txt0.decode_tokens(rec["body"], EXE_CACHE)
                rows.append(
                    {
                        "id": mid,
                        "id_hex": f"0x{mid:02X}",
                        "marker": rec["marker"].hex(),
                        "off": i,
                        "raw": rec["raw"].hex(),
                        "text": txt0.tokens_to_text(tokens),
                        "len": len(rec["raw"]),
                    }
                )
                i += max(3, len(rec["raw"]))
                continue
        i += 1
    return rows


def cmd_fmt(cmd: dict) -> str:
    args = ",".join(f"{a}" if a < 0x10000 else f"0x{a:X}" for a in cmd["args"])
    modes = ",".join(str(m) for m in cmd["modes"][: cmd["argc"]])
    return f"+0x{cmd['offset']:04X} op=0x{cmd['opcode']:02X} argc={cmd['argc']} modes=[{modes}] args=[{args}]"


def dump_module(mod: dict, ops=None) -> list[str]:
    lines = []
    for cmd in mod["commands"]:
        if ops is None or cmd["opcode"] in ops:
            lines.append(cmd_fmt(cmd))
    return lines


EXE_CACHE = b""


def main() -> int:
    global EXE_CACHE
    out = Path("local/btl0_boss")
    out.mkdir(parents=True, exist_ok=True)

    exe, pe_img = txt0.load_retail(DISC)
    EXE_CACHE = exe
    print(f"exe_ok sha256={txt0.sha256_bytes(exe)}")

    # --- opcode handlers of interest ---
    dispatch = 0x800910A0
    interesting = {
        0x0D: "message_open",
        0x1A: "rng",
        0x5A: "tagged_set",
        0x6F: "slot_alloc",
        0x70: "formation_a",
        0x89: "battle_req",
        0x94: "mode_read",
        0xB7: "formation_b",
    }
    handlers = {}
    for op, name in interesting.items():
        h = word_at(exe, dispatch + op * 4)
        handlers[op] = h
        print(f"op 0x{op:02X} {name:14s} handler=0x{h:08X}")

    # --- disassemble 0x5A / 0x70 / 0xB7 / 0x6F ---
    for op, name in interesting.items():
        h = handlers[op]
        end = find_func_end(exe, h, 0x600)
        lines = disasm_range(exe, h, end)
        (out / f"dis_{name}_0x{op:02X}.txt").write_text("\n".join(lines) + "\n")
        jals = scan_jals(exe, h, end)
        stores = scan_abs_stores(exe, h, end)
        print(f"  {name} size=0x{end-h:X} jals={[hex(j) for j in jals[:16]]}")
        print(f"    abs={[f'{s['kind']}@{s['addr']:08X}' for s in stores[:16]]}")

    # --- extract m0005i ---
    table = pst0.parse_field_table(exe)
    rec = next(r for r in table if r["index"] == 4)
    print(f"m0005i start={rec['start']:#x} end={rec['end']:#x} meta={rec['meta']:#x}")
    pkg = pe_img[rec["start"] * 2048 : rec["end"] * 2048]
    print(f"package bytes={len(pkg)} sha256={txt0.sha256_bytes(pkg)}")

    streams = txt0.slot7_streams(pkg, rec["meta"])
    print(f"slot7 count={len(streams)}")
    for s in streams:
        print(
            f"  slot7[{s['index']}] store={s['store']} size=0x{s['size']:X} "
            f"sha256={s['sha256']} pkg+0x{s['pkg_offset']:X}"
        )

    # compare stream1 to shared bank
    usa = None
    for s in streams:
        if s["index"] == 1 or s["store"] == 1:
            usa = s
    if usa is None and len(streams) > 1:
        usa = streams[1]
    print(f"usa stream sha match TXT0? {usa['sha256'] == txt0.STREAM1_SHA256 if usa else None}")
    print(f"usa size match? {usa['size'] == txt0.STREAM1_SIZE if usa else None}")

    # all markers in stream 1
    if usa:
        rows = find_all_markers(usa["bytes"])
        print(f"stream1 markers={len(rows)} ids={[r['id_hex'] for r in rows]}")
        (out / "stream1_all_messages.json").write_text(json.dumps(rows, indent=2) + "\n")
        # specifically look for 1332/1333/1334 as 16-bit or as low bytes
        for nid in (1332, 1333, 1334, 0x534, 0x535, 0x536, 49, 50, 0x31, 0x32):
            hits = [r for r in rows if r["id"] == (nid & 0xFF)]
            print(f"  id {nid} (low8=0x{nid & 0xFF:02X}) stream1 hits={len(hits)}")

        # search raw for 16-bit LE 1332/1333/1334
        for nid in (1332, 1333, 1334):
            pat = struct.pack("<H", nid)
            pos = usa["bytes"].find(pat)
            print(f"  raw u16 {nid} in stream1: {pos}")

    # also scan all slot7 stores
    for s in streams:
        blob = s["bytes"]
        for nid in (1332, 1333, 1334):
            pos = blob.find(struct.pack("<H", nid))
            pos32 = blob.find(struct.pack("<I", nid))
            print(f"  slot7[{s['index']}] u16 {nid} @ {pos}  u32 @ {pos32}")

    # script decode
    extracted = pst0.extract_script_from_package(pkg, rec["meta"])
    assert extracted
    rel, blob = extracted
    script = pst0.decode_script(blob)
    print(f"script modules={script['module_count']} declared=0x{script['declared']:X}")

    focus_ops = {0x0D, 0x1A, 0x5A, 0x6F, 0x70, 0x89, 0x94, 0xB7, 0x0A, 0x09, 0x31, 0x1C, 0x95, 0x96, 0x8A, 0x22, 0x23}
    for mod in script["modules"]:
        hits = [c for c in mod["commands"] if c["opcode"] in focus_ops]
        if not hits:
            continue
        print(f"\n=== module {mod['index']} start=+0x{mod['start']:04X} cmds={len(mod['commands'])} focus={len(hits)} ===")
        for c in hits:
            print(" ", cmd_fmt(c))

    # dump module 2 around setup and module 6 around 0x89
    for mi in (2, 6):
        mod = script["modules"][mi]
        lines = [cmd_fmt(c) for c in mod["commands"]]
        (out / f"m0005i_mod{mi}_all.txt").write_text("\n".join(lines) + "\n")
        print(f"wrote module {mi} {len(lines)} cmds")

    # 0x5A census on m0005i
    print("\n=== all 0x5A on m0005i ===")
    for mod in script["modules"]:
        for c in mod["commands"]:
            if c["opcode"] == 0x5A:
                print(f"  mod{mod['index']} {cmd_fmt(c)}")

    # 0x0D census
    print("\n=== all 0x0D/0x22 on m0005i ===")
    for mod in script["modules"]:
        for c in mod["commands"]:
            if c["opcode"] in (0x0D, 0x22, 0x23):
                print(f"  mod{mod['index']} {cmd_fmt(c)}")

    # look at 1332/1333/1334 anywhere in package
    print("\n=== package raw hits 1332/1333/1334 ===")
    for nid in (1332, 1333, 1334, 49, 50):
        for width, fmt in (("u16", "<H"), ("u32", "<I")):
            pat = struct.pack(fmt, nid)
            start = 0
            found = []
            while True:
                pos = pkg.find(pat, start)
                if pos < 0:
                    break
                found.append(pos)
                start = pos + 1
            print(f"  {nid} {width} count={len(found)} first={found[:8]}")

    # EXE refs to 1332/1333/1334 as immediates (addiu / ori / slti)
    print("\n=== EXE immediates 1332/1333/1334 ===")
    text = exe[EXE_HDR:]
    for nid in (1332, 1333, 1334, 49, 50):
        hits = []
        for off in range(0, len(text) - 4, 4):
            w = struct.unpack_from("<I", text, off)[0]
            op = w >> 26
            imm = w & 0xFFFF
            if imm == nid and op in (0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E):
                hits.append((LOAD + off, op, w))
        print(f"  imm {nid} count={len(hits)} first={[hex(h[0]) for h in hits[:12]]}")

    # battle cluster jals to 37870 / 375E0 / 17410
    print("\n=== battle-cluster jals to text fns ===")
    text_fns = {0x80037870, 0x800375E0, 0x80017410, 0x80037548, 0x800371B0, 0x8005DC4C}
    for va in range(0x80029800, 0x80031000, 4):
        w = word_at(exe, va)
        if (w >> 26) == 3:
            dest = ((w & 0x03FFFFFF) << 2) | (va & 0xF0000000)
            if dest in text_fns or dest == 0x80037870:
                print(f"  jal 0x{dest:08X} from 0x{va:08X}")

    # broader EXE jal to 37870
    print("\n=== all EXE jals to func_80037870 / 375E0 / 5DC4C ===")
    for va in range(LOAD, LOAD + len(text), 4):
        w = word_at(exe, va)
        if (w >> 26) == 3:
            dest = ((w & 0x03FFFFFF) << 2) | (va & 0xF0000000)
            if dest in (0x80037870, 0x800375E0, 0x8005DC4C, 0x80017410):
                print(f"  jal 0x{dest:08X} from 0x{va:08X}")

    print("\nDONE")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
