# DAY2-158 — early `909B4` title-cut after `92CE8_media_clear` (2 C89C)

Status: **ROOT CAUSE IDENTIFIED** — draft twin-clear patch on dig branch
(PE.IMG byte confirm of title EC gap still preferred).

Tip under dig: `28ed6b6` (`DAY2-158q: fold+wire title DecDCTout func_80191DC8`).
Throwaway branch: `dig/909b4-early-title-cut`.

## Symptom (Disc1 / DAY2-158q)

Past the `74520` / `91DC8` wall:

- Real `func_80191DC8` wired; multi-frame confirmed.
- Live shape: C89C `calls=2 out=11140`, TRACE `func_80192934_enter` +
  `func_80192CE8_media_clear`, then
  `Bootstrap … func_801909B4_post_movie_title_cut`.
- Only **two** decoder frames before media clear — not a full FMV001 play.

## What is *not* the bug

`func_801909B4_port.c` still ends the saved-bit / movie arm with the named
cut `func_801909B4_post_movie_title_cut` after `func_80192CE8(1)` returns.
That stub is intentional until the title/menu tail at `0x80191120` is
translated (DAY2-158o). A completed (or falsely completed) movie always
hits it. The early wall is **upstream**: `92CE8` returns after two frames.

`--skip-movie` / saved-bit-zero arms are unrelated.

## Call chain (post-E08 → title-cut)

```text
909B4  →  92CE8(1)
            91FB8: DBA=1, DBC=0, 801D11B0=0xFFFF
            924F8: first-frame C89C (FMV001 frame 1, out=7300)
                   EC: B0DBD=0, DBA++ → 2, DBC=1
            post-E08 loop:
              DBC>0 → 3EB04 → 92934
                DBA>=2 → enter (TRACE 92934_enter)
                91B64 got-frame → C89C #2 (FMV001 frame 2, out=11140)
                success path: if [801D0DBD]==1 → DBA--, CD teardown, return 0
              status<<24==0 → TRACE media_clear → pe_92ce8_clear_stream (DBA=0)
              70E54; DBA==0 → break
            epilogue: clear overlay 0x200; return s3
909B4  →  post_movie_title_cut + STOP unresolved-boundary
```

Retail CFG authority: `docs/evidence/pe-92ce8-post-e08/REPORT.md`,
`docs/evidence/pe-92934-media-worker/REPORT.md`.

## DBA / DBC / frame story

| Step | DBA (`800B0DBA`) | DBC (`800B0DBC`) | D0DBD (`801D0DBD`) | C89C |
| --- | --- | --- | --- | --- |
| after `91FB8` | 1 | 0 | 0 (unset) | — |
| `924F8` E0 / first `91B64` | 1 | 0 | **→ 1** (see below) | — |
| `924F8` got_frame EC (158o) | **→ 2** | **→ 1** | stays 1 (B0DBD cleared only) | #1 `out=7300` |
| first `92934` got_frame | 2 → **1** (abort `--`) | → 2 | ==1 arms abort | #2 `out=11140` |
| `pe_92ce8_clear_stream` | **→ 0** | 2 (untouched) | — | — |

- `924F8` sets `DBC=1` after got_frame (EC). `92934` bumps `DBC` each
  got-frame. Loop exit on `DBC<=0` is not the early path here.
- `out=7300` / `out=11140` match FMV001 frames 1–2 exactly
  (`docs/ai_context/DAY2_MOVIE_COMPLETE_FRAMES.md`). Frame 3 would be
  `out=12804`. FMV001.STR is ~42 MB (~9 video sectors/frame) — hundreds+
  of frames expected before a honest end.

## Why `91B64` latches `801D0DBD` on the first poll

`91FB8` seeds `801D11B0 = 0xFFFF`. `91B64` C44:

```c
if (w < (uint32_t)(int32_t)(int16_t)PE_LoadU16(0x801D11B0u))
    PE_StoreU8(0x801D0DBDu, 1u);
/* then publishes 801D11B0 = (uint16_t)w */
```

With limit `0xFFFF` sign-extended to `0xFFFFFFFF`, any small record word
(including the CDQ2d fixture word `0`) latches DBD. Native suite documents
this as retail-matching:

`test_native.c` — “C44 sets the [DBD] movie byte (record word 0 < 0xFFFF
limit)” → assert `801D0DBD==1`, `801D11B0==0`.

