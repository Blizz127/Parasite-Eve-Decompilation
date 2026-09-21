# `func_80017294` — field-VM handler: repoint the instruction cursor

Outcome: **MATCHED** on era `-O2 -G8` + `MASPSX_FORCE_ABSOLUTE_SYMBOLS=
D_8009D2F0`. Linked at its retail VMA, the object `.text` is byte-identical to
retail (`LINK_EXACT`, 0 word mismatches).

## Function hood and span

- File span `[0x7A94,0x7ABC)` = 10 words. VRAM `[0x80017294,0x800172BC)`.
- Field-VM dispatch target `D_800910A0[0x294]` (low-byte `0x94`).
- Previously inside merged asm span `0x7640`; now carved
  `[0x7640, asm)` / `[0x7A94, c, func_80017294)` / resume `[0x7ABC, c, ...)`.

## Semantics (retail bytes)

```text
80017294  8c820000  lw   v0,0(a0)        ; a0 = VM frame ptr array
80017298  3c03800a  lui  v1,0x800a
8001729c  8c63d2f0  lw   v1,%lo(D_8009D2F0)(v1)
800172a0  8c420000  lw   v0,0(v0)        ; v = **a0
800172a4  8c63009c  lw   v1,0x9c(v1)     ; state[0x27]
800172a8  00021040  sll  v0,v0,1
800172ac  00621821  addu v1,v1,v0
800172b0  af830090  sw   v1,0x90(gp)     ; D_8009CE00 (gp-relative)
800172b4  03e00008  jr   ra
800172b8  24020001  li   v0,1
```

C (`src/func_80017294.c`):

```c
int func_80017294(unsigned int **a0) {
    unsigned int v = **a0;
    D_8009CE00 = D_8009D2F0[0x27] + v * 2;
    return 1;
}
```

## Load-bearing lever: `-G8` plus absolute base / gp-relative store

The VM rewrites its instruction cursor `D_8009CE00`, a small-data word at
`0x90($gp)` (`_gp = 0x8009CD70`), while the state object `D_8009D2F0` is still
loaded absolutely `lui/lw`. Under `-G8` cc1 gp-relocates **both** (`lw
$3,0($gp)` for the base, `sw $3,0($gp)` for the cursor) and the whole schedule
shifts. Stripping the base's `.extern` directive (the repo's existing
`MASPSX_FORCE_ABSOLUTE_SYMBOLS` knob) makes gas macro-expand it to
`lui`/`lw` while the cursor store stays `%gp_rel`, reproducing retail exactly.

Object-level diff is relocation-only (`D_8009D2F0` hi/lo + the gp-rel store);
`era_leaf_match.sh` reports `MISMATCHES=9` (all of them relocation fields).

## Link-level proof

```text
MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2F0 \
  python3 tools/analysis/era_link_check.py src/func_80017294.c 0x80017294 0x28 -O2 -G8
linked .text 48 bytes, target 0x28, word mismatches=0, nonzero_pad=0
LINK_EXACT
```

(The link checker now defines `_gp=0x8009CD70` so the gp-relative store
resolves to `0x800172b0 = 0x90 + 0x8009CD70 - 0x8000`.)

## Registration

- Source `src/func_80017294.c`; YAML `- [0x7A94, c, func_80017294]`.
- Profile `era_o2_g8_force_d8009d2f0_absolute` in
  `configs/USA/disc1_build_profiles.json`.
