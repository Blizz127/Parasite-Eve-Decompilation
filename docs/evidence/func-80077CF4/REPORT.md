# func_80077CF4 — MATCHED (`LINK_EXACT`)

VRAM `0x80077CF4`, size `0x3C` (15 words), file `0x684F4` in
`asm/disc1/684EC.s` / `asm/disc1/68530.s`. era flags `-O2 -G0` (YAML
default).

## Semantics

Signed 0x1000-step wrapper: apply the sign of `a0` *first*, take the low 12
bits, call `func_80077D30`, and re-apply the sign to the result. On-path
fan-in 5 (`asm/disc1/A010.s`, `561C8.s`, `B3390.s`).

## Source

```c
extern int func_80077D30(int a0);

int func_80077CF4(int a0) {
    if (a0 < 0) {
        return -func_80077D30((-a0) & 0xFFF);
    }
    return func_80077D30(a0 & 0xFFF);
}
```

`src/func_80077CF4.c`.

The negative branch must textually negate-and-mask inside the call argument so
cc1 emits `negu $a0,$a0` then `andi $a0,$a0,0xFFF` and the negative call result
is negated after the `jal`; routing it through a named local perturbs the
post-call `negu` position.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80077CF4.c 0x80077CF4 0x3C -O2 -G0
ROM  .text 60 bytes  C .text 64 bytes  target 60
SIZE_MISMATCH C=0x40 ROM=0x3c   (4 bytes trailing gas zero pad, zero)
MISMATCHES=3 first_off=12 vram=0x80077d00
  0x80077d00: ROM 0c01df4c  C 0c000000     (jal func_80077D30 reloc)
  0x80077d08: ROM 0801df48  C 0800000b     (j .L80077D20 reloc)
  0x80077d14: ROM 0c01df4c  C 0c000000     (jal func_80077D30 reloc)
python3 tools/analysis/era_link_check.py src/func_80077CF4.c 0x80077CF4 0x3C -O2 -G0
linked .text 64 bytes, target 0x3c, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

The three object mismatches are unresolved `jal`/`j` targets (relocation
slots); the link-level compare resolves them and is authoritative.

## Provenance

`configs/USA/disc1.yaml`:

```
- [0x684EC, asm]
- [0x684F4, c, func_80077CF4]
- [0x68530, c, func_80077D30]
```
