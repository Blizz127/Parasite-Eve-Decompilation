# `func_80085918` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_passthrough_load`.

## Span

- File `[0x76118,0x7614C)`, VA `[0x80085918,0x8008594C)`, size `0x34`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0` unless the profile says otherwise.

## Source (`src/func_80085918.c`)

AND the complement of D_8009B7D4[idx] into D_8009B7CC[1]; return 1; destination-register indexed load.

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_80085918.c 0x76118 0x34 --env MASPSX_PASSTHROUGH_SYMBOL_LOAD=1
```

The cumulative authority is `bash scripts/build_us.sh`: EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
