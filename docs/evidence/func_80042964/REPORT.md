# `func_80042964` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_three_word`.

## Span

- File `[0x33164,0x3318C)`, VA `[0x80042964,0x8004298C)`, size `0x28` (10 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; the profile's only delta is the
  listed maspsx gate (none for the default profile).

## Source (`src/func_80042964.c`)

```c
/* VRAM 0x80042964 / file 0x33164 / size 0x28.
 * Twin of func_80042770 without the final mask: D_800A0EDE[i*1048]. */
extern unsigned char D_800A0EDE[];

int func_80042964(int index) {
    return D_800A0EDE[index * 1048];
}
```

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_80042964.c 0x33164 0x28 --env MASPSX_THREE_WORD_SYMBOL_STORE=1
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
