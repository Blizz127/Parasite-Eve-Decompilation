# Function Porting Queue

Conservative shortlist from `docs/ai_context/DISC1_FIRST_DECOMP_TARGETS.md`.

## Done (exact SHA-1)

| Label | VRAM | C source | Bit op at `*(arg0+0x38)` |
| --- | --- | --- | --- |
| `func_80090C38` | `0x80090C38` | `src/func_80090C38.c` | `\|= 0x10` |
| `func_80090C4C` | `0x80090C4C` | `src/func_80090C4C.c` | `&= ~0x10` |
| `func_80090F54` | `0x80090F54` | `src/func_80090F54.c` | `\|= 0x100000` |
| `func_80090C60` | `0x80090C60` | `src/func_80090C60.c` | `\|= 0x20` (PR #13) |

All are 0x14-byte pure leaves: no calls, no jump tables, no globals.

## Next

1. **`func_80090C74`** — bit-clear twin of 90C60 (prefer after #13 merges).
2. Other same-cluster accessors only if still trivial leaves.
3. Backup: `func_800C2B40` (tail setter; single global table — medium risk).

## Rules for each conversion

- One function per PR.
- Probe GCC in `/tmp` before production integration.
- Flags: `-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls -ffreestanding -fno-builtin -O1`
- Trim C object `.text` pad **0x20 → 0x14** (align-16 zeros only).
- `scripts/build_us.sh` exit 0 + original SHA-1 required.
- No semantic struct/field names yet.
