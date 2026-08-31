# Tier-2 shape partition — 2026-08-31

Status: authoritative Phase-0 work order for the campaign resumed from clean
`main` at `81ab3fe` with **380 exact matching-C leaves**. This file is
classification evidence only; it does not claim function hood, semantics, or a match
for any listed candidate. C2 must prove each selected leaf independently before C is written.

## Closure and method

The historical Tier-2 table had 214 unique rows. Current YAML already integrates
39 of them, leaving **175** active asm rows. Every surviving row was
resolved by exact file offset against the current generated assembly; all 175 body
lengths agree with the table. Current matched-C and SDK maps were then applied to call
targets. The exhaustive closure is:

| class | count | scheduling meaning |
|---|---:|---|
| W | 10 | <=12 words, exactly one direct jal, matched/SDK callee, no loop or indexed symbolic access |
| S | 1 | <=12 words, no call/branch, branchless global load/store shape |
| B | 121 | remaining call-bearing work; qualifiers below preserve why it is not W |
| C | 39 | no direct jal; self-contained arithmetic, comparison, or loop |
| SKIP | 4 | architectural handwritten code or already-exhausted accepted residual |
| **total** | **175** | exact closure |

The requested five labels do not by themselves cover a one-jal wrapper whose callee is
still asm, nor a call-bearing function longer than 20 words. Those are kept visible
inside B rather than mislabeled:

| B qualifier | count | definition |
|---|---:|---|
| B-CORE | 19 | <=20 words and has a branch or multiple direct calls |
| B-1U | 72 | <=20 words, one call, but fails a W precondition |
| B-LONG | 30 | call-bearing body over 20 words |
| **B total** | **121** | exact closure |

Work order is **W -> S -> B -> C**. SKIP rows consume no phrasing attempts.
A 0/N direct-caller count is not automatically a hood failure: an exact-start table
reference can prove function hood, but C2 must identify and document that reference.

## W — forwarding wrappers

| off | function | words | callers/refs | gp | detail |
|---:|---|---:|---:|---|---|
| `0x70464` | `func_8007FC64` | 9 | 1/1 | no | SDK-mapped callee func_8007B010 |
| `0x70488` | `func_8007FC88` | 9 | 1/1 | no | SDK-mapped callee func_8007B290 |
| `0xB8500` | `func_800C7D00` | 11 | 0/1 | no | matched callee func_800C2AF0 |
| `0xB9644` | `func_800C8E44` | 11 | 0/1 | no | matched callee func_800C2AF0 |
| `0xBA33C` | `func_800C9B3C` | 11 | 0/1 | no | matched callee func_800C2AF0 |
| `0xBAED4` | `func_800CA6D4` | 11 | 0/1 | no | matched callee func_800C2AF0 |
| `0xBC6E0` | `func_800CBEE0` | 11 | 0/1 | no | matched callee func_800C2AF0 |
| `0xBD6BC` | `func_800CCEBC` | 11 | 0/1 | no | matched callee func_800C2AF0 |
| `0xBE09C` | `func_800CD89C` | 11 | 0/1 | no | matched callee func_800C2AF0 |
| `0xBE918` | `func_800CE118` | 11 | 0/1 | no | matched callee func_800C2AF0 |

## S — short state access

| off | function | words | callers/refs | gp | detail |
|---:|---|---:|---:|---|---|
| `0x2A170` | `func_80039970` | 11 | 5/5 | no | short branchless global load/store leaf |

## B — call-bearing remainder

