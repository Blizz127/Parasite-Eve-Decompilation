# `func_800900E4` — matching C leaf

Outcome: **MATCHED** on default era `-O2 -G0` (no maspsx gate) and integrated.

## Span

- File `[0x808E4,0x80908)`, VA `[0x800900E4,0x80090108)`, size `0x24` (9 words).

## Source (`src/func_800900E4.c`)

```c
/* VRAM 0x800900E4 / file 0x808E4 / size 0x24.
 * Advance a buffer cursor, latch the byte read before it into +0xB4 << 7. */
void func_800900E4(unsigned char *obj) {
    unsigned char *cursor = *(unsigned char **)obj;
    *(unsigned char **)obj = cursor + 1;
    *(unsigned short *)(obj + 0xB4) = (unsigned short)(*(unsigned char *)cursor << 7);
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_800900E4.c 0x808E4 0x24
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
