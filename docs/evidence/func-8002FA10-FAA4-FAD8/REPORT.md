# REPORT — func_8002FA10 / func_8002FAA4 / func_8002FAD8 matching C, 275 leaves

```text
MATCHING_C — three indexed record-field writers, era -O2 -G0
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
func_8002FA10  vram 0x8002FA10  file 0x20210  size 0x94 (37 words)
func_8002FAA4  vram 0x8002FAA4  file 0x202A4  size 0x34 (13 words)
func_8002FAD8  vram 0x8002FAD8  file 0x202D8  size 0x20 (8 words)
```

Pulled from pe-continuous-decomp PE-BTL143
(`~/dev/pe-continuous-decomp/src/func_8002FA10.c` etc.). All write
`*(base + i*16 + 28)` record fields off `*a0` with an `unsigned char`
index; 2FA10 additionally takes stack extras (u8 ×4, and the
`q[0x7C..0x7F]` byte quad via `i*4`), 2FAA4 pulls u8/u16 from
16(sp)/20(sp), 2FAD8 stores two words at +4/+8. No knobs, plain era
`-O2 -G0`. One commit for the three: contiguous head-of-`20210.s` carve.

## Carve

`20210.s` (0xB24) → three C leaves 0xE8 total (0x20210..0x202F8), resume
`202F8.s` 0xA3C to 0x20D34 (the existing 30534 carve boundary).

## Verification (2026-08-21)

```text
scripts/split_us.sh    # c: 275 split
docker run --rm -v "$PWD:/workspace" -w /workspace \
  --user "$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh
# RESULT: EXACT MATCH / Compare: EXACT SHA-1 MATCH, exit 0
sha1sum build/disc1.candidate.exe
# 452fb033f2eaa4b18aa20a5bca60b8125af3a37b (fresh, built 14:25 CDT)
```

## Files

```text
src/func_8002FA10.c  src/func_8002FAA4.c  src/func_8002FAD8.c
configs/USA/disc1.yaml    [0x20210/0x202A4/0x202D8, c] + [0x202F8, asm]
scripts/build_us.sh       era -O2 -G0; 20210.s → 202F8.s wiring
```