| off | function | words | callers/refs | gp | detail |
|---:|---|---:|---:|---|---|
| `0x41860` | `func_80051060` | 9 | 0/2 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80053648 |
| `0x765C4` | `func_80085DC4` | 9 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80073CC4 |
| `0x379BC` | `func_800471BC` | 10 | 0/4 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x37B2C` | `func_8004732C` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x37CA8` | `func_800474A8` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x3BD34` | `func_8004B534` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x3BD5C` | `func_8004B55C` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x3F730` | `func_8004EF30` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x3FAE4` | `func_8004F2E4` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40150` | `func_8004F950` | 10 | 0/16 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40178` | `func_8004F978` | 10 | 0/4 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40730` | `func_8004FF30` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40758` | `func_8004FF58` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40780` | `func_8004FF80` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x407A8` | `func_8004FFA8` | 10 | 0/6 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x407D0` | `func_8004FFD0` | 10 | 0/6 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x407F8` | `func_8004FFF8` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40838` | `func_80050038` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40860` | `func_80050060` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40A04` | `func_80050204` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40CF4` | `func_800504F4` | 10 | 0/4 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80042020 |
| `0x40D1C` | `func_8005051C` | 10 | 0/2 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80042170 |
| `0x68200` | `func_80077A00` | 10 | 0/8 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80073CF4 |
| `0x75898` | `func_80085098` | 10 | 0/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80085F44 |
| `0xB852C` | `func_800C7D2C` | 10 | 0/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800C2414 |
| `0xB9670` | `func_800C8E70` | 10 | 0/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800C2414 |
| `0xBA368` | `func_800C9B68` | 10 | 0/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800C2414 |
| `0xBAF00` | `func_800CA700` | 10 | 0/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800C2414 |
| `0xBC70C` | `func_800CBF0C` | 10 | 0/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800C2414 |
| `0xBD6E8` | `func_800CCEE8` | 10 | 0/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800C2414 |
| `0xBE0C8` | `func_800CD8C8` | 10 | 0/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800C2414 |
| `0xBE944` | `func_800CE144` | 10 | 0/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800C2414 |
| `0x3FC64` | `func_8004F464` | 11 | 2/2 | yes | B-CORE: branch/multi-call <=20 words; func_8004E97C |
| `0x42F64` | `func_80052764` | 11 | 4/4 | yes | B-CORE: branch/multi-call <=20 words; func_800866A4 |
| `0x5EEA8` | `func_8006E6A8` | 11 | 26/26 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8006E6D4 |
| `0x76CCC` | `func_800864CC` | 11 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x777F8` | `func_80086FF8` | 11 | 6/6 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x77824` | `func_80087024` | 11 | 6/6 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x4ED4C` | `func_8005E54C` | 12 | 2/2 | yes | B-CORE: branch/multi-call <=20 words; func_8005E038 |
| `0x72A7C` | `func_8008227C` | 12 | 0/2 | no | B-CORE: branch/multi-call <=20 words; func_800824C8 |
| `0x3C480` | `func_8004BC80` | 13 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80062D2C |
| `0x40A2C` | `func_8005022C` | 13 | 0/2 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800638D8 |
| `0x40B08` | `func_80050308` | 13 | 0/2 | yes | B-CORE: branch/multi-call <=20 words; func_8005EB64 |
| `0x50D28` | `func_80060528` | 13 | 3/3 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800602D0 |
| `0x50D5C` | `func_8006055C` | 13 | 6/6 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800602D0 |
| `0x50D90` | `func_80060590` | 13 | 11/11 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800602D0 |
| `0x50DC4` | `func_800605C4` | 13 | 1/1 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800602D0 |
| `0x50DF8` | `func_800605F8` | 13 | 8/8 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800602D0 |
| `0x615D0` | `func_80070DD0` | 13 | 3/3 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80070D6C |
| `0x67BD0` | `func_800773D0` | 13 | 5/5 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80073A44 |
| `0x71764` | `func_80080F64` | 13 | 0/2 | no | B-CORE: branch/multi-call <=20 words; func_80081D74 |
| `0x758C0` | `func_800850C0` | 13 | 2/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80085F44 |
| `0x76C64` | `func_80086464` | 13 | 3/3 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x76C98` | `func_80086498` | 13 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x76FB0` | `func_800867B0` | 13 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x6634C` | `func_80075B4C` | 14 | 2/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_800762BC |
| `0x77074` | `func_80086874` | 14 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x771AC` | `func_800869AC` | 14 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x772E4` | `func_80086AE4` | 14 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x775AC` | `func_80086DAC` | 14 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x77670` | `func_80086E70` | 14 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x77734` | `func_80086F34` | 14 | 2/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x7F978` | `func_8008F178` | 14 | 2/2 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008F0D0 |
| `0x33478` | `func_80042C78` | 16 | 2/2 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80042CC4 |
| `0x704BC` | `func_8007FCBC` | 16 | 1/1 | no | B-CORE: branch/multi-call <=20 words; func_8007FCFC |
| `0x76F70` | `func_80086770` | 16 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x7741C` | `func_80086C1C` | 16 | 3/3 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x775E4` | `func_80086DE4` | 16 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x776A8` | `func_80086EA8` | 16 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x7776C` | `func_80086F6C` | 16 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x405A4` | `func_8004FDA4` | 17 | 0/4 | no | B-CORE: branch/multi-call <=20 words; func_80042770 |
| `0x72C00` | `func_80082400` | 17 | 0/2 | no | B-CORE: branch/multi-call <=20 words; func_80081D74 |
| `0x770AC` | `func_800868AC` | 17 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x771E4` | `func_800869E4` | 17 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x7731C` | `func_80086B1C` | 17 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x42DEC` | `func_800525EC` | 18 | 49/49 | no | B-CORE: branch/multi-call <=20 words; func_8006DF50 |
| `0x42E34` | `func_80052634` | 18 | 35/35 | no | B-CORE: branch/multi-call <=20 words; func_8006DF50 |
| `0x42E7C` | `func_8005267C` | 18 | 22/22 | no | B-CORE: branch/multi-call <=20 words; func_8006DF50 |
| `0x42EC4` | `func_800526C4` | 18 | 23/23 | no | B-CORE: branch/multi-call <=20 words; func_8006DF50 |
| `0x76F28` | `func_80086728` | 18 | 1/1 | no | B-CORE: branch/multi-call <=20 words; func_8008CBA8 |
| `0x76FE4` | `func_800867E4` | 18 | 2/2 | no | B-CORE: branch/multi-call <=20 words; func_8008CBA8 |
| `0x7702C` | `func_8008682C` | 18 | 2/2 | no | B-CORE: branch/multi-call <=20 words; func_8008CBA8 |
| `0x7745C` | `func_80086C5C` | 18 | 7/7 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x436C0` | `func_80052EC0` | 19 | 1/1 | yes | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_80052F70 |
| `0x43724` | `func_80052F24` | 19 | 1/1 | yes | B-CORE: branch/multi-call <=20 words; func_80052F70 |
| `0x76EA4` | `func_800866A4` | 19 | 9/9 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x77624` | `func_80086E24` | 19 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x776E8` | `func_80086EE8` | 19 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x777AC` | `func_80086FAC` | 19 | 1/1 | no | B-1U: one-call non-W; callee unresolved or indexed/loop constraint; func_8008CBA8 |
| `0x4F0C4` | `func_8005E8C4` | 20 | 9/9 | yes | B-CORE: branch/multi-call <=20 words; func_800527C0 |
| `0x77890` | `func_80087090` | 20 | 2/2 | no | B-CORE: branch/multi-call <=20 words; func_800851A8 |
| `0x4F114` | `func_8005E914` | 21 | 9/9 | yes | B-LONG: call-bearing >20 words; func_800527C0 |
| `0x774A4` | `func_80086CA4` | 21 | 1/1 | no | B-LONG: call-bearing >20 words; func_8008CBA8 |
| `0x42F0C` | `func_8005270C` | 22 | 3/3 | yes | B-LONG: call-bearing >20 words; func_8006DF50 |
| `0x770F0` | `func_800868F0` | 22 | 1/1 | no | B-LONG: call-bearing >20 words; func_8008CBA8 |
| `0x77228` | `func_80086A28` | 22 | 1/1 | no | B-LONG: call-bearing >20 words; func_8008CBA8 |
| `0x77360` | `func_80086B60` | 22 | 1/1 | no | B-LONG: call-bearing >20 words; func_8008CBA8 |
| `0xBEC9C` | `func_800CE49C` | 23 | 1/1 | no | B-LONG: call-bearing >20 words; func_80071A74 |
| `0x269F4` | `func_800361F4` | 24 | 3/3 | yes | B-LONG: call-bearing >20 words; func_80017018 |
| `0x5373C` | `func_80062F3C` | 24 | 149/149 | yes | B-LONG: call-bearing >20 words; func_8006269C |
| `0x6A9E0` | `func_8007A1E0` | 24 | 0/2 | no | B-LONG: call-bearing >20 words; func_8007A244 |
| `0x641C4` | `func_800739C4` | 25 | 4/4 | no | B-LONG: call-bearing >20 words; func_80073A34 |
| `0x77148` | `func_80086948` | 25 | 1/1 | no | B-LONG: call-bearing >20 words; func_8008CBA8 |
| `0x77280` | `func_80086A80` | 25 | 1/1 | no | B-LONG: call-bearing >20 words; func_8008CBA8 |
| `0x773B8` | `func_80086BB8` | 25 | 1/1 | no | B-LONG: call-bearing >20 words; func_8008CBA8 |
| `0x55164` | `func_80064964` | 27 | 1/1 | no | B-LONG: call-bearing >20 words; func_80071A24 |
| `0xB85E4` | `func_800C7DE4` | 27 | 0/1 | no | B-LONG: call-bearing >20 words; func_800CEDA8 |
| `0xB9728` | `func_800C8F28` | 27 | 0/1 | no | B-LONG: call-bearing >20 words; func_800CEDA8 |
| `0xBA420` | `func_800C9C20` | 27 | 0/1 | no | B-LONG: call-bearing >20 words; func_800CEDA8 |
| `0xBAFB8` | `func_800CA7B8` | 27 | 0/1 | no | B-LONG: call-bearing >20 words; func_800CEDA8 |
| `0x412D8` | `func_80050AD8` | 28 | 0/8 | yes | B-LONG: call-bearing >20 words; func_8005EB64 |
| `0x44720` | `func_80053F20` | 28 | 8/8 | yes | B-LONG: call-bearing >20 words; func_8005415C |
| `0x63D54` | `func_80073554` | 28 | 4/4 | no | B-LONG: call-bearing >20 words; func_80072DF4 |
| `0x551D0` | `func_800649D0` | 30 | 3/3 | yes | B-LONG: call-bearing >20 words; func_80071A24 |
| `0x43630` | `func_80052E30` | 32 | 44/44 | yes | B-LONG: call-bearing >20 words; func_80052F70 |
| `0x4481C` | `func_8005401C` | 32 | 2/2 | yes | B-LONG: call-bearing >20 words; func_80052F70 |
| `0x55480` | `func_80064C80` | 34 | 5/5 | yes | B-LONG: call-bearing >20 words; func_80062090 |
| `0x7A758` | `func_80089F58` | 34 | 1/1 | no | B-LONG: call-bearing >20 words; func_8008F178 |
| `0x44790` | `func_80053F90` | 35 | 8/8 | no | B-LONG: call-bearing >20 words; func_8005DB44 |
| `0x6F388` | `func_8007EB88` | 35 | 3/3 | no | B-LONG: call-bearing >20 words; func_80080998 |
| `0xBE884` | `func_800CE084` | 37 | 0/1 | no | B-LONG: call-bearing >20 words; func_800C22F8 |

