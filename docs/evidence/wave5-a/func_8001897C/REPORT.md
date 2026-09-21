# func_8001897C — VRAM 0x8001897C / file 0x917C / size 0xCC

**Landed.** Profile `era_o2_g0` (default). No new profile.

Record writer: ten pointer-table fields from the Ctx1817C pointer table are
forwarded to `func_8002FA10` together with `D_8009D2F0`, then returns 1.
The task sheet listed size 0xD4; the splat/splat worklist size is **0xCC**
(`func_80018A48` starts at 0x9248, which the 0xD4 figure would have swallowed).

## Lever
`func_8002FA10`'s parameters a6..a9 (fields at 0x14/0x18/0x1C/0x20) are stored
as bytes by the callee, so the low 8 bits are all that matter. Declaring those
four prototype parameters `signed char` in this translation unit (rather than
`unsigned char`) makes cc1 emit retail's sign-extending `lb` instead of `lbu`.
The first four register args and a4/a5/a10/a11 keep their unsigned loads.

## Evidence
`try_leaf src/func_8001897C.c 0x917C 0xCC` -> `WORDS MATCH (+4 pad bytes...)`.
Fresh split + build + verify (commit `dde81b15`): EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, 875 registered C leaves,
`VERIFY_US=PASS`.
