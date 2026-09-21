# Memory-card BIOS event path: TestEvent implemented, frontier advanced

Branch `agent/memory-card`, base `d0f20fe` (Merge agent/decompile-100 cont.).

## What the route wall was

The Day-1 `--route-pad` autopilot reaches frame 38000 / story `0x48` and
stopped at the first BIOS event drain inside `func_800405A4`:

```
[ROUTE] frame=38000 token=A8002048 story=00000048 victories=0
[STUB:BOOTSTRAP_RET] card status BIOS call (first invocation)
[FB] vsyncs=39710 drawsyncs=38573 presents=38495 main_iters=1
[HOST] stop_reason=unresolved-boundary
```

`pc_port/game/boot/func_800405A4_port.c` routed every card call through one
nonreturning `card_status_call()` boundary, including the four `TestEvent`
calls that only drain the card event handles.  The retail callers *discard*
each `TestEvent` result, so stopping there was a port boundary, not a retail
control-flow edge.

## Retail authority

`func_800405A4` (`asm/disc1/307CC.s` line 449) and its siblings
`func_800403C8` / `func_80040438` issue four `jal func_800726F4` calls and
never read `$v0`.  `func_800726F4` is the three-word BIOS veneer at
`asm/disc1/621E4.s` line 1044 (`addiu $t2,$zero,0xB0; jr $t2;
addiu $t1,$zero,0xB`), i.e. **B0(0Bh)**.

BIOS table identity (MAME `src/devices/cpu/psx/psx.cpp` `bioscalls[]` and
problemkaputt psx-spx BIOS Event Functions):

| service | name |
| --- | --- |
| B0(08h) | `OpenEvent(class, spec, mode, func)` |
| B0(0Bh) | `TestEvent(event)` |
| B0(0Ch) | `EnableEvent(event)` |
| A0(ABh) | `_card_info()` |
| A0(ACh) | `_card_load()` |
| B0(4Eh) | `_card_write()` |
| B0(50h) | `_new_card()` |

`TestEvent` returns 0 when the event is busy or disabled, otherwise 1 (and
switches it back to busy); **callback-mode (1000h) events never become
ready**.  `func_800409B4` (`pc_port/platform/pe_libcard.c`) opens all eight
card events with mode `0x1000`, so the retail kernel returns 0 for every card
`TestEvent` — no card result is being substituted here.

The game functions the status machine calls next are pure BIOS veneers:
`func_8007DD44` = A0(ABh) `_card_info`, `func_8007DD54` = A0(ACh)
`_card_load`, and `func_8007DD74` = `_new_card` + `_card_write`
(`asm/disc1/6E538.s` lines 20/29/47/68/77).  They need the card controller /
kernel completion events, which the port does not model, so they stay named
boundaries.

## Port change

1. `pc_port/platform/pe_libetc.c`: a 64-slot kernel event table now records
   each `PE_Event_Open` mode/enabled/ready, and `func_800726F4` implements
   BIOS B0(0Bh) `TestEvent` with the documented semantics (0 for
   disabled/callback/busy, 1 + consume for a ready mode-2000h event).
   `PE_Sdk_ResetState` clears the table.
2. `pc_port/platform/pe_sdk.h`: declaration for `func_800726F4`.
3. `pc_port/game/boot/func_800405A4_port.c`: `card_status_events` now issues
   the four real `func_800726F4` calls and ignores the results, exactly like
   retail.  The remaining card kernel calls keep a **nonreturning** named
   boundary, now named per target:
   `card _card_info A0(AB) kernel call`, `card _card_load A0(AC) kernel call`,
   `card _new_card/_card_write kernel call` (no stub was widened).
4. `pc_port/tools/pe_battle_hud_oracle.py`: the shared oracle gained a
   `pc == 0xB0` BIOS hook (B0(0Bh) TestEvent -> `$v0 = 0`, documented above)
   and an optional `stop_pc` capture.  The A0/B0 handlers now model the real
   kernel call instead of stopping on the veneer.
5. `pc_port/tools/pe_card_status_oracle.py`: `stop_at` no longer includes
   `0x800726F4`; the regenerated cases pin the new frontier (target + a0).
6. `pc_port/tests/test_card_status.h`: asserts the regenerated `target` as
   well as the argument; `retail_card_status_cases.h` regenerated.

## Proof 1 - original behavior (native oracle)

```
python3 pc_port/tools/pe_card_status_oracle.py --write-header
  PASS4096 original card status cases; TestEvent modeled, first card kernel boundary
cmake --build pc_port/build --target pe-native-tests -j
./pc_port/build/pe-native-tests
  Results: 1376 run, 1376 passed, 0 failed, 0 skipped
```

The 4096 original cases now execute the real B0(0Bh) service in the oracle
and stop only at the first card kernel operation, so the RAM hash and the
target/argument frontier are pinned one level deeper than before.

## Proof 2 - route frontier

```
./pc_port/build/parasite-eve-port --disc-image "$(cat local/pe_disc1.path)" \
    --headless --route-pad --max-frames 39000 --boundary-report \
    --trace /tmp/card_route_br.log
```

After:

```
[ROUTE] frame=38000 token=A8002048 story=00000048 victories=0
[STUB:BOOTSTRAP_RET] card _card_info A0(AB) kernel call (first invocation)
[FB] vsyncs=39710 drawsyncs=38573 presents=38495 main_iters=1
[HOST] stop_reason=unresolved-boundary
```

The old `card status BIOS call` (the TestEvent drain) is gone; the route now
advances through the event drain and stops at the next, precisely named
frontier, `_card_info` (A0 ABh).  The presents count is unchanged only because
the new stop is a few calls later inside the same frame's menu processing.

## What is real vs still a named boundary

- **Real:** BIOS B0(0Bh) `TestEvent` semantics (enabled/mode/ready, callback
  events never ready), used by the whole card status graph.
- **Still a named nonreturning boundary:** `_card_info` A0(ABh),
  `_card_load` A0(ACh), and `_new_card` B0(50h) + `_card_write` B0(4Eh) via
  `func_8007DD74`.  Their completion arrives asynchronously through card
  events the port does not deliver; returning 0 from them would silently drop
  the card edge, so they keep the stop.

## Matching C leaf

Not attempted in this pass.  `func_800403C8` / `func_80040438` (0x70 bytes
each, the event-drain helpers) are the natural small candidates, but the
whole `0x307CC` span is a single `asm` entry in `configs/USA/disc1.yaml`, so
registering them requires a carve (asm prefix + 2 C leaves + asm resume) and
a byte-exact `scripts/build_us.sh` run.  `func_800726F4` itself is a
hand-written SDK veneer, not a C leaf.  No matching claim is made.

## Blockers / next

The next frontier is the card kernel/controller model or an evidence-based
"no card inserted" completion path for `_card_info`/`_card_load`/`_new_card`/
`_card_write`, including the timeout events the card events are opened with
(spec `0x100`).  That is required before the route can pass frame 38495.
Full Day-1/Day-2 acceptance remains unfinished.
