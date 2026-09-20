# func_800CE688 — wave7-a

- **VRAM** 0x800CE688, **file** 0xBEE88, **size** 0x104 (splat).
- **Profile** default `era_o2_g0` (`-O2 -G0`), no per-leaf gates.
- **Source** `src/func_800CE688.c`.

## What it is

Effect-slot callback list update. Walks `arg0->4` records of `arg0->0` bytes
from `arg0+0xC`; for each live slot (halfword at +0) it saves `D_800E27EC`,
publishes the halfword at +2, invokes the callback at `arg0+8` with
`(1, slot+4, D_800E2368->8)` and either clears the slot or bumps the +2
counter, restores `D_800E27EC`, and returns the live-slot count.

## Levers

1. **Phantom frame.** Retail reserves 8 bytes of frame that are never
   addressed. An address-taken dead local `int tmp[2]; (void)tmp;` reproduces
   it; without it cc1 emits a 0x38 frame instead of 0x40 and every save slot
   shifts down by 8.
2. **Declaration initializer.** `char *slot = arg0 + 0xC;` must carry its
   initializer on the declaration. Writing `slot = arg0 + 0xC;` as a separate
   statement lets cc1 schedule the `addiu` into the `blez` delay slot, which
   swaps the whole prologue save/init order.
3. **`register int stride asm("$21")`.** Pins retail's `$s5` home; with `$s4`
   for the live count this closes the leaf. Pinning *both* `$s4` and `$s5`
   makes cc1 exit 33.

`try_leaf` before the carve: `WORDS MATCH (+12 pad bytes, trimmed by the
build)`.

## Authority

Fresh `scripts/split_us.sh` (host) + `scripts/build_us.sh` +
`scripts/verify_us.sh` (pe-mipsel) on the carved tree:
`EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (904 registered C leaves)`, plan
`1312 spans = 904 c + 406 asm + 2 rodata`, `VERIFY_US=PASS`.
Commit `88790708`.
