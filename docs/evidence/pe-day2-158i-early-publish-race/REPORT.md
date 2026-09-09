# DAY2-158i early-publish race (got_frame without StreamFrameReady)

Tip base: `1fa9a48` (DAY2-158i). Scope: demux/`924F8` / `7C484` / `91B64` /
`7C564` / `7C214`. **Not** `92934`/`92CE8`.

## Symptom

Live Disc1 reaches `func_801924F8` `got_frame` with nonzero `s1` from
`func_80191B64`, then `Stage1b_pad_terminated_frame` because the stream is
not an immediate pad **and** `PE_Port_TakeStreamFrameReady()==0`.

## 1. When does `func_8007C484` return 0? What is `MV1_SP16` / `s1`?

Source: `pc_port/game/boot/func_8007A214_port.c` (`func_8007C484`),
`pc_port/game/boot/func_80191B64_port.c`.

`7C484(dst_a, dst_b)`:

- Reads slot at `D_800C0DC8 + (D_800BE9EC << 5)`.
- If slot status `== 1`: clears `BE9EC` to 0; if `C0DBC!=0` clears that
  slot; recomputes slot at index 0.
- If slot status `!= 2`: **returns 1** (not ready).
- If status `== 2`: promotes status to **4**, publishes:
  - `*dst_a` = stream **body** =
    `C0DC8 + (C20C4 << 5) + ((BE9EC * 63) << 5)`
    (CDQ2d plant: base `0x801E0000`, count `0x40` → body `0x801E0800`
    when active index 0).
  - `*dst_b` = slot pointer.
  - **Returns 0** (success).

`91B64`:

- Polls `7C484(MV1_SP16=0x801FFF20, MV1_SP20=0x801FFF24)` up to 2000 times.
- On success (`v0==0`): uses `MV1_SP20` as slot for window/dim bookkeeping,
  then **returns `(int32_t)PE_LoadU32(MV1_SP16)`** — the demuxed VLC body
  cursor. That value is E0’s `s1` into `got_frame`.

So nonzero `s1` means: a **state-2** record was promoted to 4 and the body
address was published. It does **not** by itself mean E0 latched
`StreamFrameReady`.

## 2. How CD stream / `7C564` sets `B89F4` vs when a body can publish

Source: `pc_port/game/boot/cd_stream_port.c` (`func_8007C564`),
`pc_port/platform/pe_libcd.c` (`func_8007C214`),
`docs/evidence/pe-b54kao-stream-completion-selector/REPORT.md`.

Last video chunk in `7C564`:

```
last = (rec+6)-1 == (rec+4);
if (last) PE_StoreU32(0x800B89F4, 1);
… DMA body via 7CEAC(..., interrupt=last) …
PE_StoreU16(rec, 3);   /* in-flight, not yet state-2 */
if (C0DB8 && B89F4) func_8007C214();  /* production C0DB8==0: skip */
```

Production path (`C0DB8==0`):

1. `7C564` sets `B89F4=1`, issues DMA3 with channel-3 IRQ enabled on last.
2. Port `PE_CdReg_ServiceDMA3` completes synchronously on CHCR write and
   bridges a DICR rising edge.
3. `HostFB_PumpCdProgress` → `HostFB_DeviceTime` → `HostFB_ServiceDeviceIrq`
   → `PE_IRQ_ServicePending…` → `DispatchDmaCallback(0x8007C214)`.
4. `7C214` sets record status **2**, then **clears `B89F4`**.

Only after step 4 is the slot state-2, so only then can `7C484` return 0
and `91B64` return a body pointer.

E0 pump (`func_801924F8_port.c`) currently:

```
last_chunk = (B89F4 == 1);
if (last_chunk || ConsumeStreamPromote()) {
    if (last_chunk) NoteStreamFrameReady();  /* ONLY latch site today */
    func_8007C214();
} else {
    HostFB_PumpCdProgress();  /* can run 7C564→DMA→7C214 itself */
}
s1 = func_80191B64(...);
```

## 3. Early-publish race: can `91B64` return nonzero before `B89F4==1` / before `NoteStreamFrameReady`?

**Yes — and that is the live Disc1 failure mode.**

Hypothesis (port timing, not invented retail semantics):

Within a single `HostFB_PumpCdProgress` on the last sector, the host does:

`7C564` sets `B89F4=1` → DMA3 completes → IRQ services `7C214` → status=2,
`B89F4=0` — **before E0’s next `last_chunk` check**.

