# func_80067730

- **VRAM**: 0x80067730
- **File offset**: 0x57F30 (size 0x70)
- **Unit**: 5747C
- **Build profile**: era_o2_g0
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave3-b, agent/wave3-b)

## Behaviour
Sets/clears bit 3 (0x08) then stores (0x10000-arg2)>>8 and (0x10000-arg3)>>8 to +0x1C/+0x1E. Returns 0. Shift operands unsigned for retail srl.

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
