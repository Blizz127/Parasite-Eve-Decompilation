# `func_8008FCBC` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0 (default)`.

## Span

- File `[0x804BC,0x804E4)`, VA `[0x8008FCBC,0x8008FCE4)`, size `0x28` (10 words).

## Source (`src/func_8008FCBC.c`)

```c
/* VRAM 0x8008FCBC / file 0x804BC / size 0x28.
 * Advance a cursor and store the fetched byte sign-extended into +0xE0. */
void func_8008FCBC(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    *(unsigned char **)obj = cursor + 1;
    *(short *)(obj + 0xE0) = (signed char)*cursor;
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_8008FCBC.c 0x804BC 0x28 --flags "-O2 -G0"
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
