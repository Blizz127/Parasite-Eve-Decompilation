# `func_8009071C` — matching C leaf

Outcome: **MATCHED** and integrated. Era build profile: `era_o2_g0`.

## Span

- File `[0x80F1C,0x80F54)`, VA `[0x8009071C,0x80090754)`, size `0x38`.
- gcc 2.7.2-psx (`tools/era`), `-O2 -G0` unless the profile says otherwise.

## Source (`src/func_8009071C.c`)

Ring-buffer insert of *a0 at +4+4*idx; clear +0x62+2*idx.

## Single-leaf triage

Command (inside `pe-mipsel`):

```text
python3 tools/analysis/try_leaf.py src/func_8009071C.c 0x80F1C 0x38
```

The cumulative authority is `bash scripts/build_us.sh`: EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
