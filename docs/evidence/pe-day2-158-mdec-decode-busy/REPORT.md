# DAY2-158s/t — `MDEC_decode_busy` after sticky-D0DBD clear

Status: **HOST MDEC MODEL FIX (158t)** — 158s insufficient live; no
`74520` / `909B4` widen.

## Live chronology

**158r tip `d827581`:** sticky D0DBD cleared; new wall:

```text
e0_promote → got_frame → c89c#1 out=7300
92934_enter ×2
STUB: MDEC_decode_busy
C89C final: calls=2 out=11140
```

**158s tip `42978a5` — FAILED RETEST (identical shape):**

```text
92934_enter ×2
STUB: MDEC_decode_busy
C89C final: calls=2 out=11140
```

Idle-`FE00` drain + direct-`BeginDecode` supersede did not clear live.

## Cause

### 158s (partial)

Title `91DC8` final-slice stops `DecDCTout` once the bank rect is covered.
C89C end-fills `FE00` / leaves orphan FIFO or mid-MB pixels. Retail `C308`
would wait; without another `DecDCTout` that wait cannot drain.

### 158t (why 158s missed live)

Live path is `92934` → `BFA0` → `PE_MDEC_SubmitInputTable` (sets
`dma0_active=1`) → later `PE_MDEC_Service` → `MdecBeginCommand` **while
`dma0_active` was still 1**. Any non-padding residue then took
`MDEC_decode_busy` before the supersede arm. The 158s unit test called
`PE_MDEC_BeginDecode` directly (`dma0_active=0`) — false green.

## Fix (`pc_port/platform/pe_mdec.c`)

1. **158s (kept):** `MdecConsumeIdlePadding` after DMA1 / before busy check;
   padding-only remainder is not busy; non-padding orphan with **no** DMA →
   supersede. **Do not** clear `g_decode_valid` on FIFO drain (`HostFB_VSync`
   gates DMA IRQ on `PE_MDEC_HasDecode()`).
2. **158t:** on DMA0 completion in `Service`, clear `dma0_active` / CHCR busy
   **before** `BeginCommand` (words already in hand). Completing DecDCTin
   must not block orphan supersede.
3. Busy dig: encode dma0/dma1 flags in the `MDEC_decode_busy` boundary value
   (no `Trace_Direct` — not linked into every field-runtime consumer).
4. Test: `DAY2_mdec_dma` adds BFA0→Service orphan path (158t).

No game CFG invent; no admit-gate / cursor ±32; `909B4` unchanged.

## Expect on Matt Disc1

Past `MDEC_decode_busy` with C89C `calls≥3` / `out=12804` before the next
wall. Still expect eventual `post_movie_title_cut`. No Day2-complete claim.

## Linux

`./pc_port/build/pe-native-tests`: **1354 run / 1308 pass / 0 fail / 46 skip**.
Recorded in `ACTIVE_HANDOFF` DAY2-158t.
