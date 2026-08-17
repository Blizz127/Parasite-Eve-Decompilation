# PE battle-system realization ledger

Authority: Disc 1 EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. Goal: NYPD `0x55(2)` becomes the first instance of
the retail battle runtime, not a one-off script.

| ID | Item | Status | Evidence |
|---|---|---|---|
| BTL-CMD | Writer B type-0 `idB=4` = `0x6C14` / 18 frames; actor `+0x1B0` | PORTED | `pe-btl3-first-command` |
| BTL-HP | `293F4` HP triple; `29810` tail → `1A680` command 4 | PORTED | `pe-btl2-hp-layout`, `pe-btl3-first-command` |
| BTL-3B | `144FC` `0x3B` @ `0x80014630` 12w; `lw/sw 0($s0)` overlay[0]; v0=1 iff bits clear | PORTED | `pe_btl6_14630_oracle.py` sha `3e531fb7…` |
| BTL-3F074-POLL | `s0=1`; jal 6C5BC; `beq v0,s0` until v0=0 | PROVEN | `pe_btl7_3f074_poll_oracle.py` |
| BTL-6CC68-EXIT | EE=0 idle after bits clear: jal 6CC68, 6C5BC v0=0, poll exits | PROVEN | same; 79w sha `262dcfc6…` |
| BTL-6C4C4 | Setter `ori 1/2/3`, 62/62 | PORTED | `func_8006C4C4_port.c` |
| BTL-6C5BC-WIN | `0x8006C5BC..0x8006CC68` 427 words, SHA-256 `d15126b6…686a` | PROVEN | `pe_btl5_overlay_wait_oracle.py` |
| BTL-6C5BC-CALL | TEXT jals only `35B24`/`3F22C`/`6C358`; `3F3C4`→`3F074` poll + `35558` | PROVEN | same oracle |
| BTL-6C5BC-CUT | CE2 `[10,14]`, EE 0/11/12; EE=13 prefix then 6CC2C clearer, return 1 | PORTED | `func_8006C5BC` named cut |
| BTL-3A-D1A0 | `144FC` `0x3A` `D_8009D1A0 \|= 2` | PORTED | `func_800144FC_state3A_d1a0_cut` |
| BTL-LIVE-3B | 3F074: one `6C4C4` then poll `6C5BC` to 0; TRACE `overlay_wait` | PORTED | `func_8003F074_poll_cut` |
| BTL-6CC68 | 79w always v0=0; D10=actor+0x1B4; 661A4/661CC OFX; 3A088 empty | PORTED | `pe_btl7_6cc68_oracle.py` |
| BTL-EE13-PREFIX | EE=13 `lw +0x158` walk → `+0x1C0`; zeros `+0x10`/`+0x134`; D254/D1A0 a1 | PORTED | `func_8006C5BC_ee13_prefix_cut` |
| BTL-EE13-3D050 | `jal 3D050` (505w) a0=`overlay+0x14` a3=704; then `6698C` live 117w, `3D834` 70w, then `andi 0xFC` | PORTED | `pe-btl6-6cc2c-epilogue` |
| BTL-6CC2C | EE=13 after jal 3D834: `andi 0xFC` / `sb +0xE` / `sb 0 → +0xEE`; v0=1 | PORTED | `func_8006C5BC_ee13_epilogue_cut` |
| BTL-14544 | `144FC` state 0: if bits clear sb 0x37; overlay[0] `|= 0x800000`; v0=0 | PORTED | `func_800144FC_state0_cut` |
| BTL-14570 | `144FC` state 0x37: jal 42EDC unless overlay bit `0x400000`; sb 0x38 | PORTED | `func_800144FC_state37_cut` |
| BTL-42EDC | 17w; `lbu D_800BD024` clamp into gp+0x16C; gp+0x168/174=1 | PORTED | `func_80042EDC` |
| BTL-3D050-PFX | Pointer ladder `+0/4/8/C/10`, `+0x54=a2`, `+0xBA=1` | PORTED | `func_8003D050_prefix_cut` |
| BTL-3D050-R | ptr14 `+0x14..+0x20`; post-3D94C-skip; epilogue after `3C5D8`; `3D94C` skipped | PORTED | `pe-btl6-3d050-remainder` |
| BTL-3C5D8 | 24 words; live a1=50 → `+0x8D=50`, `128/50=2` | PORTED | `func_8003C5D8` |
| BTL-6698C | Live first leaf 117w; `D_800BEA40` fill; 215w window is four `jr`s | PORTED | `func_8006698C` |
| BTL-3D834 | 70 words to `0x8003D94C`, not 489; a1==0 skips first 3 jals | PROVEN | remainder oracle |
| BTL-3DFD8 | Live copy leaf 51w; `jal 3DFD8(0x800B1638, dest+0x34, 1)` | PORTED | `func_8003DFD8` |
| BTL-794C4 | First leaf 163w RotMatrix; live zero angles → identity at dest+0x34 | PORTED | `func_800794C4` |
| BTL-3A088 | 392w; mode-0 empty + live walk; RTIR/RTV0 integer MVMVA; 2-bone `(0,20,-7)` | PORTED | `pe-btl6-3a088-walk` |
| BTL-3B97C | 217w, 0 jals; empty dest+0 / +0xBA / obj+2==0; lighting RTIR+NCCT at 0x8003BA24 | PORTED | `pe-btl6-3b97c-lighting` |
| BTL-3BCE0 | 245w, 0 jals; four directory packet walks; keep-byte overlaps sw+4 | PORTED | `pe-btl6-3d834-callees` |
| BTL-158 | Only `6E6A8` dest `+0x158` is EE=4; first `6C4C4(-1)` sets CE4=1; CE2=11 → PE.IMG `[428,434)` `obj+0x18=2` | PROVEN | `pe-btl6-3d834-callees` |
| BTL-6914C | 274-word loader; `0x39`/`mode7`; no `+0xE` store | PORTED | state 0 tables + tail + 0x34/0x35; jalr 0x800E0xxx not entered |
| BTL-6D60C | `144FC` `0x38` jal `6D60C(1)`; 387w sha256 `14e5794d…`; +0xF2 JT; no `+0xE` | PROVEN | `pe-btl6-6d60c` |
| BTL-6D078 | 117w `0x8006D078..0x8006D24C`; +0xF3 JT; state0 sb 0x28; 0x28 jals 6CDA4 | PORTED | empty 0x2A → F3=0; bit0x10+half≥2 → 0x2B |
| BTL-6D79C | 6D60C after 6D078=0; live F2 0x3F→0x2F; 6CDA4(0) → 87198 | PORTED | B0E64=D11614-8 only; NYPD 0x2A empty |
| BTL-87198 | 5w `D_8009D270=1` return 0; 6CDA4 state0 a0=0 | PORTED | matching `src/` already; native port |
| BTL-86464 | 13w `CD80=0x10` `CD84=a0` jal 8CBA8; cmd 0x10 = 85084 fail -1 | PORTED | no stream-complete |
| BTL-86C1C | 16w `CD80=0xC0` `CD84=a1&7F` `CD90=a0` jal 8CBA8 | PORTED | 0xC0 default ring |
| BTL-42F20 | 6w `gp+0x168=5` `gp+0x174=-1`; 144FC 0x38 after 6D60C==0 | PORTED | `pe_btl6_42f20_oracle.py` |
| BTL-145DC | `144FC` 0x39 jal `6914C(1)`; v0==1 parks; no sb 0x3A | PORTED | no-disc 6E6A8 -1 stays 0x34 |
| BTL-693F8 | 0x34 jal 6E6A8 dest `0x801ED800` LBA PE.IMG+`0x7E` n=5 | PORTED | sha256 `3b2ff0b8…d3c9`; sync poll → EF=0x36 |
| BTL-69468 | 0x36 jal 6E1C0 ×64 stride `0x14` then 6E498(`+0x18C`, `0x73DECD80`) | PORTED | TIM-like; EF=0 overlay&=~8 v0=0; not overlay |
| BTL-145F8 | `144FC` 0x3A: D1A0\|=2, `lbu(*binder)` jal 29810, sb 0x3B, park | PORTED | `pe_btl6_145f8_oracle.py` |
| BTL-20EFC | 7w 5×`sb 0` gp-rel; 29810 jal void(void), delay `s0=a0` | PORTED | sha256 `9136c11e…1e7b`; already matching `src/` |
| BTL-71A64 | 3w BIOS A(0x30) puts; 29810 `a0=lw D_8009D250` | PORTED | live a0=0 no stores; no invented puts |
| BTL-29854 | 29810 after 20EFC: D1AC&=~0x300, A7FF0×10 `{0,-1}`, B8A90×7 | PORTED | `pe_btl6_29854_oracle.py`; not `0x800C8A90` |
| BTL-29810-PFX | 29810 prologue 15w: zeros D1E8/D290/D28C=0 (not 7), D278=lw(*D254) | PORTED | `pe_btl6_29810_oracle.py` sha256 `a8fd26f9…64e8` |
| BTL-209F0 | 161w; D278 sb +0x12=4..+0x19=11; jal 6C4C4(lh(*(D278+0x68)+6)) | PORTED | D278=*D254=0x6F slot body |
| BTL-209F0-68 | Slot tmpl `109B0+0x68=0`; no non-zero sw to D278/+*D254 +0x68 | PORTED | actor+0x68 is motion (35038/35558/D2F0), not this pointer |
| BTL-145F8-A0 | 0x3A `lw 0(s1); lbu 0(v0)`; s1=144FC a0=binder | PORTED | 0x55(2) mode 0 → lbu(2), not overlay/`*actor`/encounter 2 |
| BTL-RAM-LOW | Host zeros at addr 2 and 6 | APPROXIMATION | EXE has no 0x80000000 image; replace from post-boot RAM |
| BTL-339A0 | 32w 0 jals; 4 pairs at 0x80010E38 → CE80/CE84/CE86 | PORTED | `pe_btl6_339a0_oracle.py`; a0=lbu(*binder) |
| BTL-87414 | 5w `D_8009D270=2` return 0; 6CDA4 state0 a0=3 | PORTED | matching `src/` already; native port |
| BTL-6CDA4 | 181w +0xF0 SM; live a0=1 sb 7 + table 0x0F; state7 real 6E6D4 | PORTED | -1 → F0=0; ok → F0=8 |
| BTL-6E7E8 | 19w poll; state8 -1→7 pending→8 0→9; PE.IMG 8C6 AKAO | PORTED | `func_8006CDA4_state8_cut` parks at 9 |
| BTL-87090 | 20w jal 851A8; state9 a0=1 dest,0; AKAO magic-check 0 | PORTED | -1 → F0=0; ok → F0=0xA |
| BTL-870E0 | 4w return D_8009D24C; stateA -1→0 busy stay 0→F0=7 | PORTED | no DMA stub; writers 85098/850C0/851A8 |
| BTL-MODE7 | `0x8002CEE0` jal `6914C(0)` then `D_8009D28C=7` | PROVEN | not issued |
| BTL-ATB | ATB / menus / AI / damage / death / field return | RESEARCH_REQUIRED | do not invent |
| opcode_0x55_complete | 3F074 poll-until-0 then 0x3B v0=1 (matching+vis3) | RETAIL_DERIVED | not battle-over; next op `0x89` |
| M1_0x55_real_completion | 0x3B after drained poll | PROVEN | `test_BTL7_3F074_poll_opens_3B` |
| BTL-1A918 | 56w rebase `D_800B1620`; writer `6B8E8` overlay+0x948 | PORTED | `pe_btl8_1a918_oracle.py` |
| BTL-E0060 | 27w EXE-resident list clear; REJECTED loaded | PORTED | same; sha `cfa139eb…` |
| M2_battle_overlay_entry | E0060 is not overlay entry | NO | EXE tsize covers 0x800E0060 |
| next_live_va | `0x800371B0` | PROVEN | then `0x800125E0` |
| func_800339A0_a0_provenance | 0x3A `lbu 0($s1)` binder; 14630 does not consume 339A0 | DEFERRED | `8E22` vs `8E02` |