Then:

- E0 sees `B89F4==0` → does **not** call `NoteStreamFrameReady`.
- `91B64` finds state-2 → returns nonzero body (`s1`).
- `got_frame`: demuxed VLC is not immediate pad; `TakeStreamFrameReady()==0`
  → `Stage1b_pad_terminated_frame`.

So this is not “incomplete body published early” in the STR sense: state-2
still means last-chunk DMA completion. The port Stage-1b **latch is on the
wrong observer** (E0 seeing `B89F4`) instead of the completion site that
clears it (`7C214`).

Secondary paths with the same latch miss:

- `7C564` tail `C0DB8 && B89F4` → inline `7C214` (non-production selector).
- Any IRQ-delivered `7C214` outside E0’s `if (last_chunk)` arm.

E0’s own `if (last_chunk) Note…; 7C214()` path still works when E0 wins the
race (fixture / delayed IRQ). Live PumpCdProgress last-chunk loses it.

## 4. Smallest honest fix (hypothesis — do not invent retail)

| Option | Idea | Notes |
|--------|------|--------|
| **A (preferred)** | In `func_8007C214`, if `B89F4==1` before clear, call `PE_Port_NoteStreamFrameReady()` after successful publish (after chain-dead path that clears the flag) | Latch at true last-chunk completion. Fixes IRQ/Pump path. Idempotent with E0’s existing Note. Fixture `ArmStreamPromote` without `B89F4` still relies on immediate pad (MV1d preserved). |
| B | After `PumpCdProgress`, if a state-2 appeared, Note | More heuristic; duplicates completion meaning. |
| C | Reject nonzero `s1` unless latch/pad (already done) | Gate is correct; latch never set → permanent Stage-1b stop. |
| D | Require live `B89F4` at `got_frame` | **Wrong** for IRQ path: flag already cleared when body is ready. |
| E | Wait for last-chunk before allowing `7C484` success | Would mean changing retail `7C484`/`7C214` contract; heavier than A. |

**Recommended next port change: Option A.**

Keep E0’s `Note` on observed `B89F4` (harmless duplicate) or drop it later
once A is proven on Bazzite — not required for correctness.

## 5. Draft patch

Applied on branch `analysis/day2-158i-race` (uncommitted): latch inside
`func_8007C214` when clearing a true last-chunk `B89F4==1`.

Files touched (draft): `pc_port/platform/pe_libcd.c` only.

**Not committed / not claimed tested on live Disc1.** Unit implication:
`CDQ1_7C214_publish` uses `B89F4=0xA5A5A5A5` (not `1`) so it must not latch;
only exact `==1` matches E0’s last-chunk predicate.

## Evidence map

| Claim | Evidence |
|-------|----------|
| `7C484` returns 0 iff state-2→4 + body publish | `func_8007A214_port.c:147-180` |
| `91B64` returns `MV1_SP16` body | `func_80191B64_port.c:29-30,57,153` |
| `7C564` sets `B89F4` on last; status 3 then DMA | `cd_stream_port.c:470-493` |
| `7C214` publishes state 2, clears `B89F4` | `pe_libcd.c:450-467` |
| Production `C0DB8==0` → DMA IRQ not inline 7C214 | `pe-b54kao-stream-completion-selector` |
| Pump services CD + IRQ in one tick | `host_framebuffer.c:181-192`, `127-131` |
| Latch only on E0-observed B89F4 | `func_801924F8_port.c:178-184,259-269` |
| got_frame Stage-1b gate | same file `got_frame` |

## Non-claims

- Not claiming retail CPU/IRQ interleaving bit-exact with host Pump.
- Not claiming Stage-1b complete on live Disc1 until Bazzite retest.
- Did not modify `92934`/`92CE8`.

## Draft verification (local, not Disc1)

Uncommitted working tree on `analysis/day2-158i-race` built
`pe-native-tests`. Focused filters all PASS:

- `CDQ1_7C214_publish`, `CDQ1_7C214_chain_boundary`
- `B54K_C89C_gated_until_pad`, `B54K_C89C_last_chunk_frame_ready`
- `B54KAD_fmv2_filename_threshold`, `B54KAE_movie_state_setup`
- `DAY2_movie_player`

No commit. Live Bazzite Disc1 retest still required to confirm the
Stage-1b stop is gone.
