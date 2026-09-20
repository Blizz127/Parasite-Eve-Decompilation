# PE-MUSIC-KEYON — SPU init translated; main EXE has no gameplay KON write

Branch `agent/music-keyon` from `ca698c96`. Scope: `func_8007D1D4` (SPU init) /
`func_8007DAE0` (SPU register write) and the hoped-for music key-on path.

## What was implemented (retail-transcribed)

`pc_port/platform/pe_spu_init.c`, from `asm/disc1/6D874.s` + `6E4AC.s`:

| function | bytes | role |
|---|---:|---|
| `func_8007DCAC` | 0x58 | SPU busy-wait (60 iterations of a x13 multiply; no MMIO) |
| `func_8007D454` | 0x1C0 | SPU transfer-port write (0x1A6 address, 0x1A8 data, 0x1AA/0x1AE handshake) |
| `func_8007D1D4` | 0x280 | SPU hardware init; arg 0 = full reset |
| `func_8007DAE0` | 0x44 | SPU register write through the `D_8009B3FC` base |

Every retail `sh/lhu off($base)` becomes `PE_SpuRegister_StoreU16/LoadU16`, and
the `D_8009B40C` control word (loaded as `0x1F8010F0`, DMA DPCR) goes to
`PE_GPU_WriteDPCR`.  `func_8007D054` (SsInit) now calls `func_8007D1D4(0)` and
`func_8007DAE0(0xD1, D_8009B46C, 0)` instead of the collapsed no-ops.

`func_8007D1D4(0)` is the full reset: clear main volumes + SPUCNT, program the
transfer address, write the 24 voice defaults (vol 0, pitch `0x3FFF`, start
`0x200`, ADSR 0), then the **all-voice KON/KOFF reset pulse**, then
`SPUCNT = 0xC000`.

## Key finding: the main EXE never keys a voice after init

A full scan of `asm/disc1/*.s` (all instruction operands, plus a per-function
scan of every store whose base register was loaded from `D_8009B3FC`) finds
**exactly one** writer of the KON register (`0x188`/`0x18A`):

```
func_8007D1D4 (6D874.s): sh 0xFFFF -> 0x188 ; sh 0x00FF -> 0x18A   (reset pulse)
```

No gameplay, music or streaming function writes KON.  The music-start consumers
(`func_8008A068`/`func_8008AE94`/`func_8008B040`) manipulate guest-side score,
voice-bank and instrument structures (`func_8008F1B0`, `func_80089F58`, …) and
call the SDK voice-parameter writers (`func_80087798`, `func_8008780C`, …),
which write voice volume/pitch/ADSR — **not** KON.  `func_80087798` is the only
game-side function that computes the SPU base inline (`0x1F801C00`), and it
writes only voice volume.

Conclusion: Parasite Eve's main-EXE music path is a **score/instrument data
driver**, not an SPU key-on generator.  Audible music must come from either
XA/CD-DA streaming (CD `CdlModeSM` bit4, still an explicit boundary) or an
SPU driver inside a runtime overlay (PE.IMG), neither of which is in the
main-EXE asm covered here.

## Bug found and fixed

`pe_spu.c`'s `PE_Spu_OnRegisterWrite` walked all 16 bits of the KON/KOFF **high**
halfword as voices 16..31, but only 24 voices exist.  The retail init writes
`0xFFFF` to both KOFF halves, so the model indexed `g_voices[24..31]`
(out of bounds) and inflated `key_offs`.  Now bounded by `PE_SPU_VOICE_COUNT`.

## Verification

```
cmake -S pc_port -B pc_port/build && cmake --build pc_port/build -j
./pc_port/build/pe-native-tests   -> 1385 run / 1385 passed / 0 failed / 0 skipped
ctest (pc_port/build)             -> 11/11 passed
```

New test `DAY2_spu_init_keyon`: after `func_8007D1D4(0)`, `key_ons == 24 &&
key_offs == 24`, `SPUCNT == 0xC000`, voice 0 pitch `0x3FFF`, start `0x200`.

Live real Disc 1, `--headless --route-pad --max-frames 42000`:

```
PE_AUDIO_WAV=/tmp/pe_music2.wav ./pc_port/build/parasite-eve-port \
    --disc-image "$(cat local/pe_disc1.path)" --headless --route-pad --max-frames 42000
[SPU] reg_writes=541 key_ons=24 key_offs=24 active=0
WAV frames 28392315 peak 0
```

`key_ons` moved from **0 to 24** (the init reset pulse), so the requested
`key_ons > 0` criterion is met — but the WAV is still **silent**: the retail
reset keys the voices with volume 0 and immediately keys them off, and no
gameplay path keys a voice with nonzero volume.  Non-silent output therefore
needs a different subsystem (XA decode or an overlay driver), not this path.

## Non-claims

No audible music, no retail audio golden, no XA/CD-DA, no claim that the
score/instrument path produces sound.  The SPU init translation is exact
retail behavior; the silence is a property of the retail program, not a
missing host path.
