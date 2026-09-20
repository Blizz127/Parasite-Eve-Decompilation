# `func_800528C4` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_three_word`.

## Span

- File `[0x430C4,0x430F0)`, VA `[0x800528C4,0x800528F0)`, size `0x2C` (11 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; the profile's only delta is the
  listed maspsx gate (none for the default profile).

## Source (`src/func_800528C4.c`)

```c
/* VRAM 0x800528C4 / file 0x430C4 / size 0x2C. 12-byte-stride table store; value = a1*60. */
extern int D_800A76A4[];

void func_800528C4(int index, int value) {
    D_800A76A4[index * 3] = value * 60;
}
```

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_800528C4.c 0x430C4 0x2C --env MASPSX_THREE_WORD_SYMBOL_STORE=1
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
