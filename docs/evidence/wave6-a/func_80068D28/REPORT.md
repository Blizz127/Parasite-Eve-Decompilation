# func_80068D28 — display-record data initializer (LANDED)

- **Carve**: VRAM `0x80068D28`, file `0x59528`, size `0xFC` (63 words). Split out
  of the `[0x59528, asm]` run (resume asm at `0x59624`). Commit `fa3e2f9` (see
  branch log).
- **Profile**: default `era_o2_g0` (no assignment entry needed).
- **Method**: m2c draft + hand-fixes. The single decisive lever was the
  **indexed access form**: writing `base[i*0x10 + n]` (and `base[i*8 + n]`) made
  cc1 keep `base` in `$a2` and seed the two running record/entry pointers at
  `base` instead of folding `+0x36`/`+0x54` into them. The explicit running
  pointer form (`rec += 0x10`) folded the offsets and failed.
- The two trailing stores are **base-relative** (`*(short*)(base+0x6E/0x70)`),
  not absolute `D_800BCFF6/D_800BCFF8`; that ordering puts `move v0,zero`
  before them and the `0x70` store in the `jr $ra` delay slot, exactly retail.
- gp score pair lives in the unnamed gap above `D_8009CFB0`; not relevant here.
- **Evidence**: `try_leaf.py src/func_80068D28.c 0x59528 0xFC --flags "-O2 -G0"`
  → `WORDS MATCH (+4 pad bytes, trimmed by the build)`.
- **Authority**: fresh `split_us.sh` + `build_us.sh` + `verify_us.sh` →
  `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
  `Matching claim: YES (887 registered C leaves)`, `VERIFY_US=PASS`.
