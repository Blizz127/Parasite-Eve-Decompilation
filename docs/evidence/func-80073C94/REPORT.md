# func_80073C94 / func_80073CC4 — MATCHED (`LINK_EXACT`)

VRAM `0x80073C94` / `0x80073CC4`, size `0x30` (12 words) each, file
`0x64494` / `0x644C4` in `asm/disc1/621E4.s`. era flags `-O2 -G0`
(YAML default). Matching-C leaves 624/625.

## Semantics

Tail-dispatch through the handler table behind the `D_8009566C` pointer
global:

- `func_80073C94` calls the slot at `+0xC`.
- `func_80073CC4` calls the slot at `+0x8`.

Both discard the handler's return value and return void.

## Source

```c
extern unsigned char *D_8009566C;
void func_80073C94(void) {
    unsigned int (*f)() = *(unsigned int (**)())(D_8009566C + 0xC);
    f();
}
```

```c
extern unsigned char *D_8009566C;
void func_80073CC4(void) {
    unsigned int (*f)() = *(unsigned int (**)())(D_8009566C + 8);
    f();
}
```

`src/func_80073C94.c`, `src/func_80073CC4.c`.

## Lever — argument-less function-pointer slot

The slot type must be declared **argument-less** (`unsigned int (*)()`, not
`(void)` and not with a parameter). Retail's `jalr $v0` delay slot is a bare
`nop` — a prototype with any parameter makes cc1 materialize `$a0` in the
delay slot (`MISMATCHES=5`). Same lever as the `func_8006Fxxx` cluster.

## Commands

```
tools/analysis/era_leaf_match.sh src/func_80073C94.c 0x80073C94 0x30 -O2 -G0
MISMATCHES=2  (both `lui %hi` / `lw %lo` relocation placeholders)
python3 tools/analysis/era_link_check.py src/func_80073C94.c 0x80073C94 0x30 -O2 -G0
LINK_EXACT
# identical accounting for src/func_80073CC4.c 0x80073CC4 0x30
```

## Provenance

`configs/USA/disc1.yaml`:

```
- [0x64494, c, func_80073C94]
- [0x644C4, c, func_80073CC4]
- [0x644F4, asm]
```

The asm resumes at `0x644F4`, four bytes before the next unregistered leaf
(`func_80073CF4`).
