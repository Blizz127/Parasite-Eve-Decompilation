# func_800124F8 — boot-table clear leaf matching C (31 words)

```text
MATCHING_C — func_800124F8 era -O2 -G8, 31/31 words
branch      phase5fm-main-barrier-revisit
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
vram        0x800124F8..0x80012574 exclusive
file        0x2CF8 size 0x7C
scripts/build_us.sh RESULT: EXACT MATCH
scripts/verify_us.sh RESULT: EXACT MATCH
yaml C entries: 235
func_800124F8.c.o .text: 0x7C
```

The leaf has no frame and no calls. It zeros the gp-relative fields at
`+0x590`, `+0x598`, `+0x8C`, `+0x90`, and `+0x94`, clears 72 rows of 11 words
at `D_8009D310`, and clears 16 words at `D_8009DF70`. Hard-register locals
preserve the retail `$a1`/`$a2`/`$a3` loop homes and `$v1` row-offset copy;
the generated object reproduces every `addu`, branch, and delay-slot increment
in the 31-word retail body.

The former `2A0C.s` chunk is now split as asm prefix `0x2A0C..0x2CF8`, C at
`0x2CF8..0x2D74`, and asm remainder beginning at `2D74.s`. No other function
or boundary was changed.

## Files

```text
src/func_800124F8.c
configs/USA/disc1.yaml          [0x2CF8, c, func_800124F8], [0x2D74, asm]
scripts/build_us.sh              C registration / era compile / ROM order
scripts/verify_us.sh             expected boundary and 235-leaf report
docs/ai_context/ACTIVE_HANDOFF.md
```
