# PE-CH1 — Carnegie Hall field→battle matching worklist

Ordered from in-tree evidence only. Sizes from SHA-1-exact
`build/disc1.candidate.exe` (first `jr $ra` + delay). yaml file offset =
VRAM − `0x80010000` + `0x800`. Matching `src/` carves listed when present.

Do not continue PE-B54K-B (`func_80030894`) on this path: it is a boot
GPU-primitive builder with no mailbox / `0x89` / slot-alloc site
(`docs/evidence/pe-mbx1-task-state/MAILBOX_MECHANISM.md` battle census).

## First 10 functions

| # | Symbol | Role | VRAM | Words | yaml / src | Status |
|---|---|---|---|---|---|---|
| 1 | `func_800653B8` | mailbox append | `0x800653B8` | 18 | inside `[0x55430, asm]` (`55430.s`); no `src/` | **native-ported** (`func_800653B8_port.c`); matching blocked (no era/asm) |
| 2 | `func_80017764` | opcode `0x1C` send | `0x80017764` | 18 | `[0x2A0C, asm]` file `0x7F64` (before matching `17E9C`); no `src/` | **native-ported** (`func_80017764_port.c`); sole `jal 653B8`; extra 0 |
| 3 | `func_80065400` | mailbox drain | `0x80065400` | 117 | same `55430.s` after 653B8; no `src/` | **native-ported** (`func_80065400_port.c`); jal real `12700` |
| 4 | `func_80012700` | task spawn | `0x80012700` | 29 | `[0x2A0C, asm]` file `0x2F00`; no `src/` | **native-ported this rung** (`func_80012700_port.c`); replaced 65400 hook |
| 5 | `func_800177AC` | opcode `0x1F` poll | `0x800177AC` | 7 | `[0x2A0C, asm]` file `0x7FAC`; no `src/` | **native-ported this rung** (`func_800177AC_port.c`); no ACK |
| 6 | `func_80019154` | opcode `0x94` mode read | `0x80019154` | 7 | `[0x98BC, asm]` file `0x9954` (before matching `192B8`); no `src/` | **native-ported this rung** (`func_80019154_port.c`); twin of matching `17FF0` |
| 7 | `func_8002F7D8` | opcode `0x6F` slot alloc | `0x8002F7D8` | 102 | `[0x11718, asm]` prefix before matching `2F970`; no `src/` | **native-ported this rung** (`func_8002F7D8_port.c`); 216B template; jal 1A680 unresolved |
| 8 | `func_8002FA10` | opcode `0x70` formation | `0x8002FA10` | 37 | **head of** `[0x20210, asm]` after matching `2F9CC`; no `src/` | **native-ported this rung** (`func_8002FA10_port.c`); `*actor+i*16+0x1C` / `+i*4+0x7C` |
| 9 | `func_8002FAA4` | opcode `0xB7` formation | `0x8002FAA4` | 13 | same `20210.s`; no `src/` | **native-ported this rung** (`func_8002FAA4_port.c`); 0x70 subset |
| 10 | `func_8002FF78` | opcode `0x5A` Aya tagged setter | `0x8002FF78` | 101 | `[0x20210, asm]` file `0x20778`; no `src/` | **native-ported** (`func_8002FF78_port.c`); BTL1 40–52 are `func_80030220` |
| 11 | `func_80030220` | opcode `0x5A` slot tagged setter | `0x80030220` | 197 | same `20210.s` file `0x20A20`; no `src/` | **native-ported** (`func_80030220_port.c`); JT `D_80010C90` |
| 12 | `func_800299CC_consume_cut` | BTL1 `D_8009D28C` 6→0 consume | `0x800299CC` | 16 (named cut) | inside `[0x11718, asm]` file `0x1A1CC`; exclusive `0x80029A0C`; no `src/` | **native-ported this rung** (`func_800299CC_port.c`); `sb 6` `gp+0x10C` |

## Already matching / native (not in the 10)

| Symbol | Role | Status | Evidence |
|---|---|---|---|
| `func_80017FF0` | opcode `0x89` `D_8009D28C = 6` | **matching** `src/func_80017FF0.c` | BTL1 must; FIELD_ENTRY.md |
| `func_80017FDC` / `192B8` / `192C8` | mode 5/0/8 | matching | same word |
| `func_8002F9CC` / `func_8002F970` | slot-table clear / search-and-clear | matching | proves 7×220 shape `0x6F` walks |
| `func_8006536C` | boot-clear 28×12 + count | **native** `func_8006536C_port.c`; matching-adjacent extern from `src/func_8003E680.c` | PE-MBX1 |

## Prefix camera (reach Carnegie; not battle engine)

| Symbol | Opcode | Words | Status |
|---|---|---|---|
| `func_80065954` | `0x75` via handler `0x80018B98` | 18 | **native-ported** (`func_80065954_port.c`); 18/18 oracle |
| `func_800659C8` | `0x7B` via handler `0x80018C58` | 12 | **native-ported** (`func_800659C8_port.c`); 12/12 oracle |
| `func_80066800` | `0x82` via handler `0x80018E58` | 99 | **native-ported** (`func_80066800_port.c`); 99/99 oracle + VIS1-B stores |

## Next after this rung

BTL1 TRACE integration (`docs/evidence/pe-ch1-btl1-trace/`) is complete:
mailbox 3 and mailbox 4 independently run through real
`func_80017BB4_btl1_cut` `0x31` → slots → `0x89` → consume. Real `0x1A`
records the 19-word pre-call RNG image and selects formation 49/50.
Do not start ATB. Matching `src/` still blocked (no era/asm).
Prefix camera is complete as PE-CH2: m0003i `0x7B`/`0x75`, then
m0372i/m0004i `0x82(1)`, then the existing m0005i hop. This is a static
leaf trace, not a playable field/projection claim; see
`docs/evidence/pe-ch2-prefix-camera/`.

BTL2 HP cut is native-ported (`func_800293F4_hp_cut`, 21 words):
`0x55` → `144FC` → `29810` → `293F4(0)` copies record `+0x0C` →
`+0x0E` after a signed clamp against `+0x1C`. First-command is not
on that path. NYPD `0x55` freeze still holds. Evidence:
`docs/evidence/pe-btl2-hp-layout/`.