## Rejected

| Hypothesis | Why |
|---|---|
| `6914C` is the `0x3B` wait | State `0x39` only; zero `+0xE` stores |
| `144FC`/`29810` jal `6C5BC` | TEXT census: three sites, none inside those leaves |
| Auto-clear `+0xE` or stub `6914C=0` | Fabrication; forbidden |
| Bind idle/`0x20`/idA=2 | Wrong table row |
| `10928+0x68` / actor+0x68 prove 209F0's pointer | Wrong object; D278 is the 0x6F slot body |
| 0x3A `lbu(*overlay)` or 29810(2) | ROM is `lbu(*binder)`; 0x55(2) → `lbu(2)` |
| State 0 / 0x3B `*s1` is the binder | `8E02` rs=$s0 overlay[0]; `8E22` is 0x3A binder only |
| EE=13 return 1 completes `0x55` | 3F074 keeps polling while v0==1; exit is EE=0 → 6CC68 v0=0 |
| `6CC68` is the EE=13 body | Six jal sites, all before `0x8006C9F8` |
| One 6C5BC per 0x55 tick completes the wait | 6C4C4 re-ORs bit1; only the tight poll lets 0x3B see the clear |
| `0x800E0060` is loaded overlay / battle entry | EXE tsize `0x1EE000` contains 27w leaf; no jal |

