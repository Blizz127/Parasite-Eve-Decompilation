# func_80038910 — gp-relative record seed (LINK_EXACT)

- Span: file `0x29110` / VRAM `0x80038910` / size `0x30` (12 words), in
  `asm/disc1/28070.s`.
- Profile: `era_o2_g8` (`-O2 -G8`, default toolchain).
- Source: `src/func_80038910.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_80038910.c 0x80038910 0x30 -O2 -G8
python3 tools/analysis/era_link_check.py src/func_80038910.c 0x80038910 0x30 -O2 -G8
```

## Result

```
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

Object-level: the `-G0` rung emits `lui $at,%hi` + indexed stores (9 mismatches,
object 0x50 vs ROM 0x30); `-G8` keeps every destination gp-relative
(`sh a0,0x148(gp)` …) and is byte-identical apart from the relocation immediates.

## Notes / durable lever

All seven destinations are gp-relative (`_gp = 0x8009CD70`): halfwords at
`0x148/0x14C/0x150`, bytes at `0x144/0x154/0x158/0x15C`. Args 5 and 6 arrive on
the stack (`0x10/0x14($sp)`); the `lbu`s for them are hoisted before the literal
`1` materialisation. The `-G8` rung is the load-bearing part — under `-G0` cc1
splits each store into an absolute `lui $at` sequence.
