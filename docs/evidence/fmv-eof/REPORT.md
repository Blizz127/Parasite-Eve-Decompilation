# MDEC end-of-stream: the opening FMV completes without a boundary

Task: **FMV-EOF / MDEC-BOUNDARY**. Determine what retail does when the MDEC
input is exhausted at end-of-stream, implement that behavior faithfully, and
retire the `MDEC_missing_block` boundary that stopped the opening FMV.

Outcome: **FIXED**. The opening FMV now plays to the frame limit with
`stop_reason=frame-limit`, **0 invoked bootstrap stubs and 0 boundaries**, and
the native suite is back to **1404/1404** with a new regression assertion.

## Symptom

Before the fix, a headless run of the opening movie stopped at the last
DecDCTout of the stream:

```text
2078 func_80192934_enter
2080 func_80192934_dbd_abort
2081 func_80192CE8_media_clear
...
[STUB:BOOTSTRAP_RET] MDEC_missing_block
[HOST] stop_reason=unresolved-boundary
```

Backtrace at the boundary:

```text
#0 Bootstrap_ReturnVoid1
#1 MdecMacroblock
#2 PE_MDEC_ReadPixels
#3 PE_MDEC_Service
#4 HostFB_StreamTick
#5 func_80192934
#6 func_80192CE8
#7 func_801909B4
#8 func_8001220C
#9 main
```

The movie's own abort path (`func_80192934_dbd_abort`) and the media clear
(`func_80192CE8_media_clear`) are the retail end-of-movie sequence. The
boundary fired *after* the movie ended, when one more macroblock was requested
from input that was already fully consumed.

## Retail behavior

Two independent pieces of evidence fix the semantics.

### 1. `0xFE00` is the MDEC end-of-data code

The RLE stream uses `0xFE00` as the end-of-data marker, and a coefficient run
is `(code>>10, sign_extend_10(code))`. PCSX-ReARMed encodes this directly:

```c
#define MDEC_END_OF_DATA 0xfe00
```

and its `rl2blk()` never aborts a macroblock on end-of-data — it only
terminates the current block's coefficient run, leaving the block decoded to
that point. `mdec1Interrupt()` uses `*mdec.rl == MDEC_END_OF_DATA` solely to
clear the busy flag. End-of-data is a normal stream terminator, not an error.

Reference: PCSX-ReARMed `mdec.c`
(<https://github.com/LRG-CarbonEngine/PCSX-ReARMed/blob/main/mdec.c>),
psx-spx MDEC / RLE documentation.

### 2. Retail's own decoder pads the buffer with `0xFE00` to the bound

The guest VLC decoder `func_8010C89C` end-fills the DecDCTin buffer with
`0xFE00` up to the bound before it returns. The translated body records the
retail addresses:

```c
pad_exit:                       /* CB84: pad with FE00 to the bound */
    t0 = GA_VLC_BOUND;
    t1 = PE_LoadU32(t0);        /* CB8C */
    while ((int32_t)(a1 - t1) < 0) {    /* CB94: subu + bgez */
        PE_StoreU16(a1, 0xFE00u);    /* CBA0 */
        a1 += 2u;               /* CBA8 delay */
    }
```

So the MDEC is *never* handed a genuinely "missing" block: every trailing
block in the submitted buffer is explicit `0xFE00` end-of-data padding. A
block whose only remaining input is end-of-data decodes to zero coefficients —
which is exactly what retail produces. `MDEC_missing_block` was a port
artifact of stopping at the padding instead of decoding it.

## Fix

`pc_port/platform/pe_mdec.c`, `MdecBlock()`: exhaustion reached **at a block
boundary**, after the leading `0xFE00` pad has been consumed, now decodes the
block as all-zero and returns success instead of raising a boundary.

```c
if(g_input_pos==g_input_count) {
    /* End of stream, reached at a block boundary after the 0xFE00
     * end-of-data padding. ... a block with no remaining data decodes to
     * zero coefficients, so this is not an error.  Exhaustion in the
     * middle of a block stays a loud boundary below. */
    MdecIDCT(block);return 1;
}
```

The distinction is deliberate and keeps the loud contract where it belongs:

- **Exhaustion at a block boundary** (all remaining input is `0xFE00` padding,
  i.e. `func_8010C89C`'s end-fill) → end-of-stream, zero block. This is the FMV
  case that used to stop the run.
- **Exhaustion mid-block** (a DC/AC code was read, then the data stops before
  the block terminator) → still `MdecBoundary("MDEC_unterminated_block", ...)`.
  This is genuinely malformed input and must stay loud, so the existing
  `DAY2_mdec_pixels` "malformed block fabricated output" assertion still holds.

No other boundary was touched, and `g_decode_valid` is not cleared (the
existing `HostFB_VSync` gate on `PE_MDEC_HasDecode()` is unchanged).

## Verification

All commands run in a dedicated worktree at branch `agent/fmv-eof`.

| Check | Command | Result |
| --- | --- | --- |
| Opening FMV | `parasite-eve-port --disc-image <disc1> --headless --max-frames 4000` | `stop_reason=frame-limit`, `missing_block=0`, `unterminated=0`, `BOOTSTRAP_RET=0`, `invoked bootstrap stubs: 0` |
| Movie end path | trace inspection | `func_80192934_dbd_abort` + `func_80192CE8_media_clear` at frame 2080, then execution continues |
| Native suite | `./pc_port/build/pe-native-tests` | **1404 run, 1404 passed, 0 failed** |
| CTest | `ctest` | **11/11 passed** |
| Decomp-port generator | `gen_decomp_ports.py --check --allow-orphans` | `check: OK`, `stale boundaries: 0`, `mismatched: 0` |

The matching PS1 build is untouched: the change is confined to `pc_port/`
(native platform + native test), which is not compiled into `SLUS_006.62`, so
the retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` is unaffected.

## Regression coverage

Added to `pc_port/tests/test_mdec_pixels.h` (`DAY2_mdec_pixels`): one complete
block (`DC=0`, terminated by `0xFE00`) followed by one DecDCTout for a second
block. The trailing block must drain as all-zero pixels (`128`) with
`MDEC_missing_block` never logged and `PE_Port_ShouldStop()` false. The
pre-existing mid-block malformed case is kept immediately above it, so both
sides of the distinction are pinned.

## Files changed

- `pc_port/platform/pe_mdec.c` — end-of-stream block-boundary zero block.
- `pc_port/tests/test_mdec_pixels.h` — end-of-data regression assertions.
