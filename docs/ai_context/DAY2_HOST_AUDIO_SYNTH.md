# Host SPU synthesis + score fold (AUD1-E0 … E8)

Fold branch `cursor/audio-score-fold-4797` / PR #39 merges Decomp Bot tip
`98e0bf0` (`cursor/audio-score-leaves-85f74-862f4-8db7c`): matching guest leaves
under `src/` + tip host Akao path. Earlier host-only E3–E8 folds were replaced
by that tip. Movie/PR #38 untouched; movie/boot `92CE8`/`92934` not pulled.

**No Day2-complete claim.** Byte-match not claimed for the new score leaves.

## What is now audible (when enabled)

- `pe_spu_synth.c` / `pe_host_audio.c` as in E0 (ENVX ADSR machine; mix still
  unscaled).
- `PE_SpuScore_ApplyDirtyVoices` publishes volume, pitch (`0x10` from
  `+0x30`/LFO), start, ADSR1/ADSR2, key-on/off when `D_8009D2C4` bit `0x100`
  is set (prefers HW index at `voice+0xF0`).
- Host `func_8008DB7C` runs tip ports: `87AA8`/`87FA0`, `8E8D0`, `89328`
  (`8900C`/`89250`), `8D844`/`89784`, WriteVoiceParam/`89F08`, then command
  drain via `8CA84`.

Set `PE_AUDIO_DISABLE=1` to force silent host output.

## What remains open

- `88344` Akao_SetVoiceKeyOff — named empty stub (flush is `89784`)
- Gaussian interpolation, reverb/type-5 wet path, mix-scaled ENVX
- Full-tree EXACT SHA-1 for new matching leaves (retail EXE + jtbl pool)

## Verification

- `PE_TEST_FILTER=DAY2_spu` — mode/dma/synth/voice_bridge (5 PASS)
- `PE_TEST_FILTER=DAY2_akao_tick` — tempo/slides + 87AA8 + pitch `0x10` +
  tip `8900C` + E8 ADSR/ENVX
- `scripts/verify_us.sh --public` — YAML matching-C count 578
- Windowed `parasite-eve-port` may log `[AUDIO] host PCM output enabled`;
  audible output still needs retail sample data in SPU RAM (not claimed by
  RAM-only tests).
