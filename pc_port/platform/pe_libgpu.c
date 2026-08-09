/*
 * Phase 6E-A — libgpu: ResetGraph, SetGraphDebug, SetDefDrawEnv, SetDefDispEnv.
 *
 * ROM evidence:
 *   func_80074924 SetDefDrawEnv — asm/disc1/64F70.s @ file 0x65124
 *   func_800749D8 SetDefDispEnv — asm/disc1/64F70.s @ file 0x651D8 (15 words)
 *   func_80074A44 ResetGraph    — asm/disc1/65238.s @ file 0x65244
 *   func_80074BB8 SetGraphDebug — asm/disc1/65238.s (follows ResetGraph)
 *   func_80074A28 getter D_800956EC — src C leaf (32-bit global getter)
 * Classification: 2 (SDK host implementation).
 *
 * Collapsed retail effects (hardware-only):
 *   - func_80077A54(D_80095744 & 0xFFFFFF): GPU DMA channel init — no-op.
 *   - func_80077144(mode): GPU hardware init + video-standard query.
 *     B53E restores its retail DPCR channel-2 enable RMW at the collapsed
 *     owner; the USA disc on an NTSC host still deterministically yields
 *     standard 0.  Its return becomes the mode byte at D_8009574C and
 *     indexes the ROM video tables below.
 *   - ResetGraph m∈{0,3} version printf via func_80071A74, SetGraphDebug
 *     debug print via fn-ptr D_80095748: debug output only — skipped.
 *   - ResetGraph light path jalr D_80095744->0x34(1): GPU hw call — no-op.
 *
 * Video tables baked from retail ROM (SLUS_006.62, little-endian, lhu at
 * stride 4):  file 0x85FCC D_800957CC = {0x400,0x400,0x400}
 *             file 0x85FD8 D_800957D8 = {0x200,0x200,0x400}
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_gpu.h"
#include <string.h>

static const uint16_t kResetGraphW[3] = { 0x0400, 0x0400, 0x0400 };
static const uint16_t kResetGraphH[3] = { 0x0200, 0x0200, 0x0400 };

/* func_80077144 collapsed: modes 0/1/3/5 write DMA2 CHCR=0x401 and perform
 * DPCR = DPCR | 0x800 before their mode-specific GPU work.  Retain the exact
 * DPCR RMW here at its retail owner.  The canonical first lifecycle already
 * has B53B's idle CHCR; exact active-DMA ResetGraph abort semantics remain
 * outside this narrow prerequisite correction.  NTSC standard 0 is
 * deterministic for the USA disc. */
static int PE_Gpu_SetVideoMode(int mode)
{
    int m = mode & 7;

    if (m == 0 || m == 1 || m == 3 || m == 5) {
        PE_GPU_EnableDMA2();
    }
    return 0;
}

/* func_80074A28 — src C leaf getter for 32-bit D_800956EC. */
static int PE_Gpu_VideoStandard(void)
{
    return (int)PE_LoadU32(0x800956ECu);
}

int func_80074A44(int mode)
{
    int m = mode & 7;
    if (m == 0 || m == 3 || m == 5) {
        int std;
        memset(PE_Translate(0x8009574Cu, 0x80), 0, 0x80);
        func_80073C94();            /* ResetCallback (retail full path) */
        std = PE_Gpu_SetVideoMode(mode);
        PE_StoreU8(0x8009574Cu, (uint8_t)std);
        PE_StoreU8(0x8009574Du, 1);
        PE_StoreU16(0x80095750u, kResetGraphW[std]);
        PE_StoreU16(0x80095752u, kResetGraphH[std]);
        memset(PE_Translate(0x8009575Cu, 0x5C), 0xFF, 0x5C);
        memset(PE_Translate(0x800957B8u, 0x14), 0xFF, 0x14);
        return PE_LoadU8(0x8009574Cu);
    }
    /* Light path: no guest-RAM state changes (debug print + GPU hw call). */
    return 0;
}

int func_80074BB8(int level)
{
    int old = PE_LoadU8(0x8009574Eu);
    PE_StoreU8(0x8009574Eu, (uint8_t)level);
    return old;
}

pe_addr_t func_80074924(pe_addr_t env, int x, int y, int w, int h)
{
    int std = PE_Gpu_VideoStandard();
    PE_StoreU16(env + 0x00, (uint16_t)x);       /* clip.x */
    PE_StoreU16(env + 0x02, (uint16_t)y);       /* clip.y */
    PE_StoreU16(env + 0x04, (uint16_t)w);       /* clip.w */
    PE_StoreU16(env + 0x06, (uint16_t)h);       /* clip.h */
    PE_StoreU32(env + 0x0C, 0);                 /* tw zeroed @0x0C..0x13 */
    PE_StoreU32(env + 0x10, 0);
    PE_StoreU8(env + 0x19, 0);                  /* r0 */
    PE_StoreU8(env + 0x1A, 0);                  /* g0 */
    PE_StoreU8(env + 0x1B, 0);                  /* b0 */
    PE_StoreU8(env + 0x16, 1);                  /* dtd */
    PE_StoreU8(env + 0x17,
               (uint8_t)(std ? (h < 0x121) : (h < 0x101))); /* dfe */
    PE_StoreU16(env + 0x08, (uint16_t)x);       /* ofs[0] */
    PE_StoreU16(env + 0x0A, (uint16_t)y);       /* ofs[1] */
    PE_StoreU16(env + 0x14, 0xA);               /* tpage */
    PE_StoreU8(env + 0x18, 0);                  /* isbg */
    return env;
}

pe_addr_t func_800749D8(pe_addr_t env, int x, int y, int w, int h)
{
    PE_StoreU16(env + 0x00, (uint16_t)x);       /* disp.x */
    PE_StoreU16(env + 0x02, (uint16_t)y);       /* disp.y */
    PE_StoreU16(env + 0x04, (uint16_t)w);       /* disp.w */
    PE_StoreU16(env + 0x06, (uint16_t)h);       /* disp.h */
    PE_StoreU32(env + 0x08, 0);                 /* screen zeroed @0x08..0x0F */
    PE_StoreU32(env + 0x0C, 0);
    PE_StoreU8(env + 0x11, 0);                  /* isrgb24 */
    PE_StoreU8(env + 0x10, 0);                  /* isinter */
    PE_StoreU8(env + 0x13, 0);                  /* pad1 */
    PE_StoreU8(env + 0x12, 0);                  /* pad0 */
    return env;
}
