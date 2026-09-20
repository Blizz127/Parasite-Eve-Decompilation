# PE-FMV-MEDIA-LOOP — opening FMV plays to its true end and the game leaves the movie state

Branch `agent/fmvloop`, base `ec097017`. Scope: the guest-side completion of the
title/opening-FMV media loop (`func_80192CE8` post-E08 → `func_80192934` →
`func_80191B64` → `func_8010C89C`, CD reader `func_8007C564`). Only `pc_port/`
changed, so the retail matching build is untouched.

## Outcome

The real Disc 1 opening movie (FMV001, record limit 2077) now decodes every
frame, the retail completion latch latches at the true retail moment, the guest
runs its own abort teardown, and the post-E08 media loop clears. The old
`CD_B0CD0_pending_unresolved` stop is gone; the run now stops at a genuinely
different, named downstream boundary (`MDEC_missing_block`).

| | before | after |
| --- | --- | --- |
| `func_80192934_enter` | 319 | **2078** (frames 1..2077 + final) |
| FMV end / latch | never | `func_80192934_dbd_abort` + `func_80192CE8_media_clear` |
| stop | `CD_B0CD0_pending_unresolved` | `MDEC_missing_block` (new) |
| vsyncs / presents / mask | 117143 / 801 / 1 | 735834 / 2565 / 0 |
| XA | 399 sectors / 804384 frames | **2600 sectors / 5241600 frames** |
| WAV peak / nonzero | 19491 / 466788 of 1177470 | **31766 / 3053062 of 3770550** |

The new run transitions through the movie teardown (`Pause` issued at LBA
210535, the byte just past FMV001) and only then stops, at `MDEC_missing_block`,
on the *next* stream the title tail opens. That is a real downstream frontier,
not the media loop.

## Root cause 1 — `func_8010C89C` CA7C zero-first-table escape kept a stale `at`

The VLC decoder's main loop looks the symbol up in the first table at CA7C.
When that word is zero it takes the CA98 escape: shift in eight more bits, look
up the **second** table at `a3`, then shift `v0` by the second word's low byte.
`CAD4` is the delay slot of the `b CADC` rejoin and re-reads `at = t1 & 0xFF`
from that second word:

```
8010ca88: lw    t1,0(t0)
8010ca90: bnez  t1,0x8010cad8
8010ca94: andi  at,t1,0xff      ; delay: at from the FIRST (zero) word
...
8010cac8: lw    t1,0(t0)        ; SECOND table word
8010cacc: add   t3,zero,zero
8010cad0: b     0x8010cadc
8010cad4: andi  at,t1,0xff      ; delay: at RE-READ from the second word
8010cad8: lw    t3,4(t0)
8010cadc: sllv  v0,v0,at
```

The old transcription computed `at` once before the branch and reused it, so on
frames that take the escape the decoder shifted `v0` by 0 and desynchronised the
bitstream. It kept consuming input past the frame and emitted ~2x the declared
RLE extent. On FMV001 frame 319 that was 101510 bytes against a declared 51076;
the output buffer `[0x801D0DEC] = A+0xFA00` then overflowed into the record pool
at `A+0x1F400`, overwrote a stream-record slot, and the next frame's reader saw
an occupied record (`func_8007C564` status-4 arm), skipped the sector, exhausted
its 2000-poll, and entered the CD reissue path whose queued Pause stranded one
unread sector → the named stop.

The bug only affects frames whose data uses the two-level table escape; existing
`MV1D_*` vectors and the complete-frame frames 1–3 never took that arm, which is
why it survived. The independent oracle model in
`pc_port/tools/pe_mv1d_c89c_oracle.py` had replicated the same misreading and is
corrected with it.

## Root cause 2 — `func_80191B64` C44 never latched on reaching the record limit

The completion latch `D_801D0DBD` is written by `func_80191B64`'s C44 block.
Retail:

```
80191c58: sltu  v0,v1,v0        ; v0 = (w < D_801D11B0)
80191c5c: bnez  v0,0x80191c84
80191c60: li    v0,1            ; delay
80191c64: ...                   ; rec = D_801D11AC
80191c78: sltu  v0,v1,v0        ; v0 = (w < rec[8])
80191c7c: bnez  v0,0x80191c8c   ; still below the limit -> skip the store
80191c80: li    v0,1            ; delay
80191c84: lui   at,0x801d
80191c88: sb    v0,3517(at)     ; D_801D0DBD = 1
80191c8c: ...
```

