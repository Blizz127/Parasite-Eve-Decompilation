# `func_800877BC` — matched hardware halfword setter

Status: MATCHED, volume-campaign attempt 7, leaf 291.

## Function hood

Retail span: file `[0x77FBC,0x77FD4)`, VRAM `0x800877BC`, six words. The body
ends in canonical `jr ra` plus delay-slot `nop`. The previous boundary word at
`0x77FB8` is the delay-slot store ending `func_80087798`; the following real
function begins at `0x77FD4` (`func_800877D4`, `sll a0,a0,4`).

Three unique direct callers target the exact start:

```text
0x80087928  (file 0x78128)
0x8008C78C  (file 0x7CF8C)
0x8008C96C  (file 0x7D16C)
```

`FUNCTION_HOOD=PROVEN`: canonical return, real boundaries, and exact-start
callers. This is a callable hardware-register helper, not padding.

## Screens and minimal C

Retail computes an indexed absolute hardware address and stores a halfword:

```c
void func_800877BC(int a0, unsigned short a1) {
    *(volatile unsigned short *)(0x1F801C04 + (a0 << 4)) = a1;
}
```

- Frame: 0; no callee and no loop.
- Globals: no symbolic C global; the address is the proven PSX hardware register base `0x1F801C04`.
- Coloring: scaled index in `$a0`, absolute base in `$at`, store source in `$a1`.
- `$v0` liveness: none; return is void.
- Address retention: `$at` carries the computed I/O address through the store.
- Flags: `era_compile ... -O2 -G0`; no gp, 3W, pins, or inline assembly.

The `volatile` access is required to preserve the retail MMIO store semantics;
the source contains no guest-memory initialization or unrelated state.

## Single-leaf object comparison

```text
file     retail       candidate       instruction
77FBC    00210400     00210400        sll a0,a0,4
77FC0    801F013C     801F013C        lui at,0x1F801C04>>16
77FC4    21088100     21088100        addu at,a0,at
77FC8    041C25A4     041C25A4        sh a1,0x1C04(at)
77FCC    0800E003     0800E003        jr ra
77FD0    00000000     00000000        nop (delay slot)
```

Relocations normalized: none; `BYTE_EXACT=6/6`.

## Carve geometry

The prior asm span was `[0x77C28,0x7B31C)`, size `0x36F4`:

```text
prefix:  0x77FBC - 0x77C28 = 0x394
C leaf:  0x77FD4 - 0x77FBC = 0x18
resume:  0x7B31C - 0x77FD4 = 0x3348
closure: 0x394 + 0x18 + 0x3348 = 0x36F4
```

The packed boundary span `[0x77FB8,0x77FE0)` is identical:

```text
file     retail       candidate
77FB8    020086A4     020086A4
77FBC    00210400     00210400
77FC0    801F013C     801F013C
77FC4    21088100     21088100
77FC8    041C25A4     041C25A4
77FCC    0800E003     0800E003
77FD0    00000000     00000000
77FD4    00210400     00210400
77FD8    C2280500     C2280500
```

## Full gates

```text
build_us.sh: RESULT: EXACT MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; compare EXACT MATCH; 291 leaves
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml: 291
```
