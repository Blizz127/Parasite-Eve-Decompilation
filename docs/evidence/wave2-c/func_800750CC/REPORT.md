# func_800750CC

- VRAM: 0x800750CC
- file offset: 0x658CC
- size: 0x0x60 (96 bytes)
- source: src/func_800750CC.c
- build profile: era_o2_g0_fill_epilogue_delay_slot (-O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1)
- gate: none beyond the profile environment listed above

## Evidence

Fresh `bash scripts/build_us.sh` + `bash scripts/verify_us.sh` in the
`pe-mipsel` distrobox:

- retail SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- EXACT MATCH; VERIFY_US=PASS; matching-C count 821

## Divergences fixed to match

same double-deref fix and epilogue slot as func_8007506C
