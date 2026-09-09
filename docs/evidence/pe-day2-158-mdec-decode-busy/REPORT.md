# DAY2-158s/t/u — `MDEC_decode_busy` after sticky-D0DBD clear

Status: **HOST MDEC MODEL FIX (158u)** — 158s/158t insufficient live; no
`74520` / `909B4` widen.

## Live chronology

**158r `d827581`:** sticky D0DBD cleared; new wall `MDEC_decode_busy`,
C89C `calls=2 out=11140`.

**158s `42978a5` — FAILED RETEST** (identical shape). Idle-`FE00` + direct
`BeginDecode` supersede never ran on live BFA0 path.

**158t `b82ab62` — FAILED RETEST** (identical shape). dma0-clear before
BeginCommand on Service path did not clear the wall.

```text
92934_enter ×2
STUB: MDEC_decode_busy
C89C final: calls=2 out=11140
```

## Cause

### 158s (partial)
Title `91DC8` final-slice stops `DecDCTout`; orphan FIFO / mid-MB residue.

### 158t (partial)
Live `BFA0` → `SubmitInputTable` → `Service` called BeginCommand with
`dma0_active` still 1. Direct-BeginDecode unit test was false green.

### 158u (why 158t missed live)
Live `92934` programs **BFA0 then C01C** before Service. That arms
`dma1_chcr` for the **new** DecDCTout before BeginCommand runs for the new
DecDCTin. Treating dma1 as "still draining old output" blocked supersede.
Busy boundary encoding (bit31=dma0, bit30=dma1) was added for Disc1 dig;
expect live would have shown dma1=1 dma0=0.

## Fix (`pc_port/platform/pe_mdec.c`)

1. Idle-`FE00` drain; keep `g_decode_valid` for HostFB DMA IRQ gating.
2. Clear `dma0_active` before BeginCommand on DMA0 completion (158t).
3. **158u:** only still-active **dma0** blocks supersede; concurrent dma1
   from new C01C does not. fprintf busy value + `MDEC_orphan_supersede_dma1`.
4. Tests: direct BeginDecode (158s), BFA0→Service (158t), BFA0→C01C→Service
   (158u).

No game CFG invent; no admit-gate / cursor ±32; `909B4` unchanged.

## Expect on Matt Disc1

Past `MDEC_decode_busy` with C89C `calls≥3` / `out=12804`. If busy still
fires, stderr shows `value=0x… dma0=… dma1=… pos=…`. Still expect eventual
`post_movie_title_cut`. No Day2-complete claim.

## Linux

`./pc_port/build/pe-native-tests`: **1354 run / 1308 pass / 0 fail / 46 skip**.
Recorded in `ACTIVE_HANDOFF` DAY2-158u.
