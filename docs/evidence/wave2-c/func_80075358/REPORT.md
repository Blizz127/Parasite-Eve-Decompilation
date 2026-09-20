# func_80075358

- VRAM: 0x80075358
- file offset: 0x65B58
- size: 0x0x5C (92 bytes)
- source: src/func_80075358.c
- build profile: era_o2_g0_fill_epilogue_delay_slot (-O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1)
- gate: none beyond the profile environment listed above

## Evidence

Fresh `bash scripts/build_us.sh` + `bash scripts/verify_us.sh` in the
`pe-mipsel` distrobox:

- retail SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- EXACT MATCH; VERIFY_US=PASS; matching-C count 821

## Divergences fixed to match

double-deref fix for both method calls; epilogue slot as above