**Nothing in the port clears `801D0DBD`.** Writers: `91B64` only.
Reader (abort): `92934` success path. `924F8` EC clears **`800B0DBD`**
(opcode `A0200DBD` @ `801928F8`, same `$at` family as `sh 1,[B0DBC]` @
`80192908` — `pe_mv1d_c89c_oracle.py`), which has **no** media-loop reader.

## Player twin (high confidence)

Movie-player first-frame exit (`func_80121C04`) after C89C+7C394:

```text
223F5 = 0     ← clear end flag that 21270 may have set during acquire
B0DBA++
B0DBC = 1
```

Updater then checks `223F5==1` for abort teardown (`func_80122040`).

Title roles:

| Role | Player | Title |
| --- | --- | --- |
| poll worker end latch | `21270` → `223F5=1` | `91B64` → `801D0DBD=1` |
| first-frame clear | `21C04` → `223F5=0` | **missing for D0DBD**; EC clears `B0DBD` only |
| media abort test | `122040` reads `223F5` | `92934` reads `801D0DBD` |

So after 158o’s `DBA++` unlocked multi-frame entry, the next honest wall is
exactly one media-loop frame: sticky `D0DBD` from acquire → abort →
`media_clear` → `909B4` cut.

## Hypotheses ranked

1. **Primary (adopted):** `924F8` first-frame EC does not clear
   `801D0DBD` (player-twin of `223F5=0`). Sticky latch → first `92934`
   success returns 0 → `92CE8` media_clear after C89C×2.
2. Unexpanded title EC gap `[80192858,801928F8)` (40 words of slice-wait)
   may already contain a retail `D0DBD` clear and/or the true `DBA++`
   site — PE.IMG dump still preferred; port currently synthesizes `DBA++`
   next to the known B0DBD/B0DBC stores.
3. Pad abort arm (`PAD_HELD & 0x20000004`) — **out**: live TRACE is
   `func_80192CE8_media_clear` (status==0 arm), not the pad path.
4. `post_movie_title_cut` “too broad” — **out** as root cause of *early*
   exit; it is the designed post-movie wall. Fix playback length first.
5. `clear_stream` too eager on any status==0 — **out** as primary; retail
   post-E08 does clear on low-byte-zero (early gate *or* abort return).
   Clearing is correct once `92934` returns 0; stop returning 0 early.

## Recommended next change

Minimal honest fix (draft on this branch):

- In `func_801924F8` got_frame EC, **also** `PE_StoreU8(0x801D0DBDu, 0u)`
  (keep retail `B0DBD=0`). Comment as title twin of player `223F5=0`.
- Dig aid: TRACE when `92934` takes the `[D0DBD]==1` abort so the next
  live run can confirm the latch is gone (or name a new wall).

Do **not** invent a frame-count gate, widen the `909B4` stub, or patch the
C89C admit path. After the clear, expect C89C to advance at least to
frame 3 (`out=12804`) before the next frontier (slice-wait gap, CD refill,
or real end-of-stream DBD).

## Draft patch

`pc_port/game/boot/func_801924F8_port.c` — EC clear of `801D0DBD`.
`pc_port/game/boot/func_80192934_port.c` — TRACE on DBD abort arm.

## Validation

- Focused: existing native movie / 91FB8 / 924F8 / 92CE8 groups (no full
  suite claim here; Auto usage flaky).
- Live Disc1: expect `92934_enter` without immediate `media_clear`;
  C89C `calls>=3` / `out=12804` before any new STOP.
- Still expect `post_movie_title_cut` once `92CE8` eventually returns.

## References

- `docs/evidence/pe-day2-158o-c89c-eof-pad/REPORT.md` — one-frame cut via
  missing `DBA++` (superseded by 158o; this rung is the sequel).
- `docs/evidence/pe-92ce8-post-e08/REPORT.md` — media_clear CFG.
- `docs/evidence/pe-92934-media-worker/REPORT.md` — DBD abort teardown.
- `docs/ai_context/DAY2_MOVIE_COMPLETE_FRAMES.md` — 7300 / 11140 / 12804.
- `pc_port/game/boot/movie_overlay_port.c` — `21C04` / `223F5=0` twin.
- `pc_port/tools/pe_mv1d_c89c_oracle.py` — `A0200DBD` @ `801928F8`.

## Port fold (PR #38 / DAY2-158r)

Status updated: **APPLIED** on `cursor/movie-autonomous-stream-6f51`.
`924F8` got_frame EC clears `801D0DBD`; `92934` TRACE `dbd_abort` kept.
`909B4` stub unchanged. Linux suite recorded in ACTIVE_HANDOFF.
