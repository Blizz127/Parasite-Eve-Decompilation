# func_80089F08 — strided table halfword copy (LINK_EXACT)

- Span: file `0x7A108` / VRAM `0x80089F08` / size `0x1C` (7 words), in
  `asm/disc1/7A510.s`.
- Profile: `era_o2_g0` (default).
- Source: `src/func_80089F08.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_80089F08.c 0x80089F08 0x1C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_80089F08.c 0x80089F08 0x1C -O2 -G0
```

## Result

```
linked .text 32 bytes, target 0x1c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Notes / durable lever

The index base must be folded into `$a0` (`a0 = (a0 << 4) + (unsigned int)
D_8009B3FC;`) so the following indexed load keeps the `0xC` displacement on the
base register (`lhu v0,0xC(a0)`). Writing the load as one expression
(`*(unsigned short *)((a0 << 4) + (unsigned int)D_8009B3FC + 0xC)`) makes cc1
fold the addend into the address register instead and changes the base.
`D_8009B3FC` is a pointer global (`lui`/`lw`), not an array symbol.
