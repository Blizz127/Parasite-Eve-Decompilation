# `func_8008FBD4` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0 (default)`.

## Span

- File `[0x803D4,0x803FC)`, VA `[0x8008FBD4,0x8008FBFC)`, size `0x28` (10 words).

## Source (`src/func_8008FBD4.c`)

```c
/* VRAM 0x8008FBD4 / file 0x803D4 / size 0x28.
 * Advance a cursor and store the fetched byte sign-extended into +0xDE. */
void func_8008FBD4(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    *(unsigned char **)obj = cursor + 1;
    *(short *)(obj + 0xDE) = (signed char)*cursor;
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_8008FBD4.c 0x803D4 0x28 --flags "-O2 -G0"
```

Result: `WORDS MATCH` (relocation-normalized). `func_80089F28` needs `-O1`
so the second store fills the `jr` delay slot as retail does.

## Cumulative authority

```text
bash scripts/build_us.sh
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  RESULT: EXACT MATCH
Compile:   OK (649 generated C entries)
Matching claim: YES (649 registered C leaves)
```
