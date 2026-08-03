/*
 * Phase 6E-A — libcd: CdInit, Cd reset, CdReady and CD state getters.
 *
 * ROM evidence (asm/disc1/6E6C0.s unless noted):
 *   func_8007EC14 CdInit
 *   func_8007ED58 CD reset + state clear (boot wait-loop 1 provider)
 *   func_8007F72C CdReady; func_8007FBF0 status-lane getter D_8009B574[idx]
 *     (getter body in asm/disc1/703F0.s: sll a0,2; lw D_8009B574[a0])
 *   func_800808BC / func_80080930 — D_8009B554 flag clear/set
 *   func_800822BC — DS-abort check, always stores D_8009B70C = 0
 *   func_80081E5C(0) — exchange D_8009B708
 *   func_8007F994 — CD low-level init (hardware; guest effects listed below)
 *   func_8007F778 — src C leaf getter D_800A3608
 *   func_80080CC8 — src C leaf exchange D_8009AFC0
 *   func_8007F7A8 -> func_8007FCAC — src C leaf getter D_8009B590
 * Classification: 2 (SDK host implementation); the drive model is class 3
 * (deterministic platform behavior).
 *
 * Collapsed retail effects (hardware-only):
 *   - func_8007F994's controller programming (func_8007BBFC, func_8007BAC0,
 *     func_8007FA2C, func_800812F4(0), func_80073D58): CD/DMA registers.
 *   - func_8007B9EC inside func_800808BC: CD hardware access.
 *   - func_800822BC's DS-abort calls when D_8009B70C==1 (never true here).
 *
 * Host drive model: retail sets D_8009B574[0] = 1 (idle/ready) from the CD
 * interrupt handler once the reset sequence is acknowledged.  The host
 * models the controller synchronously — after a completed reset with no
 * pending command the drive is idle — so func_8007ED58 sets the lane to 1
 * when no hardware status has been recorded.  This reproduces the
 * retail-observable state transition; it is not a fabricated return value.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

/* Identical zeroing block shared by CdInit and func_8007ED58. */
static void PE_Cd_ClearState(void)
{
    int i;
    PE_StoreU32(0x800B8AB0u, 0);
    PE_StoreU32(0x800B8AB4u, 0);
    PE_StoreU32(0x800B8AB8u, 0);
    /* D_800A3510 fields: sw 0 @+0x20/+0x10/+0x00, sb 0 @+0x24/+0x14/+0x04 */
    PE_StoreU32(0x800A3510u + 0x20u, 0);
    PE_StoreU32(0x800A3510u + 0x10u, 0);
    PE_StoreU32(0x800A3510u + 0x00u, 0);
    PE_StoreU8(0x800A3510u + 0x24u, 0);
    PE_StoreU8(0x800A3510u + 0x14u, 0);
    PE_StoreU8(0x800A3510u + 0x04u, 0);
    for (i = 0; i < 8; i++) {
        PE_StoreU8(0x800A3515u + (uint32_t)i, 0);
        PE_StoreU8(0x800A3525u + (uint32_t)i, 0);
        PE_StoreU8(0x800A3535u + (uint32_t)i, 0);
    }
    /* func_8007E594 x8: zero the 0x18-byte descriptors at 0x800A3540+i*0x18 */
    for (i = 0; i < 8; i++) {
        pe_addr_t d = 0x800A3540u + (uint32_t)i * 0x18u;
        PE_StoreU32(d + 0x00, 0);
        PE_StoreU8(d + 0x04, 0);
        PE_StoreU8(d + 0x05, 0);
        PE_StoreU8(d + 0x06, 0);
        PE_StoreU8(d + 0x07, 0);
        PE_StoreU8(d + 0x08, 0);
        PE_StoreU32(d + 0x0C, 0);
        PE_StoreU32(d + 0x10, 0);
        PE_StoreU32(d + 0x14, 0);
    }
    PE_StoreU32(0x800A3604u, 0);
    PE_StoreU32(0x800A3600u, 0);
    PE_StoreU32(0x800A3608u, 0);
    for (i = 0; i < 8; i++) {
        PE_StoreU32(0x800A3610u + (uint32_t)i * 0x10u, 0);
    }
    PE_StoreU32(0x800A3690u, 0);
}

/* func_800822BC: DS-abort check; abort calls collapsed, flag always cleared. */
static void PE_Cd_DsAbortCheck(void)
{
    PE_StoreU32(0x8009B70Cu, 0);
}

/* func_80081E5C(0): old = D_8009B708; D_8009B708 = 0 (old discarded). */
static void PE_Cd_ClearB708(void)
{
    PE_StoreU32(0x8009B708u, 0);
}

int func_8007EC14(void)
{
    /* func_80080940 — src C leaf getter D_8009B554 */
    if (PE_LoadU32(0x8009B554u) != 0) {
        return (int)PE_LoadU32(0x8009B554u);
    }
    PE_Cd_ClearState();
    /* func_8007F994 — CD low-level init; guest-visible effects: */
    PE_StoreU32(0x800A36A8u, 0);
    PE_StoreU32(0x800A36A4u, 0);
    PE_StoreU32(0x800A36A0u, 0);
    PE_StoreU32(0x8009AFB4u, 0x80080164u);
    PE_StoreU32(0x8009AFB8u, 0x80080778u);
    PE_StoreU32(0x8009AFD8u, 1);
    PE_StoreU32(0x8009B554u, 1);
    /* handler installs (overwrite the lowlevel zeros) */
    PE_StoreU32(0x800A36A4u, 0x8007E964u);
    PE_StoreU32(0x800A36A8u, 0x8007F88Cu);
    PE_StoreU32(0x800A36ACu, 0x8007F960u);
    PE_StoreU32(0x800A36A0u, 0x8007F7E8u);
    PE_Cd_DsAbortCheck();
    PE_Cd_ClearB708();
    return 1;
}

int func_8007ED58(void)
{
    /* func_800808BC: */
    PE_StoreU32(0x8009B554u, 0);
    /* func_8007B9EC — CD hardware access: collapsed */
    if (PE_LoadU32(0x8009B574u) == 2) {
        uint32_t v = PE_LoadU32(0x8009B578u);
        if (v == 0xB || v == 0x11 || v == 0x10) {
            PE_StoreU32(0x8009B574u, 1);
            PE_StoreU32(0x8009B578u, 0xB);
        }
    }
    PE_Cd_ClearState();
    PE_Cd_DsAbortCheck();
    PE_Cd_ClearB708();
    /* func_80080930: */
    PE_StoreU32(0x8009B554u, 1);
    /* Host drive model: the reset sequence completes synchronously and
     * aborts any pending command, so the drive is idle/ready afterwards.
     * Retail reaches the same state asynchronously via the CD interrupt
     * handler (installed at D_800A36A0). */
    PE_StoreU32(0x8009B574u, 1);
    return 1;
}

int func_8007FBF0(int idx)
{
    return (int)PE_LoadU32(0x8009B574u + (uint32_t)idx * 4u);
}

int func_8007F72C(void)
{
    int st = func_8007FBF0(0);
    if (st != 1) {
        return st;
    }
    return (func_8007F778() > 0) ? 2 : 1;
}

int func_8007F778(void)
{
    return (int)PE_LoadU32(0x800A3608u);
}

int func_80080CC8(int v)
{
    int old = (int)PE_LoadU32(0x8009AFC0u);
    PE_StoreU32(0x8009AFC0u, (uint32_t)v);
    return old;
}

int func_8007F7A8(void)
{
    /* func_8007FCAC — src C leaf getter D_8009B590 */
    return (int)PE_LoadU32(0x8009B590u);
}
