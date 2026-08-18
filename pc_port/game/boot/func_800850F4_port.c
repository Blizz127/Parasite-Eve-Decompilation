/*
 * Phase 6E-B48/B48A — func_800850F4: SPU DMA transfer wrapper
 * (16 retail words / 0x40 bytes, exe 0x800850F4–0x80085133,
 * file offset 0x758F4, live split asm/disc1/74FB0.s:738–755;
 * all 16 instruction words verified against the SHA-exact executable).
 *
 * Retail signature:
 *   void func_800850F4(pe_addr_t src, uint32_t size)
 *
 * Exact wrapper order retained from B48:
 *   1. func_800850C0: D_8009D24C=1, register func_80085098.
 *   2. func_80085E54(src,size): hard-clamp to 0x7EFF0, issue
 *      func_8007D9F8, then clear D_8009B430 only if no callback exists.
 *
 * B48A correction: the mode-0 func_8007D9F8 provider records DMA4 and
 * returns.  It never applies func_80085098 effects.  A later controlled
 * DMA event performs the transfer and invokes the callback.
 */
#include "psx_compat.h"
#include "pe_spu_dma.h"

#include <stdio.h>
#include <stdlib.h>

#define GA_D_8009D24C 0x8009D24Cu
#define GA_D_8009B434 0x8009B434u
#define GA_D_8009B430 0x8009B430u
#define GA_D_8009B414 0x8009B414u
#define GA_D_8009B418 0x8009B418u
#define GA_D_8009B424 0x8009B424u

/* func_80085098, 10 retail words at 0x80085098..0x800850BF.
 * func_80085F44(0) is the first observable operation; it only changes the
 * guest callback slot.  Busy is cleared by the following store. */
void func_80085098(void)
{
    PE_StoreU32(GA_D_8009B434, 0);
    PE_StoreU32(GA_D_8009D24C, 0);
}

/* func_800850C0, 13 retail words at 0x800850C0..0x800850F3. */
static void PE_func_800850C0(void)
{
    PE_StoreU32(GA_D_8009D24C, 1);
    PE_StoreU32(GA_D_8009B434, PE_SPU_DMA_CALLBACK);
}

/* func_80085E54, 23 retail words at 0x80085E54..0x80085EAF. */
static uint32_t PE_func_80085E54(pe_addr_t src, uint32_t size)
{
    uint32_t clamped_size = size;
    uint32_t shift;
    uint32_t destination;

    if (clamped_size > PE_SPU_DMA_MAX_REQUEST) {
        clamped_size = PE_SPU_DMA_MAX_REQUEST;
    }

    /* The B48 boot chain sets func_80085F14(0), selecting the recovered
     * asynchronous DMA4 arm.  The alternate programmed-I/O arm remains a
     * truthful hard boundary rather than an invented completion policy. */
    if (PE_LoadU32(GA_D_8009B418) != 0) {
        fprintf(stderr,
                "FATAL: func_8007D9F8 programmed-I/O mode is unresolved\n");
        abort();
    }

    shift = PE_LoadU32(GA_D_8009B424);
    if (shift >= 32u ||
        ((uint64_t)PE_LoadU16(GA_D_8009B414) << shift) > UINT32_MAX) {
        fprintf(stderr, "FATAL: invalid SPU transfer-address shift %u\n",
                shift);
        abort();
    }
    destination = (uint32_t)PE_LoadU16(GA_D_8009B414) << shift;

    if (!PE_SpuDma_Begin(src, destination, clamped_size,
                         PE_LoadU32(GA_D_8009B434))) {
        fprintf(stderr,
                "FATAL: func_8007D9F8 could not issue native DMA4 transfer\n");
        abort();
    }

    /* Exact retail branch sense from 0x80085E84..0x80085E9F. */
    if (PE_LoadU32(GA_D_8009B434) == 0) {
        PE_StoreU32(GA_D_8009B430, 0);
    }
    return clamped_size;
}

void func_800850F4(pe_addr_t src, uint32_t size)
{
    PE_func_800850C0();
    (void)PE_func_80085E54(src, size);
}
