# func_800676CC

- **VRAM**: 0x800676CC
- **File offset**: 0x57ECC (size 0x64)
- **Unit**: 5747C
- **Build profile**: era_o2_g0
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave3-b, agent/wave3-b)

## Behaviour
Sets/clears bit 2 (0x04) then stores arg2>>8 and arg3>>8 to halfwords +0x1C/+0x1E. Returns 0.

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
