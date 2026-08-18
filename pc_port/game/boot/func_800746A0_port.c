/*
 * Phase 6E-B53G — complete translation of the Psy-Q DMA callback-slot
 * setter func_800746A0.
 *
 * Retail func_800746A0 is 43 words / 0xAC bytes at
 * 0x800746A0..0x8007474B (exclusive end 0x8007474C, executable file offset
 * 0x64EA0, live split asm/disc1/64CC8.s).  Its body SHA-256 is
 * ac160079410a40d719e5a8668f627d05d36d287d08959adc22fd3e069e4e99dd.
 *
 * The function is registration state only.  It never delivers an
 * interrupt, services DMA, pumps the retail ring, or invokes a callback.
 *
 * Exact retail behaviour, in instruction order:
 *
 *   0x800746A0  a2 = channel
 *   0x800746A4  v1 = &D_800956C0                (callback table base)
 *   0x800746AC  v0 = channel << 2               (no range check at all)
 *   0x800746B0  v1 = base + v0                  (raw slot address)
 *   0x800746B4  a3 = *slot                      (previous handler)
 *   0x800746BC  if (handler == previous) return previous       -- no writes
 *   0x800746C4  if (handler == 0) goto remove
 *   -- install ------------------------------------------------------------
 *   0x800746D8  *slot = handler                 (slot store precedes DICR)
 *   0x800746DC  d = *D_800956BC                 (DICR read, 0x1F8010F4)
 *   0x800746E4  d &= 0x00FFFFFF                 (drop flags 24..30 and 31)
 *   0x800746EC  bit = 1 << ((channel + 16) & 31)
 *   0x800746F8  *D_800956BC = d | bit | 0x00800000
 *   0x80074704  return previous
 *   -- remove -------------------------------------------------------------
 *   0x80074714  *slot = 0
 *   0x80074718  d = *D_800956BC
 *   0x80074720  d &= 0x00FFFFFF
 *   0x80074728  d |= 0x00800000                 (master set BEFORE clear)
 *   0x80074730  bit = 1 << ((channel + 16) & 31)
 *   0x80074738  *D_800956BC = d & ~bit
 *   0x80074740  return previous
 *
 * Both paths unconditionally set master-enable bit 23.  On the removal
 * path the master OR happens before the channel-enable clear, so channel 7
 * (whose enable bit IS bit 23) clears the master bit again.  That exact
 * asymmetry is reproduced rather than normalised.
 *
 * The 0x00FFFFFF mask means the written word carries zeros in DICR bits
 * 24..30, so the W1C completion flags are never acknowledged here, and
 * carries zero in bit 31, which is read-only master-flag state on PSX
 * hardware.  Bit 31 is read by the `lw` and then discarded; it is never
 * tested and never written as one.  No B53B platform extension is needed.
 *
 * D_800956BC is a retail .data word holding the literal MMIO address
 * 0x1F8010F4.  DICR hardware state itself stays in the single B53B
 * pe_gpu authority; this translation performs the read/modify/write
 * through PE_GPU_ReadDICR / PE_GPU_WriteDICR and creates no second copy.
 *
 * The callback table stays guest-backed at D_800956C0 (eight 32-bit guest
 * identities, 0x800956C0..0x800956DF, zero in the retail image).  No native
 * function pointer and no native mirror is introduced.
 */

#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"

/* Retail .data: eight 32-bit guest callback identities cleared by
 * func_800744D4 through func_8007474C(&D_800956C0, 8). */
#define GA_DMA_CALLBACK_TABLE   0x800956C0u

/* Retail .data word holding the DICR MMIO address literal 0x1F8010F4. */
#define GA_DMA_DICR_POINTER     0x800956BCu
#define PE_DMA_DICR_ADDRESS     0x1F8010F4u

/* `lui $v0,0xFF` + `ori $v0,$v0,0xFFFF` at 0x800746C8 / 0x800746D4 and
 * 0x800746C8 / 0x80074710. */
#define DICR_CONTROL_MASK       0x00FFFFFFu

/* `lui $v1,0x0080` at 0x800746F0 and `lui $v0,0x0080` at 0x80074724. */
#define DICR_MASTER_ENABLE      0x00800000u

/* `addiu $v1,$a2,0x10` / `sllv` at 0x800746E0..0x800746EC and
 * 0x8007471C..0x80074730.  MIPS sllv uses only the low five shift bits. */
static uint32_t dma_channel_enable_bit(uint32_t dma_channel)
{
    return 1u << ((dma_channel + 16u) & 31u);
}

pe_addr_t func_800746A0(uint32_t dma_channel, pe_addr_t handler)
{
    /* 0x800746AC/0x800746B0: `sll`+`addu` in exact 32-bit guest
     * arithmetic.  Retail performs no minimum, maximum, signedness, or
     * membership check on the channel, so none is added here. */
    pe_addr_t slot = (pe_addr_t)(GA_DMA_CALLBACK_TABLE +
                                 (uint32_t)(dma_channel << 2));
    pe_addr_t previous;
    uint32_t enable_bit;
    uint32_t dicr;

    /* 0x800746B4: the previous handler is read before any decision. */
    previous = (pe_addr_t)PE_LoadU32(slot);

    /* 0x800746BC: identical handler returns immediately.  No slot store
     * and no DICR access happens on this path — a same-handler reinstall
     * and a zero-to-zero removal are both complete no-ops. */
    if (handler == previous) {
        return previous;
    }

    enable_bit = dma_channel_enable_bit(dma_channel);

    if (handler != 0u) {
        /* 0x800746D8 then 0x800746DC: retail stores the slot before it
         * reads DICR. */
        PE_StoreU32(slot, (uint32_t)handler);
        dicr = PE_GPU_ReadDICR();
        PE_GPU_WriteDICR((dicr & DICR_CONTROL_MASK) |
                         enable_bit | DICR_MASTER_ENABLE);
    } else {
        /* 0x80074714 then 0x80074718: same ordering on the removal path. */
        PE_StoreU32(slot, 0u);
        dicr = PE_GPU_ReadDICR();
        PE_GPU_WriteDICR(((dicr & DICR_CONTROL_MASK) | DICR_MASTER_ENABLE) &
                         ~enable_bit);
    }

    /* 0x800746C0 / 0x80074704 / 0x80074740: every return path forwards the
     * full 32-bit previous guest identity, never a boolean or a pointer. */
    return previous;
}

/* Read-only evidence accessors.  They exist so tests and the host can name
 * the retail authorities without duplicating them. */
pe_addr_t PE_DMA_CallbackSlotAddress(uint32_t dma_channel)
{
    return (pe_addr_t)(GA_DMA_CALLBACK_TABLE +
                       (uint32_t)(dma_channel << 2));
}

pe_addr_t PE_DMA_DicrPointerAddress(void)
{
    return (pe_addr_t)GA_DMA_DICR_POINTER;
}

uint32_t PE_DMA_DicrMmioAddress(void)
{
    return PE_DMA_DICR_ADDRESS;
}
