# `func_80051684` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0 (default)`.

## Span

- File `[0x41E84,0x41EB4)`, VA `[0x80051684,0x800516B4)`, size `0x30` (12 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; only delta is the listed maspsx gate.

## Source (`src/func_80051684.c`)

```c
/* VRAM 0x80051684 / file 0x41E84 / size 0x30.
 * Two-level guarded store: value<<16 into +8 of the object at
 * (*D_8009D254)->[0]. */
extern unsigned char *D_8009D254;

void func_80051684(int value) {
    unsigned char *a = D_8009D254;
    if (a != 0) {
        unsigned char *b = *(unsigned char **)a;
        if (b != 0)
            *(int *)(b + 8) = value << 16;
    }
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_80051684.c 0x41E84 0x30
```

Result: `WORDS MATCH` (relocation-normalized); trailing alignment pad trimmed
by the build.

## Cumulative authority

```text
bash scripts/build_us.sh
  orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  RESULT: EXACT MATCH
Compile:   OK (641 generated C entries)
Matching claim: YES (641 registered C leaves)
```
