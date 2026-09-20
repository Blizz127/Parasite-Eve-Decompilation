# PE-AUDIO-SPU — host audio sink + SPU voice mixer

Branch `agent/audio-spu` from `abcdc2bd`.  No retail audio golden exists, so
this is **psx-spx semantics + unit vectors**, not claimed retail parity.

## What was added

| file | role |
|---|---|
| `pc_port/platform/pe_audio.h/.c` | host sink abstraction: NULL, RIFF/WAVE (16-bit stereo 44100 Hz, sizes patched per block), and a `PE_AUDIO_HAVE_LIVE` stub that reports unavailable when no audio dev library is present |
| `pc_port/platform/pe_spu.h/.c` | 24-voice mixer: SPU-ADPCM decode, pitch counter, linear interpolation, shift/step ADSR, voice/main volume, SPUCNT gate, sink output |
| `pc_port/tests/test_audio.h` | 6 unit tests (ADPCM shift0/filter1, 16-bit blocks, voice mix + release, SPUCNT gate, WAV sink) |
| `pe_spu_dma.c` | `PE_SpuRegister_StoreU16` now calls `PE_Spu_OnRegisterWrite` (key-on/off edges); added host `PE_SpuRam_StoreU8` seed |
| `pe_libetc.c` | `PE_Sdk_ResetState` also resets the voice model |
| `host_framebuffer.c` | renders one vblank (735 samples) per presented frame via `HostFB_ServiceAudio`, chasing `fb_presented` so both present paths stay at one render each |
| `port_main.c` | `PE_Audio_InitFromEnv` (`PE_AUDIO_WAV=<path>`), `PE_AUDIO_SELFTEST=1`, shutdown flush + `[SPU]` diagnostics |
| `CMakeLists.txt` | builds `pe_spu.c` / `pe_audio.c` |

## psx-spx-verified

- **SPU-ADPCM block**: 16 bytes = shift/filter byte, flag byte, 28 nibbles
  (LSB = 1st sample).  Flags bit0 Loop-End, bit1 Loop-Repeat, bit2 Loop-Start.
- **Prediction**: `s = sign_extend4(nibble << 12) >> shift;
  s += (prev1*f0 + prev2*f1 + 32) >> 6` with the pos tables
  `f0={0,60,115,98,122}`, `f1={0,0,-52,-55,-60}` and neg tables
  `{0,0,-60,-115,-98}` / `{0,0,0,52,55}` selected by `prev1 >= 0`.
- **Pitch**: `0x1000 = 44100 Hz`, Step clamped `>0x4000 -> 0x4000`.
- **ADSR**: shift/step attack (linear/exponential), exponential decay to
  `(sl+1)*0x800`, sustain direction/mode, release to zero.
- **Volumes**: fixed mode `bit15=0`, 15-bit signed ×2; main volume likewise.
- **Control**: SPUCNT bit15 enable + bit14 unmute gate the mix; key-on clears
  ENDX, loop-end sets it; End+Mute forces release.

## Explicit gaps (not hidden)

- 4-point **gaussian interpolation** — linear interpolation is used instead
  (exact at integer pitch positions).
- Volume **sweep mode** (bit15=1) is accepted but not swept.
- Noise mode, pitch modulation, reverb are not mixed.
- **XA-ADPCM is not decoded**; the CD `CD_device_read_mode` bit4 boundary stands
  unchanged (no faked CD mode).

## Commands and results

```
cmake -S pc_port -B pc_port/build && cmake --build pc_port/build -j
./pc_port/build/pe-native-tests      -> 1383 run / 1383 passed / 0 failed / 0 skipped
ctest (pc_port/build)                -> 11/11 passed
```

Live artifact (real Disc 1, `--headless --route-pad --max-frames 6000`):

```
PE_AUDIO_WAV=/tmp/pe_audio_route2.wav ./pc_port/build/parasite-eve-port \
    --disc-image "<Disc1>.bin" --headless --route-pad --max-frames 6000
[AUDIO] WAV sink '/tmp/pe_audio_route2.wav' (44100 Hz stereo s16)
[SPU] reg_writes=187 key_ons=0 key_offs=0 active=0
```

The live WAV is **silent** (peak 0).  Over the full Day-1 route
(`--max-frames 38000`, `PE_CARD=empty`) the guest still reports
`reg_writes=328 key_ons=0`.  The guest writes SPU mode/reverb registers but
**never keys a voice**: the score/instrument key-on path is not reached by the
translated code (the music-start consumers `func_8008A068`/`8AE94`/`8B040` are
not ported — see `DAY2_MUSIC_START_CONSUMERS.md`, which already states "does
not implement score interpretation or audible synthesis").  This is a
guest-side driver gap, not an audio-path gap.

Pipeline self-test (synthetic, clearly labelled; proves the decoded→mixed→sunk
path in a live run):

```
PE_AUDIO_WAV=/tmp/pe_selftest.wav PE_AUDIO_SELFTEST=1 \
  ./pc_port/build/parasite-eve-port --disc-image "<Disc1>.bin" \
  --headless --max-frames 30
[AUDIO] pipeline self-test: 1s synthetic ADPCM tone (not retail audio)
frames 66150, first-second peak 28670, nonzero 88200
```

## Non-claims

No retail audio parity, no hardware golden, no XA/CD-DA, no reverb/noise, no
gaussian interpolation.  `func_8007D1D4` (SPU init) and `func_8007DAE0` (SPU
register write) remain documented no-ops in `pe_libsnd.c`; they should be
re-implemented against the register file once the guest driver reaches key-on.
