#!/usr/bin/env python3
from __future__ import annotations
import hashlib, pathlib, struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
TADDR = 0x80010000
REGS = [
    "zero","at","v0","v1","a0","a1","a2","a3",
    "t0","t1","t2","t3","t4","t5","t6","t7",
    "s0","s1","s2","s3","s4","s5","s6","s7",
    "t8","t9","k0","k1","gp","sp","fp","ra",
]

def exe_off(a):
    return a - TADDR + 0x800

def load_u32(data, a):
    return struct.unpack_from("<I", data, exe_off(a))[0]

def jal_target(word):
    return ((word & 0x03FFFFFF) << 2) | 0x80000000

def find_end(data, start, limit=0x800):
    for addr in range(start, start + limit, 4):
        w = load_u32(data, addr)
        if w == 0x03E00008:  # jr ra
            return addr + 8
    return start

def dis(word, addr):
    op = word >> 26
    rs, rt, rd, sa = (word >> 21) & 31, (word >> 16) & 31, (word >> 11) & 31, (word >> 6) & 31
    fn = word & 63
    imm = word & 0xFFFF
    simm = imm - 0x10000 if imm >= 0x8000 else imm
    tgt = ((word & 0x03FFFFFF) << 2) | (addr & 0xF0000000)
    if op == 0:
        if word == 0:
            return "nop"
        names = {
            0: f"sll {REGS[rd]}, {REGS[rt]}, {sa}",
            2: f"srl {REGS[rd]}, {REGS[rt]}, {sa}",
            3: f"sra {REGS[rd]}, {REGS[rt]}, {sa}",
            8: f"jr {REGS[rs]}",
            9: f"jalr {REGS[rs]}",
            0x0C: "syscall",
            0x0D: "break",
            0x10: f"mfhi {REGS[rd]}",
            0x12: f"mflo {REGS[rd]}",
            0x18: f"mult {REGS[rs]}, {REGS[rt]}",
            0x19: f"multu {REGS[rs]}, {REGS[rt]}",
            0x1A: f"div {REGS[rs]}, {REGS[rt]}",
            0x21: f"addu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x23: f"subu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x24: f"and {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x25: f"or {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x27: f"nor {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2A: f"slt {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
            0x2B: f"sltu {REGS[rd]}, {REGS[rs]}, {REGS[rt]}",
        }
        return names.get(fn, f"spec {fn:02x}")
    if op == 1:
        kind = rt
        tgtb = addr + 4 + simm * 4
        if kind == 0:
            return f"bltz {REGS[rs]}, {tgtb:#x}"
        if kind == 1:
            return f"bgez {REGS[rs]}, {tgtb:#x}"
        return f"regimm {kind}"
    ops = {
        2: f"j {tgt:#x}", 3: f"jal {tgt:#x}",
        4: f"beq {REGS[rs]}, {REGS[rt]}, {addr+4+simm*4:#x}",
        5: f"bne {REGS[rs]}, {REGS[rt]}, {addr+4+simm*4:#x}",
        6: f"blez {REGS[rs]}, {addr+4+simm*4:#x}",
        7: f"bgtz {REGS[rs]}, {addr+4+simm*4:#x}",
        8: f"addi {REGS[rt]}, {REGS[rs]}, {simm}",
        9: f"addiu {REGS[rt]}, {REGS[rs]}, {simm}",
        0x0A: f"slti {REGS[rt]}, {REGS[rs]}, {simm}",
        0x0B: f"sltiu {REGS[rt]}, {REGS[rs]}, {simm}",
        0x0C: f"andi {REGS[rt]}, {REGS[rs]}, {imm:#x}",
        0x0D: f"ori {REGS[rt]}, {REGS[rs]}, {imm:#x}",
        0x0E: f"xori {REGS[rt]}, {REGS[rs]}, {imm:#x}",
        0x0F: f"lui {REGS[rt]}, {imm:#x}",
        0x20: f"lb {REGS[rt]}, {simm}({REGS[rs]})",
        0x21: f"lh {REGS[rt]}, {simm}({REGS[rs]})",
        0x23: f"lw {REGS[rt]}, {simm}({REGS[rs]})",
        0x24: f"lbu {REGS[rt]}, {simm}({REGS[rs]})",
        0x25: f"lhu {REGS[rt]}, {simm}({REGS[rs]})",
        0x28: f"sb {REGS[rt]}, {simm}({REGS[rs]})",
        0x29: f"sh {REGS[rt]}, {simm}({REGS[rs]})",
        0x2B: f"sw {REGS[rt]}, {simm}({REGS[rs]})",
    }
    return ops.get(op, f"op{op:02x} {word:08x}")

data = pathlib.Path("/home/blizz/dev/parasite-eve-port-black/build/disc1.candidate.exe").read_bytes()
assert hashlib.sha1(data).hexdigest() == SHA1

for name, start in [("39B74", 0x80039B74), ("3A6A8", 0x8003A6A8), ("3A088", 0x8003A088)]:
    end = find_end(data, start)
    words = (end - start) // 4
    sha = hashlib.sha256(data[exe_off(start):exe_off(end)]).hexdigest()
    jals = []
    for a in range(start, end, 4):
        w = load_u32(data, a)
        if w >> 26 == 3:
            jals.append((a, jal_target(w)))
    print(f"=== {name} {start:#x}..{end:#x} {words}w sha={sha} jals={len(jals)}")
    for a, t in jals:
        print(f"    jal {t:#x} @{a:#x}")

print("\n=== 39B74 body ===")
end = find_end(data, 0x80039B74)
for addr in range(0x80039B74, end, 4):
    w = load_u32(data, addr)
    print(f"  {addr:08X}  {w:08X}  {dis(w, addr)}")
