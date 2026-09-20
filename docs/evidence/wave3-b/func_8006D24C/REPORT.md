# func_8006D24C

- **VRAM**: 0x8006D24C
- **File offset**: 0x5DA4C (size 0x6C)
- **Unit**: 5C6CC
- **Build profile**: era_o2_g0
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave3-b, agent/wave3-b)

## Behaviour
Clears D_800B0DB5/DB4/DB7/DB6/DB3/DB2 to -1, masks ~0xF0 out of D_800B0CD8, calls func_80086FF8.

## Method
Read the `glabel`..`endlabel` block in `asm/disc1/5C6CC.s`, the
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
