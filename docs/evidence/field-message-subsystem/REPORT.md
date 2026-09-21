# Field-message subsystem — opcode `0xE9` closure

Status: opcode `0xE9` is **ported**. The handler `func_80015C7C` is a full
translation, and `func_8005D2B4`'s five route-reachable arms are real ports.
This supersedes the "explicit boundary" disposition in
`docs/evidence/field-vm-opcode-e9/SCOPE.md`.

Authority: SHA-1-exact retail EXE `build/extracted/disc1/SLUS_006.62`
(sha1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`). Function bodies were read
from `asm/disc1/` where the function is split; `func_8004BF08` is not in the
split and was decoded directly from the EXE.

## 1. Why the old disposition was wrong

`SCOPE.md` concluded that `func_8004BCE8` "walks the whole field-message state"
and therefore pulled in a large unported subsystem. Reading the actual body
(`asm/disc1/3BD84.s:548`) shows the opposite: every callee of
`func_8004BCE8` already has a native port in `pc_port/`:

| callee | native port |
| --- | --- |
| `func_80051510` | `game/boot/func_80051980_port.c` |
| `func_8005B91C` | `game/boot/func_8005B91C_port.c` |
| `func_80057E14` | `game/boot/field_message_port.c` |
| `func_8004BE4C` | `game/boot/field_message_port.c` |
| `func_80062A34` / `func_80063158` | `func_80062D2C_port.c` / `func_80055760_port.c` |
| `func_8005270C` | `game/boot/field_message_port.c` |
| `func_8005C144` | `game/boot/field_message_port.c` |

There was never a missing block; the closure was already ported. The one real
missing piece was `func_8005D2B4` itself, whose arms `0x453/0x454/0x456/0x457/
0x45E` are the only ones the route uses.

## 2. What was ported this session

`pc_port/game/boot/field_message_port.c` (new file), translated from the EXE
and `asm/disc1/`:

| function | source | notes |
| --- | --- | --- |
| `func_800515C0` | `41D10.s:63` | record `+0xC` u16 setter |
| `func_80051684` | `src/func_80051684.c` | already a matched C leaf; the port mirrors it |
| `func_800629B0` | EXE | `D_8009D154 != 0` |
| `func_80057E14` | `48530.s:77` | window-id remap table |
| `func_8005270C` | `42D94.s:137` | fade target via `func_8006DF50` |
| `func_8004BF08` | EXE `0x8004BF08` | 8-word two-table clear (not in the split) |
| `func_8004BE4C` | `3BD84.s:647` | "item get" window node pair |
| `func_8005C144` | `src/func_8005C144.c` | encounter-style snapshot (**`void`**) |
| `func_8004BCE8` | `3BD84.s:548` | close/commit the active window |
| `func_8005D2B4` | `4CC98.s:1024` | 21-arm dispatcher, 5 arms real |
| `func_80015C7C` | `3420.s:3510` | opcode `0xE9` handler |

`func_80051510` was **already** ported in `func_80051980_port.c` and is called,
not redefined.

### 2.1 `func_8005D2B4` arm status

Jump table `jtbl_800112DC` decoded from the EXE; indexed by `cmd - 0x44C`, 21
valid entries.

| arm | cmd | status |
| --- | --- | --- |
| 0x07 | `0x453` | ported — returns `D_800C0E06` (lhu) |
| 0x08 | `0x454` | ported — `func_800515C0(a)` |
| 0x0A | `0x456` | ported — `*out = func_800515F8(out)` |
| 0x0B | `0x457` | ported — `func_80051684(a)` |
| 0x12 | `0x45E` | ported — `func_8004BCE8(a)` |
| all others (16) | | **explicit domain guard** |

The five ported arms are exactly the `cmd` constants the retail `m0351i`
module 1 uses (`0x453/0x454/0x456/0x457/0x45E`). Every other arm reaches a
subsystem that is not translated. Those arms call `gm_arm_guard(index, cmd)`:
one-shot, loud (`[MSG] func_8005D2B4 arm cmd=0x... reaches an unported
field-message subsystem`), and returning retail's default-miss value `0`. They
are pinned by `test_SEW26_arm_domain_guard` so they cannot silently become
"implemented". `cmd < 0x44C` and `cmd > 0x460` reproduce retail's
`sltiu 0x15` miss and also return `0`.

### 2.2 Static proof that the guards are never reached on-route

Decoding every critical room's script bytes (`m0351i` is the only one that uses
`0xE9`; five sites, module 1) gives these `cmd` values, read from operand 0:

| module offset | argc | modes | args | `cmd` |
| --- | ---: | --- | --- | --- |
| 312 | 5 | `[0,0,0,1,1]` | `[1107,0,0,9,8]` | `0x453` |
| 340 | 5 | `[0,1,0,1,1]` | `[1108,9,0,8,8]` | `0x454` |
| 368 | 5 | `[0,0,0,1,1]` | `[1110,0,0,11,10]` | `0x456` |
| 396 | 5 | `[0,1,0,1,1]` | `[1111,11,0,10,10]` | `0x457` |
| 7072 | 5 | `[0,2,2,2,2]` | `[1118,311,311,311,311]` | `0x45E` |

All five are ported arms. So on the boot → Day-2 route (and in the park rooms,
which do not use `0xE9` at all) **no domain guard is reachable**. The guards
exist for scripts outside this census.

The operand layout also confirms the handler port: operand 3 (mode 1) is the
`out` cell at `args + 0xC`, which matches `func_80015C7C`'s
`lw $v1,0xC($s0)` / `sw $v0,0($v1)`. Retail's `argc` is 5; operand 4 is unused
by the handler, so the native test's `argc = 4` exercises the same path.

## 3. Durable levers found (all cost real bugs this session)

1. **`$gp` offsets must be read from the instruction immediate, not inferred
   from a nearby disassembler annotation.** `gp` is `0x8009CD70`, so
   `sw N($gp)` targets `0x8009CD70 + N`. Guessing produced four wrong guest
   globals (`0x8009D204/0x8009D214/0x8009D050/0x8009D058` instead of
   `0x8009CF74/0x8009CF84/0x8009D01C/0x8009D04C`). The wrong targets are
   silently writable, so only the value assertions caught them.
2. **`jalr` with a saved `$s0` (`func_8005D2B4`) swaps the argument order.**
   Retail prologue: `addu $s0,$a1,$0` / `addu $a1,$a2,$0` /
   `addiu $a0,$a0,-0x44C`. So the dispatcher receives `(cmd, a, b)` with
   `a = retail *arg1` first and `b = retail *arg2` second.
3. **`func_800515F8` is the real PE-cost leaf and is not the
   `PE_MenuPEValues` shape.** Retail computes
   `value = (s16)record[+0xA]`, then for each 0x24-byte unit from
   `0x800A1AA0` up to `D_8009D014` whose word 0 is `1`, subtracts
   `unit[+8]`; the out word is `(s16)record[+0x2A]`. The earlier native port
   read `record+0xA`/`record+0x2A` through the wrong globals and derived the
   limit from `PE_MenuPEValues`; both are now aligned to retail.
4. **The first dispatcher arm (index 0, `cmd 0x44C`) diverges from the
   reported arm.** An earlier session reported `func_8004BF08(value)` for the
   `0x451` arm; the EXE shows `0x451` is `.L8005D46C` (a clamp/table rebuild)
   and `func_8004BF08` actually takes no argument. Its sole call site is the
   `func_8004BE4C` tail, which passes nothing. The port stays no-argument.
5. **`func_8005C144` returns `void`, not `unsigned int`.** Retail's epilogue is
   a bare `jr $ra` (no `$v0`), and the matched C leaf
   `src/func_8005C144.c` agrees.
6. **KUSEG→KSEG0 aliasing must be preserved when a caller passes a low
   physical address.** The `INV16` oracle fixture hands `func_800515F8` the
   KUSEG alias `0x0015FA40`; the host RAM model only maps KSEG0 plus
   scratchpad, so the out-store substitutes `| 0x80000000` for addresses
   below `0x200000`. On hardware both aliases hit the same bytes; this is
   address translation, not a behavior change.

## 4. Non-claims

- **XA audio is still not decoded.** Nothing here touches XA.
- **Pixels are not hardware-exact.** The subsystem ported here is message
  state, not a renderer; the window geometry words are published but the
  rasterizer remains approximate.
- **No full end-to-end boot → Day-2 emulator run exists.** Correctness here
  is per-function against the retail bytes plus native unit tests, not a run
  of the retail executable to the Day-2 marker.
- **16 of `func_8005D2B4`'s 21 arms are unported.** They are loud guards, not
  implementations. They are not on the boot → Day-2 route (the coverage census
  shows `m0351i` only uses the five ported `cmd`s), but any *other* script that
  calls `0xE9` with one of those `cmd`s will hit a guard rather than retail
  behavior.

## 5. Evidence / reproduction

- Arm mapping: `jtbl_800112DC` at `0x800112DC` in the retail EXE.
- Bodies: `asm/disc1/3BD84.s:548` (`func_8004BCE8`),
  `asm/disc1/4CC98.s:1024` (`func_8005D2B4`),
  `asm/disc1/3420.s:3510` (`func_80015C7C`),
  `asm/disc1/48530.s:77` (`func_80057E14`),
  `asm/disc1/42D94.s:137` (`func_8005270C`),
  `asm/disc1/41D10.s:63,83` (`func_800515C0`, `func_800515F8`),
  and the EXE for `func_8004BF08`.
- Matched leaves reused: `src/func_80051684.c`, `src/func_8005C144.c`.
- Coverage census: `python3 pc_port/tools/pe_field_vm_opcode_coverage.py`
  (needs `build/disc1.candidate.exe`, symlink the extracted EXE).
  Output: all nine critical rooms report `unported=[]`.
- Tests: `test_SEW23_opcode_E9_message_query`, `test_SEW24_arm_453_454_457`,
  `test_SEW25_arm_45E_close_window`, `test_SEW26_arm_domain_guard`.
