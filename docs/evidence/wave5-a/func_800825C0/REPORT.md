# func_800825C0 — VRAM 0x800825C0 / file 0x72DC0 / size 0xC0

**Landed.** Profile `era_o2_g0` (default).

Controller state query: dispatches through the installed record getter
`D_8009B738(port)`, classifies the returned record's state byte and returns
1 (state 2/3), 4 (state 6) or the raw state byte.

## Lever
The state classification must be written as a **`switch`** (cases 3, 2, 6,
no default) rather than a nested if/else. The switch lowering reproduces
retail's signed `slti $v0,$v1,4` decision tree and its separate `$v0=1`
return blocks; the nested if/else produced a merged `sltiu` tree with one
extra register. The `$a0`-comparison on the record is untouched.

## Evidence
`try_leaf src/func_800825C0.c 0x72DC0 0xC0` -> `WORDS MATCH`.
Fresh build (commit `dde81b15`): EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, 875 leaves, `VERIFY_US=PASS`.
