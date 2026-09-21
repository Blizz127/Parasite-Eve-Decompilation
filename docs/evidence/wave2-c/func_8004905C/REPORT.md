# func_8004905C

- VRAM: 0x8004905C
- file offset: 0x3985C
- size: 0x0x54 (84 bytes)
- source: src/func_8004905C.c
- build profile: era_o2_g8 (-O2 -G8)
- gate: none beyond the profile environment listed above

## Evidence

Fresh `bash scripts/build_us.sh` + `bash scripts/verify_us.sh` in the
`pe-mipsel` distrobox:

- retail SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- EXACT MATCH; VERIFY_US=PASS; matching-C count 821

## Divergences fixed to match

none
