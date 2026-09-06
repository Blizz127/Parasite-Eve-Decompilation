# Current Status

*Last derived from repo handoff + merged PRs (2026-07-08).*

## Phase

**Phase 5E — fourth C leaf** (`func_80090C60`) ready as PR #13.

Production rebuild on `main` (after PR #12) includes three matching C functions; PR #13 adds a fourth.

Oracle: `scripts/build_us.sh` exits **0** only on exact SHA-1 match.

```
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

## Matched C leaves

| Function | Status | Notes |
| --- | --- | --- |
| `func_80090C38` | Merged (PR #9) | Bit-set `0x10` at `*(arg0+0x38)` |
| `func_80090C4C` | Merged (PR #10) | Bit-clear `0x10` at same field |
| `func_80090F54` | Merged (PR #12) | Bit-set `0x100000` at same field |
| `func_80090C60` | PR #13 open | Bit-set `0x20` at same field |

## Production split map (after PR #12 on main)

```
[0x800,     rodata]  prefix jump tables + strings
[0x2A0C,    asm]     main text through func_80090BCC
[0x81438,   c, func_80090C38]
[0x8144C,   c, func_80090C4C]
[0x81460,   asm]     … (becomes c + asm after PR #13)
[0x81754,   c, func_80090F54]
[0x81768,   asm]     resume through func_80091080
[0x818A0,   rodata]  mid-image data island
[0xB2AF8,   asm]     tail code from func_800C22F8
```

## Verified facts

- Both boot EXEs byte-identical (SHA-1 above).
- `PE.IMG` byte-identical across discs (SHA-1 `146c0ce7308bf9fdc2ba5a84230e198db0663f3b`).
- Phase 1 local verification complete; official redump.org cross-check open (non-blocking).
- Phase 3 boundary audit **parked**.
- Asm rebuild + C leaves: exact SHA-1 via `scripts/build_us.sh`.
- Toolchain: Distrobox `pe-mipsel` (binutils 2.44, GCC 14.2).

## Next planned

1. Merge PR #13 (`func_80090C60`).
2. Next C leaf: **`func_80090C74` only** (bit-clear twin of 90C60).
3. Do not invent struct/field names for `+0x38` yet.

## Not in git

Disc images, extracted files, generated asm/linker output, and build artifacts stay local and git-ignored.
