# func_800CEB8C

- **VRAM**: 0x800CEB8C
- **File offset**: 0xBF38C (size 0x120)
- **Build profile**: era_o2_g0 (default `-O2 -G0`)
- **Status**: landed (wave-6 slice B, agent/wave6-b)

## Behaviour
Beam quad geometry. `heading = -func_80079FB4(from.z-to.z, from.x-to.x)`;
`dx = func_80077CF4(heading)*radius/4096`; `dz = func_80077DC4(heading)*radius/4096`;
then fills the four points of a `short v[4][4]` (`v[i][0]`, `v[i][2]`) from
`from` (±dx/±dz) for rows 0/1 and `to` for rows 2/3, and passes it to
`func_800C6B20`.

## Method
Two cc1 levers, both required:
- the `short v[4][4]` object must be a real **array** (address-taken as a
  whole) — eight separate `short` locals let cc1 delete seven stores as
  non-escaping;
- retail's callee-saved home map is $s0=heading / $s1=from / $s2=to /
  $s3=radius, reproduced with `register int heading asm("$16")` and
  `register int dz asm("$6")` after the four loads were ordered explicitly
  (`arg1[0], arg1[2], arg0[2], arg0[0]`).

## Evidence
try_leaf `WORDS MATCH`; fresh complete build EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b, `Matching claim: YES (889
registered C leaves)`, `VERIFY_US=PASS`.
