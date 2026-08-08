/*
 * Phase 6E-B52 — func_8007506C (PsyQ LoadImage) wrapper translation with an
 * honest GPU dispatch boundary, plus its proven read-only rect validator
 * func_80074E28.
 *
 * func_8007506C: 24 words / 0x60 bytes, executable 0x8007506C..0x800750CB
 * (exclusive end 0x800750CC), file offset 0x6586C, live split
 * asm/disc1/654C8.s:283-310.  All 24 words verified exact against the
 * SHA-exact retail executable (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b).
 * String-proven PsyQ LoadImage ("LoadImage" at D_800118D4; sdk_map.md).
 *
 * func_80074E28: 71 words / 0x11C bytes, executable 0x80074E28..0x80074F43
 * (exclusive end 0x80074F44), file offset 0x65628, live split
 * asm/disc1/654C8.s:118-197.  The libgpu rect debug validator: every store
 * in its body is $sp-relative (proven by the b52 oracle's store scan), so
 * it is read-only against all persistent state.  Its only callees are the
 * two debug prints through fn-ptr D_80095748 (statically 0x80071A74, the
 * BIOS A(3Fh) printf trampoline), gated by the debug level byte
 * D_8009574E: level 0 returns immediately (the boot path's level —
 * func_8003E754 calls SetGraphDebug(0)), level 1 prints
 * "%s:bad RECT" + "(%d,%d)-(%d,%d)\n" only when the rect fails the
 * D_80095750/D_80095752 limits, level 2 always prints "%s:" + the rect.
 *
 * Dispatch contract (func_8007506C body, ROM order):
 *   prologue  addiu $sp,-0x20; save $s0/$s1/$ra; s0 = a0 (rect), s1 = a1
 *   1. jal func_80074E28(D_800118D4, rect)   read-only validator (above)
 *   2. v0 = lw D_80095744; a2 = 8; a0 = lw 0x20(v0); v0 = lw 0x08(v0);
 *      jalr v0;  delay slot: a3 = s1 (data)
 *   3. epilogue; the jalr's $v0 is forwarded to the caller.
 *
 * D_80095744 is a static initialized pointer to the 16-word libgpu jump
 * table ("jtb") at 0x80095704 (jtb[0] is the "$Id: sys.c,v 1.140" version
 * string).  The executable's references read the pointer; ResetGraph does
 * not rewrite it or the jtb.  The retail Disc 1 dispatch is therefore:
 *   target = jtb[2] = 0x80076C34  (GPU queue/transfer dispatcher)
 *   a0     = jtb[8] = 0x80076664  (immediate transfer worker)
 * Complete jtb census (all 16 entries, static exe data):
 *   [0] 0x800117AC version string   [1] 0x80076C10 (MoveImage extra arg)
 *   [2] 0x80076C34 queue dispatcher — ClearImage/ClearImage2/LoadImage/
 *       StoreImage/MoveImage/DrawOTag/PutDrawEnv/DrawOTagEnv jalr target
 *   [3] 0x80076434 (ClearImage worker arg)   [4] 0x80076B20 SetDispMask/
 *       PutDispEnv target   [5] 0x80076B58 (func_80075358 target)
 *   [6] 0x80076B98 (MoveImage/DrawOTag/PutDrawEnv/DrawOTagEnv worker arg)
 *   [7] 0x800768A0 (StoreImage worker arg)   [8] 0x80076664 (LoadImage
 *       worker arg)   [9] 0x80076EE4 queue pump (used internally)
 *   [10] 0x80076B44 (internal)   [11] 0x80076354 (ClearOTagR target)
 *   [12] 0x80076BE0 (internal)   [13] 0x80077144 GPU version query
 *       (ResetGraph/func_80074C14 target)   [14] 0x8007633C
 *       (func_80075B1C target)   [15] 0x80077294 (DrawSync/func_80075358
 *       target)
 *
 * Why the jalr target stays a centralized boundary (read-only audit):
 *   func_80076C34 maintains the GPU command ring at D_800BD030 (96-byte
 *   entries, capacity 64, producer D_80095874, consumer D_80095878),
 *   polls GPUSTAT (0x1F801814, mask 0x04000000) in the direct-transfer
 *   path, tests DMA2 CHCR (0x1F8010A8, mask 0x01000000 busy) via the
 *   func_80076EE4 pump, and can block in func_80077404 wait loops with a
 *   -1 timeout return.  The worker func_80076664 writes GP0 (0x1F801810:
 *   command 0xA0/0xB0, rect, pixel data), GP1 (0x1F801814: 0x04000000),
 *   and programs DMA2 MADR/BCR/CHCR (0x1F8010A0-A8).  Those are
 *   asynchronous GPU/DMA hardware semantics; they are NOT implemented
 *   here and nothing in this wrapper's own continuation consumes their
 *   effects (the epilogue only forwards $v0).
 *
 * Caller census (14 executable sites): func_80042FE8@0x80043020,
 * func_8005E788@0x8005E824, func_8006914C@0x80069544,
 * func_8006AD40@0x8006AF18, func_8006E1C0@0x8006E238/0x8006E2A8,
 * func_800718D0@0x80071910/0x80071920, func_80074774@0x80074820,
 * func_8007485C@0x80074894, func_800748C0@0x800748F8,
 * func_800CED3C@0x800CED84, func_800CEDA8@0x800CEDFC,
 * func_800CF4B4@0x800CF598.  12 discard the return immediately; the two
 * func_800CED3C/func_800CEDA8 sites merely forward $v0 to their own
 * (untranslated) callers.  No translated caller consumes the return.
 *
 * Boundary representation: the retail rect is a caller-stack transient
 * with no guest authority (B51), so this translation takes it as a native
 * RECT with the retail four-signed-halfword layout.  The centralized log
 * records the loaded indirect target and exact func_80076C34 registers:
 * a0=jtb[8], a1=the transient RECT pointer, a2=8, a3=data.  It snapshots
 * the eight RECT bytes synchronously so tests never dereference a dead
 * stack pointer.  The unresolved provider supplies the return value,
 * which this wrapper forwards exactly; no GPU result is fabricated here.
 *
 * Classification: 1 for both bodies (translated retail logic); the GPU
 * dispatch itself remains classification 4 (unresolved hardware-dependent)
 * behind the centralized boundary.  Strict execution now stops at
 * func_80076C34 from func_8007506C.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_GPU_NAME_LOADIMAGE  0x800118D4u   /* "LoadImage" */
