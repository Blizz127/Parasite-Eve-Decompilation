# func_8006A0E8 — wave7-b executed-path leaf (902 -> 903)

- VRAM 0x8006A0E8, file 0x5A8E8, size 0x174 (93 words). Splat size authoritative.
- Profile: `era_o2_g0_three_word` = `-O2 -G0` + `MASPSX_THREE_WORD_SYMBOL_STORE=1`.

## Retail structure
`if ((D_800B0CD8 & 0x200) == 0 && (D_8009D1A0 & 0x10))`:
write the `92*D_8009CDDC` slot of the three parallel arrays `D_800BCDE0/DF/C8`
via `func_80075424(&D_800BCDC8[idx])`, call `func_80075358(base+0x114)` and
`func_80075358(base+0x104)`, then re-write the two byte slots and call
`func_800867E4(0)`. Then `result = D_8009D1A0 & 0x20; if (result) result =
func_8008682C(0); return result;`

## Levers
- `MASPSX_THREE_WORD_SYMBOL_STORE=1` turns cc1's 4-word indexed symbol store
  (`lui at,%hi; addiu at,at,%lo; addu at,at,idx; sb 0(at)`) into retail's
  3-word `lui at,%hi; addu at,at,idx; sb %lo(sym)(at)`.
- A single local index variable **reassigned before each store**
  (`idx = D_8009CDDC * 0x5C;`) is required. Writing `D_800BCDE0[D_8009CDDC*0x5C]`
  inline makes cc1 reserve a phantom 16-byte stack frame slot that retail does
  not have (frame 0x30 vs 0x20); the reassigned local keeps `vars=0` and still
  reloads D_8009CDDC like retail.

## Authority
`scripts/split_us.sh` (host) then `build_us.sh` + `verify_us.sh` (pe-mipsel):
`EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`Matching claim: YES (903 registered C leaves)`, plan
`1310 spans = 903 c + 405 asm + 2 rodata`, `VERIFY_US=PASS`.
Commit `b41ec69b`. try_leaf: `WORDS MATCH (+12 pad bytes, trimmed by the build)`.
