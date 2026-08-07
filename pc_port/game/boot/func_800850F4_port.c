/*
 * Phase 6E-B48 — func_800850F4: SPU DMA transfer wrapper
 * (14 retail words / 0x40 bytes, exe 0x800850F4–0x80085133,
 * file offset 0x758F4, live split asm/disc1/74FB0.s:738–755;
 * all 14 instruction words verified exact against the SHA-exact
 * retail executable).
 *
 * Retail signature:
 *   void func_800850F4(pe_addr_t src, uint32_t size)
 *
 * Call sites:
 *   func_800851A8 @0x80085218 (extended prefix, B47)
 *   func_80085644 @0x80085764 (streaming bring-up, pe_stream.c)
 *   func_800872FC, func_800873BC, func_800874F0, func_800875A0,
 *   func_8008769C (other streaming callers, not yet translated)
 *
 * ROM-order operation map:
 *   Prologue: save $s0/$s1/$ra on 0x20-frame.
 *   $s0 = a0 (src), $s1 = a1 (size).
 *
 *   1. Call func_800850C0() — sets D_8009D24C = 1 (busy), registers
 *      func_80085098 as completion callback via func_80085F44.
 *
 *   2. Call func_80085E54(src, size) — clamps size to 0x7EFF0 max,
 *      calls func_8007D9F8 (SPU DMA transfer), checks callback.
 *
 *   Epilogue: restore $s1/$s0/$ra, jr $ra (nop delay slot).
 *
 * Classification: 1 — translated retail logic (platform provider).
 * The SPU DMA transfer is collapsed to synchronous completion in the
 * port (pe_stream.c pattern): D_8009D24C is set to 1 (busy), the
 * callback is registered, then immediately cleared (D_8009B434=0,
 * D_8009D24C=0).  This matches retail post-completion state.
 *
 * Dependency boundary:
 *   func_800850C0  INLINED (set busy + register callback)
 *   func_80085E54  INLINED (clamp size + collapsed DMA)
 *   func_8007D9F8  COLLAPSED (SPU DMA hardware — synchronous)
 *   func_80085098  COLLAPSED (completion callback — clears flags)
 *   func_80085F44  COLLAPSED (callback registration)
 */
#include "psx_compat.h"

/* ── Guest globals ──────────────────────────────────────────────────── */
#define GA_D_8009D24C  0x8009D24Cu   /* transfer completion flag          */
#define GA_D_8009B434  0x8009B434u   /* callback function pointer         */
#define GA_D_8009B430  0x8009B430u   /* callback state flag               */

/* func_80085098 address (completion callback) */
#define FUNC_80085098  0x80085098u

/* Maximum transfer size (from retail func_80085E54) */
#define PE_DMA_MAX_SIZE  0x7EFF0u

/* ── func_800850F4: SPU DMA transfer wrapper ────────────────────────── */
void func_800850F4(pe_addr_t src, uint32_t size)
{
    uint32_t clamped_size;

    (void)src;  /* src is passed through to DMA hardware (collapsed) */

    /* 1. func_800850C0(): set busy flag, register completion callback. */
    PE_StoreU32(GA_D_8009D24C, 1);
    PE_StoreU32(GA_D_8009B434, FUNC_80085098);

    /* 2. func_80085E54(src, size): clamp size, do DMA.
     *    func_8007D9F8 SPU DMA upload — collapsed; completion applied
     *    synchronously via func_80085098 callback effects. */
    clamped_size = (size > PE_DMA_MAX_SIZE) ? PE_DMA_MAX_SIZE : size;

    /* func_8007D9F8 DMA transfer — collapsed (no actual hardware).
     * In retail, this would start the DMA and the callback would fire
     * on completion.  In the port, completion is synchronous. */

    /* func_80085098 callback effects: clear callback, clear busy flag. */
    if (PE_LoadU32(GA_D_8009B434) != 0) {
        PE_StoreU32(GA_D_8009B430, 0);
    }
    PE_StoreU32(GA_D_8009B434, 0);
    PE_StoreU32(GA_D_8009D24C, 0);

    /* func_80085174 completion barrier — already complete, no spin needed. */
}
