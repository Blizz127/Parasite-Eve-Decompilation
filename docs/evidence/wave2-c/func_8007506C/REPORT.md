# func_8007506C

- VRAM: 0x8007506C
- file offset: 0x6586C
- size: 0x0x60 (96 bytes)
- source: src/func_8007506C.c
- build profile: era_o2_g0_fill_epilogue_delay_slot (-O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1)
- gate: none beyond the profile environment listed above

## Evidence

Fresh `bash scripts/build_us.sh` + `bash scripts/verify_us.sh` in the
`pe-mipsel` distrobox:

- retail SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- EXACT MATCH; VERIFY_US=PASS; matching-C count 821

## Divergences fixed to match

first draft used single-deref function-pointer cast (jalr straight to base+8); fixed to (*(T**)(base+8)). Epilogue slot as above
