# func_8004BB80 — score/mode gate (LANDED)

- **Carve**: VRAM `0x8004BB80`, file `0x3C380`, size `0x100` (64 words). Split
  out of the `[0x3BD84, asm]` run; next span is the existing
  `[0x3C480, c, func_8004BC80]`.
- **Profile**: **`era_o2_g8_aspsx_230`** (`-O2 -G8` + `ERA_ASPSX_VER=2.30`);
  registered the leaf under that profile in `disc1_build_profiles.json`.
- **Method / levers**:
  - The gp score pair at `0x278/0x27C($gp)` (`0x8009CFE8/EC`) has **no symbol**
    (unnamed gap above `D_8009CFB0`), so it is addressed as
    `*(int*)((char*)&D_8009CFB0 + 0x38/0x3C)` — cc1 folds the `+0x38` into the
    gp-relative load.
  - `D_800C0E00` is outside the `$gp` window and must stay absolute inside this
    `-G8` unit, so it is declared an **incomplete array** (`extern int
    D_800C0E00[];`).
  - aspsx **2.30** removes the `nop` that `<2.30` injects between the gp load
    and the expanded `$at` store (`lw $v0; lui $at; sw $v0`), which is the
    `ERA_ASPSX_VER=2.30` lever from the handoff.
- **Evidence**: `try_leaf.py src/func_8004BB80.c 0x3C380 0x100 --flags "-O2 -G8"
  --env ERA_ASPSX_VER=2.30` → `WORDS MATCH`.
- **Authority**: fresh build → `EXACT SHA-1
  452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (887
  registered C leaves)`, `VERIFY_US=PASS`.
