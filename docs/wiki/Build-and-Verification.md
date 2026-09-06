# Build and Verification

## Oracle

`scripts/build_us.sh` is the final matching claim.

| Exit | Meaning |
| --- | --- |
| 0 | Exact SHA-1 match to original `SLUS_006.62` |
| 1 | Failure or non-match |
| 2 | Usage / missing prerequisites |

Expected SHA-1:

```
452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

## Typical gate sequence

```bash
scripts/split_us.sh --check
scripts/verify_us.sh
scripts/build_us.sh
```

## Toolchain

- **Distrobox** container: `pe-mipsel` (Debian trixie)
- Assembler/linker: `mipsel-linux-gnu-{as,ld,objcopy,readelf}` (binutils 2.44)
- Compiler: `mipsel-linux-gnu-gcc` 14.2.0
- Host PATH intentionally has no mipsel tools

## C leaf flags (Phase 4J+)

```
-EL -mips1 -mfp32 -mabi=32 -G0 -fno-pic -mno-abicalls -ffreestanding -fno-builtin -O1
```

## Pad trim

GNU as / GCC emit section sizes padded to alignment (often `.text` 0x20 for a 0x14-byte body).
`tools/trim_elf_section_pad.py` strips proven trailing zero pad and lowers align to 4 so the linker does not shift ROM layout.

## What is never committed

Generated `asm/`, `linkers/`, `build/`, objects, maps, and candidate EXEs are git-ignored.
