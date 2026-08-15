# PE-RD5-F9-DIFF — m0372i close vs RD5-C2 OP_0x22 edge

```text
PE-RD5-F9 SUCCESS — RETAIL MESSAGE CLOSE SEMANTICS RECONCILED
```

Evidence only. No production runtime edit. No RD5-C2 reel repair.
No RD6-A / RD7-R / PT1 / BGM reopen. No push.

```text
authorities
  RD5-C2   1e7f0df   (m0004i reel: OP_0x22 waits for new 0x100)
  PE-TXT0  1d47df3   (0xF9 auto-closes; MSG_0x14..MSG_0x20 need no OP_0x22)
disc_sha256 = 7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4
exe_sha256  = 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
exe_sha1    = 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
tool        = python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" --scene m0372i --all-slice
```

Question: after RD5-C2 made every `OP_0x22` wait for a newly-pressed
`D_8009D1F4 & 0x100` edge, does the m0372i reel now wait for a
player press where retail auto-closes?

Answer: **no.** The first-play m0372i reel never issues `OP_0x22`.
RD5-C2 did not touch that implementation. Control-restore timing
does not shift.

## 1. Retail close for the m0372i reel

Command (this checkout, registered Disc 1):

```text
python3 tools/research/pe_txt0_decode.py "$PE_DISC1_BIN" \
  --scene m0372i --all-slice
```

`script_message_ops` on m0372i:

```text
module1+0x025C OP_0x0D MSG_0x14
module1+0x0274 OP_0x0D MSG_0x15
...
module1+0x037C OP_0x0D MSG_0x20
module3+0x09F4 OP_0x0D MSG_0x07
module3+0x0A00 OP_0x22 MSG_0x07
```

Thirteen first-play opens. Zero `OP_0x22` on `MSG_0x14`..`MSG_0x20`.
The sole package `OP_0x22` is the `persist[0]&4` arm (`MSG_0x07`).
First play `persist[0]=0` skips it (RD4-D).

USA stream 1 (`231da625…`) bodies:

| id | start marker | body yield | next stream bytes | close |
|---|---|---|---|---|
| `MSG_0x14` | `FF FE 14` | `FB 07 78` | `F9 FE 15` | `0xF9` auto |
| `MSG_0x15` | `F9 FE 15` | `FB 07 2D` | `F9 FE 16` | `0xF9` auto |
| `MSG_0x16` | `F9 FE 16` | `FB 07 55` | `F9 FE 17` | `0xF9` auto |
| `MSG_0x17` | `F9 FE 17` | `FB 07 78` | `F9 FE 18` | `0xF9` auto |
| `MSG_0x18` | `F9 FE 18` | `FB 07 2D` | `F9 FE 19` | `0xF9` auto |
| `MSG_0x19` | `F9 FE 19` | `FB 07 64` | `F9 FE 1A` | `0xF9` auto |
| `MSG_0x1A` | `F9 FE 1A` | `FB 07 2D` | `F9 FE 1B` | `0xF9` auto |
| `MSG_0x1B` | `F9 FE 1B` | `FB 07 3C` | `F9 FE 1C` | `0xF9` auto |
| `MSG_0x1C` | `F9 FE 1C` | `FB 07 3C` | `F9 FE 1D` | `0xF9` auto |
| `MSG_0x1D` | `F9 FE 1D` | `FB 07 2D` | `F9 FE 1E` | `0xF9` auto |
| `MSG_0x1E` | `F9 FE 1E` | `FB 07 3C` | `F9 FE 1F` | `0xF9` auto |
| `MSG_0x1F` | `F9 FE 1F` | `FB 07 3C` | `F9 FE 20` | `0xF9` auto |
| `MSG_0x20` | `F9 FE 20` | `FB 07 3C` | `F9 FE 21` | `0xF9` auto |

`func_80037870` at `0x80037C50` writes state `0` on body `0xF9`.
No `0x100` test. `FB 07 nn` is a pause (`record+0x0D`), then `0xF9`
closes. The following `OP_0x02` is a **script** wait, not a window
dismiss. TXT0 `TEXT_LIFECYCLE.md` already stated this; the decoder
run above re-proves the markers and the missing `OP_0x22` sites.

Contrast (m0004i, not this reel): `MSG_0x21` / `MSG_0x22` /
`MSG_0x23` terminate with body `0xFF` and pair `OP_0x0D`+`OP_0x22`.
RD5-C2's newly-pressed `0x100` rule applies **there only**. This
rung does not weaken that edge.

## 2. Implemented gate at RD5-C2

RD5-C2 `1e7f0df` file list (`git show --stat 1e7f0df`):

