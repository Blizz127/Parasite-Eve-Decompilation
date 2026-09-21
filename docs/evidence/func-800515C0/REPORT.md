# func_800515C0 — guarded +0xC short store through D_8009D254->0

VRAM `0x800515C0`, size `0x38` (14 words), file offset `0x41DC0`.
Carved from the head of the former `0x41D10` asm span (prefix `0xB0`),
closing exactly at the next span `0x41E84`:

```
- [0x41DC0, c, func_800515C0]
- [0x41DF8, asm]
```

Retail: if `*D_8009D254` and its `->0` pointer are both non-null, store
`a0` at `+0xC` of the inner object; then unconditionally store `a0` to the
absolute `D_800C0E08`.

## Profile

Default `era_o2_g0` (`-O2 -G0`).

## Commands

```
env -u MASPSX_SYMBOL_AT_TEMP -u MASPSX_SYMBOL_LOAD_DEST_TEMP \
  LD_LIBRARY_PATH=$PWD/tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  tools/analysis/check_leaf.sh func_800515C0 0x800515C0 0x38 -O2 -G0
```

Result: `LINK_EXACT`; deep span-size exact.

## Source

```c
extern unsigned char *D_8009D254;
extern unsigned short D_800C0E08;
void func_800515C0(unsigned short a0) {
    unsigned char *p = D_8009D254;
    if (p != 0) {
        unsigned char *q = *(unsigned char **)p;
        if (q != 0) {
            *(unsigned short *)(q + 0xC) = a0;
        }
    }
    D_800C0E08 = a0;
}
```

The explicit inner-pointer local `q` defeats cc1's CSE of the two
`*D_8009D254` loads: retail performs two independent `lui`/`lw` pairs with
`nop` before each `beqz`, and only the second load feeds the store.
