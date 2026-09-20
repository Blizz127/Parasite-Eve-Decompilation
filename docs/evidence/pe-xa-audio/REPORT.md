# PE-XA-AUDIO — CD-XA ADPCM decode and host playback

Branch `agent/xa-audio` from `a598ddd0`. Scope: the XA portion of the audio
gap only.

## Why this exists

`agent/audio-spu` gave the port a host sink and a 24-voice SPU mixer, and
`agent/music-keyon` gave it retail SPU init — but then proved the retail main
EXE has **no gameplay KON writer**: the music-start consumers manipulate
score/instrument structures and write voice parameters, never keying a voice.
Audible FMV/music audio therefore has to come from **CD-XA ADPCM**, which the
drive's CXD1199 decoder plays in hardware. The port previously served every CD
read mode from the raw sector but did nothing with XA audio.

## What was added

`pc_port/platform/pe_xa.{h,c}` — a host CD-XA decoder and resampler:

- **4-bit XA-ADPCM**: the standard sound-group layout (18 groups of 128 bytes
  per Form-2 sector; 8 sub-blocks per group with a shift/filter parameter byte
  each; sample nibbles packed four bytes per sample index and selected by
  sub-block). Filter/shift tables are the psx-spx values
  `pos {0,60,115,98,122}` / `neg {0,0,-52,-55,-60}`, prediction
  `(p0*fp + p1*fn + 32)/64`, shift clamp `>12 -> 9`, 16-bit clamp.
- **8-bit XA-ADPCM**: four 28-byte blocks per group (one parameter byte + 28
  signed samples each).
- **Sector classification and decode**: a raw 2352-byte sector is XA audio when
  the Form-2 subheader submode has bit 2 (`0x04`); the coding-info byte selects
  stereo/mono, 37800/18900 Hz and 4/8-bit. The 18 groups at offset 24 are
  decoded into a stereo PCM ring at the source rate.
- **Resampling to the 44100 Hz sink** with linear interpolation (the hardware's
  4-point gaussian interpolation is a documented gap).

Wiring:
- `pe_cdreg.c` calls `PE_Xa_ConsumeSector(g_sector)` on every raw sector read —
  the drive's decoder plays XA independent of the CPU/BFRD path.
- `pe_spu.c`'s `PE_Spu_RenderVBlank` mixes the XA ring into the vblank buffer
  before the sink write, so XA and SPU voices share the one output stream.
- `port_main.c` resets the decoder at startup and prints an `[XA]` diagnostic.

## Verification

Unit vectors (`pc_port/tests/test_xa.h`, 6 tests) hand-compute the decode:
shift-0 nibble scaling, the filter-1 prediction (`4096*60/64 = 3840`), signed
nibble `0xF -> -4096`, shift-1 halving, 8-bit `1 -> 0x0100` / `-1 -> -0x0100`,
sector audio classification, and a non-silent resample.

```
PE_TEST_FILTER=XA ./pc_port/build/pe-native-tests
  Results: 1391 run, 6 passed, 0 failed, 1385 skipped
./pc_port/build/pe-native-tests
  Results: 1391 run, 1391 passed, 0 failed, 0 skipped
ctest
  100% tests passed, 0 failed out of 11
```

Live opening FMV (real Disc 1, no `--skip-movie`):

```
PE_AUDIO_WAV=/tmp/pe_xa.wav ./pc_port/build/parasite-eve-port \
  --disc-image "<Disc1>.bin" --headless --max-frames 1500 --trace /tmp/xa_live.log
[SPU] reg_writes=220 key_ons=24 key_offs=24 active=0
[XA]  sectors=399 frames=804384 channels=2
WAV: 588735 frames, peak 19491, RMS 1283, 466788/1177470 non-zero
```

`399 sectors * 2016 frames = 804,384` matches the 4-bit stereo layout exactly.
The skip-movie `--route-pad` run decodes **0** XA sectors (field music is not
XA on the disc's stream path), which is expected and not a regression.

## Honest limits (no retail audio golden)

- Linear resampling, not the hardware gaussian interpolation.
- 8-bit layout is implemented from the standard four-28-byte-block
  interpretation; only 4-bit stereo was exercised live (the FMV case).
- Mono/18900 Hz paths are implemented but not live-exercised.
- Volume/emphasis and the CD `ATV0..3` mix matrix are not applied to XA.
- The matching build is untouched (no `src/func_*.c` or `configs/` changes):
  699 leaves, EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
