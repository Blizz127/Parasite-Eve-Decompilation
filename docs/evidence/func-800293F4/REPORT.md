# REPORT — func_800293F4 matching C (124 words), 271 leaves

```text
MATCHING_C — func_800293F4 era -O2 -G8 + MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_8009D2E8
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x800293F4..0x800295E4 exclusive (0x1F0 / 124 words)
file        0x19BF4
```

HP clamp/copy + record flag storm, pulled from pe-continuous-decomp
PE-BTL139 (`~/dev/pe-continuous-decomp/src/func_800293F4.c`).

- gp-relative: D_8009D278 (record*), D_8009D1D0, D_8009D244; lui:
  D_8009D234, D_8009D2E8, D_8009D298/9A/9B/9C.
- Separate `rec` temporaries so each D278 load can take a different
  register (ROM: a1, v1, a1, a0, v1, v0); `register asm` pins r2/r3/flags
  to a1/a0/v0; `asm volatile("":::"memory")` keeps the join store before
  the mask hoist.
- **New era_compile knob ported from the source repo:**
  `MASPSX_FORCE_ABSOLUTE_SYMBOLS=SYM[,SYM...]` strips cc1's
  `.extern SYM, size` so GNU as emits the 2-word lui/lw + lui/$at sw for
  a 4-byte scalar that ROM addresses absolutely but -G8 would classify
  as sdata. Used here for D_8009D2E8's RMW.

## Carve

Replaces the `19BF4.s` resume from the 29388 carve: C 0x1F0 at 0x19BF4,
resume `19DE4.s` 0x61F4 to 0x1FFD8 (2F7D8).

## Verification (2026-08-21)

```text
scripts/split_us.sh    # c: 271 split
docker run --rm -v "$PWD:/workspace" -w /workspace \
  --user "$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh
# RESULT: EXACT MATCH / Compare: EXACT SHA-1 MATCH, exit 0
sha1sum build/disc1.candidate.exe
# 452fb033f2eaa4b18aa20a5bca60b8125af3a37b (fresh, built 14:21 CDT)
```

## Files

```text
src/func_800293F4.c
configs/USA/disc1.yaml    [0x19BF4, c, func_800293F4] + [0x19DE4, asm]
scripts/build_us.sh       era -O2 -G8 + FORCE_ABSOLUTE knob; 19DE4 wiring
```
