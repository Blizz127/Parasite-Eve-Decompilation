# Unreviewed 17Exx / post-baseline candidates — preserved 2026-08-23

Baseline of record: `481a911` (279 matching C leaves, EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`). Everything here was wired
into the tree BEYOND that baseline without screening/selection/review.

## Provenance

| Object | Where it came from | Disposition |
| --- | --- | --- |
| `func_80017EC4` | commit `9eec0f9` (yaml `[0x86C4,c,...]` + full build-script wiring) | reverted; candidate preserved as `.from-9eec0f9` |
| `func_80017F44` | untracked `src/func_80017F44.c` + uncommitted edits to `scripts/build_us.sh` (`SIZE_C_17F44=0x44`, era_compile, trim, link refs) and `configs/USA/disc1.yaml` (`[0x8744, c, func_80017F44]`) | reverted/deleted; preserved `.as-found` and `.nop-removed` |
| `func_8006E7E8` | commit `278006a` | reverted; re-integrated diagnostically per morning correction 3 (candidate copy: `.from-278006a`) |

Not in scope (pre-baseline reviewed leaves that merely appear in every trim
log): `func_80017E9C` @ `657b997`, `func_80017EA4` @ `3d89b20`,
`func_80017EFC` @ `dbda9d9`, `func_80017F20` @ `ac6bdb4`.

## func_80017F44 findings

1. **RULE VIOLATION:** the candidate contained a hand-inserted
   `asm volatile("nop");` — padding to hit a word count. Never acceptable.
   Deleted on instruction before preservation.
2. **Trim gate evidence:** `ERROR: target size 0x44 > current 0x40` — even
   WITH the nop the object was one word short of retail's 0x44. The deficit
   is structural: retail has one more word than this C's branch/store shape
   produces (delay-slot fill or branch form), which is a finding about the
   C's structure, not missing filler. No retry authorized.
3. The attempt was never screened or selected per the run rules.

## Build-script/yaml registrations reverted

- `configs/USA/disc1.yaml`: `- [0x8744, c, func_80017F44]`,
  `- [0x86C4, c, func_80017EC4]`, `- [0x5EFE8, c, func_8006E7E8]`
- `scripts/build_us.sh`: `SIZE_C_17EC4/SIZE_C_17F44/SIZE_C_6E7E8`,
  their object refs, era_compile lines, trim lines, and .text/.data/.rodata/.bss
  link blocks — all back at the `481a911` state via reset.
