# func_8007DBC8 — table read with optional global shift (LINK_EXACT)

- Span: file `0x6E3C8` / VRAM `0x8007DBC8` / size `0x3C` (15 words), in
  `asm/disc1/6D874.s`.
- Profile: `era_o2_g0` (default).
- Source: `src/func_8007DBC8.c`.

## Command

```
LD_LIBRARY_PATH=tools/mipsel-host/usr/lib/x86_64-linux-gnu \
  AS=mipsel-linux-gnu-as OBJDUMP=mipsel-linux-gnu-objdump \
  tools/analysis/era_leaf_match.sh src/func_8007DBC8.c 0x8007DBC8 0x3C -O2 -G0
python3 tools/analysis/era_link_check.py src/func_8007DBC8.c 0x8007DBC8 0x3C -O2 -G0
```

## Result

```
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

Object-level (64 vs 60 bytes ROM + 4-byte gas pad) mismatches are relocation
immediates only (`lui` %hi): `BYTE_EXACT`-ignoring-pad.

## Notes / durable lever

`D_8009B3FC` is a **pointer global** (`extern unsigned short *`) — retail emits
`lui`/`lw` for one loaded base, not `la`. The loaded value must be a **narrow
`unsigned short` local**, which homes it in `$a0` (`lhu a0,0(a0)` /
`sllv v0,a0,v0` / `move v0,a0`). An `unsigned int` local lands in `$v1` and adds
a redundant `move v0,v1`, and reassigning the parameter `a0` shares `$a0` before
the shift, changing the shift source register. `D_8009B424` is `unsigned int` so
the `sllv` count is unsigned.
