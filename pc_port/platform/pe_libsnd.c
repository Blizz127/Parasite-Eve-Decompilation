/*
 * Phase 6E-A — libsnd: SsInit.
 *
 * func_8007D054 — boot-path wrapper; retail tail-calls func_8007D074(arg=0)
 * = SsInit (asm/disc1/6C93C.s @ file 0x6D874).
 * func_8007D15C — SPU IRQ event install (asm/disc1/6C93C.s @ file 0x6D95C),
 * also called from the streaming bring-up func_80085644; its guard
 * D_8009B3EC makes the second call a no-op, so it is transcribed verbatim
 * here instead of being collapsed.
 * Classification: 2 (SDK host implementation).
 *
 * Collapsed retail effects (hardware-only):
 *   - func_8007D1D4(arg): SPU hardware register init — no-op.
 *   - func_8007DAE0(0xD1, D_8009B46C, 0): SPU register write — no-op.
 *   - func_8007DD14(func_8007D614): kernel SPU callback install — no-op.
 *   - func_800726E4/func_80072704 (OpenEvent/EnableEvent): host event shims
 *     (pe_libetc.c); kernel Event Control Blocks live outside the guest
 *     window, only the returned handle is guest-visible.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

/* func_8007D15C — SPU IRQ event install (verbatim guest-visible effects).
 * Non-static: also called verbatim from func_80085644 (pe_stream.c). */
void func_8007D15C(void)
{
    if (PE_LoadU32(0x8009B3ECu) != 0) {
        return;                             /* one-time guard */
    }
    PE_StoreU32(0x8009B3ECu, 1);
    func_80072714();                        /* EnterCriticalSection */
    PE_StoreU32(0x8009B384u,
                (uint32_t)PE_Event_Open(0xF0000009u, 0x20, 0x2000, 0));
    PE_Event_Enable((int)PE_LoadU32(0x8009B384u));
    func_80072724();                        /* ExitCriticalSection */
}

void func_8007D054(void)
{
    int i;
    /* func_8007D074(0): */
    func_80073C94();                        /* ResetCallback */
    /* func_8007D1D4(0) — SPU hardware init: collapsed no-op */
    /* arg == 0 path: 24 voice-default halfwords 0xC000,
     * 0x8009B3E6 descending to 0x8009B3B8 */
    for (i = 0; i < 24; i++) {
        PE_StoreU16(0x8009B3B8u + (uint32_t)i * 2u, 0xC000);
    }
    func_8007D15C();
    PE_StoreU32(0x8009B390u, 0);
    PE_StoreU32(0x8009B394u, 0);
    /* D_8009B3A0 block: sw 0 @+0x0, sh 0 @+0x4/+0x6, sw 0 @+0x8/+0xC */
    PE_StoreU32(0x8009B3A0u, 0);
    PE_StoreU16(0x8009B3A4u, 0);
    PE_StoreU16(0x8009B3A6u, 0);
    PE_StoreU32(0x8009B3A8u, 0);
    PE_StoreU32(0x8009B3ACu, 0);
    PE_StoreU32(0x8009B398u, PE_LoadU32(0x8009B46Cu));
    /* func_8007DAE0(0xD1, D_8009B46C, 0) — SPU register write: collapsed */
    PE_StoreU32(0x8009B45Cu, 0);
    PE_StoreU32(0x8009B460u, 0);
    PE_StoreU32(0x8009B464u, 0);
    PE_StoreU32(0x8009B38Cu, 0);
    PE_StoreU32(0x8009B418u, 0);
    PE_StoreU32(0x8009B388u, 0);
    PE_StoreU32(0x8009B3B4u, 0);
    PE_StoreU32(0x8009B3B0u, 0);
    PE_StoreU32(0x8009B3E8u, 0);
}
