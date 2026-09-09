# DAY2-158 — got_frame spin without C89C TRACE (tip `04c8078`)

## Symptom
Disc1 on `04c8078` (158j/k): Stage-1b cleared (one `e0_promote`), then ~600
`e0_poll` / `got_frame` with `7C214_last_chunk`, **no** named STOP and **no**
C89C-looking TRACE. Suspected “frames admit without decoder fire.”

## Root cause (code)

1. **Observability false negative.** On `04c8078`, `func_801924F8` `got_frame`
   always calls live `func_8010C89C(stream, out, table, 0)` unless it STOPs on
   `Stage1b_pad_terminated_frame`. There is no silent skip of the decoder after
   admit.
2. **`func_8010C89C` has no `Trace_Direct`.** Entry/exit are only recorded in
   host `PE_C89C_GetTelemetry` (`a0`/`a1`/`a2`/`ret`). Absence of a C89C TRACE
   line does not mean the call was skipped.
3. **Gate predicates at admit (`924F8`):**
   - Enter `got_frame` when `s1 != 0` and
     (`c89c_stream_is_immediate_pad(s1)` OR `PE_Port_PeekStreamFrameReady()`).
   - Inside `got_frame`: `TakeStreamFrameReady()` (clears latch); STOP only if
     `!pad && !frame_ready`; else call C89C then `func_8007C394`.
4. **`Take` cannot re-enter `got_frame` with ready=0 without STOP** unless the
   stream is immediate pad. Live last-chunk path Notes again in `7C214` when
   `B89F4==1` (matches many `7C214_last_chunk` lines).
5. **`924F8` returns after one `got_frame`.** `92CE8` calls `924F8` once, then
   the post-E08 media loop drives `92934` (no `got_frame` TRACE). A long
   `e0_poll`/`got_frame` TRACE storm implies either many outer re-entries of
   `924F8`, or conflating poll TRACE volume with decoder absence.

## `out` / `table` / pad-exit

- `out` = `[0x801D1464 + (flip^1)*4]` after toggle of `[0x801D146C]`.
- `table` = `[0x801D0DF8]` (C89C then does `a2 += 0x800`).
- C89C fresh path can **pad-exit** (`ret=0`) or **bound-exit** (`ret=1`) without
  any TRACE. Instant pad-exit on a demux body would look like “no decoder work”
  even though the call ran.

## Recommended next change

**No port patch yet** — confirm live telemetry before changing gates.

1. On Disc1/Bazzite after the spin: dump `PE_C89C_GetTelemetry` (and ideally
   call-count if Auto adds one). Expect nonzero `a0` (= `s1` body) and
   populated `a1`/`a2` if C89C ran.
2. Optional Disc1-only: one `Trace_Direct("func_8010C89C_enter")` at C89C entry
   (host observability; not guest authority).
3. If telemetry shows live calls with `ret==0` immediately / bad `a1` range:
   dig pad-vs-body at `s1`, then `7C394` / MDEC / slice-wait gap noted in
   `924F8` comments — not the Stage-1b admit gate.
4. If telemetry shows **zero** entries despite `got_frame` TRACE: link/stub
   mystery (unlikely on this tip; C89C is in-tree and called directly).

## Patch
None. Dig branch `dig/got-frame-c89c-skip` @ `04c8078` (report only).

## Related
- `docs/evidence/pe-day2-158i-early-publish-race/REPORT.md` (158i latch miss)
- `docs/evidence/pe-mv1d-c89c/REPORT.md` (decoder + OOB trap)
