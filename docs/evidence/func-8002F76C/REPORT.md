# REPORT — func_8002F76C matching C (27 words), 272 leaves

```text
MATCHING_C — func_8002F76C era -O2 -G0
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x8002F76C..0x8002F7D8 exclusive (0x6C / 27 words)
file        0x1FF6C
```

Pointer-install + three callees after default-record init, pulled from
pe-continuous-decomp PE-BTL142 (`~/dev/pe-continuous-decomp/src/func_8002F76C.c`).
Stores &D_800B8A20 through *a0, &D_800B0CB0 at D_800B8A88, &D_8009D1B0 at
D_800B8A8C, then jal 5218C / 51980(0, B8A88) / 51E64(B8A8C). No knobs.

## Carve

Tail of `19DE4.s`: prefix shrinks 0x61F4 → 0x6188, C 0x6C ends exactly at
the existing 2F7D8 C boundary (0x1FFD8) — no asm resume needed.

## Verification (2026-08-21)

```text
scripts/split_us.sh    # c: 272 split
docker run --rm -v "$PWD:/workspace" -w /workspace \
  --user "$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh
# RESULT: EXACT MATCH / Compare: EXACT SHA-1 MATCH, exit 0
sha1sum build/disc1.candidate.exe
# 452fb033f2eaa4b18aa20a5bca60b8125af3a37b (fresh, built 14:23 CDT)
```

## Files

```text
src/func_8002F76C.c
configs/USA/disc1.yaml    [0x1FF6C, c, func_8002F76C]; 19DE4 prefix 0x6188
scripts/build_us.sh       era -O2 -G0; SIZE_19DE4 0x6188; 2F76C wiring
```
