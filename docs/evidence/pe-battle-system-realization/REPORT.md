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
| BTL-371B0 | 169w window init; sw a0 → gp+0x120; TILE 320×54 y=170 | PORTED | `pe_btl9_371b0_oracle.py` sha `83a0b015…` |
| BTL-371B0-A0 | live m0005i uses `D_800B162C` (USA `overlay[0]\|=0x40000000`) | PROVEN | 3F074 @ `0x8003F244`; writer `6B94C` |
| BTL-12574 | 27w sole publisher of `gp+0x94`; 6B4F8 @ `0x8006B8BC` | PORTED | sha `bf4a0017…`; overlay+0x944 |
| BTL-125E0 | 35w DrawSync(0) then `35038(desc+1+i*2,0,1)` | PORTED | sha `7c30399d…`; not battle render |
| BTL-35038-EMPTY | empty `D_8009D2AC` returns 0; no ctor | PORTED | 328w EXE-resident |
| BTL-34FC4 | 29w 14-slot pool at `D_800BEA90`; 3F074@`3F0B0` | PORTED | head → `D_8009D2AC` |
| BTL-1266C | 37w 72-task pool at `D_8009D310`; 3F074@`3F0B8` | PORTED | sha `34d11f74…`; `12700` pops `gp+0x8C` |
| BTL-35038-A1 | a1=0 pop/insert/init/`12700`; `+0x1AC==0` OR `0xE0` | PORTED | type0 `2F76C` and `+0x1AC` body not this cut |
| BTL-35558-WALK | 459w; 3F3C4@`3F4F0`; D20C jalr `+0x190` unless D1A0&4 | PORTED | walk cut only; 29 later jals not this cut |
| BTL-35E04 | 83w types 1–9 vtable; pose snap + 361F4 | PORTED | live `+0x98` bit1 clear; no motion |
| BTL-361F4 | 24w `D2F0=actor`; `+0xA0`×3 → `D300`; jal 17018 | PORTED | now calls 17018 |
| BTL-17018 | 159w task VM; jalr `D_800910A0[op]`; first tick `+0x10=1` | PORTED | ops 0/1/2/0x20 + 0x1C/0x1F; other slots advance |
| BTL-35C84 | 96w type0 vtable; snap + 361F4; D2E8&1 skips 3999C | PORTED | 3999C pad table not this cut |
| BTL-2F76C | 27w type0 stores actor+0 / B8A88 / B8A8C | PORTED | 5218C/51980/51E64 not this cut |
| BTL-6E2D0 | 26w token→6 chars; 0xA80002C8→M0005I | PORTED | charset D_800930B4 |
| BTL-6E454 | 17w atoi name[2..4]; M0005I→5 | PORTED | 6B4F8 uses index-1 |
| BTL-6B4F8-PUB | chunk2 hdr walk; 12574 → +0x944..+0x954 | PORTED | CD/LoadImage not this cut |
| BTL-M0005I-DESC | 125E0 desc count=2 type 1 then 6 | PROVEN | chunk2 sha 01a64ba3…; no type 0 |
| BTL-181CC | 53w op 0xCE; 2FF78/30220; v0=1 | PORTED | live type6 first word |
| BTL-17018-REFETCH | v0!=0 fetches gp+0x90 not *task | PORTED | ROM bne @17248 → 170F0 |
| BTL-15DAC-NOP | 727w; keys 0x193/0x194 → 168F4 v0=1 | PORTED | 0x190→16658 not this cut |
| BTL-173F4 | 7w op 0xA `*arg0=*arg1` v0=1 | PORTED | live actor+0xF0=A77F0[8] |
| BTL-17E20 | 18w op 0x1D cond CE00 | PORTED | live 0==0 no jump |
| BTL-12850 | 244w 24-entry ALU; 3708C/370A8 | PORTED | live +0x138 slt b<a |
| BTL-1731C | 16w op 0x05 skip-if-false v0=1 | PORTED | live skip +0x170 |
| BTL-17588 | 76w op 0x14 label ptr; live +0x19C=base+0xD08 | PORTED | code 2 |
| BTL-15DAC-190 | 34w search + 15w match; B0DFC=ov+0x2E838 | PORTED | live key 0x28 row2 |
| BTL-17D7C | 8w op 0x40 `D2E8\|=1`; 3999C skip setter | PORTED | live type1 +0x040 |
| BTL-16910-2900 | 314w 0xED; live 2900 `B0CD8\|=0x400000` | PORTED | other keys not this cut |
| BTL-1A374 | 7w 0xE1 sb → BCFFC | PORTED | live 0x54 |
| BTL-18E84 | 12w 0x84 sh D020/D022 | PORTED | live 0x800,0x800 |
| BTL-18F54 | 8w 0x88 BCFEE&=~0x40 | PORTED | argc 0 |
| BTL-1735C | 38w 0x08; 35038(D2F0,1); +0x28/2C/30; jal 1AA78; v0=1 | PORTED | live type3/0/5; desc at host 0x80120F70 APPROXIMATION |
| BTL-1AA78 | 154w; +0x98&0x80 / D1FC+2 / D1D8 arms; jal 1C614/3708C | PORTED | empty +0x1AC no-ops via 0x80 |
| BTL-1C614 | 114w 3-edge crossing; v0=t0 | PORTED | live D1D8!=0 when 1A918 +0x20 set |
| BTL-08-SPAWN | creation 3 then 0 then 5; D254=type0; D2E8 stays 1 | PROVEN | sibling walk after parent is 5→0→3 |
| BTL-12C20 | 151w 0x0B; 7 codes; code0 jal 1AA78 + snap +0x40 | PORTED | live type5 0 then 5 |
| BTL-17D9C | 9w 0x41 `+0x98\|=0x40` v0=1 | PORTED | live type5 after 0x0B |
| BTL-17AE8 | 19w 0x2E jal 1A680; +0x98&=~0x100 | PORTED | live cmd 0 |
| BTL-17EC4 | 14w 0x4E +0x14=min(+0x0F,imm)<<16 | PORTED | live imm 0 |
| BTL-17B34 | 16w 0x2F +0x12=min; +0x98\|=0x200 | PORTED | live imm 0 |
| BTL-17B74 | 16w 0x30 v0=0; unequal rewinds CE00 | PORTED | live equal yield |
| BTL-14694 | 147w 0x5E D254/walk pose-copy; miss -1 | PORTED | live type3 code 0 type 0 |
| BTL-14DA0 | 36w 0x77 copies 4 pairs; jal 1CAB0 n=4 | PORTED | live miss on zero pose |
| BTL-1CAB0 | 60w sra16 edge-cross; v0=toggle | PORTED | same family as 1C614 |
| BTL-15240 | 239w 0x9B; 794C4+empty dest chain; v0=1 on bit 0x10000000 | PORTED | `pe_btl27_15240_oracle.py` sha `fb261fcb…` |
| BTL-39B74 | 108w; a1==0 early-out; live +0x1B0=0 | PORTED | do not force clip |
| BTL-362B8 | 79w size-class bank; 35038 only if +0x1AC!=0 | PORTED | sha `c1d94293…` |
| BTL-3A6A8-E | dest+0==0 jals 3E188; dest+0x24==0 → KSEG0 0x84 | PORTED | named 3E188 cut; no NULL skip |
| BTL-T0-9B | type0 0x9B→0xED 0xA29→0x14×2→0x0B…→0x02 +0x2E8 | PROVEN | chunk2+0x202C8; next 0xAA |
| BTL-T3-LOOP | double miss → +0x1E4 persist<40 → +0x3E4 0x02 / goto +0xC | PROVEN | 0x05 is base+(rel<<1) |
| BTL-12E7C | 142w 0x0C; 7 codes; read twin of 12C20; code5 lh | PORTED | live type5 +0x14C code 0 |
| BTL-1A15C | 19w 0xD9; `*arg2 = 79FB4(*arg0,*arg1)`; v0=1 | PORTED | live cond[0],cond[1]→local[1] |
| BTL-79FB4 | 93w signed ratan2; table D_8009A6EC; both-zero=0 | PORTED | host skips retail div-break |
| BTL-T5-0C | type5 scratch-miss +0x128: 0x5E/0x0C/0x09×2/0xD9 | PROVEN | zero poses → ratan2(0,0)=0 |
| BTL-1784C | 12w 0x24; *arg0=D300+0x18; *arg1=D300+0x1C; v0=1 | PORTED | live type5 local[2]/[3] |
| BTL-T5-24 | both 0xD9 arms reach +0x2C4 0x24; live cone takes 0x20 first | PROVEN | 12700 does not write +0x18/+0x1C |
| BTL-130B4 | 77w 0x11; codes 0/1/2 mask-eq D26C/D1F4/D1E4; v0=1 | PORTED | code 3 COP2 not this cut |
| BTL-T5-11 | type5 +0x2FC code 1 mask 0x100; live D1F4=0 → +0x6F0 0x20 | PROVEN | do not invent pad; 0x0D is hit-only |
| BTL-18EE0 | 11w 0x86; jal 66C7C(lhu *arg0); v0=1 | PORTED | live type1 persist!=39 imm 0x1E |
| BTL-66C7C | 27w; CFEE=6; zero CFE8/EA/EC; CFF6=a0; snap→CFF0/F2/F4 | PORTED | v0=0; 0x85/66B60 is the HIT twin |
| BTL-T1-86 | persist==39 skips 0x86 via 0x00; live 0 takes 0x86 then 0x1C/0x02 | PROVEN | do not invert; 0xAA stays skipped |
| BTL-17988 | 28w 0x04; walk +0xA0[0..2]; +8\|=0x10 except D300 | PORTED | type1 0x1C 0xFF → 65400 → type0 0x1F |
| BTL-19618 | 8w 0xAA; D_800B0CD8 \|= 0x2000; v0=1 | PORTED | persist!=39 takes +0x31C; +0x68C is mailbox copy |
| BTL-1856C | 11w 0x65; zero D2F0 +0x68/6C/70 and +0x78/7C/80; v0=1 | PORTED | type0 after 0xAA/0x40 |
| BTL-18E58 | 11w 0x82; jal 66800(*arg0); v0=1 | PORTED | live imm 1; 66800 99w view apply |
| BTL-19410 | 16w 0x9C; wait while (CFEE&3)>=2; CE00-=8 | PORTED | live after 0x86 CFEE=6 |
| BTL-19638 | 8w 0xAB; D_800B0CD8 &= ~0x2000; v0=1 | PORTED | after 0x9C; then +0x608 mailbox poll |
| BTL-68E24 | 202w fade tick; CFF8++; CFEE=0 when CFF8>=CFF6 and CFEE&4 | PORTED | 3F3C4 @ 3F588; 6E9A0 @ 6EB4C |
| BTL-3F3C4 | named cut: 65400 + 35558 + 68E24 if B0CD8&0x300==0 | PORTED | 1220C sets D1C4=D280; 0x2000 is not the gate |
| BTL-19658 | 9w 0x1E; D2F0+0x98 \|= 0x80; v0=1 | PORTED | type2 after mailbox 0xB |
| BTL-18BEC | 9w 0x79; D2F0+0x98 \|= 0x20; v0=1 | PORTED | type2 after 0x1E; then 0x0B pose |
| BTL-18EB4 | 11w 0x85; jal 66B60(lhu *arg0); v0=1 | PORTED | type3 HIT imm 0x1E; do not invent hit |
| BTL-66B60 | 30w; CFEE=CFEF=2; CFE8/EA/EC=0xFF; CFF6=a0 | PORTED | boot 6E9A0(2); 68E24 needs 6A8D4 B0E38 |
| BTL-1A1F0 | 9w 0xDC; D2F0+0x98 \|= 0x01000000; v0=1 | PORTED | type2 after 0xFB 0x0B pair |
| BTL-176FC | 26w 0x1A; *arg0=70D6C or 70DD0(*arg1,*arg2) | PORTED | live 70DD0(0,0x64)→local[0x18] |
| BTL-18954 | 10w 0x6F; jal 2F7D8(*D2F0); v0=1 | PORTED | type2 after 0x1A; may record 1A680(actor,2) |
| BTL-18164 | 26w 0x5A; type0→2FF78 else 30220(D2F0,tag,val) | PORTED | live type2 uses 30220 |
| BTL-18A48 | 21w 0xB7; jal 2FAA4(D2F0,lbu×4,lhu); v0=1 | PORTED | live (3,0,6,7,cond[1]) |
| BTL-1897C | 51w 0x70; jal 2FA10 11 unpacked args; v0=1 | PORTED | live (0,0,8,9,cond[1],5,-1×3,3,15) |
| BTL-18004 | 31w 0x59; type0→2FE78 else 3010C; *arg1=v0 | PORTED | live type2 tag 44 → local[0xC] |
| BTL-2FE78 | 64w; *(*D254) tagged read; OOB -1000 | PORTED | type0 path; extra deref vs 2FF78 |
| BTL-3010C | 69w; slot=*actor; tag-41 JT; tag44 clamp +0x10 | PORTED | live type2; tag 130 destructive |
| BTL-131E8 | 70w 0x12; fork PC=base+imm<<1; &3→+0x24 else A8 | PORTED | live type2 three forks then 0x6A |
| next_live_va | type2 0x6A/6F39C; forks 0x4B/0x54; type6 0x12; type3 0x85 | RESEARCH_REQUIRED | 6F39C tables / scratch&4 / hit |
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
| `371B0` / DrawSync / mode=6 is M2 | EXE-resident window init + GPU sync; no battle tick |
| `35558` D20C walk is M2 | Field tick `3F3C4` actor jalr; 29 later jals unported |
| Force dest+0x24 / +0x1B0 on empty +0x1AC | 35038 zeros both; 39B74 early-out and 3E188 KUSEG 0x84 are authentic |
| Type-3 wait loop is M5 | Recurring 17018, not a battle-specific scheduler |

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
python3 pc_port/tools/pe_btl9_371b0_oracle.py
python3 pc_port/tools/pe_btl10_35038_oracle.py
python3 pc_port/tools/pe_btl11_35558_oracle.py
python3 pc_port/tools/pe_btl12_17018_oracle.py
python3 pc_port/tools/pe_btl13_35c84_oracle.py
python3 pc_port/tools/pe_btl14_m0005i_publish_oracle.py
python3 pc_port/tools/pe_btl15_15dac_nop_oracle.py
python3 pc_port/tools/pe_btl16_12850_oracle.py
python3 pc_port/tools/pe_btl17_17588_oracle.py
python3 pc_port/tools/pe_btl18_15dac_190_oracle.py
python3 pc_port/tools/pe_btl19_17d7c_oracle.py
python3 pc_port/tools/pe_btl20_16910_oracle.py
python3 pc_port/tools/pe_btl21_e1_84_88_oracle.py
python3 pc_port/tools/pe_btl6_20efc_oracle.py
python3 pc_port/tools/pe_btl6_29854_oracle.py
python3 pc_port/tools/pe_btl6_29810_oracle.py
python3 pc_port/tools/pe_btl6_209f0_oracle.py
python3 pc_port/tools/pe_btl6_339a0_oracle.py
python3 pc_port/tools/pe_btl22_1735c_1aa78_oracle.py
python3 pc_port/tools/pe_btl23_12c20_oracle.py
python3 pc_port/tools/pe_btl24_type5_2e_oracle.py
python3 pc_port/tools/pe_btl25_14694_oracle.py
python3 pc_port/tools/pe_btl26_14da0_oracle.py
python3 pc_port/tools/pe_btl27_15240_oracle.py
python3 pc_port/tools/pe_btl28_12e7c_d9_oracle.py
python3 pc_port/tools/pe_btl29_1784c_oracle.py
python3 pc_port/tools/pe_btl30_130b4_oracle.py
python3 pc_port/tools/pe_btl31_18ee0_oracle.py
python3 pc_port/tools/pe_btl32_17988_oracle.py
python3 pc_port/tools/pe_btl33_19618_oracle.py
python3 pc_port/tools/pe_btl34_1856c_oracle.py
python3 pc_port/tools/pe_btl35_18e58_oracle.py
python3 pc_port/tools/pe_btl36_19410_oracle.py
python3 pc_port/tools/pe_btl37_19638_oracle.py
python3 pc_port/tools/pe_btl38_68e24_oracle.py
python3 pc_port/tools/pe_btl39_19658_oracle.py
python3 pc_port/tools/pe_btl40_18bec_oracle.py
python3 pc_port/tools/pe_btl41_18eb4_oracle.py
python3 pc_port/tools/pe_btl42_1a1f0_oracle.py
python3 pc_port/tools/pe_btl43_176fc_oracle.py
python3 pc_port/tools/pe_btl44_18954_18164_oracle.py
python3 pc_port/tools/pe_btl45_18a48_1897c_oracle.py
python3 pc_port/tools/pe_btl46_18004_oracle.py
python3 pc_port/tools/pe_btl47_131e8_oracle.py
./pc_port/build/pe-native-tests   # matching_native=773/773
```

STOP/NEXT: Type-2 `0xFB` arm is ported through
`0x12`. Next is `0x6A` / `6F39C(0x75)`. Type-3
`0x85` stays hit-gated. Type-6 `0x12` waits
on scratch[0]&4. Do not invent pad /
persist==39 / scratch / hit. `D2E8` bit 0 stays
set.
