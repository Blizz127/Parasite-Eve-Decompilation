# `func_8008FCE4` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span

- File `[0x804E4,0x80510)`, VA `[0x8008FCE4,0x8008FD10)`, size `0x2C`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0` unless the profile says otherwise.

## Source (`src/func_8008FCE4.c`)

Cursor post-increment; sign-extended byte added into +0xE0 halfword.

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_8008FCE4.c 0x804E4 0x2C
```

The cumulative authority is `bash scripts/build_us.sh`: EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
