# func_8007A244

- VRAM: 0x8007A244
- file offset: 0x6AA44
- size: 0x0x60 (96 bytes)
- source: src/func_8007A244.c
- build profile: era_o2_g0 (default, -O2 -G0)
- gate: none beyond the profile environment listed above

## Evidence

Fresh `bash scripts/build_us.sh` + `bash scripts/verify_us.sh` in the
`pe-mipsel` distrobox:

- retail SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- candidate SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
- EXACT MATCH; VERIFY_US=PASS; matching-C count 821

## Divergences fixed to match

none
