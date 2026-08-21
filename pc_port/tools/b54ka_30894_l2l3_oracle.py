#!/usr/bin/env python3
"""Phase 6E-B54K-A independent oracle: func_80030894 L2/L3 prefix.

Verifies, against the SHA-1-exact retail executable SLUS_006.62, with no
production imports:

  1. Window identity: 0x80030894..0x80030AC4 exclusive (140 words),
     first excluded word `move a0,zero` (0x00002021) at 0x80030AC4.
  2. All 140 window words are cross-checked as they are executed.
  3. The 6AD40 tail 0x8006B060..0x8006B0BC: literal words, jal targets
     (func_8006E6A8 / func_800718D0 / func_80030894), retry back-edge,
     and the not-taken s2==-1 beq.
  4. Delay-slot-aware execution of the 30894 window over a seeded guest
     RAM (palette record via func_8005DADC(139)), with the six callees
     modeled from their own verified retail contracts:
       GetTPage / GetClut / SetPolyFT4 / SetSemiTrans / func_8005DADC /
       func_800370DC (incl. func_80077CB4 success: head[3]=6, tail=0).
  5. Exact final state: bank-0 record bytes/halfwords, the 40 sprite
     packets (len 6, draw-mode 0xE1000234, zeroed tail, 0x64 code byte,
     clut 0x7E13 at +0x16), and exactly one cut stop at 0x80030AC4.
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys

SHA1 = "452fb033f2eaa4b18aa20a5bca60b8125af3a37b"
WIN = 0x80030894
CUT = 0x80030AC4

PALETTE = 0x8012F000
DADC_WORD = 0x800A8030
RECORD = 0x800BE9F0
SPRITE = 0x800B01C0
STACK = 0x801FF000

GETTPAGE = 0x80077A64
GETCLUT = 0x80077AA4
DADC = 0x8005DADC
SETFT4 = 0x80077BA4
SETST = 0x80077B04
WRAP = 0x800370DC

# 6AD40 tail constants (checked, not executed, by this oracle)
TAIL = [
    (0x8006B060, 0x00008021),  # move s0, zero
    (0x8006B064, 0x3C118009),  # lui s1, 0x8009
    (0x8006B068, 0x263130F0),  # addiu s1, 0x30F0
    (0x8006B06C, 0x2412FFFF),  # s2 = -1
    (0x8006B070, 0x8EA5014C),  # lw a1, 0x14C(s5)
    (0x8006B074, 0x96220000),  # lhu v0, 0(s1)
    (0x8006B078, 0x96260002),  # lhu a2, 2(s1)
    (0x8006B07C, 0x02C22021),  # a0 = s6 + v0
    (0x8006B080, 0x0C01B9AA),  # jal func_8006E6A8
    (0x8006B084, 0x00C23023),  # delay: a2 = a2 - v0
    (0x8006B088, 0x1052FFF9),  # beq v0, s2, B070
    (0x8006B08C, 0x00000000),  # nop
    (0x8006B090, 0x24120001),  # s2 = 1
    (0x8006B094, 0x2411FFFF),  # s1 = -1
    (0x8006B098, 0x16000006),  # bnez s0, B0B4
    (0x8006B09C, 0x00000000),  # nop
    (0x8006B0A0, 0x8EA40180),  # lw a0, 0x180(s5)
    (0x8006B0A4, 0x0C01C634),  # jal func_800718D0
    (0x8006B0A8, 0x24100001),  # delay: s0 = 1
    (0x8006B0AC, 0x0C00C225),  # jal func_80030894
    (0x8006B0B0, 0x00000000),  # nop
    (0x8006B0B4, 0x1251FFEB),  # beq s2, s1, B064
    (0x8006B0B8, 0x00000000),  # nop
]


def require(cond: bool, msg: str) -> None:
    if not cond:
        raise SystemExit(f"FAIL: {msg}")


class Machine:
    """Delay-slot-aware MIPS-I interpreter for the executed window.

    Control transfers sample the condition at issue, execute the delay
    slot exactly once, then take the target (b53d/b51 pattern).  Modeled
    jals never enter the callee body: the delay slot runs, then the
    verified B54I/GPU1 contract is applied, then PC = jal+8.
    """

    def __init__(self, data: bytes) -> None:
        self.data = data
        self.ram: dict[int, int] = {}
        self.reg = [0] * 32
        # Pre-prologue SP; addiu sp,-88 lands the frame at STACK.
        self.reg[29] = (STACK + 88) & 0xFFFFFFFF
        self.writes: list[tuple[int, int, int]] = []  # (addr, size, value)
        self.calls: list[tuple[int, tuple[int, ...], int]] = []
        self.pc = WIN
        self.wrap_calls = 0
        self.steps = 0

    def setr(self, index: int, value: int) -> None:
        if index:
            self.reg[index] = value & 0xFFFFFFFF

    # -- memory ---------------------------------------------------------
    def rd(self, addr: int, size: int, signed: bool = False) -> int:
        v = 0
        for i in range(size):
            v |= self.ram.get((addr + i) & 0xFFFFFFFF, 0) << (8 * i)
        if signed and (v >> (size * 8 - 1)) & 1:
            v -= 1 << (size * 8)
        return v

    def wr(self, addr: int, size: int, val: int) -> None:
        for i in range(size):
            self.ram[(addr + i) & 0xFFFFFFFF] = (val >> (8 * i)) & 0xFF
        self.writes.append((addr & 0xFFFFFFFF, size, val))

    # -- modeled callees (verified retail contracts) ---------------------
    def call(self, target: int, a0: int, a1: int, a2: int, a3: int) -> int:
        if target == GETTPAGE:
            v = ((a0 & 3) << 7) | ((a1 & 3) << 5) | ((a3 & 0x100) >> 4)
            v |= ((a2 & 0x3FF) >> 6) | ((a3 & 0x200) << 2)
            return v & 0xFFFFFFFF
        if target == GETCLUT:
            x = a0 if a0 < 0x80000000 else a0 - 0x100000000
            return ((a1 << 6) | ((x >> 4) & 0x3F)) & 0xFFFF
        if target == DADC:
            return (self.rd(DADC_WORD, 4) + 0x800A8028 +
                    ((a0 & 0xFFFFFFFF) << 3)) & 0xFFFFFFFF
        if target == SETFT4:
            self.wr(a0 + 3, 1, 9)
            self.wr(a0 + 7, 1, 0x2C)
            return 0
        if target == SETST:
            cur = self.rd(a0 + 7, 1)
            self.wr(a0 + 7, 1, (cur | 2) if a1 else (cur & 0xFD))
            return 0
        if target == WRAP:
            self.wrap_calls += 1
            self.wr(a0 + 3, 1, 1)
            self.wr(a0 + 4, 4, 0xE1000200 | (a1 & 0x9FF))
            s = (a0 + 8) & 0xFFFFFFFF
            self.wr(s + 3, 1, 4)
            self.wr(s + 7, 1, 0x64)
            head_len = self.rd(a0 + 3, 1)
            tail_len = self.rd(s + 3, 1)
            total = head_len + tail_len + 1
            if total < 17:
                self.wr(a0 + 3, 1, total)   # success: len byte
                self.wr(s, 4, 0)            # success: zero tail tag
                return 0
            return -1                        # fail: B(38h)(-1) no-op
        raise SystemExit(f"FAIL: unmodeled callee {target:#x}")

    def word(self, addr: int) -> int:
        off = addr - 0x8000F800
        return struct.unpack_from("<I", self.data, off)[0]

    def step(self, pc: int) -> int:
        """Execute the word at `pc`; return the next PC."""
        word = self.word(pc)
        op = word >> 26
        rs = (word >> 21) & 31
        rt = (word >> 16) & 31
        rd = (word >> 11) & 31
        sa = (word >> 6) & 31
        imm = word & 0xFFFF
        simm = imm - 0x10000 if imm & 0x8000 else imm
        target_addr = ((word & 0x3FFFFFF) << 2) | 0x80000000
        R = self.reg

        if op == 0x00:
            fun = word & 63
            if fun == 0x21:  # addu (incl. move)
                self.setr(rd, R[rs] + R[rt])
                return (pc + 4) & 0xFFFFFFFF
            if fun == 0x25:  # or
                self.setr(rd, R[rs] | R[rt])
                return (pc + 4) & 0xFFFFFFFF
            if fun == 0x23:  # subu
                self.setr(rd, R[rs] - R[rt])
                return (pc + 4) & 0xFFFFFFFF
            if fun == 0x00:  # sll / nop
                self.setr(rd, (R[rt] << sa) & 0xFFFFFFFF)
                return (pc + 4) & 0xFFFFFFFF
            if fun == 0x02:  # srl
                self.setr(rd, (R[rt] & 0xFFFFFFFF) >> sa)
                return (pc + 4) & 0xFFFFFFFF
            if fun == 0x08:  # jr
                target = R[rs]
                self.step((pc + 4) & 0xFFFFFFFF)
                return target & 0xFFFFFFFF
            raise SystemExit(f"FAIL: special {fun} @{pc:#x}")
        if op == 0x02:  # j
            self.step((pc + 4) & 0xFFFFFFFF)
            return target_addr
        if op == 0x03:  # jal — modeled callee, never enter the body
            self.setr(31, pc + 8)
            self.step((pc + 4) & 0xFFFFFFFF)
            args = (R[4], R[5], R[6], R[7])
            ret = self.call(target_addr, *args)
            self.calls.append((target_addr, args, pc))
            self.setr(2, ret)
            return (pc + 8) & 0xFFFFFFFF
        if op == 0x04 or op == 0x05:  # beq / bne
            taken = (R[rs] == R[rt]) if op == 0x04 else (R[rs] != R[rt])
            target = (pc + 4 + simm * 4) & 0xFFFFFFFF
            self.step((pc + 4) & 0xFFFFFFFF)
            return target if taken else (pc + 8) & 0xFFFFFFFF
        if op == 0x08 or op == 0x09:  # addi / addiu
            self.setr(rt, R[rs] + simm)
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x0F:  # lui
            self.setr(rt, imm << 16)
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x0C:  # andi
            self.setr(rt, R[rs] & imm)
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x0D:  # ori
            self.setr(rt, R[rs] | imm)
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x0B:  # sltiu (imm sign-extended, then unsigned compare)
            self.setr(rt, int((R[rs] & 0xFFFFFFFF) < (simm & 0xFFFFFFFF)))
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x20:  # lb
            self.setr(rt, self.rd(R[rs] + simm, 1, signed=True))
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x24:  # lbu
            self.setr(rt, self.rd(R[rs] + simm, 1))
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x25:  # lhu
            self.setr(rt, self.rd(R[rs] + simm, 2))
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x23:  # lw
            self.setr(rt, self.rd(R[rs] + simm, 4))
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x28:  # sb
            self.wr(R[rs] + simm, 1, R[rt] & 0xFF)
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x29:  # sh
            self.wr(R[rs] + simm, 2, R[rt] & 0xFFFF)
            return (pc + 4) & 0xFFFFFFFF
        if op == 0x2B:  # sw
            self.wr(R[rs] + simm, 4, R[rt])
            return (pc + 4) & 0xFFFFFFFF
        raise SystemExit(f"FAIL: op {op:#x} @{pc:#x}")

    def run_to(self, cut: int, limit: int = 20000) -> None:
        while self.pc != cut:
            self.steps += 1
            if self.steps > limit:
                raise SystemExit(
                    f"FAIL: step limit at pc={self.pc:#x} (want {cut:#x})")
            self.pc = self.step(self.pc)


def find_exe() -> pathlib.Path:
    here = pathlib.Path(__file__).resolve()
    for cand in (here.parent.parent.parent / "build" / "disc1.candidate.exe",
                 here.parent.parent / "build" / "disc1.candidate.exe",
                 pathlib.Path("build/disc1.candidate.exe"),
                 pathlib.Path("pc_port/build/disc1.candidate.exe")):
        if cand.is_file():
            return cand
    raise SystemExit("FAIL: could not locate disc1.candidate.exe")


def load_exe(path: pathlib.Path) -> bytes:
    data = path.read_bytes()
    digest = hashlib.sha1(data).hexdigest()
    require(digest == SHA1, f"exe sha1 {digest}")
    return data


def run(data: bytes) -> int:
    checks = 0

    # 1. Window identity + first excluded word.
    require((CUT - WIN) % 4 == 0, "window not word aligned")
    require(data[CUT - 0x80010000 + 0x800:CUT - 0x80010000 + 0x804] ==
            struct.pack("<I", 0x00002021), "first excluded word")
    checks += 1

    # 2. 6AD40 tail literal words and targets.
    for addr, want in TAIL:
        got = struct.unpack_from(
            "<I", data, addr - 0x80010000 + 0x800)[0]
        require(got == want, f"6AD40 tail @{addr:#x} {got:#010x}")
    j0 = (TAIL[8][1] & 0x3FFFFFF) << 2 | 0x80000000
    j1 = (TAIL[17][1] & 0x3FFFFFF) << 2 | 0x80000000
    j2 = (TAIL[19][1] & 0x3FFFFFF) << 2 | 0x80000000
    require(j0 == 0x8006E6A8 and j1 == 0x800718D0 and j2 == WIN,
            "6AD40 tail jal targets")
    checks += 1

    # 3. Execute the window on seeded RAM.
    m = Machine(data)
    m.wr(DADC_WORD, 4, (PALETTE - 0x800A8028 - 139 * 8) & 0xFFFFFFFF)
    m.wr(PALETTE + 0, 1, 0x11)
    m.wr(PALETTE + 1, 1, 0x22)
    m.wr(PALETTE + 2, 2, 0xBEEF)
    m.wr(PALETTE + 4, 1, 3)
    m.wr(PALETTE + 5, 1, 7)
    seed_writes = len(m.writes)
    m.run_to(CUT)
    checks += 1

    # 4. Call census: 6 calls in ROM order.
    kinds = [c[0] for c in m.calls]
    require(kinds == [GETTPAGE, GETCLUT, DADC, SETFT4, GETTPAGE,
                      SETST] + [WRAP] * 40,
            f"call census wrong ({len(kinds)} calls)")
    require(m.calls[0][1] == (0, 1, 0x100, 0x1E0), "GetTPage args 1")
    require(m.calls[1][1][0] == 0x130 and m.calls[1][1][1] == 0x1F8,
            "GetClut args")
    require(m.calls[2][1][0] == 139, "DADC index")
    require(m.calls[4][1] == (0, 0, 0x1C0, 0), "GetTPage args 2")
    require(m.calls[5][1][1] == 1, "SetSemiTrans abr must be 1")
    require(all(c[1][1] == 0x34 for c in m.calls[6:]),
            "wrap_sprt a1 is not the masked GetTPage 0x34")
    checks += 1

    # 5. Final record state.
    require(m.rd(RECORD + 0x03, 1) == 9, "FT4 len byte")
    require(m.rd(RECORD + 0x07, 1) == 0x2E, "code byte 0x2C|2")
    require(m.rd(RECORD + 0x0C, 1) == 0x11 and
            m.rd(RECORD + 0x0D, 1) == 0x22, "u1/v1")
    require(m.rd(RECORD + 0x14, 1) == 0x14 and
            m.rd(RECORD + 0x15, 1) == 0x22, "u2/v2")
    require(m.rd(RECORD + 0x1C, 1) == 0x11 and
            m.rd(RECORD + 0x1D, 1) == 0x29, "u3/v3")
    require(m.rd(RECORD + 0x24, 1) == 0x14 and
            m.rd(RECORD + 0x25, 1) == 0x29, "u4/v4")
    require(m.rd(RECORD + 0x16, 2) == 7, "tpage halfword")
    require(m.rd(RECORD + 0x0E, 2) == 0xBEEF, "clut halfword")
    for off in (0x08, 0x0A, 0x10, 0x12, 0x18, 0x1A, 0x20, 0x22):
        require(m.rd(RECORD + off, 2) == 0, f"xy zero @{off:#x}")
    for off in (0x04, 0x05, 0x06):
        require(m.rd(RECORD + off, 1) == 0, f"byte zero @{off:#x}")
    checks += 1

    # 6. Sprite array: 40 packets, exact per-packet state.
    for j in range(10):
        for k in range(4):
            p = SPRITE + j * 140 + k * 28
            require(m.rd(p + 3, 1) == 6, f"len @{p:#x}")
            require(m.rd(p + 4, 4) == 0xE1000234, f"draw-mode @{p:#x}")
            require(m.rd(p + 8, 4) == 0, f"tail zero @{p:#x}")
            require(m.rd(p + 15, 1) == 0x64, f"code @{p:#x}")
            require(m.rd(p + 0x16, 2) == 0x7E13, f"clut @{p:#x}")
    require(m.rd(SPRITE + 10 * 140 + 0x16, 2) == 0, "array extent overrun")
    checks += 1

    # 7. Stack state: font triple 0, clut 0x7E13 at +32, bank 0 at +24,
    #    scratch i*12 = 0 at +40.
    require(m.rd(STACK + 16, 1) == 0 and m.rd(STACK + 17, 1) == 0 and
            m.rd(STACK + 18, 1) == 0, "font triple")
    require(m.rd(STACK + 32, 2) == 0x7E13, "stack clut")
    require(m.rd(STACK + 24, 1) == 0, "bank counter")
    require(m.rd(STACK + 40, 4) == 0, "scratch i*12")
    checks += 1

    # 8. Every executed word was the exe's own (the interpreter reads
    #    only from `data`); write log is bounded to the seeded RAM diff.
    writes = m.writes[seed_writes:]
    addrs = {a for a, _, _ in writes}
    require(max(addrs) < 0x80200000 and min(addrs) >= 0x80000000,
            "write outside guest RAM")
    checks += 1

    return checks


def main(argv: list[str]) -> int:
    path = pathlib.Path(argv[1]) if len(argv) == 2 else find_exe()
    data = load_exe(path)
    checks = run(data)
    print(f"B54K-A oracle: PASS ({checks}/{checks} groups; {path})")
    print(f"  window {WIN:#010x}..{CUT:#010x} (140 words, cut first word")
    print("         move a0,zero)")
    print("  calls  GetTPage/GetClut/DADC/SetPolyFT4/GetTPage/")
    print("         SetSemiTrans + 40x wrap_sprt")
    print("  state  bank-0 record + 40 sprite packets exact")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
