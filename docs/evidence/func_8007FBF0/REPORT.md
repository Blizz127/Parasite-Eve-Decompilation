# `func_8007FBF0` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_passthrough_load`.

## Span

- File `[0x703F0,0x70408)`, VA `[0x8007FBF0,0x8007FC08)`, size `0x18` (6 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; the profile's only delta is the
  listed maspsx gate (none for the default profile).

## Source (`src/func_8007FBF0.c`)

```c
/* VRAM 0x8007FBF0 / file 0x703F0 / size 0x18. 32-bit table getter. */
extern int D_8009B574[];

int func_8007FBF0(int index) {
    return D_8009B574[index];
}
```

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_8007FBF0.c 0x703F0 0x18 --env MASPSX_PASSTHROUGH_SYMBOL_LOAD=1
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
