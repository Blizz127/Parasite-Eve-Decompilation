# Field-VM opcode `0xE9` — arm mapping and the remaining boundary

> **SUPERSEDED (same day).** `0xE9` is now **ported**. The disposition below
> ("`0xE9` remains an explicit boundary") no longer holds: `func_8005D2B4`'s
> five route-reachable arms and `func_8004BCE8`'s closure are implemented in
> `pc_port/game/boot/field_message_port.c`, and the coverage census reports
> `unported=[]` for every critical room. This file is retained as the record of
> the reasoning that led there. See
> `docs/evidence/field-message-subsystem/REPORT.md`.

Status of the single remaining unported opcode on the boot → end-of-Day-2
transition-critical scripts, after `0x97`, `0x9A` and `0x71` were ported
(`docs/evidence/field-vm-opcode-coverage/REPORT.md`).

Re-run `python3 pc_port/tools/pe_field_vm_opcode_coverage.py` to reproduce the
census: the only remaining `unported` entry is opcode `E9` (handler
`0x80015C7C`, 5 uses, all in `m0351i` module 1).

## 1. This is a *blocked* boundary, not a missing leaf

`func_80015C7C` itself is small (0x130 bytes, 76 words). It is not ported here
because its two reachable callees are unported subsystems:

```
0x80015C7C:  if ((*(gp+0x590)).u16[4] & 0x20) == 0        ; task flag 0x20 clear
                 *arg3 = func_8005D2B4(*arg0, *arg1, *arg2)   ; message dispatch
             if (func_800629B0() != 0) {                   ; "message queued" flag
                 if (task.u16[4] & 0x20) == 0 {            ; not already flagged
                     D_800B0CD8 |= 0x9000
                     task.u16[4] |= 0x20
                     func_80067CBC()
                 } else {
                     D_8009D1A0 |= 4
                 }
                 D_8009CE00 -= 0x1C ; task+0x10 = 1 ; return 0
             }
             if ((int16)D_8009D2A4 != 0) {
                 *arg3 = D_8009D2A4 ; task.u16[4] &= ~0x20 }
             return 1
```

`func_800629B0` is already matched in `src/` (`D_8009D154 != 0`) and is
reproducible in the port as `PE_LoadU32(0x8009D154) != 0`. `func_80067CBC` is
already ported. So the only real blocker is `func_8005D2B4`.

## 2. Why `func_8005D2B4` cannot be stubbed

`func_8005D2B4(cmd, a, b)` (`0x8005D2B4`, 0x440 bytes) is a 21-slot jump table
(`jtbl_800112DC`, indexed by `cmd - 0x44C`; commands `0x44C..0x460`). It is a
pure *read* dispatcher: each arm reads a message-system global and returns an
`int` that the `0xE9` handler stores into the caller's `*arg3`. Stubbing it to
`return 0` would be wrong for the game (the `0x453/0x454/0x456/0x457` queries
are menu cursor/selection reads, one of which gates the `persist[1]=0x88`
write just above it at `0x80191114`).

### 2.1 What the five `m0351i` uses actually ask for

The five call sites (`m0351i` module 1) and their `cmd` constants:

| PC | cmd | arm | reads | port status |
| --- | --- | --- | --- | --- |
| `0x8018F5D0` | `0x453` | `.L8005D4C0` | `D_800C0E06` (`lhu`) → `*arg3` | global not modelled |
| `0x8018F5EC` | `0x454` | `.L8005D4D0` | `func_800515C0(*arg1)` | callee unported (leaf) |
| `0x8018F608` | `0x456` | `.L8005D4F0` | `func_800515F8(sp+0x10)` → `*arg3` | **ported** (`func_800515F8_port.c`) |
| `0x8018F624` | `0x457` | `.L8005D504` | `func_80051684(*arg1)` | callee unported (leaf) |
| `0x80191038` | `0x45E` | `.L8005D668` | `func_8004BCE8(*arg1)` | callee unported (subsystem) |

`func_800515C0` (`0x800515C0`, 14 words) writes `*arg0` (u16) into
`(*D_8009D254)+0xC` when live and into `D_800C0E08`; `func_80051684`
(`0x80051684`, 12 words) writes `(*arg0) << 16` into `(*D_8009D254)+8`. Both are
small leaves whose *state* (`D_8009D254` field object, `D_800C0E08`) drives the
field message renderer, so they too are semantic rather than inert.

The `0x45E` arm runs `func_8004BCE8(*arg1)` (`0x8004BCE8`, 0x250 bytes). That is
a window/message box closer that walks the whole field-message state
(`func_80051510`, `D_800C0E00`, `func_8005B91C`, `func_80057E14`,
`func_8004BE4C`, `func_80062A34(1,0x15)`, `func_80063158`, `func_8005270C`,
`func_8005C144`) — i.e. the full retail message subsystem, most of which has no
native port.

So the `0xE9` boundary is the *field message subsystem*, not a small opcode
leaf. Porting it means porting `func_8005D2B4` + `func_800515C0` +
`func_80051684` + `func_8004BCE8` + their closures.

## 3. Disposition

- **`0xE9` remains an explicit `PE_PORT_STOP_UNRESOLVED_BOUNDARY`** in the port.
  That is a *typed* stop that names the exact opcode PC; it is not a silent
  yield, and there is an existing regression test for the boundary behaviour
  (`test_SEW1_unported_opcode_is_explicit_boundary`).
- The three small leaves (`func_8005D2B4` arms `0x453/0x454/0x456/0x457`) could
  be ported in isolation, but the `m0351i` block that uses `0xE9` sits inside
  the Day-1/Day-2 hop room's *story-select* region, and the `0x45E` arm alone
  pulls in `func_8004BCE8`. Porting a subset would let the block run with a
  partly-stubbed message system and produce unverified writes — worse than the
  explicit boundary, per the no-invented-C / no-overclaim rules.
- **Recommended next step** (when the message subsystem lane opens): port
  `func_8005D2B4` with the `0x453/0x454/0x456/0x457` arms first, keep the other
  arms as explicit domain guards, then close `0x45E` once `func_8004BCE8` and
  `func_80051510` land. `func_800629B0` is one line and can be added at the
  same time.

## 4. What is proven here

- The handler's two callees and their roles are read from the SHA-1-exact EXE
  (`build/extracted/disc1/SLUS_006.62`, sha1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`) — see `asm/disc1/3420.s:3510`,
  `asm/disc1/4CC98.s:1024`, `asm/disc1/41D10.s:63,83,127`,
  `asm/disc1/3BD84.s:550`.
- The jump-table arm mapping is decoded from `jtbl_800112DC` at
  `0x800112DC` (`data/800.rodata.s`).
- The five `m0351i` call sites and their operands are decoded from the retail
  package (`m0351i`, script sha256
  `1cc11664e59100e6378107b99ade037704e1917702182264e459c7d1172b7203`).

This note is a *negative* result: it states precisely why the last opcode is
not closed, rather than claiming it.
