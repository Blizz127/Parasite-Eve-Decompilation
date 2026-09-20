# Known Addresses and Boundaries

## PS-X EXE (both discs)

| Field | Value |
| --- | --- |
| File size | `0x1EE800` (2,025,472 bytes) |
| SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |
| `t_addr` / VRAM base | `0x80010000` |
| `pc0` entry | `0x80072534` |
| Header | `0x0`–`0x800` |

## Solid-state subsegments (file offsets)

| Start | Type | VRAM / note |
| --- | --- | --- |
| `0x800` | rodata | `0x80010000` prefix jtbl + strings |
| `0x2A0C` | asm | `0x8001220C` first code prologue |
| `0x81438` | c | `func_80090C38` |
| `0x8144C` | c | `func_80090C4C` |
| `0x81460` | asm / c | asm on main; c after PR #13 (`func_80090C60`) |
| `0x81754` | c | `func_80090F54` |
| `0x81768` | asm | through `func_80091080` |
| `0x818A0` | rodata | mid-image data island |
| `0xB2AF8` | asm | `0x800C22F8` tail resume |
| `0x1EE800` | end | EOF |

## Anchors (do not casually split)

- **pc0 / crt0:** `func_80072534` — BSS zero-fill, stack, `jal` into main path.
- **crt0 → early main:** `func_8001220C`.
- **Tail resume:** `func_800C22F8`.
