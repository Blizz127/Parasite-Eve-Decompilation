# `func_80085174` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o1_g0`.

## Span

- File `[0x75974,0x759A8)`, VA `[0x80085174,0x800851A8)`, size `0x34`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0` unless the profile says otherwise.

## Source (`src/func_80085174.c`)

Wait while D_8009D24C stays 1 after an initial 1.

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_80085174.c 0x75974 0x34 --flags "-O1 -G0"
```

The cumulative authority is `bash scripts/build_us.sh`: EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
