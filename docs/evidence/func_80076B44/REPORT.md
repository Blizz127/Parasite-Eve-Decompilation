# `func_80076B44` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_passthrough_load`.

## Span

- File `[0x67344,0x67358)`, VA `[0x80076B44,0x80076B58)`, size `0x14` (5 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; the profile's only delta is the
  listed maspsx gate (none for the default profile).

## Source (`src/func_80076B44.c`)

```c
/* VRAM 0x80076B44 / file 0x67344 / size 0x14.
 * Byte-table getter: D_800A3348[a0] with no index scaling. */
extern unsigned char D_800A3348[];

unsigned char func_80076B44(int index) {
    return D_800A3348[index];
}
```

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_80076B44.c 0x67344 0x14 --env MASPSX_PASSTHROUGH_SYMBOL_LOAD=1
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
