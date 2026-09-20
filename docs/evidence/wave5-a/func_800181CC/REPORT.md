# func_800181CC — VRAM 0x800181CC / file 0x89CC / size 0xD4

**Landed.** Profile `era_o2_g0` (default).

Actor-list matcher: with a null key it forwards the context pairs to
`func_8002FF78`; otherwise it walks the D_8009D20C list for the first entry
whose type/owner keys match and hands it to `func_80030220`.

## Lever
The key value loaded from `*arg0->f0` must be copied to a second local
(`key = v;`) before the list walk. Without the explicit copy cc1 allocates the
key directly to `$a2` and skips retail's `lw $v0`/`addu $a2,$v0` pair; with it,
cc1 keeps the key in `$v0` and emits the preheader copy, matching retail
exactly (3-word divergence -> 0).

## Evidence
`try_leaf src/func_800181CC.c 0x89CC 0xD4` -> `WORDS MATCH (+12 pad bytes)`.
Fresh build (commit `dde81b15`): EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, 875 leaves, `VERIFY_US=PASS`.
