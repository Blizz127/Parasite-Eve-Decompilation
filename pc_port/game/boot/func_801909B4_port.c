/*
 * Phase 6E-B54K-R/Y — func_801909B4 through MoveImage, display setup, the
 * one-time overlay-local initializer, and its saved-bit branch.
 *
 * Retail overlay body: [0x801909B4,0x801918F8), 977 words.
 * Implemented caller path: [0x801909B4,0x80190D8C), 246 words through the
 * positive-arm call. MoveImage traverses the retail dispatcher and exact
 * one-packet GP0(80h) worker.
 * func_80190660 now completes its 480-frame loop.  Disc 1's saved bit then
 * enters func_80192CE8(1); the saved-bit-zero arm remains the exact named
 * structural cut at 0x80191120.
 */
#include "psx_compat.h"
#include "game_port.h"

#include <string.h>

#define GA_DRAWENV_SOURCE_0  0x800BCDC8u
#define GA_DRAWENV_SOURCE_1  0x800BCE24u
#define GA_DISPENV_SOURCE_0  0x800BCE80u
#define GA_DISPENV_SOURCE_1  0x800BCE94u
#define GA_DRAWENV_COPY_0    0x801D1498u
#define GA_DRAWENV_COPY_1    0x801D14F4u
#define GA_DISPENV_COPY_0    0x801D1550u
#define GA_DISPENV_COPY_1    0x801D1564u
#define GA_TASK_POOL         0x801D11CCu
#define GA_TASK_FREE_HEAD    0x801D136Cu
#define GA_TASK_ACTIVE_HEAD  0x801D1370u
#define GA_TASK_ACTIVE_TAIL  0x801D1374u
#define GA_TASK_PENDING_HEAD 0x801D1378u
#define GA_TASK_PENDING_TAIL 0x801D137Cu
#define GA_ATTRACT_COUNTER   0x801D1380u

extern void func_8005E57C(int value);
extern void func_8005C1EC(int enabled);
extern void func_80042538(void);
extern int func_80190660(void);
extern int func_80192CE8(int index);
extern int func_8007506C(const RECT *rect, pe_addr_t data);
extern pe_addr_t func_80075424(pe_addr_t env);
void PE_Overlay_UploadDirtyRect(void);
extern void func_8018F2F4(void);
extern void func_8018F468(void);
extern void func_80190064(void);
extern pe_addr_t func_8018FBC0(int kind);
extern void func_800425DC(void);
extern void func_8003EB04(void);

static void CopyGuestBytes(pe_addr_t destination, pe_addr_t source,
                           uint32_t size)
{
    memcpy(PE_Translate(destination, size),
           PE_TranslateConst(source, size), size);
}

