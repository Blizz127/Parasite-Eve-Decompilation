# Wave 6 slice A — executed-path functions

**Branch `agent/wave6-a`, worktree `/tmp/pe-agent-w11`.**
Baseline: `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (884 registered C leaves)`, `VERIFY_US=PASS`.

Final fresh build: `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (888
registered C leaves)`, plan `1292 spans = 888 c + 402 asm + 2 rodata`,
`VERIFY_US=PASS`.

## Landed (884 -> 888)

| function | file | size | profile | key lever |
| --- | ---: | ---: | --- | --- |
| `func_80068D28` | 0x59528 | 0xFC | `era_o2_g0` | indexed `base[i*0x10+n]` keeps base in `$a2`; base-relative tail stores |
| `func_8004BB80` | 0x3C380 | 0x100 | `era_o2_g8_aspsx_230` | unnamed gp pair off `D_8009CFB0+0x38`; `D_800C0E00[]` incomplete array; aspsx 2.30 |
| `func_80065260` | 0x55A60 | 0x10C | `era_o2_g0_expand_div_aspsx_230` (new) | `MASPSX_EXPAND_DIV` + aspsx 2.30 interlocks + `$5`/`$3` home pins |
| `func_8006E1C0` | 0x5E9C0 | 0x110 | `era_o2_g0` | overlapping packed word/byte -> explicit offset casts |

Per-leaf evidence: `docs/evidence/wave6-a/<name>/REPORT.md`.
Commits: `94e45f7f` (3 leaves), `f08a41e8` (1 leaf).

## Parked

`wave6a-*` ids in `docs/ai_context/parked_blockers.json`:
`func_8001CAB0`, `func_80067D18`, `func_8007ED58`, `func_80036F7C`,
`func_800C9C8C`. All are pure cc1 register-allocation / scheduling / addressing
divergences with the semantics fully recovered; details in the blocker notes.
