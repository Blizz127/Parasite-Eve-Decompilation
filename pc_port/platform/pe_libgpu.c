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
#include "game_port.h"
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

/* Phase 6E-B54K-S — DrawSync wrapper plus the execution-proven behavior of
 * func_80077294.  Retail hardware progresses independently between calls to
 * func_80077404.  The deterministic port admits exactly one already-active
 * DMA2 token at each such wait poll; the existing checkpoint keeps GPU,
 * DICR, CPU-IRQ, callback, and pump ordering in their established owners.
 * No polling read evolves hardware by itself. */
#define GA_GPU_RING_PRODUCER       0x80095874u
#define GA_GPU_RING_CONSUMER       0x80095878u
#define GA_GPU_TIMEOUT_DEADLINE    0x80095888u
#define GA_GPU_TIMEOUT_POLLS       0x8009588Cu
#define GPU_QUEUE_MASK             63u

static int PE_DrawSyncPendingCount(void)
{
    return (int)((PE_LoadU32(GA_GPU_RING_PRODUCER) -
                  PE_LoadU32(GA_GPU_RING_CONSUMER)) & GPU_QUEUE_MASK);
}

/* Execution-proven normal half of func_80077404.  The timeout/recovery half
 * rewrites the ring and GPU registers; it remains a named boundary rather
 * than being approximated. */
static int PE_DrawSyncWaitPoll(void)
{
    uint32_t now = PE_GPU_VSyncQuery();
    uint32_t deadline = PE_LoadU32(GA_GPU_TIMEOUT_DEADLINE);
    uint32_t polls = PE_LoadU32(GA_GPU_TIMEOUT_POLLS);

    if ((int32_t)deadline < (int32_t)now ||
        (int32_t)polls > (int32_t)0x000F0000) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80077404_timeout_recovery_cut", "func_80074DC0", -1,
            0x80077458u, deadline, now, polls, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    PE_StoreU32(GA_GPU_TIMEOUT_POLLS, polls + 1u);
    return 1;
}