int func_801909B4(void)
{
    pe_addr_t arena;
    pe_addr_t second;
    pe_addr_t environment0;
    pe_addr_t environment1;
    uint32_t saved_bit;
    RECT move_rect;
    RECT clear_rect;

    CopyGuestBytes(GA_DISPENV_COPY_0, GA_DISPENV_SOURCE_0, 0x14u);
    CopyGuestBytes(GA_DISPENV_COPY_1, GA_DISPENV_SOURCE_1, 0x14u);
    CopyGuestBytes(GA_DRAWENV_COPY_0, GA_DRAWENV_SOURCE_0, 0x5Cu);
    CopyGuestBytes(GA_DRAWENV_COPY_1, GA_DRAWENV_SOURCE_1, 0x5Cu);

    /* 0x80190A90: retained in s4 for the later 0x80190D7C branch. */
    saved_bit = PE_LoadU8(0x800B0DCDu) & 1u;

    arena = PE_LoadU32(0x80011610u);
    second = arena + 0x1C080u;
    PE_StoreU32(0x800B0E50u, arena + 0x4080u);
    PE_StoreU32(0x801D11BCu, arena);
    PE_StoreU32(0x801D11C0u, second);
    PE_StoreU32(0x800B0E54u, second + 0x4080u);
    PE_StoreU32(0x800B0E38u, arena + 0x80u);
    PE_StoreU32(0x800B0E3Cu, second + 0x80u);

    func_8005E57C(1);
    func_8005C1EC(1);
    if (PE_Port_ShouldStop())
        return -1;
    func_80042538();
    func_80074D28(0);

    move_rect.x = 320;
    move_rect.y = 0;
    move_rect.w = 160;
    move_rect.h = 256; /* jal delay slot at 0x80190C0C */
    (void)func_8007512C(&move_rect, 0x2C0, 0);
    if (PE_Port_ShouldStop())
        return -1;

    func_80074DC0(0);
    func_80073A44(0);
    func_80073A44(0);

    environment0 = PE_LoadU32(0x801D11BCu);
    environment1 = PE_LoadU32(0x801D11C0u);
    (void)func_80074924(environment0, 0, 0, 320, 240);
    (void)func_80074924(environment1, 0, 240, 320, 240);
    (void)func_800749D8(environment0 + 0x5Cu, 0, 240, 320, 240);
    (void)func_800749D8(environment1 + 0x5Cu, 0, 0, 320, 240);

    PE_StoreU16(environment1 + 0x66u, 0u);
    PE_StoreU16(environment0 + 0x66u, 0u);
    PE_StoreU16(environment1 + 0x6Au, 240u);
    PE_StoreU16(environment0 + 0x6Au, 240u);
    PE_StoreU8(environment1 + 0x6Du, 1u);
    PE_StoreU8(environment0 + 0x6Du, 1u);
    PE_StoreU16(environment1 + 0x7Cu, 0u);
    PE_StoreU16(environment0 + 0x7Cu, 0u);
    PE_StoreU16(environment1 + 0x74u, 0u);
    PE_StoreU16(environment0 + 0x74u, 0u);

    clear_rect.x = 0;
    clear_rect.y = 0;
    clear_rect.w = 480;
    clear_rect.h = 480;
    func_80074F44(&clear_rect, 0, 0, 0);
    func_80074DC0(0);
    func_80073A44(0);
    func_80073A44(0);

    if (PE_LoadU32(0x8009D1BCu) == 0u) {
        PE_StoreU32(0x8009D1BCu, 1u);
        (void)func_80190660();
        if (PE_Port_ShouldStop())
            return -1;
    }
    if (saved_bit == 0u) {
        Bootstrap_ReturnVoid4Indirect(
            "func_801909B4_80191120_cut", "func_801909B4", 0x80191120u,
            saved_bit, environment0, environment1, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }

    if (func_80192CE8(1) != 0) {
        /* 0x80190D94..0x80190DAC: 60 frames of memory-card poll + VSync
         * after a pad-skipped movie.  func_800425DC is not translated. */
        Bootstrap_ReturnVoid("func_801909B4_func_800425DC_skipwait_cut",
                             "func_801909B4");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }
    if (PE_Port_ShouldStop())
        return -1;

    /* [0x80190DB4,0x80190F28): re-copy the DrawEnv/DispEnv templates into
     * the overlay environment pair. */
    environment0 = PE_LoadU32(0x801D11BCu);
    environment1 = PE_LoadU32(0x801D11C0u);
    CopyGuestBytes(environment0, GA_DRAWENV_SOURCE_0, 0x5Cu);
    CopyGuestBytes(environment1, GA_DRAWENV_SOURCE_1, 0x5Cu);
    CopyGuestBytes(environment0 + 0x5Cu, GA_DISPENV_SOURCE_0, 0x14u);
    CopyGuestBytes(environment1 + 0x5Cu, GA_DISPENV_SOURCE_1, 0x14u);
    /* 0x80190F34/0x80190F44 isbg = 0; 0x80190F5C/60 +0x7C = 0;
     * 0x80190F84 D_801D11C8 = D_800ACDDC; 0x80190F88/8C +0x74 = 0. */
    PE_StoreU8(environment0 + 0x18u, 0u);
    PE_StoreU8(environment1 + 0x18u, 0u);
    PE_StoreU16(environment1 + 0x7Cu, 0u);
    PE_StoreU16(environment0 + 0x7Cu, 0u);
    PE_StoreU32(0x801D11C8u, PE_LoadU32(0x800ACDDCu));
    PE_StoreU16(environment1 + 0x74u, 0u);
    PE_StoreU16(environment0 + 0x74u, 0u);

    /* Four ClearImage calls (s3 = 480, s5 = 240, s1 = 0x14, s0 = 0x10). */
    clear_rect.x = 0; clear_rect.y = 0;    clear_rect.w = 480; clear_rect.h = 0x14;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.x = 0; clear_rect.y = 0xE0; clear_rect.w = 480; clear_rect.h = 0x10;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.x = 0; clear_rect.y = 240;  clear_rect.w = 480; clear_rect.h = 0x14;
    func_80074F44(&clear_rect, 0, 0, 0);
    clear_rect.x = 0; clear_rect.y = 0x1D0; clear_rect.w = 480; clear_rect.h = 0x10;
    func_80074F44(&clear_rect, 0, 0, 0);

    /* 0x8019101C..0x80191048: select the other environment. */
    {
        uint32_t next = PE_LoadU32(0x801D11C8u) == 0u ? 1u : 0u;
        PE_StoreU32(0x801D11C8u, next);
        PE_StoreU32(0x801D11C4u, PE_LoadU32(0x801D11BCu + next * 4u));
    }
    func_80074DC0(0);
    PE_Overlay_UploadDirtyRect();
    func_80073A44(0);
    (void)func_80074A44(1);
    (void)func_80075424(PE_LoadU32(0x801D11C4u));
    func_800755F0((void *)(uintptr_t)(PE_LoadU32(0x801D11C4u) + 0x5Cu));

    /* 0x80191120: title background upload, display on, task pool. */
    func_8018F2F4();
    if (PE_Port_ShouldStop())
        return -1;
    func_80073A44(0);
    func_80074D28(1);
    {
        pe_addr_t node = GA_TASK_POOL;
        const pe_addr_t last = GA_TASK_POOL + 0x16Cu;
        while (node < last) {
            PE_StoreU32(node, node + 0x34u);
            node += 0x34u;
        }
        PE_StoreU32(node, 0u);
    }
    PE_StoreU32(GA_TASK_FREE_HEAD, GA_TASK_POOL);
    PE_StoreU32(GA_TASK_PENDING_TAIL, 0u);
    PE_StoreU32(GA_TASK_PENDING_HEAD, 0u);
    PE_StoreU32(GA_TASK_ACTIVE_TAIL, 0u);
    PE_StoreU32(GA_TASK_ACTIVE_HEAD, 0u);
    PE_StoreU32(func_8018FBC0(1) + 0x14u, 0x8019319Cu);
    PE_StoreU32(GA_ATTRACT_COUNTER, saved_bit);

    /* [0x801911C0,0x80191410): the title loop. */
    if ((int32_t)saved_bit < 1000) {
        do {
            pe_addr_t env;
            pe_addr_t node, prev, pending;
            uint32_t next = PE_LoadU32(0x801D11C8u) == 0u ? 1u : 0u;
            env = PE_LoadU32(0x801D11BCu + next * 4u);
            PE_StoreU32(0x801D11C8u, next);
            PE_StoreU32(0x801D11C4u, env);
            func_800425DC();
            if (PE_Port_ShouldStop())
                return -1;
            func_8003EB04();
            func_80190064();
            if (PE_Port_ShouldStop())
                return -1;
            func_8018F468();
            if (PE_Port_ShouldStop())
                return -1;

            /* 0x80191210..0x801912B4: retire flagged tasks to the free list. */
            prev = 0u;
            node = PE_LoadU32(GA_TASK_ACTIVE_HEAD);
            while (node != 0u) {
                if (PE_LoadU32(node + 0x30u) != 0u) {
                    pe_addr_t following = PE_LoadU32(node);
                    if (prev != 0u)
                        PE_StoreU32(prev, following);
                    else
                        PE_StoreU32(GA_TASK_ACTIVE_HEAD, following);
                    if (PE_LoadU32(GA_TASK_ACTIVE_TAIL) == node)
                        PE_StoreU32(GA_TASK_ACTIVE_TAIL, prev);
                    PE_StoreU32(node, PE_LoadU32(GA_TASK_FREE_HEAD));
                    PE_StoreU32(GA_TASK_FREE_HEAD, node);
                    node = prev != 0u ? PE_LoadU32(prev)
                                      : PE_LoadU32(GA_TASK_ACTIVE_HEAD);
                } else {
                    prev = node;
                    node = PE_LoadU32(node);
                }
            }
            /* 0x801912B8..0x80191314: append the pending list. */
            pending = PE_LoadU32(GA_TASK_PENDING_HEAD);
            if (pending != 0u) {
                pe_addr_t tail = PE_LoadU32(GA_TASK_ACTIVE_TAIL);
                if (tail != 0u)
                    PE_StoreU32(tail, pending);
                else
                    PE_StoreU32(GA_TASK_ACTIVE_HEAD, pending);
                PE_StoreU32(GA_TASK_ACTIVE_TAIL, PE_LoadU32(GA_TASK_PENDING_TAIL));
                PE_StoreU32(GA_TASK_PENDING_TAIL, 0u);
                PE_StoreU32(GA_TASK_PENDING_HEAD, 0u);
            }

            func_80074DC0(0);
            PE_Overlay_UploadDirtyRect();
            if (PE_Port_ShouldStop())
                return -1;
            func_80073A44(2);
            (void)func_80074A44(1);
            (void)func_80075424(PE_LoadU32(0x801D11C4u));
            func_800755F0((void *)(uintptr_t)(PE_LoadU32(0x801D11C4u) + 0x5Cu));
            if (PE_Port_ShouldStop())
                return -1;
            PE_StoreU32(GA_ATTRACT_COUNTER,
                        PE_LoadU32(GA_ATTRACT_COUNTER) + saved_bit);
        } while ((int32_t)PE_LoadU32(GA_ATTRACT_COUNTER) < 1000);
    }

    /* 0x80191410: loop exit — selection processing / attract restart. */
    Bootstrap_ReturnVoid("func_801909B4_80191410_cut", "func_801909B4");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return -1;
}

/*
 * Shared retail idiom at 0x80191054..0x801910EC (and 0x80191320..0x801913B8,
 * 0x8018F37C..0x8018F414): when the active environment's dirty rect height
 * (+0x74) is positive, LoadImage the 24-bit RAM buffer at env + 0x8080 into
 * the display half selected by D_801D11C8, converting x and w from 24-bit
 * pixels to 16-bit VRAM units by (v * 3) >> 1.
 */
void PE_Overlay_UploadDirtyRect(void)
{
    pe_addr_t env = PE_LoadU32(0x801D11C4u);
    RECT rect;
    int16_t x, w;

    if ((int16_t)PE_LoadU16(env + 0x74u) <= 0)
        return;
    x = (int16_t)PE_LoadU16(env + 0x70u);
    rect.y = (int16_t)PE_LoadU16(env + 0x72u);
    w = (int16_t)PE_LoadU16(env + 0x74u);
    rect.h = (int16_t)PE_LoadU16(env + 0x76u);
    rect.x = (int16_t)((x * 3) >> 1);
    if (PE_LoadU32(0x801D11C8u) == 0u)
        rect.y = (int16_t)(rect.y + 0xF0);
    rect.w = (int16_t)((w * 3) >> 1);
    (void)func_8007506C(&rect, env + 0x8080u);
}
