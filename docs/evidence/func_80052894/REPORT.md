# `func_80052894` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_three_word`.

## Span

- File `[0x43094,0x430C4)`, VA `[0x80052894,0x800528C4)`, size `0x30` (12 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; the profile's only delta is the
  listed maspsx gate (none for the default profile).

## Source (`src/func_80052894.c`)

```c
/* VRAM 0x80052894 / file 0x43094 / size 0x30.
 * 12-byte-stride table read divided by 60 (multu 0x88888889 + srl 5). */
extern unsigned int D_800A76A4[];

unsigned int func_80052894(int index) {
    return D_800A76A4[index * 3] / 60u;
}
```

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_80052894.c 0x43094 0x30 --env MASPSX_THREE_WORD_SYMBOL_STORE=1
```

Result: `WORDS MATCH` (relocation-normalized), trailing alignment pad trimmed
by the build.

## Cumulative authority

```text
bash scripts/build_us.sh
...
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  RESULT: EXACT MATCH
Compile:   OK (637 generated C entries)
Matching claim: YES (637 registered C leaves)
```
