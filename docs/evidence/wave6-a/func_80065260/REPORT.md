# func_80065260 — two-stage weighted ratio update (LANDED)

- **Carve**: VRAM `0x80065260`, file `0x55A60`, size `0x10C` (67 words). Split
  out of the `[0x556B4, asm]` run (resume asm at `0x55B6C`).
- **Profile**: **new** `era_o2_g0_expand_div_aspsx_230`
  (`-O2 -G0` + `ERA_ASPSX_VER=2.30` + `MASPSX_EXPAND_DIV=1`); added to
  `profiles` and `assignments` in `disc1_build_profiles.json`.
- **Method / levers**:
  - Retail's two signed `div $zero,..` blocks require `MASPSX_EXPAND_DIV=1`
    (same as the existing `era_o2_g0_expand_div`, e.g. `func_8003C5D8`).
  - Retail has the aspsx **2.30** `mflo`/`mfhi` interlock nops; `<2.30` emits
    none. Hence the new combined profile.
  - The `f34` load must precede the NULL check (`a1 = arg0->f34; if (arg0 ==
    0) return;`), exactly as retail.
  - **Home pins** closed the leaf:
    `register Rec *a1 asm("$5");` and `register int v1 asm("$3");`.
    A sweep of pin combinations showed `{Rec($5), v1($3)}` → WORDS MATCH;
    every other subset left 6–18 words diverging.
- **Evidence**: `try_leaf.py src/func_80065260.c 0x55A60 0x10C --flags "-O2 -G0"
  --env MASPSX_EXPAND_DIV=1 --env ERA_ASPSX_VER=2.30` → `WORDS MATCH (+4 pad
  bytes, trimmed by the build)`.
- **Authority**: fresh build → `EXACT SHA-1
  452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (887
  registered C leaves)`, `VERIFY_US=PASS`.
