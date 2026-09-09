# DAY2-158 — `MDEC_decode_busy` dig (Disc1 tip `42978a5`)

Status: **APPLIED on PR #38 as DAY2-158u** (from `dig/mdec-decode-busy` @ `bf8ab79`). Linux 1354/1308/0/46. Awaiting Disc1 retest for C89C≥3 / out=12804. No `74520` / `909B4` / admit-gate invent.

## Live wall (unchanged after 158s)

```text
e0_promote → got_frame → c89c#1 out=7300
92934_enter ×2
STUB: MDEC_decode_busy
C89C final: calls=2 out=11140
NO post_movie_title_cut / NO media_clear
```

158r cleared sticky `801D0DBD`. 158s idle-FE00 + “no DMA” supersede passed
Linux `DAY2_mdec_dma` but **Disc1 still STOP’d** on the second `92934`
`BFA0`/`DecDCTin`.

## Where STOP is raised

| Item | Location |
|------|----------|
| STOP name | `MdecBoundary("MDEC_decode_busy", g_input_pos)` |
| Site | `pc_port/platform/pe_mdec.c` → `MdecBeginCommand` |
| Delivery | `Bootstrap_ReturnVoid1` + `PE_PORT_STOP_UNRESOLVED_BOUNDARY` |
| Live caller chain | `92934` → `func_8010BFA0` → `PE_MDEC_SubmitInputTable` (sets `dma0_active` + `g_input_pending`) → later `HostFB_VSync`/`PumpCdProgress` → `PE_MDEC_Service` DMA0 arm → `MdecBeginCommand` |

`MdecBeginCommand` is **not** called synchronously from `BFA0`. Decode
commit is deferred until `PE_MDEC_Service` sees `dma0_active &&
(control&0x40000000) && (DPCR&8) && g_input_pending`.

## Who sets / clears “decode busy” vs DMA

Host model has **no** separate hardware command-busy bit wired to retail
`C308` yet (`DAY2_MDEC_DMA_OUTPUT.md`: C1EC/C308 wait “remains to be
connected”). The STOP substitutes “FIFO/pixel residue + DMA flags”:

| Flag / state | Set by | Cleared by |
|--------------|--------|------------|
| `g_input_pos < g_input_count` | `MdecBeginCommand` load | `MdecBlock` / idle `FE00` skip / supersede |
| `g_pixel_pos < g_pixel_count` | `MdecMacroblock` | `ReadPixels` / supersede |
| `g_decode_valid` | successful `MdecBeginCommand` | supersede / reset (**not** idle-FE00 drain — HostFB gates DMA IRQ on `PE_MDEC_HasDecode()`) |
| `g_input_pending` | `SubmitInputTable` kind==1 (`DecDCTin`) | Service DMA0 completion |
| `dma0_active` | `SubmitInputTable` | Service DMA0 completion / `ClearDmaChannels` / reset |
| `dma1_chcr bit24` | `SubmitOutput` (`DecDCTout`/`C01C`) | Service DMA1 completion / clear / reset |
| DICR DMA1 flag | `PE_GPU_LatchDMACompletionFlag(1)` after DMA1 service | IRQ bridge / guest ack (`74520`) |
| Title final | `func_80191DC8` else-arm sets `801D1494=1`, **does not** call `C01C` | — |
| Player twin | `func_801214D4` same shape on `801228FC` | — |

`91DC8` continuing arm calls `func_8010C01C` (new DMA1). Final arm stops
output requests once the bank rect is covered — intentional orphan of any
unread RLE / partial pixels / trailing `FE00`.

## Diff 158s vs pre

Pre-158s (`671f684`):

```c
if(g_input_pos<g_input_count || g_pixel_pos<g_pixel_count)
    return MdecBoundary("MDEC_decode_busy",g_input_pos);
```

158s (`42978a5`): drain idle `FE00`; if residue remains and
`dma0_active || (dma1_chcr&0x01000000)` → still STOP; else supersede.

Unit test used `PE_MDEC_BeginDecode` + `PE_MDEC_ClearDmaChannels()` — **no**
`dma0_active` / **no** pre-armed DMA1 — so supersede always ran (false green).

## Why idle-FE00 drain would not clear live Disc1 after 2 frames

1. **Residue is usually not leading-`FE00`.** After `91DC8` final, unread
   input is typically remaining RLE (next macroblock the rect did not need)
   and/or `g_pixel_pos < g_pixel_count` (partial macroblock in `g_pixels`).
   `MdecConsumeIdlePadding` returns immediately when pixels are pending and
   only skips `0xFE00` words — it cannot clear non-pad RLE.
2. **Even if residue were pad-only, live never reached “not busy”.** Second
   `92934` does `BFA0` then `C01C` **before** Service. When Service finally
   calls `MdecBeginCommand`, `dma0_active==1` (this upload) and often
   `dma1_chcr` busy (new frame’s `DecDCTout`). 158s treated those as
   “DMA still draining the orphan” → STOP instead of supersede.
3. **`92934` does not wait on MDEC ready** between frames before `BFA0`.
   Body: optional `0DC0==2` flip → `BFA0` → `C01C` → `91B64` poll → C89C →
   `1494` wait (VSync pump). No `C308` command-busy wait on the ported
   `DecDCTin` path. Retail would spin in `C308`; host STOP was meant to be
   the loud equivalent — but the busy predicate was wrong for the deferred
   DMA0 commit.

IQ/scale / wrong channel: tables already loaded (`BE3C(0)`); STOP is not
`MDEC_missing_tables`. Channel is DMA0 (input) commit colliding with orphan
state from DMA1 title callback — not a wrong-channel decode.

## Root-cause hypothesis (high confidence)

**158s orphan supersede never runs on the live `DecDCTin` path** because
`MdecBeginCommand` is reached from `PE_MDEC_Service`’s DMA0 arm with
`dma0_active` already set (and frequently with DMA1 already armed by
`92934`’s `C01C` for the *new* frame). Those flags describe the **incoming**
command, not an in-flight drain of the **prior** `91DC8`-final orphan.
Non-`FE00` residue after final-slice makes idle-pad drain insufficient; the
false `dma_busy` then forces `MDEC_decode_busy`.

## Smallest honest next change

In `MdecBeginCommand`, when residue remains:

- If `dma0_active && g_input_pending` (Service committing this `DecDCTin`):
  **supersede** the orphan (discard input/pixel residue; clear
  `g_decode_valid` only for the replace — new command sets it again).
- Else if DMA0/DMA1 busy without that commit shape: keep STOP.
- Else (BeginDecode / idle): supersede as 158s intended.

No game CFG invent; no `74520`/`909B4` change; keep `g_decode_valid` on
idle-FE00-only drain.

Draft applied on this branch in `pc_port/platform/pe_mdec.c`, plus a
live-shaped `BFA0`→`C01C`→`Service` case in `pc_port/tests/test_mdec_dma.h`.

## Expect on Matt Disc1 after dig fix

Second `92934` `BFA0` should commit; C89C `calls≥3` / `out=12804` before the
next wall. Still expect eventual `post_movie_title_cut`. No Day2-complete
claim. Optional follow-up (separate): wire retail `C308` wait fidelity so
overlap semantics match libpress instead of host supersede — not required to
clear this STOP once orphan predicate is fixed.

## Filters

- `MDEC_decode_busy|MdecBeginCommand|MdecDecodeResidue|dma0_active`
- `func_80192934_enter|func_80191DC8|BFA0|C01C`
- `c89c_tel|801D1494`
