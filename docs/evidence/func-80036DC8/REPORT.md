# func_80036DC8 — MATCHED (`LINK_EXACT`)

VRAM `0x80036DC8`, size `0x30` (12 words), file `0x275C8` in `asm/disc1/26C48.s`.
era flags `-O2 -G0` (YAML default).

## Semantics

Three sequential init calls in one 0x18 frame: `func_80036DF8()`,
`func_80036E34()`, `func_80036E58()`.

## Source

```c
extern void func_80036DF8(void);
extern void func_80036E34(void);
extern void func_80036E58(void);

void func_80036DC8(void) {
    func_80036DF8();
    func_80036E34();
    func_80036E58();
}
```

`src/func_80036DC8.c`.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80036DC8.c 0x80036DC8 0x30 -O2 -G0
ROM  .text 48 bytes  C .text 48 bytes  target 48
MISMATCHES=3  (all three `jal` words: relocation placeholders, 0c000000
               vs 0c00db7e / 0c00db8d / 0c00db96)
python3 tools/analysis/era_link_check.py src/func_80036DC8.c 0x80036DC8 0x30 -O2 -G0
linked .text 48 bytes, target 0x30, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

## Provenance

`configs/USA/disc1.yaml`: `- [0x275C8, c, func_80036DC8]` (asm resumes at
`0x275F8`, immediately before the already-matched `func_80036E34`).
