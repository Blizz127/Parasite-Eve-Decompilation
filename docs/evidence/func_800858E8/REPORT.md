# `func_800858E8` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0_three_word`.

## Span

- File `[0x760E8,0x76118)`, VA `[0x800858E8,0x80085918)`, size `0x30`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0` unless the profile says otherwise.

## Source (`src/func_800858E8.c`)

OR indexed word D_8009B7D4[idx] into D_8009B7CC[1]; return idx < 3; three-word $at indexed load.

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_800858E8.c 0x760E8 0x30 --env MASPSX_THREE_WORD_SYMBOL_STORE=1
```

The cumulative authority is `bash scripts/build_us.sh`: EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
