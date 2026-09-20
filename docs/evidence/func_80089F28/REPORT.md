# `func_80089F28` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o1_g0`.

## Span

- File `[0x7A728,0x7A750)`, VA `[0x80089F28,0x80089F50)`, size `0x28` (10 words).

## Source (`src/func_80089F28.c`)

```c
/* VRAM 0x80089F28 / file 0x7A728 / size 0x28.
 * Mirror two u16 values into pointer-global fields +0x184/+0x186 and the
 * static D_8009B3A4[0..1]. */
extern unsigned char *D_8009B3FC;
extern unsigned short D_8009B3A4[];

void func_80089F28(unsigned short a, unsigned short b) {
    *(unsigned short *)(D_8009B3FC + 0x184) = a;
    *(unsigned short *)(D_8009B3FC + 0x186) = b;
    D_8009B3A4[0] = a;
    D_8009B3A4[1] = b;
}
```

## Single-leaf triage

```text
python3 tools/analysis/try_leaf.py src/func_80089F28.c 0x7A728 0x28 --flags "-O1 -G0"
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
