# Story `0x48` frontier: the gate is `persist[24] & 0x20`, set by an m0020i item pickup

Worktree `/tmp/pe-agent-filemenu`, branch `agent/file-menu`.
Retail authority: `build/extracted/disc1/SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Summary

* **What `m0012i` needs.** `m0012i`'s chapter-end cutscene — the field's
  script block at guest `0x801A1964` — is gated on **`persist[24] & 0x20`**
  (bit 5). When that bit is set the cutscene runs, writes `persist[1] = 12`,
  and issues the field's transfer to **`m0018i`** (token `0xA8001348`) at
  guest `0x801A1B4C` / `0x801A1E70`. It sets `persist[24] |= 0x200000`
  (bit 21) as a one-shot "already played" marker.
* **What `m0020i` needs.** The only writer of `persist[24] bit 5` on Disc 1
  is **`m0020i`, script offset `0x370`** (runtime `0x801A0BE8`): after the
  `opA7`/`opE8` item-`0xC8` pickup it executes `persist[24] |= 0x20`.
* **Where the next story beat lives.** `m0018i` is the **only** field on the
  disc whose script contains the `persist[74] < 0x54` comparisons (2 sites,
  script offsets `0x390` and `0x8f0`) — i.e. the beat *after* story `0x48`.
* **What actually blocks the route.** The recorded `--route-pad` sequence
  visits `m0020i` (f≈20500) and leaves through the east door the m0020i report
  documented, **without** satisfying the `Aya.local[4] == 3` precondition of
  the item-`0xC8` pickup. `persist[24]` therefore ends the run at
  `0x01000000` (bit 24 only), never `& 0x20`, and `m0012i`'s cutscene never
  fires. The route's recorded pad overrides also release everything to
  `0xFFFF` at frame 45036, so Aya stops walking at the frontier.
* **Port fix landed.** Script opcode `0xF0` (`D_800910A0[0xF0] ==
  0x80016FE0`, matched `src/func_80016FE0.c`) had **no handler** in the port's
  task-VM dispatch and fell through to
  `PE_PORT_STOP_UNRESOLVED_BOUNDARY`. It is now ported faithfully with a
  passing native regression test.

## Evidence

### 1. `persist[74]` really reaches `0x48`, in `m0012i`

A VM argument-address trace over the `--route-pad` run (every script op that
receives the address of `persist[74]`, `0x800A7918`) shows the story chain

```
0x00 -> 0x09 -> 0x11 -> 0x12 -> 0x18 -> 0x26 -> 0x27 -> 0x28 -> 0x30
     -> 0x38 -> 0x39 -> 0x40 -> 0x48
