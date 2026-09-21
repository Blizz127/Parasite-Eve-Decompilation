# `func_800176B8` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0 (default)`.

## Span

- File `[0x7EB8,0x7EE0)`, VA `[0x800176B8,0x800176E0)`, size `0x28` (10 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; only delta is the listed maspsx gate.

## Source (`src/func_800176B8.c`)

```c
/* VRAM 0x800176B8 / file 0x7EB8 / size 0x28.
 * OR the caller's word into field 0x98 of the D_8009D2F0 object; return 1. */
extern int *D_8009D2F0;

int func_800176B8(int **arg) {
    D_8009D2F0[0x98 / 4] |= *arg[0];
    return 1;
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_800176B8.c 0x7EB8 0x28
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
