# Inventory-help frontier: func_8004C608 ids 36-39

Branch `agent/help-frontier`, base `0ae2a69`.

## What the route wall was

`./pc_port/build/parasite-eve-port --disc-image "<disc1.bin>" --headless
--route-pad --max-frames 70000 --trace /tmp/help_route_before.log`

```
[ROUTE] frame=38000 token=A8002048 story=00000048 victories=0
[MENU] Unported help selection 36
[STUB:BOOTSTRAP_RET] func_8004C608 (first invocation)
[FB] presents=38494
[HOST] stop_reason=unresolved-boundary
```

The Day-1 autopilot reaches the inventory/menu at story `0x48` and selects
help id 36.  `func_8004C608` (`pc_port/game/boot/func_8004DF74_port.c`) handled
ids 0..35 and 40..60 but not 36..39, so it fell into the `default:` arm and
stopped at the bootstrap boundary.

## Retail behavior (authority: asm/disc1/3CE08.s)

`func_8004C608` dispatches through `jtbl_8001104C` (asm/disc1/data/800.rodata.s)
after `sltiu id,0x3D`.  The four missing entries are:

| id | target | behavior |
|----|--------|----------|
| 36 | 0x8004CA38 | if `func_800404A8()` → emit archive text 0x4C, text 0x4D; else `text = func_8005DC4C(func_80042770(0)||func_80042770(1) ? 0x4F : 0x4E)` |
| 37 | 0x8004CAAC | if `D_8009CF50` then if `D_8009CFF8` → emit 0x58, 0x59; else `text = func_8005DC4C(0x5C)`; else `text = func_8005DC4C(func_8003FFBC() ? 0x57 : 0x5B)` |
| 38 | 0x8004CAAC | identical to 37 (table alias) |
| 39 | 0x8004CB38 | emit archive text 0x50, then text 0x51 (no epilogue text) |

`func_8005F5B8(id)` is `func_8005F27C(func_8005DC4C(id))` in the port, so the
"emit" arms write directly and leave the epilogue's `s1` (text) at zero.

`func_800404A8` [0x800404A8,0x800405A4), 63 words, asm/disc1/307CC.s, was not
ported.  It is the idle-decay helper for the two 0x418-byte cursor selector
arrays: decays signed watchdog `D_800A1844` by one; a selector byte of 2 or 3
in either array reloads the watchdog to 0xC and marks `D_800A1848` /
`D_800A184C`; if a marked slot's selector is not 4 the watchdog is clamped to
at most 4 and both marks are cleared; returns `D_800A1844 > 0`.  It is
hand-transcribed in `pc_port/game/boot/func_800404A8_port.c` (no matching C
leaf exists).

## Port change

- `pc_port/game/boot/func_8004DF74_port.c`: cases 36, 37+38, 39 added.
- `pc_port/game/boot/func_800404A8_port.c`: new faithful asm transcription.
- `pc_port/include/pe_port_compat.h`: declarations for `func_800404A8`,
  `func_8003FFBC`.
- `pc_port/CMakeLists.txt`: build the new source.
- `pc_port/tools/pe_inventory_help_oracle.py`: groups 36..39 no longer
  excluded; the original binary now executes all 62 groups.
- `pc_port/tests/retail_inventory_help_cases.h`: regenerated, now 255 cases
  (was 251), including groups 36..39.

## Proof 1 — exact original behavior (native oracle)

The oracle runs the original retail function in the emulator over `RANGES`
and records an FNV-1a fingerprint of the resulting RAM plus the result word.
Regenerated header, then:

```
cmake --build pc_port/build -j2
./pc_port/build/pe-native-tests
Results: 1375 run, 1375 passed, 0 failed, 0 skipped
```

All 255 original inventory-help cases pass, including the four new group
36..39 fingerprints:

```
{'entry': 9, 'group': 36, 'index': 1} result 0x0 hash 0x362f98c1bca32fde
{'entry': 9, 'group': 37, 'index': 1} result 0x0 hash 0xd545911f6fa7041b
{'entry': 9, 'group': 38, 'index': 1} result 0x0 hash 0x6b278752c60ffdf8
{'entry': 9, 'group': 39, 'index': 1} result 0x0 hash 0x2108c03b8fe59a51
```

This pins the port's `func_8004C608` cases and the transcribed
`func_800404A8` to the original's RAM effects, not just the return value
(the entry is `void`; the oracle result word is always 0 for entry 9).

## Proof 2 — route wall

```
./pc_port/build/parasite-eve-port --disc-image "<disc1.bin>" --headless \
    --route-pad --max-frames 42000 --trace /tmp/help_route_after.log
```

```
[ROUTE] ... frame=38000 token=A8002048 story=00000048 victories=0
[STUB:BOOTSTRAP_RET] card status BIOS call (first invocation)
[FB] presents=38495
[HOST] stop_reason=unresolved-boundary
```

`[MENU] Unported help selection 36` and
`[STUB:BOOTSTRAP_RET] func_8004C608` are gone.  The route now advances to the
next boundary: the memory-card BIOS edge inside `func_800405A4`
(`pc_port/game/boot/func_800405A4_port.c`), which is a deliberately
nonreturning boundary (`BIOS 726F4` / card operation semantics are still
unfinished per `docs/ai_context/CARD_RECORD_CONTRACT.md` and the handoff).

**So the route does not run "well past frame 38000".**  The help wall was
masking the card wall; both occur at ~frame 38495 (`presents` 38494 -> 38495,
a one-frame shift).  It was verified by a temporary, reverted diagnostic that
bypassing only `card_status_call` lets the route run cleanly to the 42000
frame limit with no unresolved boundary.  That bypass was **not** committed:
faking the BIOS card edge would be widening a stub.

The card wall is the next real frontier, not this change.

## Part 2 — matching C leaf attempt (not byte-exact)

`src/func_8004C608.c` is a complete, parked C reconstruction of the whole
402-word function (not registered in `configs/USA/disc1.yaml`; registering it
would break the exact SHA-1 rebuild).  Triage with
`tools/analysis/try_leaf.py src/func_8004C608.c 0x3CE08 0x648`:

| flags | candidate size | first mismatch | differing words |
|-------|----------------|----------------|-----------------|
| `-O2 -G0` | 1568 / 1608 | 0x00F4 | 210 |
| `-O2 -G8` | 1520 / 1608 | 0x00F8 | — |
| `-O1 -G0` | 1584 / 1608 | 0x00F8 | — |
| `-O1 -G8` | 1520 / 1608 | 0x00F4 | — |

Under `-O2 -G0` the prologue and the first ~0xF4 bytes match; the first
divergence is at function offset 0x00F4 (retail `00031843` = `sra v1,v1,1`,
candidate `00008821` = `addu s1,zero,zero`).  The candidate is 40 bytes short,
so the switch/epilogue shape differs, not just scheduling.  This function is
already tagged `nonmatching` in `asm/disc1/3CE08.s`; byte-exact was not
reached in this session.

Exact behavior is nevertheless guaranteed by Proof 1, so the port uses the
exact behavior rather than an invented default.
