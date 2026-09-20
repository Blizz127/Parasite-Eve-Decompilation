# func_8005C498

- **VRAM**: 0x8005C498
- **File offset**: 0x4CC98 (size 0xFC)
- **Unit**: 4CC98
- **Build profile**: era_o2_g8 (`-O2 -G8`)
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave5-c, agent/wave5-c)

## Behaviour
Frame-start handler. If func_80042ED0() == 0: stores arg0 to the absolute
D_8009D1E0, runs the eight startup calls (51504/5E6F0/46334/5E30C/4F464/
42B6C/62FEC/5E788(1)), reads the gp counter 0x2C0 (D_8009D030) and either
calls func_800425DC (>= 2) or increments it (> 0), clears the gp flag 0x2C4
(D_8009D034) via func_800512AC(9,0), and if func_800514F8() != 0 calls
func_800339A0(D_8009D02C); returns func_800514F8(). Otherwise func_80042F44()
and return 0.

## Method
Matched on the first try once the small-data model was right: the gp accesses
0x2C0/0x2C4/0x2BC demand `-O2 -G8` and **scalar** declarations for
D_8009D030/D_8009D034/D_8009D02C, while D_8009D1E0 is written through the
absolute store macro, so it is declared as an **incomplete array**
(`extern int D_8009D1E0[];`) and stored via `D_8009D1E0[0] = arg0;`.

## Evidence
Fresh complete retail build: `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (874
registered C leaves)`, `VERIFY_US=PASS`.

## Divergences
None.
