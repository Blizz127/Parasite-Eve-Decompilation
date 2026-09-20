# decompile-continue-6 — 3 new matching C leaves (701 -> 704)

Authority: retail Disc 1 `SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

| leaf | file | size | what | profile |
|---|---|---:|---|---|
| `func_80076B98` | 0x67398 | 0x48 | publish 0x04000002 / a0 / 0 / 0x01000401 through the four `D_8009585x` pointer globals | `era_o2_g0_fill_indexed_store_delay_slot` |
| `func_8008F784` | 0x7FF84 | 0x38 | advance cursor, latch `((byte+0x40)&0xFF)<<8` at +0x76, set +0xF4 bits 0/1, clear +0x78 | `era_o2_g0_fill_indexed_store_delay_slot` |
| `func_80090A20` | 0x81220 | 0x44 | two cursor reads through `D_8009D2C8` at +0x60/+0x5C, clear +0x62/+0x5E | `era_o2_g0_fill_indexed_store_delay_slot` |

All three end `jr $ra` + an indexed register-base store, matched with the
`MASPSX_FILL_INDEXED_STORE_DELAY_SLOT` maspsx gate (`--env`); the build trims
8/8/12 tail pad bytes respectively.

Command (host split, distrobox build):

```
bash scripts/split_us.sh
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp7 && bash scripts/build_us.sh'
```

Result: `RESULT: EXACT MATCH`, `Matching claim: YES (704 registered C leaves)`.

## Tool fix: try_leaf false-diff masking

`tools/analysis/try_leaf.py` parsed `objdump -r` offsets from **every** section
and zeroed those offsets in the `.text` image on both sides. A `.pdr`
relocation at offset 0 therefore masked the function's first instruction,
under-reporting diffs (it reported 1 differing word for `func_8009090C` when
there were 2, and could in principle hide a real one entirely). Fixed to
consider only `RELOCATION RECORDS FOR [.text]`.
