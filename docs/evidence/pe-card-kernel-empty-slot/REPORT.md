# Memory-card kernel: empty-slot model, route passes the `_card_info` wall

Branch `agent/card-kernel`, base `75b3fa35`. Worktree `/tmp/pe-agent-cardkernel`.

## What the route wall was

Day-1 `--route-pad` stopped at the first card kernel operation inside
`func_800405A4`, at frame 38000 / story `0x48`:

```
[ROUTE] frame=38000 token=A8002048 story=00000048 victories=0
[STUB:BOOTSTRAP_RET] card _card_info A0(AB) kernel call (first invocation)
[FB] vsyncs=39710 drawsyncs=38573 presents=38495 mask=1 main_iters=1
[HOST] stop_reason=unresolved-boundary
```

The machine had already drained the card events (previous pass); the frontier
was the kernel operation itself. `func_800405A4_port.c` routed
`func_8007DD44`/`func_8007DD54`/`func_8007DD74` through one nonreturning named
boundary even though it discards their return values and only waits for the
card completion events to latch its status flags.

## Retail authority

Callers (`asm/disc1/307CC.s` 30E94/30F4C/30FD0/3118C) call the veneers and
**discard `$v0`**; each call is immediately followed by the state-byte store
(`addiu $v0,0,1|2|3; sb $v0,8($s0)`). The veneers (`asm/disc1/6E538.s`) are
BIOS forwarders:

| port function | BIOS call | psx-spx name |
| --- | --- | --- |
| `func_8007DD44` | A0(ABh) | `_card_info(port)` |
| `func_8007DD54` | A0(ACh) | `_card_load(port)` |
| `func_8007DD74` | B0(50h)+B0(4Eh) | `_new_card()` + `_card_write(port,3Fh,NULL)` |

Empty-slot behavior is a timeout: psx-spx `B(5Ch) _card_status` value **11h =
failed/timeout (eg. when no cartridge inserted)**. The kernel reports it to the
game through the documented events (psx-spx BIOS Event Summary):

| layer | event | callback | guest flag |
| --- | --- | --- | --- |
| higher-level file/device | `F4000001h,2000h` card err eject | `func_80042C14` | `D_800A1828 = 1` |
| lower-level hardware I/O | `F0000011h,2000h` err | `func_80042C64` | `D_800A1834 = 1` |

`func_800409B4` opens all eight card events with mode 1000h; each callback is a
verified matching leaf under `pc_port/game/decomp/` that latches exactly that
flag. `func_800405A4` polls those flags, so synchronous host delivery at the
veneer is behaviorally identical for its consumer.

`_card_write` return follows psx-spx: sectors `0..3FFh` are valid, `400h` is
accepted by the documented retail bug, anything else returns 0 (rejected, no
I/O). `_card_info`/`_card_load` return 0 (no info) on an empty slot.

## Port change

1. `pc_port/platform/pe_libetc.c`: the 64-slot kernel event table now records
   each `OpenEvent` class/spec/handler. New `PE_Event_Deliver(cls,spec)` is the
   BIOS B(07h) adapter: it matches the first enabled event, executes the
   mode-1000h callback or marks a mode-2000h event ready, and returns whether a
   delivery was consumed.
2. `pc_port/platform/pe_libcard.c`: host implementations of the card kernel
   operations (`func_8007DD44`/`54`/`C4`/`B4`). Both slots are empty, so each
   operation reports the documented timeout by delivering `F4000001h,2000h`
   (higher-level) or `F0000011h,2000h` (lower-level). New `PE_Card_OpenEvents()`
   registers the eight events without the `func_800409B4` guard/RAM writes.
3. `pc_port/game/decomp/func_8007DD74_port.c`: the two callees now have host
   implementations, so its boundary shims are replaced by direct calls. The
   function body is unchanged.
4. `pc_port/game/boot/func_800405A4_port.c`: the three card calls now invoke the
   real veneers and keep the retail state-byte stores. No stub was widened.
5. `pc_port/tests/test_card_status.h`: registers the real card events
   (`PE_Card_OpenEvents`) so the empty-slot completion executes its callback.
