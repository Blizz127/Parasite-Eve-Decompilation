# PE battle-system realization ledger

Authority: Disc 1 EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. Goal: NYPD `0x55(2)` becomes the first instance of
the retail battle runtime, not a one-off script.

| ID | Item | Status | Evidence |
|---|---|---|---|
| BTL-CMD | Writer B type-0 `idB=4` = `0x6C14` / 18 frames; actor `+0x1B0` | PORTED | `pe-btl3-first-command` |
| BTL-HP | `293F4` HP triple; `29810` tail → `1A680` command 4 | PORTED | `pe-btl2-hp-layout`, `pe-btl3-first-command` |
| BTL-3B | `144FC` `0x3B` = `lbu +0xE; andi 3`; not `6914C` | PROVEN | `pe-btl5-overlay-wait` |
| BTL-6C4C4 | Setter `ori 1/2/3`, 62/62 | PORTED | `func_8006C4C4_port.c` |
| BTL-6C5BC-WIN | `0x8006C5BC..0x8006CC68` 427 words, SHA-256 `d15126b6…686a` | PROVEN | `pe_btl5_overlay_wait_oracle.py` |
| BTL-6C5BC-CALL | TEXT jals only `35B24`/`3F22C`/`6C358`; `3F3C4`→`3F074` poll + `35558` | PROVEN | same oracle |
| BTL-6C5BC-CUT | CE2 `[10,14]`, EE 0/11/12, no auto-clear; EE 1-7/13 return 1 | PORTED | `func_8006C5BC` named cut |
| BTL-3A-D1A0 | `144FC` `0x3A` `D_8009D1A0 \|= 2` | PORTED | `func_800144FC_state3A_d1a0_cut` |
| BTL-LIVE-3B | Next tick `6C4C4(CE4)` then `6C5BC` once; TRACE `overlay_wait` | PORTED | tests + TRACE_CONTRACT |
| BTL-6CC68 | 79 words; six TEXT sites all in EE 0/1-7; **not** on live bit1→EE13 | PROVEN | `pe_btl6_ee13_oracle.py` |
| BTL-EE13-PREFIX | EE=13 `lw +0x158` walk → `+0x1C0`; zeros `+0x10`/`+0x134`; D254/D1A0 a1 | PORTED | `func_8006C5BC_ee13_prefix_cut` |
| BTL-EE13-3D050 | `jal 3D050` (505w) a0=`overlay+0x14` a3=704; then `6698C` (215w, 0 jals), `3D834` (489w), then `andi 0xFC` | RESEARCH_REQUIRED | body after prefix |
| BTL-3D050-PFX | Pointer ladder `+0/4/8/C/10`, `+0x54=a2`, `+0xBA=1` | PORTED | `func_8003D050_prefix_cut` |
| BTL-6914C | 274-word loader; `0x39`/`mode7`; no `+0xE` store | PROVEN | not the 0x3B wait |
| BTL-6D60C | `144FC` `0x38` jal `6D60C(1)`; 387 words; no `+0xE` | PROVEN | before 0x39 |
| BTL-MODE7 | `0x8002CEE0` jal `6914C(0)` then `D_8009D28C=7` | PROVEN | not issued |
| BTL-ATB | ATB / menus / AI / damage / death / field return | RESEARCH_REQUIRED | do not invent |

## Rejected

| Hypothesis | Why |
|---|---|
| `6914C` is the `0x3B` wait | State `0x39` only; zero `+0xE` stores |
| `144FC`/`29810` jal `6C5BC` | TEXT census: three sites, none inside those leaves |
| Auto-clear `+0xE` or stub `6914C=0` | Fabrication; forbidden |
| Bind idle/`0x20`/idA=2 | Wrong table row |
| `6CC68` is the EE=13 body | Six jal sites, all before `0x8006C9F8` |

## Verify

```text
python3 pc_port/tools/pe_btl5_overlay_wait_oracle.py
python3 pc_port/tools/pe_btl6_ee13_oracle.py
python3 pc_port/tools/pe_btl3_29810_tail_oracle.py
python3 pc_port/tools/pe_btl2_hp_trace_oracle.py
./pc_port/build/pe-native-tests   # 668/668
```

STOP/NEXT: `func_8003D050` (505 words) so EE=13 can reach `6698C` /
`3D834` / `andi 0xFC`. Do not invent those callees or the clear.
