# PE-BTL5 — overlay +0xE wait producers; 6914C is not the 0x3B callee

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C was added. Overlay returns, mode 7, `0x55`
completion, ATB, menus, damage, PE, and AI were not invented.

## Proven windows

```text
0x800145DC..0x800145F8   7 words   144FC state 0x39 jal 6914C(1)
0x800145F8..0x80014630  14 words   144FC 0x3A D1A0|=2 then jal 29810
0x80014630..0x80014658  10 words   144FC state 0x3B lbu +0xE; andi 3
0x8002CEE0..0x8002CF2C  19 words   mode-7 jal 6914C(0) then D_8009D28C=7
0x8006914C..0x80069594 274 words   func_8006914C: CD/LoadImage SM on +0xEF
0x8006C4C4..0x8006C5BC  62 words   func_8006C4C4 wait-bit setter
0x8006C5BC..0x8006CC68 427 words   func_8006C5BC CD poll; SHA-256 d15126b6…
0x8006CC20..0x8006CC38   6 words   6C5BC andi 0xFC clear of bits 0-1
```

JT `0x800100A0`: state `0x39` = `0x800145DC`, `0x3A` = `0x800145F8`
(`jal 29810`), `0x3B` = `0x80014630`. `144FC` does not jal `6C4C4`
or `6C5BC`.

## `func_8006914C` is not the 0x3B wait

`$s3 = a0`, `$s4 = D_800B0CD8`. Switch on overlay `+0xEF` (0 / 0x34 /
0x35 / 0x36): CD `6E6A8`, poll `6E7E8`, LoadImage `6E1C0`/`7506C`.
Return 1 while that SM is in flight; 0 on idle/success. Stores overlay
word bit 3 (`0x8`) and `+0xEF`. **Zero `sb`/`sh`/`sw` cover
`D_800B0CE6`.**

Four `jal` sites:

```text
0x800145DC  144FC state 0x39   a0=1   before 29810 / first command
0x80016D94                 a0=1
0x8002CEE0  mode-7 gate        a0=0   after the NYPD 0x55 path
0x8006F4F8                 a0=0
```

State 0x3B only `lbu +0xE; andi 3`. Do not stub 6914C to return 0.

## Bit meanings on `D_800B0CD8+0xE` (`D_800B0CE6`)

| Bits | ROM | Role |
|---|---|---|
| `0x1` | `ori 1` / wait `andi 1` | slot/load busy |
| `0x2` | `ori 2` / wait `andi 2` | overlay/D1A0-related busy |
| `0x3` | 144FC states 0 and 0x3B | park while `(byte & 3) != 0` |
| `0x4` | `ori 4` then 6C4C4 `andi 0xFB` | promote into bits 0-1 |
| boot | `func_8006A674` `sb $zero` | starts at 0 |

## Writers that can make `(byte & 3) == 0`

`func_8006C5BC` completion RMW `0x8006CC2C`: `andi $v1, 0xFC` then
delay-slot `sb $v1, 0xE($s4)`. `0xFC = ~3`. Also `sb $zero, +0xEE`.
Callers (TEXT census, three sites only):

```text
0x8003F22C  func_8003F074   3F3C4 first jal; 6C4C4(CE4) then poll 6C5BC
0x80035B24  func_80035558   after actor-list walk; 3F3C4 @ 0x8003F4F0
0x8006C358  func_8006C1CC   JT state 6; not jal'd by 144FC/29810
```

`144FC` / `29810` / `6914C` / `6D60C` / `6C4C4` / `6BECC` do not jal it.
CE2 must be in `[10,14]` (`lbu D_800B0CE2; addiu -10; sltiu 5`). Dispatch
is `+0xEE` via JT `0x800113F0`. EE=11 `ori 1`; EE=8/9/10 return 0.
Completion is the `andi 0xFC` RMW. Callees: `6CC68`×6, `6E6A8`×2,
`6E7E8`×2, `6E1C0`, `6CDA4`, `3D050`, `6698C`, `3D834`.

Setters (busy):

```text
func_8006C4C4  a0==-1           ori 3
               D1A0 bit1 or
               overlay word bit1 ori 2; D_8009D2E8 &= ~2
               +0xE bit 4 set    (e|3) & ~4
               a0 in 1..8, != +0xD  ori 1 and sb a0 to +0xC/+0xD
func_8006C5BC  0x8006C9DC       ori 1 (nested wait)
func_8006E60C  0x8006E680       ori 2 if overlay word bit 0x08000000
pre-29810      0x800297EC       ori 2 through &$D_800B0CE6
```

`6B4F8` has no `+0xE` store. `6BEC0` (`ori 4`) is the function
*before* `6BECC`. `6C1CC` region `ori 4` is bit 2, ignored by `andi 3`.
`func_8003F3C4` can `sb` a script byte into `D_800B0CE6` at
`0x8003FDF8` (not this NYPD producer).

## Native scope

Matching native ports `func_8006C4C4` (62/62), the 6C5BC CE2/EE
named cut, the completion RMW, and `144FC` `0x3A` `D1A0|=2`. The live
0x3B wait is `3F074`'s `6C4C4(CE4)` + one `6C5BC` after command bind.
EE=13 does not invent CD or `andi 0xFC`. Tests do not stub 6914C or
store mode 7. TRACE
`encounter_55 → hp_copied → first_command → command_bound → overlay_wait`.

## Verify

```text
python3 pc_port/tools/pe_btl5_overlay_wait_oracle.py
python3 pc_port/tools/pe_btl3_29810_tail_oracle.py
PE_TEST_FILTER=BTL ./pc_port/build/pe-native-tests
python3 pc_port/tools/pe_btl2_hp_trace_oracle.py
```

STOP: superseded by PE-BTL6. Live EE=13 does **not** jal `6CC68`
(six sites are EE 0/1-7 only). Next is `3D050` after the ported
package-walk prefix. No matching `src/` C.
