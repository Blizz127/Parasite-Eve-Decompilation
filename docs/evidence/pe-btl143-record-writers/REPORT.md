# PE-BTL143 REPORT — 2FA10 / 2FAA4 / 2FAD8 matching C

```text
PE-BTL143 MATCHING_C — three indexed record-field writers
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/build_us.sh RESULT: EXACT MATCH
yaml C entries: 233
```

Contiguous carve at the old 20210.s head, immediately after 2F9CC:

| Function | VRAM | Size | Words |
|---|---|---|---:|
| func_8002FA10 | 0x8002FA10 | 0x94 | 37 |
| func_8002FAA4 | 0x8002FAA4 | 0x34 | 13 |
| func_8002FAD8 | 0x8002FAD8 | 0x20 | 8 |

All three: `base = *a0 + (a1&0xFF)*16 + 28`. 2FAD8 stores a2/a3 at +4/+8.
2FAA4/2FA10 zero byte0, store a2/a3 as bytes, and take extra args from
the stack (`lbu 16(sp)`, `lhu 20(sp)`, …). 2FA10 also writes four bytes
at `*a0 + (a1&0xFF)*4 + 0x7C..0x7F`.

era `-O2 -G0`. Unlinked objects BYTE_EXACT (gas pad only). Resume
`202F8.s` size `0x3F28`.

## Files

```text
src/func_8002FA10.c
src/func_8002FAA4.c
src/func_8002FAD8.c
configs/USA/disc1.yaml
```
