# DAY2-158s — `MDEC_decode_busy` after sticky-D0DBD clear

Status: **HOST MDEC MODEL FIX APPLIED** (no `74520` / `909B4` widen).

Live Disc1 on tip `d827581` (DAY2-158r):

```text
e0_promote → got_frame → c89c#1 out=7300
92934_enter ×2
STUB: MDEC_decode_busy
C89C final: calls=2 out=11140
NO post_movie_title_cut / NO media_clear
```

Sticky `801D0DBD` clear worked (early title-cut gone). New wall is host
`MDEC_decode_busy` inside `MdecBeginCommand` when the second `92934` issues
`BFA0`/`DecDCTin`.

## Cause

`pe_mdec.c` rejected a new decode while `g_input_pos < g_input_count` (or
unread pixels). After title `91DC8` covers the bank rectangle it takes the
**final-slice** arm and stops calling `DecDCTout`. C89C end-fills with
`FE00` (see `pe-day2-158-c89c-pad-vs-body`); `ReadPixels` only skips that
padding when a DMA1 drain completes. Trailing `FE00` (or any orphaned
remainder once `1494` is set) therefore sat in the FIFO. Retail `C308`
would wait on command-busy, but without another `DecDCTout` that wait
cannot drain — the host STOP was the loud equivalent.

## Fix (`pc_port/platform/pe_mdec.c`)

1. `MdecConsumeIdlePadding` — skip trailing `FE00` when the pixel buffer is
   idle. **Does not** clear `g_decode_valid` (HostFB_VSync only services
   DMA IRQ while `PE_MDEC_HasDecode()`; clearing before `1214D4`/`91DC8`
   dropped the final-slice frame-complete store — Linux `DAY2_mdec_dma`
   regression).
2. Call it after each completed DMA1 service.
3. `MdecBeginCommand`: drain idle padding first; padding-only remainder is
   not busy. If non-padding residue remains **and** DMA0/DMA1 are inactive,
   supersede the orphaned prior command. If DMA is still active, keep
   `MDEC_decode_busy`.

No game CFG invent; no admit-gate / cursor ±32; `909B4` unchanged.

## Expect on Matt Disc1

C89C `calls≥3` / `out=12804` before the next wall. Still expect eventual
`post_movie_title_cut` when the movie ends. No Day2-complete claim.

## Linux

`./pc_port/build/pe-native-tests`: **1354 run / 1308 pass / 0 fail / 46 skip**
(includes `DAY2_mdec_dma` orphan-supersede coverage). Recorded in
`ACTIVE_HANDOFF` DAY2-158s.
