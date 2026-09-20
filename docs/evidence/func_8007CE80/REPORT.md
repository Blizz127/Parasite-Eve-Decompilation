# `func_8007CE80` — matching C leaf

Outcome: **MATCHED** on default era `-O2 -G0` (no maspsx gate) and integrated.

## Span

- File `[0x6D680,0x6D6AC)`, VA `[0x8007CE80,0x8007CEAC)`, size `0x2C` (11 words).

## Source (`src/func_8007CE80.c`)

```c
/* VRAM 0x8007CE80 / file 0x6D680 / size 0x2C.
 * Word copy of `count` elements; retail rotates to a bottom-tested loop. */
void func_8007CE80(unsigned int *dst, unsigned int *src, unsigned int count) {
    unsigned int i = 0;
    if (count != 0) {
        do {
            *dst = *src;
            src++;
            i++;
            dst++;
        } while (i < count);
    }
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_8007CE80.c 0x6D680 0x2C
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