## C — self-contained

| off | function | words | callers/refs | gp | detail |
|---:|---|---:|---:|---|---|
| `0x32F70` | `func_80042770` | 10 | 4/4 | no | self-contained arithmetic/comparison/loop |
| `0x33164` | `func_80042964` | 10 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0x47414` | `func_80056C14` | 11 | 2/2 | no | self-contained arithmetic/comparison/loop |
| `0x6D680` | `func_8007CE80` | 11 | 3/3 | no | self-contained arithmetic/comparison/loop |
| `0x43094` | `func_80052894` | 12 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0x5AAE8` | `func_8006A2E8` | 12 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0x6ED94` | `func_8007E594` | 12 | 2/2 | no | self-contained arithmetic/comparison/loop |
| `0x76714` | `func_80085F14` | 12 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0x58340` | `func_80067B40` | 13 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0x75344` | `func_80084B44` | 13 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0x79A18` | `func_80089218` | 14 | 2/2 | no | self-contained arithmetic/comparison/loop |
| `0x275F8` | `func_80036DF8` | 15 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0x45EE8` | `func_800556E8` | 15 | 19/19 | yes | self-contained arithmetic/comparison/loop |
| `0x486D8` | `func_80057ED8` | 15 | 1/1 | yes | self-contained arithmetic/comparison/loop |
| `0x49608` | `func_80058E08` | 15 | 3/3 | yes | self-contained arithmetic/comparison/loop |
| `0x4C708` | `func_8005BF08` | 15 | 1/1 | yes | self-contained arithmetic/comparison/loop |
| `0x7A82C` | `func_8008A02C` | 15 | 3/3 | no | self-contained arithmetic/comparison/loop |
| `0x4A6C8` | `func_80059EC8` | 16 | 4/4 | yes | self-contained arithmetic/comparison/loop |
| `0x71150` | `func_80080950` | 18 | 5/5 | no | self-contained arithmetic/comparison/loop |
| `0x71198` | `func_80080998` | 18 | 6/6 | no | self-contained arithmetic/comparison/loop |
| `0x539DC` | `func_800631DC` | 20 | 3/3 | yes | self-contained arithmetic/comparison/loop |
| `0xB76F8` | `func_800C6EF8` | 21 | 6/6 | no | self-contained arithmetic/comparison/loop |
| `0xB774C` | `func_800C6F4C` | 21 | 6/6 | no | self-contained arithmetic/comparison/loop |
| `0x42D94` | `func_80052594` | 22 | 5/5 | no | self-contained arithmetic/comparison/loop |
| `0x6E4AC` | `func_8007DCAC` | 23 | 16/16 | no | self-contained arithmetic/comparison/loop |
| `0x26A54` | `func_80036254` | 25 | 4/4 | no | self-contained arithmetic/comparison/loop |
| `0x43408` | `func_80052C08` | 25 | 8/8 | no | self-contained arithmetic/comparison/loop |
| `0x74E44` | `func_80084644` | 26 | 0/2 | no | self-contained arithmetic/comparison/loop |
| `0x2D74` | `func_80012574` | 27 | 1/1 | yes | self-contained arithmetic/comparison/loop |
| `0x62B34` | `func_80072334` | 27 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0xC4E98` | `func_800D4698` | 27 | 0/1 | no | self-contained arithmetic/comparison/loop |
| `0x257C4` | `func_80034FC4` | 29 | 1/1 | yes | self-contained arithmetic/comparison/loop |
| `0x4C0A8` | `func_8005B8A8` | 29 | 5/5 | no | self-contained arithmetic/comparison/loop |
| `0xC4E20` | `func_800D4620` | 30 | 0/1 | no | self-contained arithmetic/comparison/loop |
| `0x27C6C` | `func_8003746C` | 31 | 1/1 | no | self-contained arithmetic/comparison/loop |
| `0x4C510` | `func_8005BD10` | 33 | 1/1 | yes | self-contained arithmetic/comparison/loop |
| `0x4C594` | `func_8005BD94` | 34 | 2/2 | yes | self-contained arithmetic/comparison/loop |
| `0x2681C` | `func_8003601C` | 38 | 1/1 | yes | self-contained arithmetic/comparison/loop |
| `0x685C4` | `func_80077DC4` | 40 | 73/73 | no | self-contained arithmetic/comparison/loop |

## SKIP — do not attempt

| off | function | words | callers/refs | gp | detail |
|---:|---|---:|---:|---|---|
| `0x2F2C8` | `func_8003EAC8` | 15 | 20/20 | no | handwritten architectural ops: mtc2,swc2 |
| `0x45F24` | `func_80055724` | 15 | 2/2 | yes | PARKED-SCHEDULING |
| `0x534E4` | `func_80062CE4` | 18 | 6/6 | yes | PARKED-SCHEDULING |
| `0x27CE8` | `func_800374E8` | 24 | 12/12 | no | PARKED-ALLOCATION |

## Gate result

`PHASE0_CLASSIFICATION=PASS`
`BASE_MATCHING_C=380`
`ACTIVE_TIER2=175`
`WORK_ORDER=W->S->B->C`


