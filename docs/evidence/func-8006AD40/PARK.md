# func_8006AD40 — PARKED (+1 word join `move $v0,$0`; not LINK_EXACT)

**Status:** not YAML `c`. Draft `src/func_8006AD40.c` is
`in-progress-checkpoint`.

## Target

- VRAM `0x8006AD40`, file `0x5B540`, size `0x61C` (391 words), frame `0x30`.
- Direct `jal` from `func_8001220C`. One terminal `jr $ra`.

## What already matches

`era -O2 -G0` + `MASPSX_SYMBOL_AT_TEMP=1`:

- `.frame $sp,48` = **0x30**, `regs=8/0` (`$s0`–`$s6` + `$ra`), no `$s7`.
- Prologue through `andi 1` / `beqz` / `move $v0,$zero` delay.
- All six CD-read / poll pairs, both inlined CLUT pack loops (`$a1` offset,
  `$a0`/`$v0`/`$v1` homes, split `c = c | b; a = (a & 0x200) << 2; c = c | a`).
- Walk loops: `s1` from `$zero`, `sltu $v0,$s0,$v0`, delay `addu $a0,$s4,$v1`,
  `s0 = a0`. Third walk `beqz $v1` form + inner `lw`/`addiu`/`srl`/`sltu`.
- `func_8006E498(s4, 0xABADC06C)` copy-to-`$s0` then `lw 0($s0)`.
- Work2 skip delay `addiu $v0,-1`, CLUT delay rematerialize, `s0 = 1`.
- Work4: `lui`/`ori` of `0xC4B5BA04` before `lw $a0`, `s0=1` in first `jal`
  delay, subsequent results in the next `jal` delay (`0x11C` / `0x120` /
  `0x124` = first / second / third).
- `0x40` dual `-1` (`$v1` for `sh`, `$a0` for `sb`), `s5[0xEA]=0` always
  (delay of `bnez`), `0x80` test `lw`/`andi`/`bnez $v0` with delay
  `addu $v0,$zero,$zero`.
- 29 `jal`s, same callees as retail.

Command:

```
MASPSX_SYMBOL_AT_TEMP=1 python3 tools/analysis/era_link_check.py \
  src/func_8006AD40.c 0x8006AD40 0x61C -O2 -G0
```

Result: linked `.text` **1568** bytes vs target **1564** (`0x61C`),
**12** word mismatches, `nonzero_pad=0`. Every mismatch is the epilogue
shifted by one word.

## Residual

One extra `addu $v0,$zero,$zero` immediately before the shared epilogue.

Retail puts that zero in the `bnez` delay of `D_800B0CD8 & 0x80` (always
executed on both arms) and does **not** re-zero at the join:

```
andi    $v0, $v0, 0x80
bnez    $v0, mask
addu    $v0, $zero, $zero    # delay
sb      ...                  # 0x80-clear stores
mask:
lw / li -2 / and / sw
epilogue                     # $v0 already 0
```

cc1 2.7.2 can be forced to fill that delay (`v0 = 0` first in the false
arm + empty `"=r"(v0):"0"(v0)` barrier, test stays in `$v0`) but it does
**not** credit a not-taken-arm delay fill to the taken arm, so `return 0`
after the bit-0 work remains as a join `move $2,$0`. Returning the pinned
`$v0` instead drops the join but moves the `andi` dest to `$v1` and loses
`.set noreorder` (GNU as inserts a nop delay).

That is a delay-slot accounting residual, not a missing C statement. Not
YAML `c`.

## Levers that landed (keep in the draft)

- `asm volatile("" : "=r"(s2) : "0"(s2))` before `s0 = 0` so the zero is
  `addu $s0,$zero,$zero` rather than CSE from known-zero `$s2`.
- `register int z asm("$0"); s1 = z;` for the walk counters.
- `a0 = s4 + mask` **before** `if ((unsigned)s0 < (w >> 22))` so the add
  fills the `beqz` delay.
- `register unsigned int v0 asm("$2"); v0 = w >> 22; v0 = (unsigned)s0 < v0;
  if (v0)` so `sltu` dest is `$v0` not `$s0`.
- Mask `0x3FFFFF` pinned `$3` on the first two walks, `$2` on the third.
- Always-executed `s5[0xEA] = 0` (retail delay of the `0x40` `bnez`).
- Work4 store order is first→`+0x11C`, second→`+0x120`, third→`+0x124`.

## Evidence

`src/func_8006AD40.c`. ROM `asm/disc1/5B1E4.s`.
Scratch log `{SCRATCH}/check_func_8006AD40.log`.
