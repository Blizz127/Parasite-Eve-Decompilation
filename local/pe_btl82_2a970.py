#!/usr/bin/env python3
from __future__ import annotations
import hashlib, pathlib, struct

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
ROOT = pathlib.Path("/home/blizz/dev/parasite-eve-port-black")
EXE = ROOT / "build" / "disc1.candidate.exe"
TADDR, HDR = 0x80010000, 0x800
REGS = [
    "zero","at","v0","v1","a0","a1","a2","a3",
    "t0","t1","t2","t3","t4","t5","t6","t7",
    "s0","s1","s2","s3","s4","s5","s6","s7",
    "t8","t9","k0","k1","gp","sp","fp","ra",
]

def load_u32(blob, va):
    return struct.unpack_from("<I", blob, va - TADDR + HDR)[0]
def jal_target(w): return ((w & 0x03FFFFFF) << 2) | 0x80000000
def jal_sites(blob, target):
    want = 0x0C000000 | ((target & 0x0FFFFFFF) >> 2)
    text = blob[HDR:HDR+0x1EE000]
    return [TADDR+i for i in range(0,len(text),4) if struct.unpack_from("<I",text,i)[0]==want]
def op(w): return w>>26
def rs(w): return (w>>21)&31
def rt(w): return (w>>16)&31
def rd(w): return (w>>11)&31
def imm16(w): return w&0xFFFF
def simm16(w):
    v=w&0xFFFF
    return v-0x10000 if v>=0x8000 else v
def funct(w): return w&0x3F
def dis(w, va):
    o=op(w)
    if o==0:
        f=funct(w)
        if w==0: return "nop"
        if f==8: return f"jr ${REGS[rs(w)]}"
        if f==9: return f"jalr ${REGS[rd(w)]},${REGS[rs(w)]}"
        if f==0x21: return f"addu ${REGS[rd(w)]},${REGS[rs(w)]},${REGS[rt(w)]}"
        return f"special {f:#x}"
    if o==2: return f"j {(va&0xF0000000)|((w&0x03FFFFFF)<<2):#x}"
    if o==3: return f"jal {jal_target(w):#x}"
    if o==4: return f"beq ${REGS[rs(w)]},${REGS[rt(w)]}, {va+4+simm16(w)*4:#x}"
    if o==5: return f"bne ${REGS[rs(w)]},${REGS[rt(w)]}, {va+4+simm16(w)*4:#x}"
    if o==9: return f"addiu ${REGS[rt(w)]},${REGS[rs(w)]},{simm16(w)}"
    if o==0x0F: return f"lui ${REGS[rt(w)]},{imm16(w):#x}"
    names={0x20:"lb",0x21:"lh",0x23:"lw",0x24:"lbu",0x25:"lhu",0x28:"sb",0x29:"sh",0x2B:"sw"}
    if o in names: return f"{names[o]} ${REGS[rt(w)]},{simm16(w):#x}(${REGS[rs(w)]})"
    return f"op{o:#x} {w:08x}"

def dump(blob, start, n):
    for i in range(n):
        va=start+i*4
        w=load_u32(blob,va)
        print(f"  {va:08X}  {w:08X}  {dis(w,va)}")

blob = EXE.read_bytes()
assert hashlib.sha1(blob).hexdigest()==SHA1
print("=== 2A8C0..2A9B4 ===")
dump(blob, 0x8002A8C0, 40)
print("\n=== 2CEC0 jals ===")
print("2CEC8 jals", [hex(x) for x in jal_sites(blob, 0x8002CEC8)])
print("2CEB0 func guess dump")
dump(blob, 0x8002CEB0, 16)
print("\n=== CDB4C ptr file+0xd171c context ===")
off=0xD171C
print(hex(struct.unpack_from("<I", blob, off)[0]), "at", hex(off))
for i in range(-4,8):
    w=struct.unpack_from("<I", blob, off+i*4)[0]
    print(f"  file+{off+i*4:#x} {w:#010x}")
