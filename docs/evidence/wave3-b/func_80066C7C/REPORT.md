# func_80066C7C

- **VRAM**: 0x80066C7C
- **File offset**: 0x5747C (size 0x6C)
- **Unit**: 5747C
- **Build profile**: era_o2_g0
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave3-b, agent/wave3-b)

## Behaviour
Window-clear sibling of func_80066BD8: snapshot D_800BCFE8/EA/EC into F0/F2/F4, zero the window, mode byte D_800BCFEE=6, arg0 into signed halfword F6, F8=0; returns 0.

## Method
Read the `glabel`..`endlabel` block in `asm/disc1/5747C.s`, the
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
