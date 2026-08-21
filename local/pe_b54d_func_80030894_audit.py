#!/usr/bin/env python3
"""PE-B54D evidence-only audit of func_80030894 and the AF54 poll gate.

Read-only. Does not import production C, invent poll=0, or move the cut.
"""
from __future__ import annotations

import hashlib
import struct
import sys
from collections import defaultdict
from pathlib import Path

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
LOAD = 0x80010000
EXE_HDR = 0x800

FUNC = 0x80030894
FUNC_SIZE = 0xC50
FUNC_END = FUNC + FUNC_SIZE  # 0x800314E4 exclusive

AD40 = 0x8006AD40
AD40_END = 0x8006B35C
AF54 = 0x8006AF54
B0AC = 0x8006B0AC

REG = [
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra",
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FATAL: {msg}")


def exe_off(addr: int) -> int:
    return addr - LOAD + EXE_HDR


def load_u32(data: bytes, addr: int) -> int:
    return struct.unpack_from("<I", data, exe_off(addr))[0]


def jal_target(word: int) -> int:
    return ((word & 0x03FFFFFF) << 2) | 0x80000000


def sext16(x: int) -> int:
    return x - 0x10000 if x & 0x8000 else x


def branch_target(pc: int, imm: int) -> int:
    return (pc + 4) + (sext16(imm) << 2)


def decode(pc: int, w: int) -> dict:
    op = (w >> 26) & 0x3F
    rs = (w >> 21) & 0x1F
    rt = (w >> 16) & 0x1F
    rd = (w >> 11) & 0x1F
    sa = (w >> 6) & 0x1F
    fn = w & 0x3F
    imm = w & 0xFFFF
    info = {
        "pc": pc, "word": w, "op": op, "rs": rs, "rt": rt, "rd": rd,
        "sa": sa, "fn": fn, "imm": imm, "mnem": "unk", "kind": "other",
        "text": f"{w:#010x}",
    }
    if op == 0:
        special = {
            0x00: "sll", 0x02: "srl", 0x03: "sra", 0x04: "sllv",
            0x06: "srlv", 0x07: "srav", 0x08: "jr", 0x09: "jalr",
            0x0C: "syscall", 0x0D: "break", 0x10: "mfhi", 0x11: "mthi",
            0x12: "mflo", 0x13: "mtlo", 0x18: "mult", 0x19: "multu",
            0x1A: "div", 0x1B: "divu", 0x20: "add", 0x21: "addu",
            0x22: "sub", 0x23: "subu", 0x24: "and", 0x25: "or",
            0x26: "xor", 0x27: "nor", 0x2A: "slt", 0x2B: "sltu",
        }
        mnem = special.get(fn, f"spec_{fn:#x}")
        info["mnem"] = mnem
        if fn == 0x08:
            info["kind"] = "jr"
            info["text"] = f"jr ${REG[rs]}"
        elif fn == 0x09:
            info["kind"] = "jalr"
            info["text"] = f"jalr ${REG[rd]}, ${REG[rs]}" if rd != 31 else f"jalr ${REG[rs]}"
        elif fn in (0x00, 0x02, 0x03):
            info["text"] = f"{mnem} ${REG[rd]}, ${REG[rt]}, {sa}"
        elif fn in (0x10, 0x12):
            info["text"] = f"{mnem} ${REG[rd]}"
        elif fn in (0x11, 0x13):
            info["text"] = f"{mnem} ${REG[rs]}"
        elif fn in (0x18, 0x19, 0x1A, 0x1B):
            info["text"] = f"{mnem} ${REG[rs]}, ${REG[rt]}"
        else:
            info["text"] = f"{mnem} ${REG[rd]}, ${REG[rs]}, ${REG[rt]}"
    elif op == 1:
        # REGIMM
        if rt == 0:
            info["mnem"] = "bltz"
            info["kind"] = "branch"
            info["target"] = branch_target(pc, imm)
            info["text"] = f"bltz ${REG[rs]}, {info['target']:#010x}"
        elif rt == 1:
            info["mnem"] = "bgez"
            info["kind"] = "branch"
            info["target"] = branch_target(pc, imm)
            info["text"] = f"bgez ${REG[rs]}, {info['target']:#010x}"
        elif rt == 16:
            info["mnem"] = "bltzal"
            info["kind"] = "branch_link"
            info["target"] = branch_target(pc, imm)
            info["text"] = f"bltzal ${REG[rs]}, {info['target']:#010x}"
        elif rt == 17:
            info["mnem"] = "bgezal"
            info["kind"] = "branch_link"
            info["target"] = branch_target(pc, imm)
            info["text"] = f"bgezal ${REG[rs]}, {info['target']:#010x}"
        else:
            info["mnem"] = f"regimm_{rt}"
            info["text"] = f"regimm rt={rt} ${REG[rs]}, {imm:#x}"
    elif op == 2:
        tgt = jal_target(w)
        info["mnem"] = "j"
        info["kind"] = "jump"
        info["target"] = tgt
        info["text"] = f"j {tgt:#010x}"
    elif op == 3:
        tgt = jal_target(w)
        info["mnem"] = "jal"
        info["kind"] = "jal"
        info["target"] = tgt
        info["text"] = f"jal {tgt:#010x}"
    elif op in (4, 5, 6, 7):
        names = {4: "beq", 5: "bne", 6: "blez", 7: "bgtz"}
        info["mnem"] = names[op]
        info["kind"] = "branch"
        info["target"] = branch_target(pc, imm)
        if op in (4, 5):
            info["text"] = f"{info['mnem']} ${REG[rs]}, ${REG[rt]}, {info['target']:#010x}"
        else:
            info["text"] = f"{info['mnem']} ${REG[rs]}, {info['target']:#010x}"
    elif op == 15:
        info["mnem"] = "lui"
        info["text"] = f"lui ${REG[rt]}, {imm:#x}"
    elif op in (8, 9, 10, 11, 12, 13, 14):
        names = {8: "addi", 9: "addiu", 10: "slti", 11: "sltiu", 12: "andi", 13: "ori", 14: "xori"}
        info["mnem"] = names[op]
        simm = sext16(imm) if op in (8, 9, 10, 11) else imm
        info["text"] = f"{info['mnem']} ${REG[rt]}, ${REG[rs]}, {simm:#x}"
    elif op in (32, 33, 34, 35, 36, 37, 38, 40, 41, 43):
        names = {
            32: "lb", 33: "lh", 34: "lwl", 35: "lw", 36: "lbu", 37: "lhu",
            38: "lwr", 40: "sb", 41: "sh", 43: "sw",
        }
        info["mnem"] = names[op]
        info["kind"] = "load" if op < 40 else "store"
        info["text"] = f"{info['mnem']} ${REG[rt]}, {sext16(imm):#x}(${REG[rs]})"
    elif op == 16:
        info["mnem"] = "cop0"
        info["text"] = f"cop0 {w:#010x}"
    elif op == 17:
        info["mnem"] = "cop1"
        info["text"] = f"cop1 {w:#010x}"
    elif op == 18:
        info["mnem"] = "cop2"
        info["text"] = f"cop2 {w:#010x}"
    else:
        info["text"] = f"op{op:02x} {w:#010x}"
    return info


def scan_range(data: bytes, start: int, end: int) -> list[dict]:
    out = []
    pc = start
    while pc < end:
        out.append(decode(pc, load_u32(data, pc)))
        pc += 4
    return out


def find_jals_to(data: bytes, target: int, text_lo: int, text_hi: int) -> list[int]:
    want = 0x0C000000 | ((target >> 2) & 0x03FFFFFF)
    sites = []
    pc = text_lo
    while pc < text_hi:
        if load_u32(data, pc) == want:
            sites.append(pc)
        pc += 4
    return sites


def find_lui_addiu_materializations(data: bytes, target: int, text_lo: int, text_hi: int) -> list[int]:
    hi = (target >> 16) & 0xFFFF
    lo = target & 0xFFFF
    # addiu form: lo may be signed; if bit15 set, hi is often target_hi+1 for la
    sites = []
    pc = text_lo
    while pc + 4 < text_hi:
        w0 = load_u32(data, pc)
        w1 = load_u32(data, pc + 4)
        if ((w0 >> 26) & 0x3F) == 15:  # lui
            lui_imm = w0 & 0xFFFF
            lui_rt = (w0 >> 16) & 0x1F
            if ((w1 >> 26) & 0x3F) == 9:  # addiu
                rs = (w1 >> 21) & 0x1F
                imm = w1 & 0xFFFF
                if rs == lui_rt and lui_imm == hi and imm == lo:
                    sites.append(pc)
                # signed la: lui hi+1, addiu negative lo
                if lo & 0x8000 and rs == lui_rt:
                    if lui_imm == ((hi + 1) & 0xFFFF) and imm == lo:
                        sites.append(pc)
        pc += 4
    return sites


def classify_repo(root: Path) -> dict[int, tuple[str, str]]:
    """addr -> (class, evidence path)."""
    out: dict[int, tuple[str, str]] = {}

    def add(addr: int, cls: str, ev: str) -> None:
        prev = out.get(addr)
        # prefer more specific / translated over matching-only
        rank = {
            "TRANSLATED_FAITHFUL": 4,
            "HOST_SDK_OR_STREAM": 3,
            "HOST_SDK_SHIM": 3,
            "MATCHING_C_LEAF": 2,
            "BOOTSTRAP_PREFIX": 2,
            "UNRESOLVED": 0,
        }
        if prev is None or rank.get(cls, 1) >= rank.get(prev[0], 0):
            out[addr] = (cls, ev)

    for p in (root / "src").glob("func_*.c"):
        try:
            addr = int(p.stem.split("_")[1], 16)
        except (IndexError, ValueError):
            continue
        add(addr, "MATCHING_C_LEAF", str(p.relative_to(root)))

    for p in (root / "pc_port" / "game" / "boot").glob("func_*_port.c"):
        name = p.stem.replace("_port", "")
        try:
            addr = int(name.split("_")[1], 16)
        except (IndexError, ValueError):
            continue
        add(addr, "TRANSLATED_FAITHFUL", str(p.relative_to(root)))

    for p in (root / "pc_port" / "bootstrap").glob("func_*_port.c"):
        name = p.stem.replace("_port", "")
        try:
            addr = int(name.split("_")[1], 16)
        except (IndexError, ValueError):
            continue
        add(addr, "BOOTSTRAP_PREFIX", str(p.relative_to(root)))

    for p in (root / "pc_port" / "platform").glob("func_*_port.c"):
        name = p.stem.replace("_port", "")
        try:
            addr = int(name.split("_")[1], 16)
        except (IndexError, ValueError):
            continue
        add(addr, "HOST_SDK_OR_STREAM", str(p.relative_to(root)))

    # Known host shims / providers from B54A census and pe_sdk.h comments.
    known_shim = {
        0x80074DC0: ("HOST_SDK_SHIM", "DrawSync / pe_libgpu.c"),
        0x80074A44: ("HOST_SDK_SHIM", "ResetGraph / pe_libgpu.c"),
        0x80073A44: ("HOST_SDK_SHIM", "VSync / pe_libetc.c"),
        0x800755F0: ("HOST_SDK_SHIM", "PutDispEnv / pe_libgpu.c"),
        0x80074D28: ("HOST_SDK_SHIM", "SetDispMask / pe_libgpu.c"),
        0x80087024: ("HOST_SDK_OR_STREAM", "stream cmd 0xF1 / pe_stream.c"),
        0x800811E4: ("HOST_SDK_OR_STREAM", "CD poll / pe_libcd.c"),
        0x8007506C: ("TRANSLATED_FAITHFUL", "pc_port/game/boot/func_8007506C_port.c"),
        0x8006E7E8: ("TRANSLATED_FAITHFUL", "pc_port/game/boot/func_8006E7E8_port.c"),
        0x8006E6A8: ("TRANSLATED_FAITHFUL", "pc_port/game/boot/func_8006E6A8_port.c"),
        0x8006E498: ("TRANSLATED_FAITHFUL", "pc_port/game/boot/func_8006E498_port.c"),
        0x8006E1C0: ("TRANSLATED_FAITHFUL", "pc_port/game/boot/func_8006E1C0_port.c"),
        0x800718D0: ("TRANSLATED_FAITHFUL", "pc_port/game/boot/func_800718D0_port.c"),
    }
    for addr, (cls, ev) in known_shim.items():
        add(addr, cls, ev)
    return out


def dump_block(insns: list[dict], start: int, end: int) -> str:
    lines = []
    for ins in insns:
        if start <= ins["pc"] < end:
            lines.append(f"{ins['pc']:08X}  {ins['word']:08X}  {ins['text']}")
    return "\n".join(lines)


def main() -> int:
    root = Path(__file__).resolve().parents[1]
    exe_path = root / "build" / "disc1.candidate.exe"
    data = exe_path.read_bytes()
    got = hashlib.sha1(data).hexdigest()
    require(got == SHA1, f"{exe_path} sha1 {got}")
    require(data[:8] == b"PS-X EXE", "not PS-X EXE")
    taddr = struct.unpack_from("<I", data, 0x18)[0]
    require(taddr == LOAD, f"taddr {taddr:#x}")

    words = (FUNC_END - FUNC) // 4
    require(words == 788, f"expected 788 words, got {words}")
    require(FUNC_SIZE == 0xC50, "size")

    # Confirm body ends on a jr ra / nop if possible, or next prologue.
    last = decode(FUNC_END - 8, load_u32(data, FUNC_END - 8))
    last_delay = decode(FUNC_END - 4, load_u32(data, FUNC_END - 4))
    after = decode(FUNC_END, load_u32(data, FUNC_END))

    insns = scan_range(data, FUNC, FUNC_END)
    ad40 = scan_range(data, AD40, AD40_END)

    jals = [i for i in insns if i["kind"] == "jal"]
    jalrs = [i for i in insns if i["kind"] == "jalr"]
    jrs = [i for i in insns if i["kind"] == "jr"]
    jumps = [i for i in insns if i["kind"] == "jump"]
    blinks = [i for i in insns if i["kind"] == "branch_link"]
    returns = [i for i in jrs if i["rs"] == 31]

    # MMIO / coprocessor
    mmio = []
    for i in insns:
        if i["mnem"] in ("lui",) and i["imm"] == 0x1F80:
            mmio.append(i)
        if i["mnem"] in ("cop0", "cop1", "cop2"):
            mmio.append(i)

    repo = classify_repo(root)
    callee_counts: dict[int, list[int]] = defaultdict(list)
    for i in jals:
        callee_counts[i["target"]].append(i["pc"])

    # Callers of 30894 in main+tail text (conservative range).
    text_lo, text_hi = 0x8001220C, 0x80091080
    tail_lo, tail_hi = 0x800C22F8, 0x800E0600
    callers = find_jals_to(data, FUNC, text_lo, text_hi)
    callers += find_jals_to(data, FUNC, tail_lo, tail_hi)
    mats = find_lui_addiu_materializations(data, FUNC, text_lo, text_hi)
    mats += find_lui_addiu_materializations(data, FUNC, tail_lo, tail_hi)

    # Static words holding the address.
    word_hits = []
    blob = data[EXE_HDR:]
    packed = struct.pack("<I", FUNC)
    off = 0
    while True:
        idx = blob.find(packed, off)
        if idx < 0:
            break
        word_hits.append(LOAD + idx)
        off = idx + 4

    print("=== FUNC GEOMETRY ===")
    print(f"func={FUNC:#010x} end={FUNC_END:#010x} bytes={FUNC_SIZE:#x} words={words}")
    print(f"last-8 {last['text']}  last-4 {last_delay['text']}")
    print(f"after  {after['text']}")
    print(f"jals={len(jals)} unique={len(callee_counts)} jalr={len(jalrs)} "
          f"jr={len(jrs)} jr_ra={len(returns)} j={len(jumps)} blink={len(blinks)}")
    print(f"mmio_or_cop={len(mmio)}")

    print("\n=== CALLEES ===")
    for tgt in sorted(callee_counts):
        cls, ev = repo.get(tgt, ("UNRESOLVED", ""))
        sites = ",".join(f"{p:#010x}" for p in callee_counts[tgt])
        print(f"{tgt:#010x}  n={len(callee_counts[tgt]):2d}  {cls:22s}  {ev}  sites={sites}")

    print("\n=== INDIRECT ===")
    for i in jalrs:
        print(f"{i['pc']:#010x}  {i['text']}")
    for i in blinks:
        print(f"{i['pc']:#010x}  {i['text']}")

    print("\n=== RETURNS ===")
    for i in returns:
        delay = decode(i["pc"] + 4, load_u32(data, i["pc"] + 4))
        print(f"{i['pc']:#010x}  {i['text']}  delay={delay['text']}")

    print("\n=== INTERNAL JUMPS OUT OF RANGE ===")
    for i in jumps:
        t = i["target"]
        inside = FUNC <= t < FUNC_END
        print(f"{i['pc']:#010x}  {i['text']}  inside={inside}")

    print("\n=== CALLERS jal ===")
    for s in callers:
        print(f"jal site {s:#010x}")
    print("=== MATERIALIZATIONS la ===")
    for s in mats:
        print(f"la site {s:#010x}")
    print("=== STATIC WORDS ===")
    for s in word_hits:
        print(f"word {s:#010x}")

    print("\n=== AF54..B0C0 AD40 WINDOW ===")
    print(dump_block(ad40, AF54, 0x8006B0C0))

    print("\n=== AD40 CALL SITES AF54..B274 ===")
    for i in ad40:
        if i["kind"] == "jal" and AF54 <= i["pc"] <= 0x8006B274:
            cls, ev = repo.get(i["target"], ("UNRESOLVED", ""))
            print(f"{i['pc']:#010x}  {i['text']}  {cls}  {ev}")

    # Control-flow facts at AF54
    print("\n=== AF54 BRANCH DECODE ===")
    for pc in (0x8006AF44, 0x8006AF48, 0x8006AF4C, 0x8006AF50, 0x8006AF54,
               0x8006AF58, 0x8006AF5C, 0x8006AF60, 0x8006AF64, 0x8006AF68,
               0x8006AE04, 0x8006AE08, 0x8006AE0C, 0x8006ADD8):
        ins = decode(pc, load_u32(data, pc))
        extra = ""
        if "target" in ins:
            extra = f"  -> {ins['target']:#010x}"
        print(f"{pc:08X}  {ins['word']:08X}  {ins['text']}{extra}")

    # First few / interesting 30894 instructions
    print("\n=== 30894 HEAD (64 words) ===")
    print(dump_block(insns, FUNC, FUNC + 64 * 4))

    print("\n=== 30894 TAIL (32 words) ===")
    print(dump_block(insns, FUNC_END - 32 * 4, FUNC_END))

    # Unique lui immediates that look like globals
    luis = defaultdict(int)
    for i in insns:
        if i["mnem"] == "lui":
            luis[i["imm"]] += 1
    print("\n=== LUI HIGHS ===")
    for hi, n in sorted(luis.items()):
        print(f"  {hi:#06x}  n={n}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
