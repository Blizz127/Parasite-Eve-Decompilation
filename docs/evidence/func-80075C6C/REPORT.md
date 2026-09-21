# `func_80075C6C` — rectangle-command flag-word setter

Outcome: **MATCHED** on era `-O2 -G0` (`BYTE_EXACT` object; `LINK_EXACT`,
0 word mismatches at the retail VMA).

## Function hood and span

- File span `[0x6646C,0x66494)` = 10 words. VRAM `[0x80075C6C,0x80075C94)`.

## Semantics (retail bytes)

```text
80075c6c  li    v0,2
80075c70  beqz  a1,.L80075C84
80075c74  sb    v0,3(a0)          ; delay slot: a0[3] = 2
80075c78  lui   v0,0xe600
80075c7c  j     .L80075C88
80075c80  ori   v0,v0,1           ; delay slot: 0xE6000001
80075c84  lui   v0,0xe600         ; 0xE6000000
80075c88  sw    v0,4(a0)
80075c8c  jr    ra
80075c90  sw    zero,8(a0)        ; store in the jr delay slot
```

C (`src/func_80075C6C.c`):

```c
void func_80075C6C(unsigned char *a0, int a1) {
    a0[3] = 2;
    *(int *)(a0 + 4) = a1 ? 0xE6000001u : 0xE6000000u;
    *(int *)(a0 + 8) = 0;
}
```

## Exact residual accounting

Object-level `MISMATCHES=1` — the absolute `j .L80075C88` target, which is
resolved by the defsym link. Everything else, including the flag materializing
in the `j` delay slot and the closing `sw zero,8(a0)` landing in the `jr` slot,
is byte-identical, so this leaf needs no maspsx knob.

## Single-leaf object

```text
tools/analysis/era_leaf_match.sh src/func_80075C6C.c 0x80075C6C 0x28 -O2 -G0
```

## Link-level proof

```text
python3 tools/analysis/era_link_check.py src/func_80075C6C.c 0x80075C6C 0x28 -O2 -G0
LINK_EXACT
```

## Registration

- Source `src/func_80075C6C.c`; YAML carve `[0x6646C, c, func_80075C6C]`.
- Profile: default `era_o2_g0` (`-O2 -G0`).
