# Host SPU synthesis slice (AUD1-E0 / AUD1-E1)

Stage AUD1-E0 adds the first bounded host audio path on top of the existing
`pe_spu_dma.c` SPU-RAM image and register window. Stage AUD1-E1 adds a guest
voice→SPU register bridge. Stage AUD1-E2 adds matching-intent `func_80085F74`
(SpuSetCommonAttr) / `func_800862F4` (SpuSetVoiceAttr), host `func_8008DB7C`
(Akao_Tick) with stubbed callees, and renames the pending-bit publisher to
`PE_SpuVoice_ApplyPending`. Mix-exact reverb and bytecode sample loaders remain
open.

## What is now audible (when enabled)

- `pe_spu_synth.c` decodes PSX ADPCM blocks from SPU RAM for keyed SPU
  voices (register key-on/off at offsets `0x188` / `0x18A` relative to
  `0x1F801C00`). Linear interpolation only (DEBT-SYS0-002).
- `pe_host_audio.c` submits mixed stereo s16 PCM at 44100 Hz through
  PulseAudio (`libpulse-simple.so.0` via dlopen) when a windowed run starts.
- `HostFB_VSync` mixes one NTSC frame quantum (`736` samples) after
  `PE_Event_ServiceAudioCommands`.
- `PE_SpuScore_ApplyDirtyVoices` publishes guest voice `+0xF4` pending bits
  (instrument-init `0x1FF80` cluster) to SPU registers when `D_8009D2C4` bit
  `0x100` is set (`audio_dirty`).

Set `PE_AUDIO_DISABLE=1` to force silent host output. Headless CI and builds
without PulseAudio continue to run; synthesis is still hashed in native tests.

## What remains silent / unported

- Akao_Tick callees (`89328` / `8E8D0` / `87AA8` / `87FA0` / `8D844` /
  `89784`) still stubbed on host — bank walks run but do not advance samples.
- ADSR envelopes, Gaussian interpolation, reverb/type-5 wet path.
- Full-tree EXACT SHA-1 for the new matching leaves (needs retail EXE + jtbl
  pool strip).

## Verification

- `PE_TEST_FILTER=DAY2_spu_synth` — ADPCM mix hash + `HostFB_VSync` hook.
- `PE_TEST_FILTER=DAY2_spu_voice_bridge` — guest voice dirty publish → key-on
  → non-silent mix hash.
- Windowed `parasite-eve-port` logs `[AUDIO] host PCM output enabled` when
  PulseAudio opens; audible output still requires retail sample data in SPU
  RAM and keyed voices (not claimed by RAM-only tests).
