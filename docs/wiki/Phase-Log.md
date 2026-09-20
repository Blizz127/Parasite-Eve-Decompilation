# Phase Log

High-level milestone record. Evidence lives in `docs/ai_context/ACTIVE_HANDOFF.md` and PR history.

## Phase 0 — Scaffold ✅

Repository structure, ignore rules, documentation skeleton, legal boundaries.

## Phase 1 — Disc verification ✅

- Both USA discs extracted and hashed (`scripts/extract_us.sh`).
- Boot EXEs byte-identical; `PE.IMG` byte-identical across discs.
- Facts in `docs/disc_info.md`.
- Official redump.org cross-check: open, non-blocking.

## Phase 2 — Splat infrastructure ✅

- Minimal `configs/USA/disc1.yaml` from verified Phase 1 values.
- Pinned `splat64[mips]==0.41.0` via `scripts/setup_env.sh`.
- `scripts/split_us.sh` with `--check` and gitignore guard.
- First real split local-only (git-ignored).

## Phase 3 — Boundary audit ✅ (parked)

| File offset | VRAM | Type | Notes |
| --- | --- | --- | --- |
| `0x800` | `0x80010000` | rodata | Prefix jump tables + strings |
| `0x2A0C` | `0x8001220C` | asm | First MIPS prologue; crt0 `jal` target |
| `0x818A0` | `0x800910A0` | rodata | Mid-image data island |
| `0xB2AF8` | `0x800C22F8` | asm | Tail code resume |

## Phase 4 — Rebuild harness ✅

| Sub-phase | Result |
| --- | --- |
| 4E | `scripts/verify_us.sh` split-artifact checker |
| 4G | Distrobox `pe-mipsel` + binutils 2.44 |
| 4H | `scripts/build_us.sh` asm-only rebuild |
| 4I | ELF section pad trim → **exact SHA-1** |
| 4J | GCC 14.2 codegen probe for first leaf |

## Phase 5 — C replacement (in progress)

| Sub-phase | Function | PR | Status |
| --- | --- | --- | --- |
| 5B | `func_80090C38` | #9 | Merged |
| 5C | `func_80090C4C` | #10 | Merged |
| 5D | `func_80090F54` | #12 | Merged |
| 5E | `func_80090C60` | #13 | Open |
| 5F (planned) | `func_80090C74` | — | After #13 |

## Planned (not started)

- Broader symbol naming and more C beyond the trivial leaf cluster.
- PC port: **out of scope**.
