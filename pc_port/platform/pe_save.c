/*
 * Phase 6E-A — save manager bring-up.
 *
 * func_800844E4 (asm/disc1/746F8.s @ file 0x74CE4): save-manager init.
 * func_80082534 -> func_80082CF0 (asm/disc1/734F0.s): save-manager reset.
 * func_80084644 (asm/disc1/746F8.s @ file 0x74E44): per-slot state reset.
 * Classification: 1 (translated game logic over guest RAM).
 *
 * Collapsed retail effects:
 *   - func_80071A24(&D_800A5B70, 0x1E0) is BIOS A(28h) block-zero: host memset.
 *   - func_8007E1F4/func_8007E1E4(2, &D_800A5AB0): kernel SysDeqIntRP /
 *     SysEnqIntRP priority-queue manipulation — no guest-RAM effect: no-ops.
 *   - DAY1-62 restores func_80073C84(3,0) BIOS clear-mode state.
 *   - func_80082CF0 dereferences D_8009B784 (*v1 = -2; v1[1] |= 1).  That
 *     pointer is installed by the collapsed card-controller hardware init
 *     (func_8007DDD4 path in libcard) and names kernel state outside the
 *     2 MiB guest window; on the host it reads as 0, so these two writes are
 *     not reproducible and are skipped (documented divergence).
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include <string.h>

/* func_80084644 — per-slot reset, verbatim. */
static void PE_Save_SlotReset(pe_addr_t slot)
{
    int j;
    if (PE_LoadU8(slot + 0x49) != 0) {
        PE_StoreU8(slot + 0x49, 0);
        PE_StoreU8(slot + 0x46, 0);
        PE_StoreU16(slot + 0xE6, 0);
        PE_StoreU32(slot + 0x14, 0);
        PE_StoreU32(slot + 0x18, 0);
        PE_StoreU8(slot + 0xE3, 0);
        PE_StoreU8(slot + 0xE4, 0);
        PE_StoreU8(slot + 0xE9, 0);
        PE_StoreU8(slot + 0xEA, 0);
        PE_StoreU32(slot + 0x00, 0);
        PE_StoreU32(slot + 0x04, 0);
        PE_StoreU32(slot + 0x08, 0);
        for (j = 0; j < 6; j++) {
            PE_StoreU8(slot + 0x5D + (uint32_t)j, 0xFF);
        }
    }
}

void func_800844E4(pe_addr_t base, pe_addr_t base2)
{
    int i, j;
    PE_StoreU32(0x8009B75Cu, 0);
    PE_StoreU32(0x8009B770u, 0);
    /* func_80084B44 — install 3 function pointers */
    PE_StoreU32(0x8009B73Cu, 0x80084B78u);
    PE_StoreU32(0x8009B740u, 0x80084F8Cu);
    PE_StoreU32(0x8009B744u, 0x80084C4Cu);
    /* install 7 function pointers + state base */
    PE_StoreU32(0x8009B724u, 0x800846ACu);
    PE_StoreU32(0x8009B728u, 0x80084644u);
    PE_StoreU32(0x8009B72Cu, 0x800847B0u);
    PE_StoreU32(0x8009B730u, 0x8008486Cu);
    PE_StoreU32(0x8009B734u, 0x80084AE8u);
    PE_StoreU32(0x8009B738u, 0x80084B20u);
    PE_StoreU32(0x8009B748u, 0x800847A0u);
    PE_StoreU32(0x8009B758u, 0x800A5B70u);
    /* func_80071A24(&D_800A5B70, 0x1E0) — BIOS A(28h) block zero */
    memset(PE_Translate(0x800A5B70u, 0x1E0), 0, 0x1E0);
    /* slot base pointers: slot0 +0x30 = base, slot1 +0x30 = base2
     * (retail stores base2 at 0x800A5B70+0x120 = slot1+0x30) */
    PE_StoreU32(0x800A5B70u + 0x30u, base);
    PE_StoreU32(0x800A5B70u + 0x120u, base2);
    for (i = 0; i < 2; i++) {
        pe_addr_t s0 = 0x800A5B70u + (uint32_t)i * 0xF0u;
        pe_addr_t bp = PE_LoadU32(s0 + 0x30);
        PE_StoreU32(s0 + 0x0C, 0);
        PE_StoreU32(s0 + 0x10, s0);
        PE_StoreU8(bp, 0xFF);
        PE_StoreU8(bp + 1, 0);
        PE_StoreU32(s0 + 0x3C, 0x800A5AE0u + (uint32_t)i * 0x23u);
        PE_StoreU32(s0 + 0x40, 0x800A5B28u + (uint32_t)i * 0x23u);
        for (j = 0; j < 6; j++) {
            PE_StoreU8(s0 + 0x5D + (uint32_t)j, 0xFF);
        }
    }
    /* func_80082ADC */
    PE_StoreU32(0x800A5AB0u, 0);
    PE_StoreU32(0x800A5AB4u, 0x80082B70u);
    PE_StoreU32(0x800A5AB8u, 0x80082B08u);
    PE_StoreU32(0x800A5ABCu, 0);
    PE_StoreU32(0x8009B75Cu, 1);
}

void func_80082534(void)
{
    /* func_80082CF0: */
    func_80072714();                        /* EnterCriticalSection */
    PE_StoreU32(0x8009B75Cu, 0);
    /* SysDeqIntRP/SysEnqIntRP + D_8009B784 block: collapsed (see header) */
    (void)func_80073C84(3u,0u); /* original82CF0 VBlank clear policy */
    func_80072724();                        /* ExitCriticalSection */
    PE_Save_SlotReset(0x800A5B70u);         /* func_80084644(D_8009B758) */
    PE_Save_SlotReset(0x800A5C60u);         /* func_80084644(D_8009B758+0xF0) */
    PE_StoreU32(0x800A5AC0u, 0);
    PE_StoreU32(0x800A5AC4u, 0);
    PE_StoreU32(0x8009B75Cu, 1);
}
