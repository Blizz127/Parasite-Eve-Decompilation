# Host SPU synthesis + score fold (AUD1-E0 … E6 fold)

Stage AUD1-E0 adds host ADPCM synth + PulseAudio. E1 adds the guest→SPU
bridge. E2 adds matching-intent `85F74`/`862F4` and host `8DB7C`. This fold
adds host semantic ports for Akao callees that were still stubs (E3–E6 scope):
`87AA8`/`87FA0`, `89328`, `89724`, `89784`, plus ApplyPending pitch bit
`0x10`. Remote Decomp Bot tip was still E2 at fold time; donors are khasinski
matching/candidate C (not byte-match).

Mix-exact reverb and bytecode sample loaders remain open. **No Day2-complete
claim.**

## What is now audible (when enabled)

- `pe_spu_synth.c` / `pe_host_audio.c` as in E0.
- `PE_SpuScore_ApplyDirtyVoices` publishes volume, **pitch (`0x10`)**, start,
  ADSR, key-on/off pending bits when `D_8009D2C4` bit `0x100` is set.
- Host `func_8008DB7C` advances tempo/timers/slides and runs:
  - `87AA8`/`87FA0` voice-register slides (dirty → synth publish)
  - `89328` voice-queue scaffolding (`8900C`/`89250` still no-op)
  - `89784` key-off flush via `89724` mask compose
  - command drain via `8CA84`

Set `PE_AUDIO_DISABLE=1` to force silent host output.

## What remains silent / unported

- `8E8D0` / `8F0D0` — sample/bytecode step (seq opcode tables)
- `8D844` — SPU_StepReverbLoad
- `8900C` / `89218` — StepVoiceNote (and related small leaf)
- `89250` UpdateVoiceEnvelopes is hosted (ENVX → voice free)
- ADSR envelopes fidelity, Gaussian interpolation, reverb/type-5 wet path
- Full-tree EXACT SHA-1 for matching leaves

## Verification

- `PE_TEST_FILTER=DAY2_spu` — mode/dma/synth/voice_bridge (5 PASS)
- `PE_TEST_FILTER=DAY2_akao_tick` — tempo/slides + 87AA8 dirty + pitch `0x10`
- Windowed `parasite-eve-port` may log `[AUDIO] host PCM output enabled`;
  audible output still needs retail sample data in SPU RAM (not claimed by
  RAM-only tests).
