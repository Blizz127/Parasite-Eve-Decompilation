# `func_8007DC5C` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0 (default)`.

## Span

- File `[0x6E45C,0x6E484)`, VA `[0x8007DC5C,0x8007DC84)`, size `0x28` (10 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; the profile's only delta is the
  listed maspsx gate (none for the default profile).

## Source (`src/func_8007DC5C.c`)

```c
/* VRAM 0x8007DC5C / file 0x6E45C / size 0x28.
 * RMW: clear bits 24-27 of the word at the pointer global, set 0x20000000. */
extern unsigned int *D_8009B410;

void func_8007DC5C(void) {
    *D_8009B410 = (*D_8009B410 & 0xF0FFFFFFu) | 0x20000000u;
}
```

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_8007DC5C.c 0x6E45C 0x28
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
