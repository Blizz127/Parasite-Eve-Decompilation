# decompile-continue-6 — 5 new matching C leaves (701 -> 706)

Authority: retail Disc 1 `SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

| leaf | file | size | what | profile |
|---|---|---:|---|---|
| `func_80076B98` | 0x67398 | 0x48 | publish 0x04000002 / a0 / 0 / 0x01000401 through the four `D_8009585x` pointer globals | `era_o2_g0_fill_indexed_store_delay_slot` |
| `func_8008F784` | 0x7FF84 | 0x38 | advance cursor, latch `((byte+0x40)&0xFF)<<8` at +0x76, set +0xF4 bits 0/1, clear +0x78 | `era_o2_g0_fill_indexed_store_delay_slot` |
| `func_80090A20` | 0x81220 | 0x44 | two cursor reads through `D_8009D2C8` at +0x60/+0x5C, clear +0x62/+0x5E | `era_o2_g0_fill_indexed_store_delay_slot` |
| `func_80090A64` | 0x81264 | 0x48 | cursor byte into `*D_8009D2C8 +0x64`, then OR the next byte << 8 | `era_o2_g0_fill_indexed_store_delay_slot` |
| `func_800909C0` | 0x811C0 | 0x4C | sign-extended 16-bit big-endian pair from the cursor to +0x14, +0x6A/+0x38 latches | `era_o2_g0_fill_indexed_store_delay_slot` |

All five end `jr $ra` + an indexed register-base store, matched with the
`MASPSX_FILL_INDEXED_STORE_DELAY_SLOT` maspsx gate (via the build profile); the
build trims the tail pad bytes.

Commands (host split, distrobox build/verify):

```
bash scripts/split_us.sh
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp7 && bash scripts/build_us.sh'
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp7 && bash scripts/verify_us.sh'
```

- `build_us.sh`: `RESULT: EXACT MATCH`, `Matching claim: YES (706 registered C leaves)`.
- `verify_us.sh`: `VERIFY_US=PASS`, `PASS all 706 packed C spans equal retail`.

## Tool fix: try_leaf false-diff masking

`tools/analysis/try_leaf.py` parsed `objdump -r` offsets from **every** section
and zeroed those offsets in the `.text` image on both sides. A `.pdr`
relocation at offset 0 masked the function's first instruction, under-reporting
diffs (it reported 1 differing word for `func_8009090C` when there were 2, and
could in principle hide a real difference entirely). Fixed to consider only
`RELOCATION RECORDS FOR [.text]`.

## Triaged non-matches (not registered)

- `func_8009090C` (0x8110C): 2-word register-allocation skew — retail
  `lhu $3`/`sll $3,$3` vs cc1's `lhu $2`/`sll $3,$2`.
- `func_8008FC28` (0x80428): 4-word cursor-register swap (`$v0`/`$v1`) that
  persists with pointer or integer cursors.
- `func_8007E594` (0x6ED94): loop pointer/counter registers swap.
- `func_800CC244`/`CC284`/`CC440`, `func_800CD07C`, `func_800CAB88`,
  `func_800C811C` (0x40 each): symbol loads reordered relative to the
  immediate stores; need a symbol-load scheduling gate, not the indexed-store
  gate.
