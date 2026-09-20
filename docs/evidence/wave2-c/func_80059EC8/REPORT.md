# func_80059EC8

- VRAM: 0x80059EC8
- file offset: 0x4A6C8
- size: 0x0x40 (64 bytes)
- source: src/func_80059EC8.c
- build profile: era_o2_g8_aspsx_230 (-O2 -G8 + ERA_ASPSX_VER=2.30)
- gate: none beyond the profile environment listed above

## Evidence

Fresh `bash scripts/build_us.sh` + `bash scripts/verify_us.sh` in the
`pe-mipsel` distrobox:

- retail SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- EXACT MATCH; VERIFY_US=PASS; matching-C count 821

## Divergences fixed to match

none
