/*
 * Phase 6E-B54C — func_800718D0: TIM walker (font atlas upload).
 *
 * Complete retail body:
 *   29 words / 0x74 bytes, exe 0x800718D0–0x80071944 (exclusive),
 *   file offset 0x620D0, live split asm/disc1/61C64.s.
 *   All 29 words verified against SHA-1
 *   452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * ABI: pe_addr_t func_800718D0(pe_addr_t tim)
 *   a0 = TIM base (canonical Disc 1: lw(D_800B0CD8+0x174) = 0x8012F1A0)
 *   v0 = image-pixel pointer (s1 = image_block + 12). Callers at
 *   0x8006AFA8 / 0x8006B0A4 discard it.
 *
 * Body (ROM order):
 *   flag = lw(tim+4); s0 = 0 in the beq delay slot
 *   if (flag & 8):                 # TIM has a CLUT chunk
 *       s0 = tim + 8
 *       v0 = s0 + lw(tim+8)        # skip CLUT bnum+RECT+data
 *   else:
 *       v0 = tim + 8
 *   jal func_8007506C(v0+4, v0+12) # IMAGE first
 *   if (s0 != 0)
 *       jal func_8007506C(s0+4, s0+12) # then CLUT
 *   return s1 = v0+12
 *
 * Sole callee is the already-faithful B52 func_8007506C. This translation
 * does not invent poll=0, does not enter func_80030894, and does not add
 * a DMA checkpoint. B54F reaches this walker from the live 6AD40 prefix.
 *
 * Classification: 1 — complete translated retail leaf.
 */
#include "psx_compat.h"
#include "game_port.h"

static void loadimage_tim_chunk(pe_addr_t block)
{
    RECT rect;

    rect.x = (int16_t)PE_LoadU16(block + 4u);
    rect.y = (int16_t)PE_LoadU16(block + 6u);
    rect.w = (int16_t)PE_LoadU16(block + 8u);
    rect.h = (int16_t)PE_LoadU16(block + 10u);
    (void)func_8007506C(&rect, block + 12u);
}

pe_addr_t func_800718D0(pe_addr_t tim)
{
    uint32_t flag = PE_LoadU32(tim + 4u);
    pe_addr_t clut_block = 0u;          /* s0; delay-slot zero of the beq */
    pe_addr_t image_block;

    if ((flag & 8u) != 0u) {
        clut_block = tim + 8u;          /* 0x800718F8 */
        image_block = clut_block + PE_LoadU32(tim + 8u);
    } else {
        image_block = tim + 8u;         /* 0x80071904 */
    }

    loadimage_tim_chunk(image_block);   /* 0x80071910 image first */
    if (clut_block != 0u)
        loadimage_tim_chunk(clut_block); /* 0x80071920 CLUT second */

    return image_block + 12u;           /* 0x80071928 v0 = s1 */
}
