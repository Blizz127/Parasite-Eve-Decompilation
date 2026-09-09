# Host SPU synthesis slice (AUD1-E0)

Stage AUD1-E0 adds the first bounded host audio path on top of the existing
`pe_spu_dma.c` SPU-RAM image and register window. It does **not** port the
retail `func_8008DB7C` score sequencer, guest voice→SPU attribute writers
(`func_80085F74` / `func_800862F4` / `func_80085A64`), or mix-exact reverb.

## What is now audible (when enabled)

- `pe_spu_synth.c` decodes PSX ADPCM blocks from SPU RAM for keyed SPU
  voices (register key-on/off at offsets `0x188` / `0x18A` relative to
  `0x1F801C00`). Linear interpolation only (DEBT-SYS0-002).
- `pe_host_audio.c` submits mixed stereo s16 PCM at 44100 Hz through
  PulseAudio (`libpulse-simple.so.0` via dlopen) when a windowed run starts.
- `HostFB_VSync` mixes one NTSC frame quantum (`736` samples) after
  `PE_Event_ServiceAudioCommands`.

Set `PE_AUDIO_DISABLE=1` to force silent host output. Headless CI and builds
without PulseAudio continue to run; synthesis is still hashed in native tests.

## What remains silent / unported

- Score bytecode interpretation (`8DB7C` voice ticks).
- Guest voice attribute publication to SPU hardware.
- ADSR envelopes, Gaussian interpolation, reverb/type-5 wet path.
- AKAO/seq26/27 music start does not yet reach keyed SPU voices without the
  score bridge.

## Verification

- `PE_TEST_FILTER=DAY2_spu_synth` — ADPCM mix hash + `HostFB_VSync` hook.
- Windowed `parasite-eve-port` logs `[AUDIO] host PCM output enabled` when
  PulseAudio opens; audible output still requires retail sample data in SPU
  RAM and keyed voices (not claimed by RAM-only tests).
