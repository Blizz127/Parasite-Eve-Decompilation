# m0020i field-script gate: the room's only transfer is a door 118 units east of the entry

Worktree `/tmp/pe-agent-m0020i`, branch `agent/m0020i`, base `ec097017`.
Retail authority: `build/extracted/disc1/SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Summary

* **What was wrong:** the recorded Day-1 `--route-pad` sequence walks Aya
  **west** the moment she enters `m0020i` (entry `(537,-19)`, frame ~20434).
  `m0020i` has exactly **one** room transfer and its trigger rectangle is
  **east** of the entry point, so the recorded input can never leave the room.
  The baseline token therefore stays `0xA8002048` through frame 80000 and no
  `op31` (`func_80017BB4`) ever executes — exactly the reported symptom.
* **What was changed:** two recorded pad frames in `kDay1RoutePads`
  (`20600:FFDF`, `21000:FFFF`) that walk Aya east through that door.  The
  transfer now fires at **f=20655** and the route exits to token
  `0xA8001148` (`m0012i`) instead of parking in the room.
* **What is still open:** the `local[4]==3` key gate.  It is a *separate*
  body-contact path and is documented below as an unresolved, evidence-backed
  blocker.  **Story stays `0x00000048`** after the transfer; no story write
  fires in `m0012i` on this route.

## Method and the room script

The `m0020i` field package is field-table entry 19 (`start` LBA 12124,
`meta` 0x03605A21 → chunks 33/90/3); `extract_script_from_package` +
`decode_script` (`tools/research/pe_pst0_scan.py`) yield an 8416-byte script
with **8 modules** (chunk 2, LBA 13260 — the address SEW17 recorded).  The
live VM addresses are the package offsets plus `0x801A0878`; that mapping is
confirmed independently by the report's `local[4]` gate:

| runtime | package off | command |
| --- | --- | --- |
| `0x801A0AE0` | `0x0268` | `op09 sub 0x0B (a==b) cnd0 = Aya.local[4], 3` |
| `0x801A0AF8` | `0x0280` | `op05 skip-if-false -> 0x801A0CC4` |
| `0x801A0BA4` | `0x032C` | `opA7 item 0xC8 -> local[6]` |
| `0x801A0BDC` | `0x0364` | `opE8 0xC8` pickup dialog |

`op05` (`func_8001731C`) jumps when its condition is **zero**; the block after
it runs when the condition is true.  Module 4 is actor **type 4, serial 5**
(confirmed live with a per-`(type,module)` dispatch census):

```
0x153C op02 1
0x1548 op5E (code 0, type 0 -> Aya's pose)  -> loc0=x, loc1=y, loc2=z
0x156C op77 rectangle (655,352)-(655,-357)-(851,-357)-(851,352) -> loc4
0x15A0 cnd0 = (loc4 == 1)
0x15B8 op05 -> end if not hit
0x15C8 op1C (0,0,0xFE)          ; mailbox to Aya
0x15DC op85 0x1E                ; fade
0x15E8 op9C                     ; fade wait
0x15F0 per[1] = 0x14
0x1600 op31 0xA8001148          ; <-- the room's ONLY op31
0x160C op20
```

`grep op31` over the whole script returns that single site.  Aya enters at
`(537,-19)`; the rectangle starts at `x=655`, so the door is ~118 map units
east of the entry point and **west** of every subsequent recorded pad.

### Live confirmation (default route, instrumented)

`[OP77] type=4 ser=5 pt=(537,-19) hit=0 rect=(655,352)-(655,-357)-(851,-357)-(851,352)`
is the first `m0020i` test; `hit` stays 0 for the whole recorded segment, and
the trajectory moves from `(537,-19)` to `(94,173)`/`(-546,*)` — never near
`x>=655`.

### Input that reaches the door

A four-direction sweep from the entry frame shows only one mask moves Aya into
the rectangle:

| pad (frames 20600..23000) | Aya at f=23000 | token |
| --- | --- | --- |
| `FFDF` | `m0012i (-570,-5390)` | **`0xA8001148` (transfer fired)** |
| `FFBF` | `(568,-277)` | `0xA8002048` |
| `FF7F` | `(-545,-245)` | `0xA8002048` |
| `FFEF` | `(537,241)` | `0xA8002048` |

## The change

`pc_port/tests/route_rehearsal_pads.h` — inserted
`20600:FFDF,21000:FFFF` after `20400:FFFF`.  Host input only; no guest state
is written and no script gate is bypassed.

`pc_port/tests/test_route_boot_day2.c` — `TraceSawTokenAfter()` plus a guard:
the route must show token `0xA8001148` **after** the first `m0020i` trace
entry.  The earlier `m0012i` visit (f~18000) cannot satisfy it.

### The guard is non-vacuous

```
# with the new pads
route: 27/27 ordered milestones reached
PASS: boot -> Day-1 rooms -> m0020i -> room transfer 0xA8001148 at f=20655
      (27 milestones, frames=42000 stop=frame-limit)

# same run with PE_ROUTE_PAD_SEQUENCE = kDay1RoutePads minus the two new pads
FAIL: m0020i never took its only room transfer (no token 0xA8001148 after
      m0020i at frame 20434; missing the 20600:FFDF door-walk input)
```

## Before / after route outcome

| | before | after |
| --- | --- | --- |
| `m0020i` entered | f=20434 | f=20434 |
| room transfer | **never** (`D_8009D280` frozen) | `op31` -> `0xA8001148` at **f=20655** |
| final token (f=80000) | `0xA8002048` (`m0020i`) | `0xA8001148` (`m0012i`) |
| final story | `0x00000048` | `0x00000048` |
| stop | `frame-limit` | `frame-limit` |
| `[STUB:BOOTSTRAP_RET]` | 0 | 0 |

## Open blocker: the `local[4]==3` key gate (NOT solved)

The key award guarded by `local[4]==3` is real (item 200 + pickup dialog) but
still unreached.  Instrumented findings:

* Aya's `local[4]` is her task mailbox payload, written by `op1F`
  (`func_800177AC`, `task+0x14`), delivered by `op1C` (`func_80017764`) through
  `func_800653B8`/`func_80065400`.
* Only **one** `op1C` in the script sends payload `3`: module 2 (actor type 2,
  serial 3 — the body's contact/interaction script, entry `0x801A1620`) at
  package off `0x1048`.
* That block is reached from `0x0E10`, which takes the payload-3 path only when
  `persist[0x18] & 0x200` is **set**.  The first body contact sets that bit:
  live, `per18` goes `0x01000000` -> `0x01000280` as the body sends `4` then
  `0xFF`.  A **second** body-contact task is therefore required to reach the
  payload-3 send.
* In the recorded route the body contact fires **exactly once**.  Live
  `[CTASK]`: body(type2,ser3) -> Aya at `(81,154)`; the body task runs
  `0x801A1620`, sends `4`/`0xFF`, then is retired (`[CTRET] task=8009D368
  pc=801A1620 flags=0010`).  Every later approach (including stopping on the
  body, `(81,159)`) produces **no** new body contact task, so module 2 never
  runs the payload-3 block and `opA7` is invoked 0 times.
* The broad contact test always passes (radius 561, `d2 < radius^2`), so this
  is not a range miss: after retirement the pair does not re-create a task.
  Whether retail re-arms the body task, or the port's task-retirement/re-arm
  path diverges from the original `func_80036448`/`func_80012774` pair, was
  **not** resolved inside this session.

Consequence: getting the key (and therefore a story write past `0x48`)
requires either a proven re-contact input sequence (a search over the small
contact window) or a fix to the contact/task lifecycle above.  The transfer
fix in this report does not touch that path.

## Verification

```
cmake --build pc_port/build -j$(nproc)                       # exit 0
ctest --test-dir pc_port/build --output-on-failure           # 11/11
./pc_port/build/pe-native-tests                              # 1394/1394
python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans   # check: OK
PE_CARD=build/pe_card1.mcr ./pc_port/build/parasite-eve-port \
  --disc-image <disc1> --headless --route-pad --max-frames 80000 \
  --trace /tmp/m0020i_after.log                              # frame-limit, 0 stubs
```

Only `pc_port/` (route table + harness guard) and docs changed, so
`scripts/build_us.sh` is not affected by this change; the retail SHA is
untouched.  No `src/` leaf, no `configs/USA/*.yaml`, and no game data were
modified.