```

with the final write at **`opc=0x801A1284` (`op 0x0A`, `P[74] = 0x48`)** while
token `A8001148` (`m0012i`) is current. After that the field polls
`0x801A34DC` (`persist[74] < 0x39`), `0x801A0C4C` and `0x801A11BC`
(`persist[74] == 0x40`) — all of which are now false — and idles.

### 2. No script branches on `0x48`

Every `op 0x09` expression comparison on the whole Disc 1 image whose argument
is `D_800A77F0 + 74*4` was decoded (`expr`, `rhs`). The value `0x48` appears as
the right-hand side **nowhere**; the band containing `0x48` is bounded by
`persist[74] < 0x54`. Extracting and decoding every field script in the field
table (`tools/research/pe_pst0_scan.py`) locates both `< 0x54` sites in a
single field: **`m0018i`** (script offsets `0x390`, `0x8f0`).

### 3. `m0012i` -> `m0018i` transfer and its guard

`m0012i`'s decoded module 0, runtime addresses (script offset + `0x801A0878`):

```
801A1964 op09 sub(expr 3 = AND)  cnd0 = persist[24] & 0x20
801A197C op05                    skip -> 0x801A11DC when cnd0 == 0
801A198C op2E 22 ; opB8 camera move
801A19B8 op09 sub(expr 3 = AND)  cnd0 = persist[24] & 0x200000
801A19D0 op09 sub(expr 7 = NOT)  cnd1 = !cnd0
801A19E8 op05                    skip -> 0x801A1156 when cnd1 != 0
801A19F8 opEA fade ; op0D ; op22
801A1A30 op09 sub(expr 2 = OR)   cnd0 = persist[24] | 0x200000
801A1A48 op0A                    persist[24] = cnd0          ; one-shot marker
801A1A58 opED 2100               sound
801A1A80 opEA fade ; op7B ; op75 ; op85 ; opB8/op44 camera
801A1B3C op0A                    persist[1] = 12
801A1B4C op31                    transfer -> 0xA8001348
```

The second op31 with the same target (`0x801A1E70` in the RAM dump) is the
duplicate of the same statement.

### 4. `m0018i`'s own token

Cross-referencing tokens per field: `m0012i` references
`{0xA8001248, 0xA8001348, 0xA8001448, 0xA8002048, 0xA8002148}`; `m0018i`
references exactly `{0xA8001148}` and `m0020i` references exactly
`{0xA8001148}`. `0xA8002048` is `m0020i`'s own token, so the only field that
links back to `m0012i` besides `m0020i` is `m0018i`, making `0xA8001348`
`m0018i`. The m0018i script also holds the `persist[74] < 0x54` beats.

### 5. `persist[24]` on the live route

VM trace of every op receiving `0x800A7850` (`persist[24]`) or `0x800A7918`:

```
opc=801B1A88  p24: 00000000 -> 00000001   (token A80002C8, story 0x28)
opc=80192708  p24: 00000001 -> 00000001
opc=801A0200  p24: 00000000 -> 01000000   (token A80010C8, story 0x30)
... persists at 01000000 through story 0x48 (token A8001148)
```

`persist[24] & 0x20` is never set. The only `persist[24] |= 0x20` on the disc:

```
m0020i  script offset 0x370  (runtime 0x801A0BE8)
801A0BA4 opA7 item 0xC8 -> Aya.local[6]
801A0BB4 op09 sub(expr 11 = EQ)  cnd0 = (Aya.local[6] == 0)
801A0BCC op05                    skip -> +446 when cnd0 == 0
801A0BDC opE8 0xC8               pickup dialog
801A0BE8 op09 sub(expr 2 = OR)   cnd0 = persist[24] | 0x20
801A0C00 op0A                    persist[24] = cnd0
```

This is the same block the earlier `pe-m0020i-gate` report left open under the
`Aya.local[4] == 3` precondition (its `0x801A0AE0` / `0x801A0AF8` guard
immediately precedes this block).

### 6. Route pads

`kDay1RoutePads` ends at `45036:FFFF` (`FFFF` = no button held, active-low),
and `RoutePadSource` overlays the recorded mask on the stage table, so from
frame 45036 the port holds neutral plus the 8-frame Cross autopilot pulse.

## Port change

`pc_port/game/boot/func_80017018_port.c`

* new `func_80016FE0(pe_addr_t args)` — `*dst = (D_8009D2E8 & 1) ? 0 : 1`,
  `return 1`, matching `src/func_80016FE0.c`;
* `if (fn == 0x80016FE0u) return func_80016FE0(args);` in
  `pe_17018_dispatch`.

Retail encoding observed in the m0012i bank at `0x801A6460` (word
`0x000220F0`: op `0xF0`, argc 1, kind 1 -> `actor+0xAC`) matches the test.

`pc_port/tests/test_native.c`

* `test_VM_opcode_F0_D8009D2E8_flag` — drives `func_80017018` over a real
  task whose first instruction is `0x000220F0`, asserting both branches
  (`D_8009D2E8 = 1 -> local = 0`, `= 0 -> local = 1`) and the PC advance.

## Verification

| Check | Result |
| --- | --- |
| `cmake --build pc_port/build` | clean |
| `pe-native-tests` | 1405 run, 1355 passed, 6 failed, 44 skipped |
| the 6 failures | all pre-existing `missing local/pe_disc1.path` (worktree has no disc path file) |
| `ctest` | **11/11 passed** |
| `pe-route-boot-day2-tests` | PASS, 27 milestones, transfer `0xA8001148` at f=20655 |
| `--route-pad --max-frames 80000` | `stop_reason=frame-limit`, token `A8001148`, story `0x48`, 0 unported opcodes, 257 non-stopping `func_80076C34` GPU boundaries (identical to baseline) |

## What is still open

Crafting the pad input that satisfies `Aya.local[4] == 3` in `m0020i` so the
item-`0xC8` pickup runs. That is an input/geometry problem, not a port
fidelity problem: the game logic on both sides of the gate
(`m0020i` offset `0x370`, `m0012i` `0x801A1964`) is faithful script data and
executes correctly as written.
