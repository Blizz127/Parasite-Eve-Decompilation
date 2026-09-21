# func_8004AE1C

- **VRAM**: 0x8004AE1C
- **File offset**: 0x3B61C (size 0x120)
- **Build profile**: era_o2_g0_dispatch_8011034
  (`-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1` +
  `MASPSX_DISPATCH_FOLD=jtbl_80011034`) — new profile registered for this leaf.
- **Status**: landed (wave-6 slice B, agent/wave6-b)

## Behaviour
Save/load file-menu page input handler (reached from the 0x20 window). On the
confirm bit 0x10000 it asks `func_80063428(list)` for the selected row and, for
`selected < 6`, dispatches through the shared `jtbl_80011034`: 0..3 open the
0x21/0x23/0x2E/0x38 sub-pages, 4 and 5 share the status block
(`func_8005D994(sel-4)`, `func_80062F1C`, `func_800439D8`, `func_800525EC`).
The shared tail issues `func_800525EC` again, so 4/5 run it twice. On the cancel
bit 0x40 it runs `func_80062F1C`/`func_800439D8`/`func_80052634`. Always
returns 1. Written from the retail assembly; the pc_port transcription was used
only as a specification.

## Method
A `switch` with **explicit `case 4:` / `case 5:` labels sharing the default
body** is required — with only `default:` cc1 (4 sparse cases) emits a compare
chain instead of a jump table. With the explicit cases it emits the table; the
indexed table load is the compound `lw $r,$L($b)` form, which maspsx only
expands to retail's `lui $at,%hi(jtbl) / addu $at,$at,$v0 / lw $v0,%lo(jtbl)($at)`
with **both** `MASPSX_THREE_WORD_SYMBOL_STORE=1` and
`MASPSX_DISPATCH_FOLD=jtbl_80011034`.

## Evidence
try_leaf `WORDS MATCH`; fresh complete build EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b, `Matching claim: YES (891
registered C leaves)`, `VERIFY_US=PASS`.
