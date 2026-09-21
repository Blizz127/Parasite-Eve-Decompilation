# func_8005DB44 — 0x48 — MATCHED (era -O1 -G0)

Retail VRAM `0x8005DB44` (file offset `0x4E344`, size `0x48` = 18 words), in
`asm/disc1/4E2FC.s` between the asm heads `func_8005DAFC` (`0x48`) and
`func_8005DB8C`. On-path fan-in 9. Now a registered `c` span.

```
      - [0x4E2FC, asm]
      - [0x4E344, c, func_8005DB44]
      - [0x4E38C, c, func_8005DB8C]
```

## Semantics (proven from retail)

`D_800A8034` is a pool `start` word and `D_800A8038` the pool `end` word.
`a0` is in range iff `a0 < (end - start) >> 5` (unsigned); the record is

```
start + ((a0 << 5) + (&D_800A8038 - 0x10))
```

i.e. retail forms the base **in place** on the `&D_800A8038` register
(`addiu $v1,$v1,-0x10`), not on a copy (`(base - 0x10)` into `$v0`).

## Residual that was parked before, and the two levers that closed it

The previous park (`docs/evidence/func-8005DB44/PARK.md`) was a *reassociation*
residual: cc1 folded any `+` chain with an address constant `base` back to
"constants first" (`(a0<<5)+base+start`), so retail's `(a0<<5)+(base-0x10)` was
unreachable. The unblocker was to stop treating the base as an address constant
and make it a **local pointer that is decremented**, plus a **block-local shift
temp**:

1. **In-place symbol-pointer decrement** — `int *p = &D_800A8038; … p -= 4;
   return start + (sh + (int)p);` yields retail's `addu $v1,$v1,-16` on the
   symbol register, reusing `$v1` (a copy `(int *)(p - 4)` lands the `addiu` in
   `$v0` — 2 words off).
2. **Block-local `int sh = a0 << 5;`** — the named temp, assigned before the
   `p -= 4`, makes cc1 emit `sll $v0,$a0,5` *in the `beq` delay slot* and the
   `addu $v0,$v0,$v1` after it, which is retail's branch-delay schedule.
   Without the named temp the `sll` is sunk past the `addiu` (1 word off).

`-O1 -G0` (profile `era_o1_g0`) is load-bearing: `-O2 -G0` gives 2 word
mismatches (the `sll`/`addiu` order flips back). No register pins needed — the
natural allocation already gives `$v1`=base, `$v0`=end/shift, `$a1`=start,
`$a0`=index.

## Commands

```
env -u MASPSX_SYMBOL_AT_TEMP -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/check_leaf.sh func_8005DB44 0x8005DB44 0x48 -O1 -G0
# == [func_8005DB44] link-level check ==
# linked .text 80 bytes, target 0x48, word mismatches=0, nonzero_pad=0
# LINK_EXACT
# == [func_8005DB44] deep span-size check ==
# disc1_preflight: PASS (deep, 751 c / 330 asm / 2 rodata)
# == [func_8005DB44] OK (link exact + span size exact) ==
```

## Source

```c
extern int D_800A8034;
extern int D_800A8038;
int func_8005DB44(unsigned int a0) {
    int *p = &D_800A8038;
    int end = *p;
    int start = D_800A8034;
    if (a0 >= ((unsigned int)(end - start) >> 5)) return 0;
    {
        int sh = a0 << 5;
        p -= 4;
        return start + (sh + (int)p);
    }
}
```

## Adjacent twin `func_8005DAFC` (0x48) — PARKED

The neighbouring pool getter `func_8005DAFC` reads `D_800A802C` (stride `<<1`)
and the `-0x4` relation is the same family, but it is a genuine register
allocation residual: retail keeps the load base in `$v0`, the running cursor in
`$v1`, and the shifted index in `$v0` **in the `bnez` delay slot**; at `-O2 -G0`
cc1 emits the correct instruction sequence but with `$v0`/`$v1` swapped
(13/index-diff), and `-O1` is worse (16). Not registered; stays `asm`. Reopen
with a lever that forces `$v0`=base / `$v1`=cursor for a two-live-value
sequence.
