# Host SPU synthesis slice (AUD1-E0 / AUD1-E1)

Stage AUD1-E0 adds the first bounded host audio path on top of the existing
`pe_spu_dma.c` SPU-RAM image and register window. Stage AUD1-E1 adds a
**partial** guest voice→SPU register bridge (`func_80087798`, partial
`func_80085F74`, `PE_SpuScore_ApplyDirtyVoices`) wired into
`PE_Event_ServiceAudioCommands` after command drain. It does **not** port the
retail `func_8008DB7C` score bytecode sequencer, `func_800862F4` init, or
mix-exact reverb.

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

- Score bytecode interpretation (`8DB7C` / `8E4E8` voice ticks).
- Full `func_80085F74` instrument-table paths and `func_800862F4` boot init.
- ADSR envelopes, Gaussian interpolation, reverb/type-5 wet path.
- AKAO/seq26/27 music playback without score bytecode advancing sample
  addresses each tick.

## Verification

- `PE_TEST_FILTER=DAY2_spu_synth` — ADPCM mix hash + `HostFB_VSync` hook.
- `PE_TEST_FILTER=DAY2_spu_voice_bridge` — guest voice dirty publish → key-on
  → non-silent mix hash.
- Windowed `parasite-eve-port` logs `[AUDIO] host PCM output enabled` when
  PulseAudio opens; audible output still requires retail sample data in SPU
  RAM and keyed voices (not claimed by RAM-only tests).
