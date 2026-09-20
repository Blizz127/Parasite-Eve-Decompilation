# PE-XA-FIDELITY — gaussian resampling, 8-bit/mono/18900 verification, CD-XA volume chain

Branch `agent/xapolish` from `ec097017`. Scope: close named gaps (a) linear XA
resampling, (b) unverified mono/18900 Hz/8-bit paths, (c) unapplied XA/CD
volume.  (d) noise/pitch-mod/reverb stay out of scope.

## Algorithm sources

- **4-point gaussian interpolation + the 512-entry `gauss` table** — psx-spx,
  *Sound Processing Unit (SPU)*, "4-Point Gaussian Interpolation"
  (<https://psx-spx.consoledev.net/soundprocessingunitspu/>; raw
  <https://raw.githubusercontent.com/psx-spx/psx-spx.github.io/master/docs/soundprocessingunitspu.md>):
  `out = (gauss[0FFh-i]*oldest + gauss[1FFh-i]*older + gauss[100h+i]*old +
  gauss[i]*newest) SAR 15`, `i` = pitch-counter bits 4..11.  The table is
  normalised to 255/256 (each phase's four taps sum to `7F80h +/- 1`).
  Cross-checked entry-for-entry against DuckStation `src/core/spu.cpp`
  `GenerateInterpolationCoefficients()`
  (<https://github.com/stenzek/duckstation/blob/master/src/core/spu.cpp>),
  which uses the identical table and tap ordering.
- **CD controller ATV0..3 matrix** — psx-spx, *CDROM Drive*,
  `1f801802h (write, bank 2) ATV0 (L->L)`, `1f801803h (bank 2) ATV1 (L->R)`,
  `1f801801h (bank 3) ATV2 (R->R)`, `1f801802h (bank 3) ATV3 (R->L)`;
  `00h`=off, `80h`=unity, `FFh`=double, saturation applies up to double volume
  (<https://psx-spx.consoledev.net/cdromdrive/>).  `CHNGATV` = ADPCTL bit5.
- **SPU CD audio input volume / main volume** — psx-spx SPU,
  `1F801DB0h/DB2h` (signed 16-bit, `7FFFh` = unity) and `1F801D80h/82h`
  (fixed mode: bits 0..14 are volume/2), plus SPUCNT bit0 CD-audio-enable and
  bit14 mute.  Same application order as DuckStation `SPU::Execute()`
  (`ApplyVolume(x, cd_volume)` then the main volume).
- **8-bit XA sound-group layout** — DuckStation `CDROM::DecodeXAADPCMChunks()`
  (<https://github.com/stenzek/duckstation/blob/master/src/core/cdrom.cpp>):
  parameter bytes at `+4`, then 28 little-endian 32-bit words at `+16`; byte
  `N` of each word is sub-block `N`'s sample (corrected in this change — see
  "What changed").
- **Honest caveat (not implemented):** DuckStation's *CDROM* module models the
  37800->44100 XA upsampler as a 7-phase, 29-tap "zigzag" FIR and the 18900
  path with a separate Mednafen-derived interpolator — i.e. the CD-audio
  resampler is *not* documented by psx-spx as using the gaussian.  This change
  implements the gaussian as directed by the task ("the PS1 4-tap gaussian
  interpolation filter ... where the hardware applies it") and as the
  documented PS1 interpolation kernel; a measured-FIR XA resampler is noted as
  possible future refinement, not claimed here.

No retail audio golden exists; correctness is argued from psx-spx plus
signal-level unit vectors and the live-WAV before/after metrics below.  This is
**not** a claim of byte-exact retail audio.

## What changed

`pc_port/platform/pe_xa.{c,h}`:

1. **Gaussian resampling.**  Added the 512-entry `kGauss` table and
   `PE_Xa_GaussInterp(oldest, older, old, newest, index)` (sum-then-SAR-15,
   matching psx-spx and DuckStation).  `PE_Xa_Mix` now resamples the ring with
   it; the four taps are `s[tail-1..tail+2]`, so the output is centred on the
   source position.  The `double` fractional phase was replaced by an exact
   integer accumulator (`s_phase/44100`), removing float drift.  The ring
   keeps 4 frames of history headroom so the oldest tap is never the slot
   being overwritten.
2. **8-bit layout correction.**  `PE_Xa_DecodeGroup8` read data from
   `group[8 + blk*28 + i]`; the correct location is the byte `blk` of the
   32-bit word `i` at `group[16 + i*4 + blk]`.  The old offset read the
   header/unused region as PCM.
3. **CD-XA volume chain.**  `PE_Xa_Mix` now applies, per sink sample: the
   ATV0..3 matrix from `pe_cdreg` (>>7, saturated), the SPU CD audio input
   volume `1F801DB0h/DB2h` (>>15) and the SPU main volume `1F801D80h/82h`
   (fixed-mode decode).  The contribution is gated by SPUCNT bit0 (CD audio
   enable) and bit14 (mute).  Applying the main volume here (pe_spu already
   applies it to the voice mix) makes the composite `main*(voices +
   cd_gain*xa)` exact.  Retail `func_8007BAC0`
   (`pc_port/game/boot/cd_stream_port.c`) sets ATV `{80h,0,80h,0}`, CD volume
   `3FFFh` (~0.5) and SPUCNT `C001h`; the live FMV therefore drops ~6 dB, as
   the hardware does.

`pc_port/tests/test_xa.h`: 6 -> 14 tests (8 new), see "Tests".

`pc_port/tools/wav_metrics.py` (new): peak / RMS / non-zero-frame metrics for
the before/after WAV comparison.

## Before / after (live opening FMV, real Disc 1, headless 20000 frames)

```
PE_AUDIO_WAV=... PE_CARD=build/pe_card1.mcr parasite-eve-port \
  --disc-image "<Disc 1>.bin" --headless --max-frames 20000
```

| | sectors | frames decoded | peak | RMS | non-zero frames |
|---|---:|---:|---:|---:|---:|
| before (`ec097017`) | 399 | 804384 | 19491 | 1283.2 | 233712 / 588735 |
| after  (this branch) | 399 | 804384 | 11620 | 756.4 | 233689 / 588735 |

Same sector/frame decode, same non-zero coverage; the level change is the
retail CD input volume (`3FFFh` ~= 0.5) finally being applied.  The residual
difference between the measured peak ratio (0.596) and 0.5 is the gaussian
reconstructing source peaks better than the old linear interpolation at
37800 -> 44100.

## Tests

```
./pc_port/build/pe-native-tests            -> 1402 run / 1402 passed / 0 failed / 0 skipped
ctest --test-dir pc_port/build             -> 11/11 passed
python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans -> check: OK
```

New non-vacuous vectors (all 14 XA tests pass; 8 new):

- `XA_gauss_taps` — isolated-tap impulse responses against hand-computed
  `(gauss[k]*A)>>15` values (`gauss[0FFh]=12C7h`, `gauss[1FFh]=59B3h`,
  `gauss[100h]=1307h`, `gauss[0]=FFFFh`, and four `i=80h` entries).
- `XA_gauss_normalization` / `XA_gauss_step` — the four taps of every phase
  sum to `7F80h +/- 1` (constant input settles to `A*7F80h>>15`, no
  overshoot); the step response is monotone from silence to
  `gauss[0FFh]*A>>15`.
- `XA_adpcm_8bit_layout` — data at `+16` decodes; the `+8` byte does **not**;
  word byte 1 is sub-block 1.  Also `XA_adpcm_8bit_basic` /
  `XA_resample_8bit_stereo` updated to the corrected layout.
- `XA_resample_37800_mono`, `XA_resample_18900_mono`,
  `XA_resample_8bit_stereo` — decode frame counts (4032 / 4032 / 1008) and
  resampled output lengths (~4702 / ~9404 / ~1174 sink frames), both channels.
- `XA_volume_matrix` — identity/mono/zero ATV, zero CD volume, SPUCNT bit0/bit14
  gating, double ATV, half CD volume.

**Real-disc coverage scan.**  A raw scan of the whole Disc 1 image (210685
sectors, `submode & 04h` = XA audio, classified by `codinginfo`) finds **13452
XA audio sectors, every one of them 4-bit stereo 37800 Hz** (`ci` bit0=1,
bit1=0, bit2=0).  The mono, 18900 Hz and 8-bit paths therefore have **no
real-disc case on this title**; they are verified with the synthetic
psx-spx/DuckStation-layout vectors above.  The live FMV case (399 sectors) is
4-bit stereo 37800 and is covered by the before/after WAV.

**Fail-on-pre-change demonstration** (temporarily reverting each change and
rebuilding):

- legacy 8-bit offset restored -> `XA_adpcm_8bit_basic`, `XA_adpcm_8bit_layout`,
  `XA_resample_8bit_stereo` FAIL (3 failures).
- volume application made a no-op -> `XA_volume_matrix` FAILs ("mono matrix
  makes L == R").
- the gaussian function/table did not exist before, so `XA_gauss_*` are new
  coverage rather than regressions.

Retail rebuild after the change (in the `pe-mipsel` distrobox,
`bash scripts/build_us.sh`): **EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`**, "Matching claim: YES (709
registered C leaves)".

## Honest limits

- No retail audio golden: peak/RMS/impulse/step are signal-level evidence.
- The gaussian is psx-spx's documented interpolator; DuckStation's CDROM uses a
  measured FIR for the XA upsampler (documented above) — not implemented here.
- Emphasised (`codinginfo` bit6) XA is still not de-emphasised; noise,
  pitch-modulation and reverb remain unmodelled (gap (d)).
- SPU voice interpolation in `pe_spu.c` still uses linear; the task scoped (a)
  to the CD-XA resampler path.
