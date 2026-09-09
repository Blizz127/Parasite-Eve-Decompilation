# DAY2-158 — title DecDCTout DMA1 callback `func_80191DC8`

Status: **MATCHED STRUCTURAL PORT** (leaf body ready; `DispatchDmaCallback`
arm left for Port).

Tip under dig: `50b2a58` (`origin/cursor/movie-autonomous-stream-6f51` /
DAY2-158o). Branch: `dig/91dc8-decdctout`.

Prior wall dig: `docs/evidence/pe-day2-158-74520-dma/REPORT.md` (`546c9b9`)
— STOP is the unported `0x80191DC8` handler, not a missing `74520` body.

## Carve identity

```text
PE.IMG SHA-1          146c0ce7308bf9fdc2ba5a84230e198db0663f3b
overlay LBA           [0x03D2,0x0457)  (133 sectors)
overlay load VA       [0x8018EFF0,0x801D17F0)
overlay size          0x42800
overlay SHA-1         a0b604ead810592dd9a8d74f7c74fe94151f4d32
function VA           [0x80191DC8,0x80191FB8)
function size         0x1F0 / 124 words
function SHA-256      d4d36c74fdae7ccb189a0d98c170806fc84b1fe24c700a447bee5eafbd941086
PE.IMG byte offset    0x1EBDD8  (= 0x03D2*0x800 + (0x80191DC8-0x8018EFF0))
prologue              lui/lb 800B0DBB ; addiu sp,-0x20 ; sw ra
epilogue              lw ra/s0 ; addiu sp,0x20 ; jr ra ; nop
prev leaf end         0x80191DC0 (func_80191B64)
next leaf start       0x80191FB8 (func_80191FB8, already ported)
```

Artifacts in this directory: `func_80191DC8.bin`, `func_80191DC8.s.txt`,
`carve_meta.json`.

Reproduction (private Disc1 MODE2/2352):

```bash
python3 tools/extract/psxiso.py extract disc1.bin PE.IMG /tmp/PE.IMG
# overlay[0x2DD8:0x2DD8+0x1F0] == func body; SHA-256 as above
```

## Behavior summary

Same CFG role as player `func_801214D4` (also 124 words), title BSS:

| Role | Player `1214D4` | Title `91DC8` |
| --- | --- | --- |
| format / pending | `800B0DBB` / `800B0CD0` | same |
| stream drain | `func_8007C564` | same |
| slice RECT | `801228F4..FA` | `801D148C..92` |
| buffer toggle | `801228E0` | `801D1478` |
| bank byte | `801228F2` | `801D148A` |
| bank x/w | `801228E2/E6 + bank*8` | `801D147A/7E + bank*8` |
| DMA1 dst words | `801228D8 + next*4` | `801D1470 + next*4` |
| DecDCTout | `func_8010C01C` | same |
| final-slice flag | `801228FC` | `801D1494` |
| reconfigure latch | `801223F8` | `801D0DC0` |
| display helper | `func_80121004` | `func_801918F8` |
| LoadImage | `func_8007506C` | same |

Arms:

1. Optional XA stream drain when signed `DBB` and pending `B0CD0`.
2. Save RECT; flip buffer; advance slice X by W; compare to bank X+W.
3. **Continuing slice:** `DecDCTout` next buffer with signed `(w*h)/2`
   (toward-zero via `(p+(p>>31))>>1`).
4. **Final slice:** set `1494=1`, flip bank, reload X/Y; if `0DC0==1`,
   XOR `DBB`, set slice W to 24/16, call `918F8((s8)(9CDDC^1),(s8)wide)`,
   store `0DC0=2`.
5. `LoadImage` saved RECT from **old** buffer (signed low-byte index).

No invented MDEC/GPU behavior beyond the existing ported callees and the
same host-only `PE_MDEC_HasDecode` guard used by `1214D4`.

## Psy-Q / donor comparison

- `DecDCToutCallback` in-tree is only the 9-word registration wrapper
  `func_8010C0D8` → `setIntrDMA(1, cb)` (B54K-AH). That is **not** this leaf.
- khasinski lists `trapIntrDMA` / libpress wrappers; **no** matched C for
  `0x80191DC8` (game title-overlay callback).
- Closest in-repo twin: `func_801214D4` in `movie_overlay_port.c`.

## Port leaf

```text
pc_port/game/boot/func_80191DC8_port.c
declaration: pe_port_compat.h  (void func_80191DC8(void))
CMake:       GAME_SRCS += game/boot/func_80191DC8_port.c
```

Match confidence: **high** (carve SHA + word-for-word CFG vs `1214D4` with
title addresses). Named host cut only: `PE_MDEC_HasDecode` →
`Bootstrap_ReturnVoid4("func_8010C01C","func_80191DC8",…)` (mirrors player).

## Handoff — Port `DispatchDmaCallback` arm

File: `pc_port/platform/pe_irq_delivery.c` — `DispatchDmaCallback`.

Insert **beside** the existing `0x801214D4` arm (do not alias to it):

```c
    if (handler == 0x80191DC8u) {
        func_80191DC8();
        return PE_Port_ShouldStop() ? PE_IRQ_SERVICE_BOUNDARY
                                    : PE_IRQ_SERVICE_RETURNED;
    }
```

Ensure `func_80191DC8` is visible (already declared in `pe_port_compat.h`
once this dig lands). Do **not** rewrite `PE_func_80074520_Dispatch`; do
**not** retarget title registration from `91DC8` to `1214D4`.

After wire: Disc1 path past first multi-frame MDEC DMA1 completion should
leave `func_80074520_dma_indirect_call` and enter title slice continue /
final-slice / `7506C` upload. Optional dig aid (unchanged advice from
74520 report): `Trace_Direct("func_80192934_enter")` before `BFA0`.

## Compile / tests

Leaf added to `GAME_SRCS` so it links into `pe_field_runtime`.
`DispatchDmaCallback` **not** wired on this dig (Port owns the arm).

Focused filter expectation after Port wires:

```text
PE_TEST_FILTER=B54KAH ./pc_port/build/pe-native-tests   # registration still
# Live Disc1 / DAY2 movie path: STOP dma_indirect_call for 91DC8 should clear
```

No new focused oracle in this dig (player `pe_movie_callback_oracle.py` is
BSS-specific to `801228xx`; a title twin would need `801D14xx` fixtures).

## Branch / SHA

```text
branch  dig/91dc8-decdctout
base    50b2a58 docs: DAY2-158o Linux suite counts (1354/1308/0/46)
leaf    pc_port/game/boot/func_80191DC8_port.c
report  docs/evidence/pe-day2-158-91dc8/REPORT.md
```
