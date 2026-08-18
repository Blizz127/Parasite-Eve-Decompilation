/*
 * Phase 6E-B53E — Psy-Q LoadImage command-issue worker func_80076664.
 *
 * Retail body: 143 words / 0x23C bytes, 0x80076664..0x8007689F
 * (exclusive end 0x800768A0), executable file offset 0x66E64.  The exact
 * body SHA-256 is 79dd44e3819f51eb5928c9af3ec0d6906cc3d95765dc2718c3c10e49ef78e0f7.
 *
 * This is retail command-issue logic, not hardware authority.  It retains no
 * RECT or source host pointer and drives only the single B53B GPU/DMA2
 * platform.  The CPU-fed prefix is visible immediately; an issued DMA2
 * suffix remains busy and invisible until explicit platform completion.
 */

#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"

#include <stddef.h>
#include <stdint.h>

#define GA_GPU_LIMIT_W 0x80095750u
#define GA_GPU_LIMIT_H 0x80095752u
#define GA_GPU_TIMEOUT_DEADLINE 0x80095888u
#define GA_GPU_TIMEOUT_POLLS    0x8009588Cu

#define LOADIMAGE_GP1_DMA_OFF 0x04000000u
#define LOADIMAGE_GP1_DMA_CPU 0x04000002u
#define LOADIMAGE_GP0_CACHE   0x01000000u
#define LOADIMAGE_GP0_A0      0xA0000000u
#define LOADIMAGE_DMA_CHCR    0x01000201u

/* Convert a MIPS 32-bit register bit pattern to its exact signed value
 * without relying on an implementation-defined unsigned-to-signed cast. */
static int32_t MipsSigned32(uint32_t value)
{
    if (value <= 0x7FFFFFFFu) return (int32_t)value;
    return -1 - (int32_t)(0xFFFFFFFFu - value);
}

static int32_t MipsSra(uint32_t value, unsigned shift)
{
    int32_t signed_value = MipsSigned32(value);

    if (signed_value >= 0) return signed_value / (int32_t)(1u << shift);
    return -1 - (int32_t)(((uint32_t)(-(int64_t)signed_value - 1)) >> shift);
}

static int SourceSpanIsSafe(pe_addr_t source, uint32_t transfer_words)
{
    uint64_t bytes = (uint64_t)transfer_words * 4u;

    return source != 0u && (source & 3u) == 0u &&
           bytes <= SIZE_MAX && PE_RangeIsRam(source, (size_t)bytes);
}

/* Worker-bounded prefix of func_80077404, PCs 0x80077404..0x80077454.
 * Its ordinary path is entirely deterministic software state: inert VSync
 * query, signed deadline comparison, and one exact prior-count test.  The
 * recovery suffix beginning at 0x80077458 is deliberately still exposed as
 * func_80077404 rather than inventing its diagnostics, DMA abort, queue
 * reset, DPCR, or GP1 effects. */
static int WaitTimeoutPrefix(void)
{
    uint32_t current_vsync = PE_GPU_VSyncQuery();
    uint32_t deadline = PE_LoadU32(GA_GPU_TIMEOUT_DEADLINE);
    uint32_t old_polls;

    if (MipsSigned32(deadline) < MipsSigned32(current_vsync)) goto recovery;

    old_polls = PE_LoadU32(GA_GPU_TIMEOUT_POLLS);
    PE_StoreU32(GA_GPU_TIMEOUT_POLLS, old_polls + 1u);
    if (MipsSigned32(0x000F0000u) < MipsSigned32(old_polls)) goto recovery;
    return 0;

recovery:
    (void)Bootstrap_ReturnInt(
        "func_80077404", "func_80076664", -1);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return -1;
}

/* 0x8007674C..0x8007679C.  Status reads and the timeout prefix do not make
 * the GPU ready.  Canonical Disc 1 enters ready and bypasses the helper. */
static int WaitForGp0Ready(void)
{
    while ((PE_GPU_ReadStatus() & PE_GPU_STATUS_READY_GP0) == 0u) {
        if (WaitTimeoutPrefix() != 0) return 0;
    }
    return 1;
}

