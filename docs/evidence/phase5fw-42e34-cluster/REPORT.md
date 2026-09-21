# Phase 5FW — 42E34 cluster: three sound twins + the fade-stop companion

Date: 2026-09-18

```text
IMPLEMENTED=func_80052634 0x80052634..0x8005267C  (0x48, sound 0x44D)
            func_8005267C 0x8005267C..0x800526C4  (0x48, sound 0x44E)
            func_800526C4 0x800526C4..0x8005270C  (0x48, sound 0x44F)
            func_80052764 0x80052764..0x80052790  (0x2C, fade stop)
RESIDUAL=   func_8005270C 0x8005270C..0x80052764  (0x58, sound 0x450) — still asm
WORDS=18+18+18+11 = 65
BYTES=0x48+0x48+0x48+0x2C = 0x104
SOURCES=src/func_80052634.c, src/func_8005267C.c, src/func_800526C4.c, src/func_80052764.c
PROFILES=default era_o2_g0 (-O2 -G0) for the three twins; era_o2_g8 for func_80052764
YAML=[0x42E34 c func_80052634][0x42E7C c func_8005267C][0x42EC4 c func_800526C4]
     [0x42F0C asm][0x42F64 c func_80052764][0x42F90 c func_80052790]
LINK_CHECK=LINK_EXACT for all four (tools/analysis/era_link_check.py, 0 word mismatches)
PREFLIGHT=disc1_preflight: PASS (deep, 802 c / 349 asm / 2 rodata)
GATE=EXACT_REBUILD_GATE=PASS
GATE_SHA1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b (orig == cand)
GATE_PLAN=dc4ec738400cffbefbeafe0f38c39f5cd65581939305e7ff4d3464d39530dca0
GATE_SWEEP=VERIFY_SWEEP=PASS leaves=802
METRIC=funcs 373/979 and asm_funcs 530 are unchanged: these four leaves sit
       outside the 979-function direct-call union, so only the plan
       (798 -> 802 c spans) and the gate prove them.  See ROUTE_COVERAGE.md.
PLANTED_STATE=NO
```

## What the cluster is

The former `[0x42E34, asm]` span was five functions:

| symbol | size | sound id | shape |
| --- | --- | --- | --- |
| `func_80052634` | 0x48 | 0x44D | void twin of `func_800525EC` |
| `func_8005267C` | 0x48 | 0x44E | void twin |
| `func_800526C4` | 0x48 | 0x44F | void twin |
| `func_8005270C` | 0x58 | 0x450 | plays, then publishes the result |
| `func_80052764` | 0x2C | — | stops the published target |

All five test `D_800B0E08` (the sound package pointer) and call
`func_8006DF50(package, id, 0x100, 0x80, 0x7F)`.

## The three void twins

Byte-identical in shape to the already-verified `func_800525EC` (Phase 5FV),
differing only in the sound id, so the same source form applies unchanged:

```c
extern volatile int D_800B0E08;
void func_8006DF50(int a0, int a1, int a2, int a3, int a4);
void func_80052634(void) {
    volatile int *p = &D_800B0E08;
    if (*p != 0) { func_8006DF50(*p, 0x44D, 0x100, 0x80, 0x7F); }
}
```

All three verified `LINK_EXACT` at `-O2 -G0` on the first try.

## `func_80052764` and gp-relative data

`func_80052764` touches `D_8009D01C` through `0x2AC($gp)`:

```c
extern int D_8009D01C;
void func_80052764(void) {
    int target = D_8009D01C;
    if (target != 0) { func_800866A4(target, 0); D_8009D01C = 0; }
}
```

Confirmed `LINK_EXACT` at `-O2 -G8` (profile `era_o2_g8`, which the neighbouring
`func_80052790` already uses). Two facts made this leaf cheap and are worth
reusing:

- the build links `build/abs_syms.ld`, assembled from splat's
  `undefined_{syms,funcs}_auto.txt` plus `_gp = 0x8009CD70`, and
  `disc1_build.py` **probe-links first and auto-derives**
  `D_<addr> = 0x<addr>;` for every `D_` symbol the link reports as undefined — so
  naming a data global `D_<its VRAM address>` is all that is needed;
- the address is checkable independently: `_gp + 0x2AC = 0x8009D01C`, which is
  exactly the `GM_D_8009D01C` the port already tracks in
  `pc_port/game/boot/field_message_port.c` (`/* gp+0x2AC: func_8005270C fade
  target */`), and `battle_reward_port.c` already performs this function's body.

## Residual — `func_8005270C` (0x58)

Left as `asm`. The structure is understood and the port already implements it,
but the codegen is not yet matched. Retail:

```text
addiu sp,sp,-0x20
lui   a0,%hi(D_800B0E08)        <- address computed once into $a0
addiu a0,a0,%lo(D_800B0E08)
sw    ra,0x18(sp)
lw    v0,0x0(a0)                <- load 1 (test)
nop
beqz  v0,.L8005274C
  addiu v0,zero,0x7F            <- 5th argument in the delay slot
addiu a1,zero,0x450
addiu a2,zero,0x100
sw    v0,0x10(sp)
lw    a0,0x0(a0)                <- load 2 (call argument), same $a0
jal   func_8006DF50
  addiu a3,zero,0x80
j     .L80052750
  nop
.L8005274C: addu v0,zero,zero
.L80052750: sw v0,0x2AC($gp)
lw    ra,0x18(sp) ; addiu sp,sp,0x20 ; jr ra ; nop
```

The source must keep `$a0` live across the branch so both loads go through it
(the `lui`+`addiu` pair rather than a fused `lui`/`lw`). Exhaustive best attempt:
`-O1 -G8` with the volatile-pointer if/else form reproduces the whole structure —
22 instructions, the `j`/`nop` over the else arm, `move v0,zero`, one store —
but materialises the address twice and ends at 11/22 words.

| form | flags | word mismatches |
| --- | --- | --- |
| volatile ptr, `result=0` then `if` | `-O2 -G8` | 17 |
| plain global, if/else | `-O2 -G8` | 20 (compiler CSEs to one load — proves `volatile` is required) |
| volatile ptr, if/else | `-O2 -G8` | 21 |
| volatile ptr, if/else | **`-O1 -G8`** | **11** (structure identical, 22 instructions) |
| inverted if/else, `p[0]` forms | various | 17–20 |

`MASPSX_SYMBOL_AT_TEMP`, `MASPSX_SYMBOL_LOAD_DEST_TEMP`,
`MASPSX_THREE_WORD_SYMBOL_STORE` and `MASPSX_FILL_STORE_DELAY_SLOT` were each
tried on the best form and changed nothing. Next lever is the register
allocator, not the assembler: keep the address in `$a0` (e.g. by finding a source
spelling whose first use of the pointer is as an address rather than a loaded
value), then a `<8` mismatch should collapse.
