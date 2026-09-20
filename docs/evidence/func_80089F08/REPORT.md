# `func_80089F08` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0 (default)`.

## Span

- File `[0x7A708,0x7A724)`, VA `[0x80089F08,0x80089F24)`, size `0x1C` (7 words).
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0`; the profile's only delta is the
  listed maspsx gate (none for the default profile).

## Source (`src/func_80089F08.c`)

```c
/* VRAM 0x80089F08 / file 0x7A510 / size 0x1C.
 * 16-byte-stride struct field read through pointer global D_8009B3FC. */
extern unsigned char *D_8009B3FC;

void func_80089F08(int index, unsigned short *out) {
    *out = *(unsigned short *)(D_8009B3FC + index * 16 + 0xC);
}
```

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_80089F08.c 0x7A708 0x1C
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
