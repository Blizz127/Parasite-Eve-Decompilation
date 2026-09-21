# func_8007E594 — 0x18-byte object clear (LINK_EXACT)

- Span: file `0x6ED94` / VRAM `0x8007E594` / size `0x30` (12 words), in
  `asm/disc1/6E6C0.s`.
- Profile: `era_o2_g0` (default).
- Source: `src/func_8007E594.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_8007E594.c 0x8007E594 0x30 -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8007E594.c 0x8007E594 0x30 -O2 -G0
```

## Result

```
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Notes / durable lever

The byte walk uses a **parallel pointer `q = a0 + 3`** with a down-counted index:
`q[5]` is `a0[8]` and `q--` walks toward `a0[5]`. Indexing `a0` directly makes
cc1 emit `addu v0,v1,a0` inside the loop and hoist the store (11 mismatches); an
ascending `for (i = 5; i < 9; i++)` byte loop grows the frame to `0x40`
(SIZE_MISMATCH). The `int i` loop counter is also required — an `unsigned` index
changes the `bgez` to `bnez`.
