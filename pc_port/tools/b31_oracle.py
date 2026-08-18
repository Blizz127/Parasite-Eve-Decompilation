#!/usr/bin/env python3
"""Independent Phase 6E-B31 oracle for func_80042CC4.

The words below are an independent transcription.  They are checked against
the SHA-verified executable before this interpreter executes the transcription;
production C is never called.
"""
import hashlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
BASE = 0x80042CC4
GP = 0x8009CD70
RAM_BASE = 0x80000000
RAM_END = 0x80200000

W = [
    0x3C02800A, 0x24421878, 0xA0400000, 0x00403021,
    0x24020100, 0x00443823, 0x24C3000F, 0x00C3102B,
    0x1040000F, 0x00042200, 0x00604021, 0x90C30000,
    0x00000000, 0x0065102A, 0x10400009, 0x00E30018,
    0x00004812, 0x00891021, 0x00021203, 0xA0C20001,
    0x24C60001, 0x00C8102B, 0x1440FFF4, 0x00000000,
    0x3C02800A, 0x24421878, 0x00C21023, 0x24420001,
    0xAF820170, 0x03E00008, 0x00000000,
]

def u32(x): return x & 0xFFFFFFFF
def s32(x):
    x = u32(x)
    return x - 0x100000000 if x & 0x80000000 else x

class Oracle:
    def __init__(self, exe):
        data = open(exe, "rb").read()
        got = hashlib.sha1(data).hexdigest()
        if got != SHA1:
            raise SystemExit(f"FATAL: executable SHA-1 {got} != {SHA1}")
        taddr = struct.unpack_from("<I", data, 0x18)[0]
        for i, want in enumerate(W):
            got = struct.unpack_from("<I", data, BASE + i * 4 - taddr + 0x800)[0]
            if got != want:
                raise SystemExit(f"FATAL: word {i} @ {BASE+i*4:08X}: "
                                 f"{got:08X} != {want:08X}")
        print(f"exe SHA-1 {SHA1} OK; func_80042CC4: {len(W)} words verified")
        self.ram = bytearray(0x200000)
        self.r = [0] * 32
        self.r[28] = GP
        self.lo = 0
        self.pc = 0
        self.steps = 0
        self.reads = []
        self.writes = []

    def off(self, a, n):
        if not (RAM_BASE <= a and a + n <= RAM_END):
            raise SystemExit(f"FATAL: guest access {a:08X}/{n}")
        return a - RAM_BASE

    def load(self, a, n):
        o = self.off(a, n)
        v = int.from_bytes(self.ram[o:o+n], "little")
        self.reads.append((a, n, v))
        return v

    def store(self, a, n, v):
        o = self.off(a, n)
        v = u32(v) & ((1 << (8*n)) - 1)
        self.ram[o:o+n] = v.to_bytes(n, "little")
        self.writes.append((a, n, v))

    def one(self, w, addr):
        op = (w >> 26) & 63
        rs, rt, rd = (w >> 21) & 31, (w >> 16) & 31, (w >> 11) & 31
        sa, fn = (w >> 6) & 31, w & 63
        imm = w & 0xFFFF
        simm = imm if imm < 0x8000 else imm - 0x10000
        r = self.r
        if w == 0:
            return
        if op == 0:
            if fn == 0: r[rd] = u32(r[rt] << sa)
            elif fn == 3: r[rd] = u32(s32(r[rt]) >> sa)
            elif fn == 0x21: r[rd] = u32(r[rs] + r[rt])
            elif fn == 0x23: r[rd] = u32(r[rs] - r[rt])
            elif fn == 0x2A: r[rd] = int(s32(r[rs]) < s32(r[rt]))
            elif fn == 0x2B: r[rd] = int(u32(r[rs]) < u32(r[rt]))
            elif fn == 0x12: r[rd] = self.lo
            elif fn == 0x18: self.lo = u32(s32(r[rs]) * s32(r[rt]))
            elif fn == 8: raise SystemExit("FATAL: unexpected jr")
            else: raise SystemExit(f"FATAL: SPECIAL {fn:02X} @{addr:08X}")
        elif op == 9: r[rt] = u32(r[rs] + simm)
        elif op == 0x0F: r[rt] = u32(imm << 16)
        elif op == 0x20: r[rt] = self.load(u32(r[rs] + simm), 1)
        elif op == 0x24: r[rt] = self.load(u32(r[rs] + simm), 1)
        elif op == 0x28: self.store(u32(r[rs] + simm), 1, r[rt])
        elif op == 0x2B: self.store(u32(r[rs] + simm), 4, r[rt])
        else: raise SystemExit(f"FATAL: opcode {op:02X} @{addr:08X}")
        r[0] = 0

    def run(self, a0, a1, seed=None):
        self.r[4], self.r[5] = u32(a0), u32(a1)
        if seed:
            for i, value in enumerate(seed): self.store(0x800A1878 + i, 1, value)
        while True:
            if self.steps > 200: raise SystemExit("FATAL: oracle did not terminate")
            self.steps += 1
            addr = BASE + self.pc * 4
            w = W[self.pc]
            op = (w >> 26) & 63
            rs, rt = (w >> 21) & 31, (w >> 16) & 31
            imm = w & 0xFFFF
            simm = imm if imm < 0x8000 else imm - 0x10000
            fn = w & 63
            delay = W[self.pc + 1]
            def d(): self.one(delay, addr + 4)
            if op in (4, 5):
                take = (self.r[rs] == self.r[rt]) if op == 4 else (self.r[rs] != self.r[rt])
                d(); self.pc = ((addr + 4 + (simm << 2) - BASE) // 4) if take else self.pc + 2
                continue
            if op == 0 and fn == 8:
                d(); return self.r[2]
            self.one(w, addr)
            self.pc += 1

def main():
    if len(sys.argv) != 2: raise SystemExit("usage: b31_oracle.py executable")
    o = Oracle(sys.argv[1]); o.run(0x90, 0xFF)
    assert o.writes[-1] == (0x8009CEE0, 4, 9)
    assert [v for a, n, v in o.writes if a == 0x800A1878][:1] == [0]
    o = Oracle(sys.argv[1]); o.run(0x90, 0)
    assert o.writes[-1] == (0x8009CEE0, 4, 1)
    o = Oracle(sys.argv[1]); o.run(0x90, 0x80)
    assert o.writes[-1] == (0x8009CEE0, 4, 2)
    print("PASS: B31 independent transcription, delay slots, ramp writes, threshold paths, and count")

if __name__ == "__main__": main()
