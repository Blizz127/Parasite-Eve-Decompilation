# func_80078C94 — three-word copy into a0[5..7]

VRAM `0x80078C94`, size `0x24` (9 words), file offset `0x69494`.
YAML edge: `[0x69494, c, func_80078C94]`, closed by `[0x694B8, asm]`.

Retail semantics: copy `a1[0..2]` into `a0[5..7]`, return `a0`. Retail
puts the return `addu $v0,$a0,$zero` **before** `jr $31` (an unfilled
delay slot), which no default rung reproduces: `-O2` reorders the loads
and plain `-O1` fills the slot with the return move after the `jr`.

## Profile

`era_o1_g0_no_delayed_branch` — era `-O1 -G0 -fno-delayed-branch`
(new profile, added to `configs/USA/disc1_build_profiles.json`).

## Commands

```
env -u MASPSX_SYMBOL_AT_TEMP -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/check_leaf.sh func_80078C94 0x80078C94 0x24 \
  -O1 -G0 -fno-delayed-branch
```

Result: `LINK_EXACT`; deep span-size exact; full deep preflight PASS.

## Source

```c
int *func_80078C94(int *a0, int *a1) {
    register int t0 asm("$8") = a1[0];
    register int t1 asm("$9") = a1[1];
    register int t2 asm("$10") = a1[2];
    a0[5] = t0;
    a0[6] = t1;
    a0[7] = t2;
    return a0;
}
```

## Durable lever

`-fno-delayed-branch` (already used by `func_80051E48`) is the missing
rung for a leaf whose retail body ends `<instr>; jr $31; nop` with the
return-value `addu` sitting *before* the `jr`. Plain `-O1` fills the slot.