static int IssueLoadImage(RECT *rect, pe_addr_t guest_rect,
                          int guest_writeback, pe_addr_t source)
{
    int16_t width_limit;
    int16_t height_limit;
    int16_t width;
    int16_t height;
    uint32_t pixels;
    uint32_t rounded;
    int32_t transfer_words_signed;
    int32_t blocks_signed;
    uint32_t transfer_words;
    uint32_t blocks;
    uint32_t remainder;
    uint32_t i;
    pe_addr_t dma_source;
    uint32_t bcr;

    /* 0x80076690..0x800766D8: signed width clamp, with the width store in
     * the height bltz delay slot.  ResetGraph(0) establishes 1024. */
    width_limit = (int16_t)PE_LoadU16(GA_GPU_LIMIT_W);
    width = rect->w;
    if (width < 0) width = 0;
    else if (width_limit < width) width = width_limit;
    rect->w = width;
    if (guest_writeback) PE_StoreU16(guest_rect + 4u, (uint16_t)width);

    /* 0x800766CC..0x80076718: signed height clamp/store, then signed
     * 16x16 multiplication.  x/y are deliberately untouched. */
    height_limit = (int16_t)PE_LoadU16(GA_GPU_LIMIT_H);
    height = rect->h;
    if (height < 0) height = 0;
    else if (height_limit < height) height = height_limit;
    rect->h = height;
    if (guest_writeback) PE_StoreU16(guest_rect + 6u, (uint16_t)height);

    pixels = (uint32_t)((int32_t)width * (int32_t)height);

    /* 0x80076720..0x80076748.  Retail forms pixels+1 modulo 2^32, adds
     * its sign bit, then uses arithmetic shifts by one and five. */
    rounded = pixels + 1u;
    rounded += rounded >> 31;
    transfer_words_signed = MipsSra(rounded, 1u);
    blocks_signed = MipsSra(rounded, 5u);
    if (transfer_words_signed <= 0) return -1;

    transfer_words = (uint32_t)transfer_words_signed;
    blocks = (uint32_t)blocks_signed;
    remainder = transfer_words - blocks * 16u;

    /* Native safety envelope required before the first GP0 mutation or
     * CPU lw.  The later B53B DMA validation alone cannot protect the
     * 0..15 words consumed synchronously below. */
    if (!SourceSpanIsSafe(source, transfer_words)) return -1;

    /* 0x8007674C..0x8007679C: exact GPUSTAT bit-26 poll. */
    if (!WaitForGp0Ready()) return -1;

    /* The raw retail worker relies on ResetGraph/dispatcher invariants.
     * Native adapters must reject a dirty parser, active DMA, or missing
     * channel-2 prerequisite before any GP command can be misparsed or any
     * CPU-prefix pixel becomes visible.  This query is inert and does not
     * enable DMA2 inside the worker. */
    if (!PE_GPU_CanBeginImageLoad(blocks != 0u)) return -1;

    /* 0x800767A0..0x800767FC: GP1 DMA-off, GP0 cache clear, A0 command,
     * raw x/y word, then the clamped w/h word. */
    if (!PE_GPU_WriteGP1(LOADIMAGE_GP1_DMA_OFF) ||
        !PE_GPU_WriteGP0(LOADIMAGE_GP0_CACHE) ||
        !PE_GPU_WriteGP0(LOADIMAGE_GP0_A0) ||
        !PE_GPU_WriteGP0((uint32_t)(uint16_t)rect->x |
                         ((uint32_t)(uint16_t)rect->y << 16)) ||
        !PE_GPU_WriteGP0((uint32_t)(uint16_t)rect->w |
                         ((uint32_t)(uint16_t)rect->h << 16))) {
        return -1;
    }

    /* 0x80076800..0x80076828: remainder is exactly transfer_words mod 16,
     * and each source word is written to GP0 in increasing address order. */
    for (i = 0; i < remainder; i++) {
        if (!PE_GPU_WriteGP0(PE_LoadU32(source + i * 4u))) return -1;
    }

    if (blocks != 0u) {
        /* 0x8007682C..0x80076874: asynchronous RAM-to-GPU request DMA.
         * Issue does not complete, clear busy, pump the ring, or dispatch a
         * callback.  SourceSpanIsSafe proved the complete original span. */
        dma_source = source + remainder * 4u;
        bcr = (blocks << 16) | 0x10u;
        if (!PE_GPU_WriteGP1(LOADIMAGE_GP1_DMA_CPU) ||
            !PE_GPU_DMA2Issue(dma_source, bcr, LOADIMAGE_DMA_CHCR)) {
            return -1;
        }
    }

    return 0;
}

int func_80076664(pe_addr_t rect_address, pe_addr_t source)
{
    RECT rect;

    /* 0x80076688: timeout state is initialized before retail dereferences
     * the RECT.  An invalid guest RECT is rejected by the native adapter
     * after that exact persistent prefix and before any unsafe host access. */
    (void)func_800773D0();
    if (!PE_RangeIsRam(rect_address, sizeof(rect))) return -1;

    rect.x = (int16_t)PE_LoadU16(rect_address + 0u);
    rect.y = (int16_t)PE_LoadU16(rect_address + 2u);
    rect.w = (int16_t)PE_LoadU16(rect_address + 4u);
    rect.h = (int16_t)PE_LoadU16(rect_address + 6u);
    return IssueLoadImage(&rect, rect_address, 1, source);
}

int PE_func_80076664_Inline8(uint32_t rect_word0, uint32_t rect_word1,
                             pe_addr_t source)
{
    RECT rect;

    /* The B52 caller's transient RECT is already represented by two retail
     * words.  Reconstruct it by value, use it synchronously, and retain no
     * native pointer after this call returns. */
    rect.x = (int16_t)(rect_word0 & 0xFFFFu);
    rect.y = (int16_t)(rect_word0 >> 16);
    rect.w = (int16_t)(rect_word1 & 0xFFFFu);
    rect.h = (int16_t)(rect_word1 >> 16);
    (void)func_800773D0();
    return IssueLoadImage(&rect, 0u, 0, source);
}
