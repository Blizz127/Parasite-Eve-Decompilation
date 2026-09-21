# func_800701B4

- **VRAM**: 0x800701B4
- **File offset**: 0x609B4 (size 0x128)
- **Build profile**: era_o2_g0 (default `-O2 -G0`)
- **Status**: landed (wave-6 slice B, agent/wave6-b)

## Behaviour
Second half of the battle-slot scrub (call-arg index 0xB..0x15). Walks eleven
records, calling `func_8006FC18(i,0,1)`; a non-zero return aborts the walk and
is returned. Otherwise it clears the 0x72 marker's word block at
`D_800E0EF0+0x1B0` and wipes the record header (0/FF/FF/FF + zeroed words at
+4/+8) exactly like its first-half twin func_800702DC.

## Method
Same shape as func_800702DC, with a **separate loop counter** (`n`, $s3) from
the call-argument index (`i`, $s0, starts at 0xB). The i increment must be
written **before** the loop-back test (`i++; if (n < 0xB) goto loop;`) so cc1
schedules it into the `bnez` delay slot and keeps the `s2,s1,s3` increment
order of retail.

## Evidence
try_leaf `WORDS MATCH (+8 pad bytes)`; fresh complete build EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b, `Matching claim: YES (889
registered C leaves)`, `VERIFY_US=PASS`.
