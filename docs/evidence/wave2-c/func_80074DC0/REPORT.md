# func_80074DC0

- VRAM: 0x80074DC0
- file offset: 0x655C0
- size: 0x0x68 (104 bytes)
- source: src/func_80074DC0.c
- build profile: era_o2_g0_fill_epilogue_delay_slot (-O2 -G0 + MASPSX_FILL_EPILOGUE_DELAY_SLOT=1)
- gate: none beyond the profile environment listed above

## Evidence

Fresh `bash scripts/build_us.sh` + `bash scripts/verify_us.sh` in the
`pe-mipsel` distrobox:

- retail SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- EXACT MATCH; VERIFY_US=PASS; matching-C count 821

## Divergences fixed to match

stock -O2 -G0 left addiu $sp before jr $ra; retail has it in the jr delay slot
