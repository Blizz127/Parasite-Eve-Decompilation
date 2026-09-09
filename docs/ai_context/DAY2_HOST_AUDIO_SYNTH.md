# Host SPU synthesis + score fold (AUD1-E0 … E8 fold)

Stage AUD1-E0 adds host ADPCM synth + PulseAudio. E1 adds the guest→SPU
bridge. E2 adds matching-intent `85F74`/`862F4` and host `8DB7C`. E3–E7 fold
score callees into the host tick. E8 adds semantic `878F0`
(Akao_WriteVoiceParam) + `89F08` (ENVX) with ApplyPending / `89250` wiring.
Remote Decomp Bot tip was still E2 at fold time; donors are khasinski
matching/candidate C (not byte-match).

Mix-exact reverb and bytecode sample loaders remain open. **No Day2-complete
claim.**

## What is now audible (when enabled)

- `pe_spu_synth.c` / `pe_host_audio.c` as in E0.
- `PE_SpuScore_ApplyDirtyVoices` publishes volume, **pitch (`0x10`)**, start,
  ADSR (via `878F0`), key-on/off pending bits when `D_8009D2C4` bit `0x100`
  is set.
- Host `func_8008DB7C` advances tempo/timers/slides and runs:
  - `87AA8`/`87FA0` voice-register slides (dirty → synth publish)
  - `89328` voice-queue (`8900C` StepVoiceNote + `89250` envelopes via `89F08`)
  - `89784` key-off flush via `89724` mask compose
  - command drain via `8CA84`

Set `PE_AUDIO_DISABLE=1` to force silent host output.

## What remains silent / unported

- `8E8D0` / `8F0D0` — sample/bytecode step (seq opcode tables)
- `8D844` — SPU_StepReverbLoad
- `88344` — Akao_SetVoiceKeyOff (named empty stub; flush is `89784`)
- ADSR envelopes fidelity beyond register publish, Gaussian interpolation,
  reverb/type-5 wet path
- Full-tree EXACT SHA-1 for matching leaves

## Host vs khasinski flag conflicts (ApplyPending)

| Bit | Host bridge | khasinski VoiceParams |
|-----|-------------|------------------------|
| `0x80` | legacy pitch from `+0x44` | START_ADDR |
| `0x400` | start publish | RELEASE_MODE |
| `0x1000` / `0x2000` | key-on / key-off | DECAY / SUSTAIN_RATE |

Safe ADSR bits routed through `878F0`: `0x100|0x200|0x800|0x4000|0x8000|0x10000`.

## Verification

- `PE_TEST_FILTER=DAY2_spu` — mode/dma/synth/voice_bridge (5 PASS)
- `PE_TEST_FILTER=DAY2_akao_tick` — tempo/slides + 87AA8 + pitch `0x10` +
  E7 StepVoiceNote + E8 WriteVoiceParam/ENVX
- Windowed `parasite-eve-port` may log `[AUDIO] host PCM output enabled`;
  audible output still needs retail sample data in SPU RAM (not claimed by
  RAM-only tests).