So the flag latches when `w < lim` (frame number regressed) **or** `!(w < rh)`
(reached/exceeded the record frame limit) — exactly the player twin's
regress-or-limit rule. The old transcription only latched on the regression
(`v0 = (w < rh) ? 1 : 0` and no store), so an in-order stream that simply runs
out of frames never completed. Live: `w=2077`, `lim=2076`, `rh=2077`, latch
stayed 0 and the reader ran past EOF to LBA 210535+ forever.

## Changes

- `pc_port/game/boot/func_8010C89C_port.c` — re-read `at = t1 & 0xFF` from the
  second-table word in the CA98 escape arm.
- `pc_port/game/boot/func_80191B64_port.c` — C84 fall-through store: latch
  `D_801D0DBD` when `!(w < rh)`.
- `pc_port/tools/pe_mv1d_c89c_oracle.py` — model corrected with the same re-read
  (existing vectors unchanged).
- `pc_port/tests/test_movie_production.h` + `pc_port/tests/test_native.c` — two
  non-vacuous real-disc regressions:
  - `DAY2_c89c_second_table_escape`: decodes real FMV001 frame 319 and asserts
    the declared 51076-byte extent plus a canary immediately past it. Pre-fix the
    decoder writes ~101510 bytes and clobbers the canary.
  - `DAY2_91b64_record_limit_latch`: drives `func_80191B64` with a controlled
    record. `w<rh` must leave the flag clear; `w==rh` must latch. Pre-fix the
    second case fails.

Both tests were measured failing with their fix reverted (see the "reverted"
runs below) and passing with it.

## Verification (all in `/tmp/pe-agent-fmvloop`)

```text
cmake --build pc_port/build -j$(nproc)                      # exit 0
ctest --test-dir pc_port/build --output-on-failure          # 11/11 passed, 62.15s
./pc_port/build/pe-native-tests                             # 1396/1396, 0 failed, 0 skipped
python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans   # check: OK
```

Non-vacuous discrimination:

```text
DAY2_c89c_second_table_escape, fix reverted:
  FAIL: C89C wrote past the declared RLE extent
DAY2_91b64_record_limit_latch, fix reverted:
  FAIL: record-limit frame did not latch the end flag
```

Live opening FMV, real Disc 1, headless, no `--skip-movie`:

```sh
PE_AUDIO_WAV=/tmp/pe_fmv_after.wav PE_CARD=build/pe_card1.mcr \
  ./pc_port/build/parasite-eve-port \
  --disc-image "$(cat local/pe_disc1.path)" --headless --max-frames 90000 \
  --trace /tmp/fmv_after.log
```

```text
[STUB:BOOTSTRAP_RET] MDEC_missing_block (first invocation)
[FB] vsyncs=735834 drawsyncs=3529 presents=2565 mask=0 main_iters=1
[HOST] stop_reason=unresolved-boundary
[SPU] reg_writes=220 key_ons=24 key_offs=24 active=0
[XA] sectors=2600 frames=5241600 channels=2
```

Trace markers: `2080 func_80192934_dbd_abort`, `2081 func_80192CE8_media_clear`;
2078 `func_80192934_enter`; zero `CD_B0CD0_pending_unresolved`.

WAV: 1,885,275 stereo frames at 44100 Hz, peak 31766, RMS 4288.8,
3,053,062 / 3,770,550 samples non-zero.

Only `pc_port/` changed (`src/` and `configs/` untouched), so the retail SHA-1
remains `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; the distrobox rebuild was not
required.

## Residual

The stop is now `MDEC_missing_block` on the stream the title tail opens *after*
the media loop clears. That is downstream of this task's frontier and is not
approximated here. No completion flag, story value, frame count, or decoded
pixel is faked anywhere; both fixes are transcription corrections against the
carved overlay disassembly (`asm`-equivalent PE.IMG carves, reproduced in
`/tmp/ov_19_1e.s` and `/tmp/movie_mod.s` during this rung).
