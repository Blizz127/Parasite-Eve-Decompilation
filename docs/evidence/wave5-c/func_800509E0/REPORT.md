# func_800509E0

- **VRAM**: 0x800509E0
- **File offset**: 0x411E0 (size 0xF8)
- **Unit**: 411E0
- **Build profile**: era_o2_g8 (`-O2 -G8`)
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave5-c, agent/wave5-c)

## Behaviour
BGM/SE selector. If the gp small-data flag 0x1AC (D_8009CF1C) is set, takes
the queue id from func_80059F08(0); otherwise calls
func_8005EB58(func_80052558() == 0) and reads a signed byte from either
D_800C0E20 (when gp 0x1A8 = D_8009CF18 is set) or D_800C0E22. A non-negative
id goes to func_80063198(func_80062A34(1,6)) and returns func_800536B8(id);
a negative id runs func_800631AC(func_80062A34(1,6)), clears the +0x44 slot of
the func_80062A34(2,6) record via func_80062A34(2,5)/func_80062CB8, and
returns func_8005F5B8(0x39).

## Method
The three `0xNNN($gp)` accesses fix the profile: these objects live in the
small-data window, so the containing symbols must be **scalar** declarations
(`extern int D_8009CF1C;`, `extern int D_8009CF18;`,
`extern unsigned char D_8009D02C;`) and the unit is `-O2 -G8`. The two
absolute `lb` reads D_800C0E20/D_800C0E22 must stay absolute, so they are
declared as **incomplete arrays** (`extern signed char D_800C0E20[];`).
Two further shaping levers were needed:
- The two `var_s0` byte reads must go through a temporary assigned inside the
  branch and copied after the join (`temp_byte = ...; var_s0 = temp_byte;`),
  otherwise cc1 folds the load straight into `$s0` and drops retail's
  `lb $v0` + `addu $s0,$v0` pair.
- The `+0x44` clear must reuse **one** `temp_v1` pointer variable for both the
  func_80062A34(2,6) and func_80062A34(2,5) results, which is what produces
  retail's `addu $v1,$v0` before the `func_80062CB8` argument move.

## Evidence
Fresh complete retail build: `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (874
registered C leaves)`, `VERIFY_US=PASS`.

## Divergences
None.
