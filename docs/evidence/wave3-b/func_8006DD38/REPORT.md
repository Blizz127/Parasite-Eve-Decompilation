# func_8006DD38

- **VRAM**: 0x8006DD38
- **File offset**: 0x5E538 (size 0x94)
- **Unit**: 5E418
- **Build profile**: era_o2_g0_aspsx_230
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave3-b, agent/wave3-b)

## Behaviour
Projects (x,y,z) via func_8006DFA8 then queues the index-selected sound from the D_800B0CE8 arena when enabled. Returns func_80086608 result or 0.

## Method
Read the `glabel`..`endlabel` block in `asm/disc1/5E418.s`, the
`pc_port` spec where present, and nearby matched `src/func_*.c`. Wrote the
C leaf, confirmed word-exact with `tools/analysis/try_leaf.py`, then carved
`configs/USA/disc1.yaml` and re-ran `scripts/split_us.sh` + `scripts/build_us.sh`
+ `scripts/verify_us.sh` inside `pe-mipsel`.

## Evidence
Fresh complete retail build reported `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with the registered C leaf count
incremented (846 -> 855 across this wave). `VERIFY_US=PASS`.

## Divergences
none
