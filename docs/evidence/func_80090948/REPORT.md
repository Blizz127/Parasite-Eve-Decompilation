# `func_80090948` — matching C leaf

Outcome: **MATCHED** on default era `-O2 -G0` (no maspsx gate) and integrated.

## Span

- File `[0x81148,0x81170)`, VA `[0x80090948,0x80090970)`, size `0x28` (10 words).

## Source (`src/func_80090948.c`)

```c
/* VRAM 0x80090948 / file 0x81148 / size 0x28.
 * Advance a buffer cursor and latch the fetched byte into three u16 fields;
 * clear +0xD2. */
void func_80090948(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    unsigned short value;
    *(unsigned char **)obj = cursor + 1;
    value = *(unsigned char *)cursor;
    *(unsigned short *)(obj + 0xD2) = 0;
    *(unsigned short *)(obj + 0x58) = value;
    *(unsigned short *)(obj + 0x56) = value;
    *(unsigned short *)(obj + 0xD0) = value;
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_80090948.c 0x81148 0x28
```

Result: `WORDS MATCH` (relocation-normalized); trailing alignment pad trimmed
by the build.

## Cumulative authority

```text
bash scripts/build_us.sh
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  RESULT: EXACT MATCH
Compile:   OK (646 generated C entries)
Matching claim: YES (646 registered C leaves)
```
