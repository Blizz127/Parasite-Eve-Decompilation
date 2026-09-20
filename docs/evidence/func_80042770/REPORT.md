# `func_80042770` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_three_word`.

## Span

- File `[0x32F70,0x32F98)`, VA `[0x80042770,0x80042798)`, size `0x28` (10 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; the profile's only delta is the
  listed maspsx gate (none for the default profile).

## Source (`src/func_80042770.c`)

```c
/* VRAM 0x80042770 / file 0x32F70 / size 0x28.
 * Byte-table flag read, stride 1048 = ((i*33)*4 - i)*8. */
extern unsigned char D_800A0ED4[];

int func_80042770(int index) {
    return D_800A0ED4[index * 1048] & 1;
}
```

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_80042770.c 0x32F70 0x28 --env MASPSX_THREE_WORD_SYMBOL_STORE=1
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
