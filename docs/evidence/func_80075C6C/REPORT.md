# `func_80075C6C` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span

- File `[0x6646C,0x66494)`, VA `[0x80075C6C,0x80075C94)`, size `0x28`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0` unless the profile says otherwise.

## Source (`src/func_80075C6C.c`)

GPU primitive packer: byte +3 = 2, packed word +4 = 0xE6000001/0 by a1, +8 = 0.

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_80075C6C.c 0x6646C 0x28
```

The cumulative authority is `bash scripts/build_us.sh`: EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
