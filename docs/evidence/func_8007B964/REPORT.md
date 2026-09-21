# `func_8007B964` — matching C leaf

Outcome: **MATCHED** on default era `-O2 -G0` (no maspsx gate) and integrated.

## Span

- File `[0x6C164,0x6C1EC)`, VA `[0x8007B964,0x8007B9EC)`, size `0x88` (34 words).

## Source (`src/func_8007B964.c`)

```c
/* VRAM 0x8007B964 / file 0x6C164 / size 0x88.
 * Six staged byte writes to SPU-style pointer globals; returns 0. */
extern unsigned char *D_8009B27C;
extern unsigned char *D_8009B280;
extern unsigned char *D_8009B284;
extern unsigned char *D_8009B288;

int func_8007B964(unsigned char *src) {
    *D_8009B27C = 2;
    *D_8009B284 = src[0];
    *D_8009B288 = src[1];
    *D_8009B27C = 3;
    *D_8009B280 = src[2];
    *D_8009B284 = src[3];
    *D_8009B288 = 0x20;
    return 0;
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_8007B964.c 0x6C164 0x88
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
