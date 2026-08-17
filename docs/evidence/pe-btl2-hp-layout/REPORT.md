# PE-BTL2 — HP field layout from 0x55 → 29810 → 293F4

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
0x55        D_800910A0[0x55] = func_800144FC (102 words)
            D_800B0CD8+0xF4 jump table at 0x800100A0
            state 0x3A jal 0x80029810 (sole site)
29810       jal func_800293F4 with a0=0 (delay `move $a0,$zero`)
hp_cut      0x800293F4..0x80029448 exclusive, 21/21 words
```

Matching `src/` C was not added (no era/asm split). Do not auto-complete
`0x55`. NYPD still parks: 293F4's HP stores do not write
`D_800B0CD8+0xE`, do not return 1 from 144FC, and do not restore
inhibit.

## HP triple (Aya record at `D_8009D278`)

| Offset | Width | ROM | Role this rung |
|---|---|---|---|
| `+0x0C` | `lh`/`sh`/`lhu` | clamp dest; `blez` at `0x80029350` | current HP |
| `+0x0E` | `sh` of `lhu +0x0C` | copy at `0x80029444` | snapshot of current at init |
| `+0x1C` | `lh` | signed clamp source | cap; default 45 |

```text
lh  +0x1C
lh  +0x0C
if (+0x1C) < (+0x0C): +0x0C = +0x1C
sb  4 at +0x12          # 1A680 clip arg, not a menu command
sh  0 at +0x10
sw  0 at +0x34
sw  0 at gp+0x460       # D_8009D1D0
sh  +0x0C into +0x0E
```

Default rodata `D_80010928`: `+0x0C/+0x0E/+0x1C = 0x002D` (45).
`0x5A` tag 4/`5` write `+0x0C`/`+0x0E`. `0x59` selector 4 reads
`+0x0C`; selector 5 is a JT nop.

## First command — not in this cut

No battle-menu command store on `144FC → 29810 → 293F4(0)`.
`sb 4` at `+0x12` is the `func_8001A680` argument. With `a0=0`, the
`sb 1, gp+0x4D4` at `0x80029464` is skipped, so `func_800299CC`
stays idle (`beqz` at `0x80029A7C`). The post-HP `29810` tail now
issues that command and binds actor `+0x1B0`; see
`docs/evidence/pe-btl3-first-command/REPORT.md`. This HP cut still
does not write actor `+0x0E` / `+0x1B0` itself.

## 0x55 freeze still holds

State 0x3B waits for `D_800B0CD8+0xE & 3 == 0` before return 1.
Overlay waits `6D60C`/`6914C` are not stubbed. Inhibit bit 0 is
untouched. Native playable still prints the expected NYPD stop.

Park path (`a0=0` from 29810) never reaches `sb 1, gp+0x4D4` at
`0x80029464` (that store is on the `a0==1` arm after this exclusive
end). `func_800299CC` therefore stays idle. Overlay `+0xE & 3` still
parks `0x55`. TRACE `first_command` / `command_bound` come from the
BTL3 tail, not from this HP cut.

## Verify

```text
python3 pc_port/tools/pe_btl2_144fc_oracle.py
# PASS: func_800144FC 102/102 + JT + sole jal 29810 + 293F4(0)
python3 pc_port/tools/pe_btl2_293f4_oracle.py
# PASS: func_800293F4_hp_cut 21/21 words + HP +0x0C/+0x0E/+0x1C
PE_TEST_FILTER=293F4 ./pc_port/build/pe-native-tests
# 657 run, 3 passed, 0 failed, 654 skipped
PE_TEST_FILTER=BTL2 ./pc_port/build/pe-native-tests
# 657 run, 1 passed, 0 failed, 656 skipped
python3 pc_port/tools/pe_btl2_hp_trace_oracle.py
# PASS: BTL2 TRACE encounter_55 → hp_copied; no first_command; mode 0
```
