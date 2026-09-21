# func_8004FA10

- **VRAM**: 0x8004FA10
- **File offset**: 0x40210 (size 0xE8)
- **Unit**: 401A0
- **Build profile**: era_o2_g8 (`-O2 -G8`)
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave5-c, agent/wave5-c)

## Behaviour
Battle-start setup. func_80062A34(1,7) yields the primary record; a second
record func_80063428(func_80062A34(2,0x36)) feeds func_80052E30(... == 1).
When the primary record exists, its +0x48 word is zero and
func_80063428(func_80062A20(rec,0)) >= 0, it plays
func_80059EC8(1, func_800556E8(func_80063428(func_80062A34(2,7)))).
Then stores func_8005332C(func_80059F08(1)) into the gp slot 0x1B0
(D_8009CF20), calls func_800647D0(arg0, *(u8 *)(rec+0x14)), records arg0 in
gp 0x184 (D_8009CEF4), and returns func_800638D8(arg0, func_80050AD8).

## Method
Same era_o2_g8 small-data model as the sibling `func_800509E0`:
D_8009CF20/D_8009CEF4 are scalar declarations so they emit `0x1B0($gp)`/
`0x184($gp)`. One residual: the +0x14 field of the func_8005332C result is a
**byte** load in retail (`lbu $a1,0x14($v0)`), so the call argument must be
`*(unsigned char *)((char *)temp_v0 + 0x14)`, not `temp_v0[5]`.

## Evidence
Fresh complete retail build: `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (874
registered C leaves)`, `VERIFY_US=PASS`.

## Divergences
None.