#define GA_GPU_FMT_BAD         0x80011898u   /* "%s:bad RECT" */
#define GA_GPU_FMT_RECT        0x800118A4u   /* "(%d,%d)-(%d,%d)\n" */
#define GA_GPU_FMT_NAME        0x800118B8u   /* "%s:" */
#define GA_GPU_DEBUG_LEVEL     0x8009574Eu   /* env+2: SetGraphDebug byte */
#define GA_GPU_LIMIT_W         0x80095750u   /* env+4: 0x400 after ResetGraph(0) */
#define GA_GPU_LIMIT_H         0x80095752u   /* env+6: 0x200 after ResetGraph(0) */
#define GA_GPU_PRINT_FN        0x80095748u   /* -> 0x80071A74 BIOS A(3Fh) */
#define GA_GPU_JTB_PTR         0x80095744u   /* -> 0x80095704 (immutable) */

void func_80074E28(pe_addr_t name, const RECT *rect)
{
    uint32_t level = PE_LoadU8(GA_GPU_DEBUG_LEVEL);
    pe_addr_t print_target;
    int16_t x, y, w, h;
    int16_t wlim, hlim;
    int bad;

    if (level != 1u && level != 2u)
        return;                             /* 0x80074E5C: silent return */

    x = rect->x;
    y = rect->y;
    w = rect->w;
    h = rect->h;

    if (level == 2u) {
        /* 0x80074EF0: unconditional "%s:" + rect dump. */
        print_target = PE_LoadU32(GA_GPU_PRINT_FN);
        Bootstrap_ReturnVoid4Indirect(
            "func_80071A74", "func_80074E28", print_target,
            GA_GPU_FMT_NAME, name, 0, 0);
    } else {
        /* 0x80074E64..0x80074EE0: signed bounds checks in retail order
         * (lh loads, 32-bit addu sums, signed slt predicates). */
        wlim = (int16_t)PE_LoadU16(GA_GPU_LIMIT_W);
        hlim = (int16_t)PE_LoadU16(GA_GPU_LIMIT_H);
        bad = (wlim < w) || (wlim < w + x) ||
              (hlim < y) || (hlim < y + h) ||
              (w <= 0) || (x < 0) || (y < 0) || (h <= 0);
        if (!bad)
            return;                         /* 0x80074EDC: bgtz pass */
        print_target = PE_LoadU32(GA_GPU_PRINT_FN);
        Bootstrap_ReturnVoid4Indirect(
            "func_80071A74", "func_80074E28", print_target,
            GA_GPU_FMT_BAD, name, 0, 0);
    }

    /* 0x80074F0C..0x80074F30: "(%d,%d)-(%d,%d)\n" dump; retail passes h
     * as the fifth printf argument on the stack (diagnostic only). */
    print_target = PE_LoadU32(GA_GPU_PRINT_FN);
    Bootstrap_ReturnVoid5Indirect(
        "func_80071A74", "func_80074E28", print_target,
        GA_GPU_FMT_RECT,
        (uintptr_t)(intptr_t)(int32_t)x,
        (uintptr_t)(intptr_t)(int32_t)y,
        (uintptr_t)(intptr_t)(int32_t)w,
        (uintptr_t)(intptr_t)(int32_t)h);
}

int func_8007506C(const RECT *rect, pe_addr_t data)
{
    pe_addr_t jtb;
    pe_addr_t target;
    uint32_t worker;

    /* 0x8007508C: jal func_80074E28 — read-only validator (delay slot
     * a1 = s0 already accounted for by the host RECT). */
    func_80074E28(GA_GPU_NAME_LOADIMAGE, rect);

    /* 0x80075098..0x800750A8: the dispatch words, re-read from guest RAM
     * in retail order (a2 = 8 is a body constant). */
    jtb    = PE_LoadU32(GA_GPU_JTB_PTR);
    worker = PE_LoadU32(jtb + 0x20u);       /* a0 = jtb[8] */
    target = PE_LoadU32(jtb + 8u);          /* v0 = jtb[2] -> jalr */

    /* 0x800750B0: jalr jtb[2] with a0 = jtb[8], a1 = rect, a2 = 8,
     * a3 = data (delay slot) — the centralized GPU boundary. */
    return Bootstrap_ReturnInt4Indirect(
        "func_80076C34", "func_8007506C", 0, target,
        worker, (uintptr_t)rect, 8u, data, rect, sizeof(*rect));
}
