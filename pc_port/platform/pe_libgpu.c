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

/* PE-BTL88 — DrawOTagEnv software: 75EE0 SetDrawEnv + 71A34 memcpy.
 * 76C34(76B98) GPU enqueue is not this cut (B53 deferred). */

static uint32_t pe_clip_cmd(uint32_t cmd, int x, int y)
{
    int16_t xs = (int16_t)x;
    int16_t ys = (int16_t)y;
    int16_t lim_w = (int16_t)PE_LoadU16(0x80095750u);
    int16_t lim_h = (int16_t)PE_LoadU16(0x80095752u);
    uint32_t a0 = (uint32_t)x;
    uint32_t a1 = (uint32_t)y;

    if (xs < 0)
        a0 = 0u;
    else if ((int)lim_w - 1 < (int)xs)
        a0 = (uint32_t)((uint16_t)PE_LoadU16(0x80095750u) - 1u);
    if (ys < 0)
        a1 = 0u;
    else if ((int)lim_h - 1 < (int)ys)
        a1 = (uint32_t)((uint16_t)PE_LoadU16(0x80095752u) - 1u);
    return cmd | ((a1 & 0x3FFu) << 10) | (a0 & 0x3FFu);
}

static uint32_t func_80076150(uint32_t dfe, uint32_t dtd, uint32_t tpage)
{
    uint32_t v1 = 0xE1000000u;
    uint32_t v0;

    if (dtd != 0u)
        v1 |= 0x200u;
    v0 = tpage & 0x9FFu;
    if (dfe != 0u)
        v0 |= 0x400u;
    return v1 | v0;
}

static uint32_t func_800762A0(int x, int y)
{
    return 0xE5000000u | (((uint32_t)y & 0x7FFu) << 11) | ((uint32_t)x & 0x7FFu);
}

static uint32_t func_800762BC(pe_addr_t tw)
{
    uint32_t x, y, w, h;

    if (tw == 0u)
        return 0u;
    x = (PE_LoadU8(tw) >> 3) << 10;
    y = (PE_LoadU8(tw + 2u) >> 3) << 15;
    w = ((uint32_t)(-(int16_t)PE_LoadU16(tw + 4u)) & 0xFFu) >> 3;
    h = (((uint32_t)(-(int16_t)PE_LoadU16(tw + 6u)) & 0xFFu) >> 3) << 5;
    return 0xE2000000u | x | y | w | h;
}

static void func_80075EE0(pe_addr_t dr, pe_addr_t env)
{
    int16_t clip_x = (int16_t)PE_LoadU16(env);
    int16_t clip_y = (int16_t)PE_LoadU16(env + 2u);
    uint16_t clip_w = PE_LoadU16(env + 4u);
    uint16_t clip_h = PE_LoadU16(env + 6u);
    int16_t ofs_x = (int16_t)PE_LoadU16(env + 8u);
    int16_t ofs_y = (int16_t)PE_LoadU16(env + 0xAu);
    uint32_t t0 = 7u;
    uint16_t tx, ty, tw, th;
    int16_t lim_w;
    int16_t lim_h;
    int16_t ws;
    int16_t hs;

    PE_StoreU32(dr + 4u, pe_clip_cmd(0xE3000000u, clip_x, clip_y));
    PE_StoreU32(dr + 8u, pe_clip_cmd(0xE4000000u,
        (int)clip_x + (int)clip_w - 1, (int)clip_y + (int)clip_h - 1));
    PE_StoreU32(dr + 0xCu, func_800762A0(ofs_x, ofs_y));
    PE_StoreU32(dr + 0x10u, func_80076150(PE_LoadU8(env + 0x17u),
        PE_LoadU8(env + 0x16u), PE_LoadU16(env + 0x14u)));
    PE_StoreU32(dr + 0x14u, func_800762BC(env + 0xCu));
    PE_StoreU32(dr + 0x18u, 0xE6000000u);
    if (PE_LoadU8(env + 0x18u) == 0u) {
        PE_StoreU8(dr + 3u, (uint8_t)(t0 - 1u));
        return;
    }
    tx = (uint16_t)clip_x;
    ty = (uint16_t)clip_y;
    tw = clip_w;
    th = clip_h;
    lim_w = (int16_t)PE_LoadU16(0x80095750u);
    lim_h = (int16_t)PE_LoadU16(0x80095752u);
    ws = (int16_t)tw;
    if (ws < 0)
        tw = 0u;
    else if ((int)lim_w - 1 < (int)ws)
        tw = (uint16_t)(PE_LoadU16(0x80095750u) - 1u);
    hs = (int16_t)th;
    if (hs < 0)
        th = 0u;
    else if ((int)lim_h - 1 < (int)hs)
        th = (uint16_t)(PE_LoadU16(0x80095752u) - 1u);
    if (((tx & 0x3Fu) != 0u) || ((tw & 0x3Fu) != 0u)) {
        uint32_t tile = 0x60000000u |
            ((uint32_t)PE_LoadU8(env + 0x1Bu) << 16) |
            ((uint32_t)PE_LoadU8(env + 0x1Au) << 8) |
            (uint32_t)PE_LoadU8(env + 0x19u);
        int16_t px = (int16_t)tx - ofs_x;
        int16_t py = (int16_t)ty - ofs_y;

        t0 = 10u;
        PE_StoreU32(dr + 32u, tile);
        PE_StoreU32(dr, (uint32_t)(uint16_t)px | ((uint32_t)(uint16_t)py << 16));
        PE_StoreU32(dr + 36u, (uint32_t)tw | ((uint32_t)th << 16));
    } else {
        uint32_t fill = 0x02000000u |
            ((uint32_t)PE_LoadU8(env + 0x1Bu) << 16) |
            ((uint32_t)PE_LoadU8(env + 0x1Au) << 8) |
            (uint32_t)PE_LoadU8(env + 0x19u);

        t0 = 10u;
        PE_StoreU32(dr + 28u, fill);
        PE_StoreU32(dr + 32u, (uint32_t)tx | ((uint32_t)ty << 16));
        PE_StoreU32(dr + 36u, (uint32_t)tw | ((uint32_t)th << 16));
    }
    PE_StoreU8(dr + 3u, (uint8_t)(t0 - 1u));
}

void func_800754E4(pe_addr_t ot, pe_addr_t env)
{
    pe_addr_t dr;
    uint32_t tag;

    if (env == 0u || !PE_RangeIsRam(env, 0x5Cu))
        return;
    if (PE_LoadU8(0x8009574Eu) >= 2u)
        return;
    dr = env + 0x1Cu;
    func_80075EE0(dr, env);
    tag = PE_LoadU32(dr);
    PE_StoreU32(dr, (tag & 0xFF000000u) | (ot & 0x00FFFFFFu));
    /* jalr jtb[2](jtb[6], dr, 0x40, 0) is 76C34(76B98). Not this cut. */
    memcpy(PE_Translate(0x8009575Cu, 0x5Cu), PE_Translate(env, 0x5Cu), 0x5Cu);
}