static int PE_DrawSyncWaitStep(int prior_progress)
{
    PEPortDmaIrqCheckpointResult result;

    if (!PE_DrawSyncWaitPoll())
        return 0;

    if (PE_GPU_DMA2Pending()) {
        result = PE_Port_ServiceDmaIrqCheckpoint();
        if (result == PE_PORT_DMA_IRQ_CHECKPOINT_RETURNED)
            return 1;
        if (result == PE_PORT_DMA_IRQ_CHECKPOINT_BOUNDARY ||
            result == PE_PORT_DMA_IRQ_CHECKPOINT_STALE)
            return 0;
    }

    if (prior_progress)
        return 1;

    (void)Bootstrap_ReturnInt4Indirect(
        "func_80077404_wait_cut", "func_80074DC0", -1,
        0x80077404u, (uintptr_t)PE_DrawSyncPendingCount(),
        PE_GPU_ReadDMA2CHCR(), PE_GPU_ReadStatus(),
        PE_LoadU32(GA_GPU_TIMEOUT_POLLS), NULL, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}

static int PE_func_80077294(int mode)
{
    if (mode != 0) {
        int pending = PE_DrawSyncPendingCount();
        int returned = 0;

        if (pending != 0) {
            (void)PE_func_80076EE4_Pump(&returned);
            if (!returned || PE_Port_ShouldStop())
                return pending;
        }
        if ((PE_GPU_ReadDMA2CHCR() & PE_GPU_DMA2_CHCR_BUSY) != 0u)
            return pending;
        if ((PE_GPU_ReadStatus() & PE_GPU_STATUS_READY_GP0) != 0u)
            return pending;
        return pending != 0 ? pending : 1;
    }

    (void)func_800773D0();
    for (;;) {
        int pending = PE_DrawSyncPendingCount();

        if (pending != 0) {
            uint32_t before = PE_LoadU32(GA_GPU_RING_CONSUMER);
            int returned = 0;

            (void)PE_func_80076EE4_Pump(&returned);
            if (!returned || PE_Port_ShouldStop())
                return -1;
            if (!PE_DrawSyncWaitStep(
                    PE_LoadU32(GA_GPU_RING_CONSUMER) != before))
                return -1;
            continue;
        }

        if ((PE_GPU_ReadDMA2CHCR() & PE_GPU_DMA2_CHCR_BUSY) != 0u) {
            if (!PE_DrawSyncWaitStep(0))
                return -1;
            continue;
        }

        if ((PE_GPU_ReadStatus() & PE_GPU_STATUS_READY_GP0) == 0u) {
            if (!PE_DrawSyncWaitStep(0))
                return -1;
            continue;
        }
        return 0;
    }
}

int func_80074DC0(int mode)
{
    /* Host telemetry/presentation policy is separate from GPU authority. */
    HostFB_DrawSync(mode);
    return PE_func_80077294(mode);
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

/* Phase 6E-B54K-W — complete PsyQ PutDrawEnv wrapper
 * [0x80075424,0x800754E4), 48 words.  The debug-print branch is diagnostic
 * only. The state-bearing path builds one terminal DR_ENV packet, submits it
 * through the retail jtb[2]/jtb[6] identities, then caches exactly 0x5C
 * bytes at D_8009575C after the worker returns. */
pe_addr_t func_80075424(pe_addr_t env)
{
    const pe_addr_t jtb_pointer = 0x80095744u;
    const pe_addr_t dispatch_identity = 0x80076C34u;
    const pe_addr_t worker_identity = 0x80076B98u;
    pe_addr_t dr;
    pe_addr_t jtb;
    pe_addr_t dispatch;
    pe_addr_t worker;

    if (!PE_RangeIsRam(env, 0x5Cu)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80075424_env_span", "func_80075424", 0,
            0x80075424u, env, 0u, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return env;
    }

    dr = env + 0x1Cu;
    func_80075EE0(dr, env);
    PE_StoreU32(dr, PE_LoadU32(dr) | 0x00FFFFFFu);

    jtb = PE_LoadU32(jtb_pointer);
    if (!PE_RangeIsRam(jtb, 0x20u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076C34", "func_80075424", 0, 0u,
            0u, dr, 0x40u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    } else {
        dispatch = PE_LoadU32(jtb + 8u);
        worker = PE_LoadU32(jtb + 0x18u);
        if (dispatch == dispatch_identity && worker == worker_identity) {
            (void)func_80076C34(worker, dr, 0x40, 0u);
        } else {
            (void)Bootstrap_ReturnInt4Indirect(
                "func_80076C34", "func_80075424", 0, dispatch,
                worker, dr, 0x40u, 0u, NULL, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        }
    }
    if (PE_Port_ShouldStop())
        return env;

    memcpy(PE_Translate(0x8009575Cu, 0x5Cu),
           PE_Translate(env, 0x5Cu), 0x5Cu);
    return env;
}

void func_800754E4(pe_addr_t ot, pe_addr_t env)
{
    const pe_addr_t jtb_pointer = 0x80095744u;
    const pe_addr_t dispatch_identity = 0x80076C34u;
    const pe_addr_t worker_identity = 0x80076B98u;
    pe_addr_t dr;
    pe_addr_t jtb;
    pe_addr_t dispatch;
    pe_addr_t worker;
    uint32_t tag;

    if (env == 0u || !PE_RangeIsRam(env, 0x5Cu))
        return;
    /* Debug print only at level >= 2 (7506C pattern: record, stop,
     * return — retail prints then continues; the draw below is the
     * residual).  Previously this returned without printing. */
    if (PE_LoadU8(0x8009574Eu) >= 2u) {
        pe_addr_t print_target = PE_LoadU32(0x80095748u);
        Bootstrap_ReturnVoid4Indirect(
            "func_80071A74", "func_800754E4", print_target,
            0x80011954u, ot, env, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    dr = env + 0x1Cu;
    func_80075EE0(dr, env);
    tag = PE_LoadU32(dr);
    PE_StoreU32(dr, (tag & 0xFF000000u) | (ot & 0x00FFFFFFu));
    /* jalr jtb[2](jtb[6], dr, 0x40, 0): the Phase 6E-DRW1 completion of
     * this cut, mirroring func_80075424 above. */
    jtb = PE_LoadU32(jtb_pointer);
    if (!PE_RangeIsRam(jtb, 0x20u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076C34", "func_800754E4", 0, 0u,
            0u, dr, 0x40u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    } else {
        dispatch = PE_LoadU32(jtb + 8u);
        worker = PE_LoadU32(jtb + 0x18u);
        if (dispatch == dispatch_identity && worker == worker_identity) {
            (void)func_80076C34(worker, dr, 0x40, 0u);
        } else {
            (void)Bootstrap_ReturnInt4Indirect(
                "func_80076C34", "func_800754E4", 0, dispatch,
                worker, dr, 0x40u, 0u, NULL, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        }
    }
    if (PE_Port_ShouldStop())
        return;
    memcpy(PE_Translate(0x8009575Cu, 0x5Cu), PE_Translate(env, 0x5Cu), 0x5Cu);
}