## Verify

```text
python3 pc_port/tools/pe_btl5_overlay_wait_oracle.py
python3 pc_port/tools/pe_btl6_ee13_oracle.py
python3 pc_port/tools/pe_btl6_3d050_remainder_oracle.py
python3 pc_port/tools/pe_btl3_29810_tail_oracle.py
python3 pc_port/tools/pe_btl2_hp_trace_oracle.py
python3 pc_port/tools/pe_btl6_3d834_callees_oracle.py
python3 pc_port/tools/pe_btl6_3a088_walk_oracle.py
python3 pc_port/tools/pe_btl6_3b97c_lighting_oracle.py
python3 pc_port/tools/pe_btl6_6cc2c_oracle.py
python3 pc_port/tools/pe_btl6_14544_oracle.py
python3 pc_port/tools/pe_btl6_6d60c_oracle.py
python3 pc_port/tools/pe_btl6_6d078_oracle.py
python3 pc_port/tools/pe_btl6_6cda4_oracle.py
python3 pc_port/tools/pe_btl6_6e7e8_oracle.py
python3 pc_port/tools/pe_btl6_87090_oracle.py
python3 pc_port/tools/pe_btl6_870e0_oracle.py
python3 pc_port/tools/pe_btl6_87198_oracle.py
python3 pc_port/tools/pe_btl6_86464_oracle.py
python3 pc_port/tools/pe_btl6_42f20_oracle.py
python3 pc_port/tools/pe_btl6_693f8_oracle.py
python3 pc_port/tools/pe_btl6_69468_oracle.py
python3 pc_port/tools/pe_btl6_145f8_oracle.py
python3 pc_port/tools/pe_btl6_14630_oracle.py
python3 pc_port/tools/pe_btl7_3f074_poll_oracle.py
python3 pc_port/tools/pe_btl7_6cc68_oracle.py
python3 pc_port/tools/pe_btl8_1a918_oracle.py
python3 pc_port/tools/pe_btl6_20efc_oracle.py
python3 pc_port/tools/pe_btl6_29854_oracle.py
python3 pc_port/tools/pe_btl6_29810_oracle.py
python3 pc_port/tools/pe_btl6_209f0_oracle.py
python3 pc_port/tools/pe_btl6_339a0_oracle.py
./pc_port/build/pe-native-tests   # matching_native after this rung
```

STOP/NEXT: `0x800371B0` then `0x800125E0`. `E0060` is EXE
list-clear, not M2. Do not jalr `0x800E086C`. `0x89` is
mode-6 request, not battle-over.
