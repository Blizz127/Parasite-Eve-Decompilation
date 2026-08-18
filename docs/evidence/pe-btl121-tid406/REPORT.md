# PE-BTL121 — TID 406 provenance

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

TEXT `li 406` sites (16828, 22C64, 22CA8, 27A58) are
compares. Absolute `sh BE834` sites (20F7C, 26FA0) write
zero. `512AC` case 1 delay `addiu 387` writes `D010`,
not BE834.

Publication:

| Step | Site | Effect |
|------|------|--------|
| menu index | `46C20` `sw v0, gp+0x244` | 556E8 return |
| 57B70 | `46DE0` `lw a0, gp+0x244` | `512AC(1, &index)` |
| 512AC(1) | `5130C`/`51314`/`514CC` | `D010 = *a1 + 387` |
| 299CC | `29A68` `sh v0, gp+0x534` | `D2A4 = 5C498()` / D010 |
| 26824(1) | `2684C` lh D2A4; tid 406 `jtbl[13]=2692C` | `sh D2A4, slot+4` |

Index 19 produces 406 (`387+19`). That index is
`24250` jtbl[19], the sole `rec+0x4C` `0x80000` OR.

Do not plant 406 on BE834. `556E8` / `gp+0x244=19`
natural production and `5E30C` dispatch of `512AC(1)`
are not this cut.

## Verify

```text
python3 pc_port/tools/pe_btl121_tid406_oracle.py
PE_TEST_FILTER=BTL121 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL116 ./pc_port/build/pe-native-tests
```