6. `pc_port/tools/pe_battle_hud_oracle.py`: the shared original-code runner now
   models A0(ABh)/A0(ACh)/B0(4Eh)/B0(50h) with the same empty-slot semantics and
   latches `A1828`/`A1834` directly. `pe_card_status_oracle.py` drops the veneer
   `stop_at` entries; `retail_card_status_cases.h` was regenerated.

No success is invented: a card-present path is not implemented, and the model
always takes the documented empty-slot timeout.

## Proof 1 — native suite and every card oracle

```
cmake --build pc_port/build --target pe-native-tests -j
./pc_port/build/pe-native-tests
  Results: 1376 run, 1376 passed, 0 failed, 0 skipped

python3 pc_port/tools/pe_card_status_oracle.py                 PASS 4096
python3 pc_port/tools/pe_card_operation_oracle.py              PASS 8192
python3 pc_port/tools/pe_card_driver_oracle.py                 PASS 96 + 16
python3 pc_port/tools/pe_card_cleanup_oracle.py                PASS 192 + 64
python3 pc_port/tools/pe_card_operation_frame_oracle.py        PASS 1280
python3 pc_port/tools/pe_card_record_oracle.py                 PASS 256
python3 pc_port/tools/pe_card_modal_oracle.py                  PASS 96 + 16
python3 pc_port/tools/pe_card_confirmation_oracle.py           PASS 128 + 64
python3 pc_port/tools/pe_battle_hud_oracle.py                  PASS
python3 pc_port/tools/pe_mode7_hud_oracle.py                   PASS 240
```

The regenerated `retail_card_status_cases.h` pins the returning status machine
through the empty-slot kernel; `test_DAY1_card_status` compares the native port's
`func_800405A4` RAM effects and stop status against it.

## Proof 2 — live route frontier

```
./pc_port/build/parasite-eve-port --disc-image "$(cat local/pe_disc1.path)" \
    --headless --route-pad --max-frames 42000 --boundary-report \
    --trace /tmp/ck_after.log
```

Before (base `75b3fa35`):

```
[STUB:BOOTSTRAP_RET] card _card_info A0(AB) kernel call (first invocation)
[FB] vsyncs=39710 drawsyncs=38573 presents=38495 mask=1 main_iters=1
[HOST] stop_reason=unresolved-boundary
```

After:

```
[ROUTE] frame=42000 token=A8002048 story=00000048 victories=0
[FB] vsyncs=43215 drawsyncs=42078 presents=42000 mask=1 main_iters=1
[HOST] stop_reason=frame-limit
```

The route passes the old stop and runs to the 42000-frame cap. Zero
`[STUB:BOOTSTRAP_RET]` lines remain; the only stubs are the route's own
`HOST_ADAPTED` skip-movie/menu selectors, and every `BOUNDARY_REPORT` is the
pre-existing `func_80076C34` GPU boundary (not a stop). No new named boundary
appeared.

## Matching build

`src/`, `configs/` and `scripts/` were not touched by this change. The rebuild
was rerun in the `pe-mipsel-img` container from this worktree:

```
bash scripts/build_us.sh
  Plan:      OK (923 YAML spans; no manual span lists)
  Compile:   OK (628 generated C entries)
  Trim/link: OK (sizes/order generated from YAML edges)
  Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
  Matching claim: YES (628 registered C leaves)
```

The retail SHA-1 is unchanged. (The fresh worktree needed the git-ignored
`include/*.inc` plus `tools/era`, `build/extracted`, `asm`, `local/pe_disc1.path`
and `rom/image` copied from the main tree before the build could run.)

## What is real vs still named

- **Real:** BIOS B(0Bh) TestEvent semantics; BIOS B(07h) DeliverEvent for the
  eight card events; empty-slot `_card_info`/`_card_load` eject report and
  `_card_write` lower-level error report, with the psx-spx sector validation.
- **Still named / unimplemented:** a card-present model (real card image,
  directory parsing, success completions). The higher-level file operations
  (open/read/write/close) and the formatter remain at their existing named
  boundaries; this change does not touch them.

## Blockers / next

The card menu now retries the empty-slot path instead of stopping; story stays
`0x48` to the frame cap. The next unlock is either (a) a card-present model
grounded in a real `.mcr` image, or (b) confirming the game's no-card prompt
path (the `func_800425DC`/A1864 timeout notice) and its route-pad input so the
autopilot backs out of the save/load menu. Full Day-1/Day-2 acceptance remains
unfinished.
