# `func_8007A400` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_three_word`.

## Span

- File `[0x6AC00,0x6AC34)`, VA `[0x8007A400,0x8007A434)`, size `0x34` (13 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; only delta is the listed maspsx gate.

## Source (`src/func_8007A400.c`)

```c
/* VRAM 0x8007A400 / file 0x6AC00 / size 0x34.
 * Pointer-table lookup with 0x1C bound; fallback is the D_800119CC string. */
extern unsigned char *D_8009AFDC[];
extern unsigned char D_800119CC[];

unsigned char *func_8007A400(unsigned int index) {
    index &= 0xFF;
    if (index >= 0x1C)
        return D_800119CC;
    return D_8009AFDC[index];
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_8007A400.c 0x6AC00 0x34 --env MASPSX_THREE_WORD_SYMBOL_STORE=1
```

Result: `WORDS MATCH` (relocation-normalized); trailing alignment pad trimmed
by the build.

## Cumulative authority

```text
bash scripts/build_us.sh
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  RESULT: EXACT MATCH
Compile:   OK (641 generated C entries)
Matching claim: YES (641 registered C leaves)
```