```text
docs/ai_context/ACTIVE_HANDOFF.md
docs/evidence/pe-rd5c2-rd5x-reconciliation/*
tests/test_rd5c_m0004i_control.py
tools/retail_scene/pe_rd5c_cutscene.py
tools/retail_scene/pe_rd5c_play.py
```

`tools/retail_scene/pe_rd4e_cutscene.py` and
`tests/test_rd4e_m0372i_cutscene.py` have **no** commits from
RD4-E `9788bc4` through RD5-C2. The m0372i reel is unchanged.

That reel (`MESSAGE_SPECS`) still pairs each `OP_0x0D` with an
authored `OP_0x02` hold:

```text
MSG_0x14 320   MSG_0x15 80    MSG_0x16 90    MSG_0x17 170
MSG_0x18 60    MSG_0x19 120   MSG_0x1A 60    MSG_0x1B 80
MSG_0x1C 360   MSG_0x1D 90    MSG_0x1E 70    MSG_0x1F 110
MSG_0x20 120
```

`M0372iCutscene.tick` decrements `wait_remaining`. It never reads
pad, `D_8009D1F4`, or `0x100`. A search of `pe_rd4e_cutscene.py`
for `0x100` / `MESSAGE_ADVANCE` / `newly_pressed` / `OP_0x22` is
empty.

RD5-C2's `0x100` gate lives only in `pe_rd5c_cutscene.py`
(`MESSAGE_IDS = (MSG_0x21, MSG_0x22, MSG_0x23)`). Oracle drivers
OR bit `0x100` while that record is READY. That path is the
`0xFF`-then-edge rule. It is not applied to `MSG_0x14`..`MSG_0x20`.

Per-row verdict: `CLOSE_MECHANISM.csv`. All thirteen rows
`match=yes`. Over-gating (press-wait where retail `0xF9`-closes)
is absent.

## 3. Control-restore timing

m0372i never restores control. RD4-E hard-stop: inhibit stays 1,
no type-0 actor, token `0xA8000248` at cutscene frame 2421.

m0004i restore is `OP_0x3F` at `module0+0x05B0`, after the
`MSG_0x21`..`MSG_0x23` `OP_0x22` polls (RD5-C2). Because the
m0372i reel still uses the same thirteen `OP_0x02` holds, the
handoff frame into m0004i is unchanged. Restore does not shift.

```text
control_restore_tick_delta=0
```

A hypothetical press-gate on all thirteen `0xF9` records would
have delayed token publication by one player edge per message and
pushed m0004i `OP_0x3F` by that count. That defect is not present,
so the RD5-C2 trace is not rewritten.

## 4. Traces

RD5-C2 e2e (m0004i reel, unchanged):

```text
87b3425e7194cc0e3812e6692eee2ddccd33639ac8ea315877608c39943c8bc3
```

RD4-E e2e (m0372i reel, unchanged):

```text
f7538226773f392b67ec5c2df85de3789201357014fd24c4d625c2a46f34c576
```

No implementation edit, so neither hash is superseded.

## 5. Naming

`NAMING_CONVENTION.md` is the rule for this folder. Message ids
use `MSG_0xNN`. Opcodes use `OP_0xNN`. `MSG_0x21` / `MSG_0x22` /
`MSG_0x23` and `OP_0x21` / `OP_0x22` / `OP_0x23` are never written
bare in this evidence.

## 6. Hard negatives honored

- `0x100` newly-pressed edge on `0xFF` + `OP_0x22` is not weakened.
- RD6-A / RD7-R not reopened.
- PT1 visuals not reopened.
- No BGM added.

---

```text
base_rd5c2_commit=1e7f0df
txt0_commit=1d47df3

over_gating_present=no
messages_affected=none
control_restore_tick_delta=0

trace_changed=no
old_trace_sha256=87b3425e7194cc0e3812e6692eee2ddccd33639ac8ea315877608c39943c8bc3
new_trace_sha256=87b3425e7194cc0e3812e6692eee2ddccd33639ac8ea315877608c39943c8bc3
supersede_reason=none

naming_convention_applied=yes
tests=none

hard_blockers=none
warnings=implementation keeps current_message for the full OP_0x02 hold (placeholder display); retail 0xF9 closes earlier after FB 07 nn, which is shorter than every authored OP_0x02 on this reel — that is not a press gate and is pre-RD5-C2; module3 OP_0x22 MSG_0x07 is persist[0]&4 only; 0x100 remains an unnamed processed mask

SUCCESS

PE-RD5-F9 SUCCESS — RETAIL MESSAGE CLOSE SEMANTICS RECONCILED
```
