/*
 * Phase 6E-A — libetc: ResetCallback, critical sections, kernel events.
 *
 * func_80073C94 = PsyQ ResetCallback (asm/disc1/5F3E4.s @ file 0x5F3E4):
 * indirect call through the libetc jump table D_8009566C[3] = func_80073E28
 * (asm/disc1/64610.s).  Also called from ResetGraph, CdInit/SsInit paths —
 * the one-time guard D_800945E4 therefore matters beyond first use.
 * Classification: 2 (SDK host implementation).
 *
 * Retail func_80073E28 effects:
 *   - lhu guard D_800945E4: if set, return 0 (idempotent).
 *   - I_MASK = 0, I_STAT ack, DPCR = 0x33333333: interrupt/DMA hardware
 *     registers — host no-ops.
 *   - callback queue-block build, then sh 1 -> D_800945E4, ExitCriticalSection,
 *     returns queue-block pointer (callers on the boot path ignore it).
 * The host callback registry (pe_callback.h, Phase 6D-S) replaces the
 * retail callback queue: reset clears the registered callback.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

static int      g_irq_lock_depth;
static uint32_t g_next_event_handle = 0x100;

void func_80073C94(void)
{
    if (PE_LoadU16(0x800945E4u) != 0) {
        return;                     /* one-time guard */
    }
    PE_StoreU16(0x800945E4u, 1);
    PE_Callback_Reset();
}

/* EnterCriticalSection: retail suspends interrupt delivery and returns the
 * previous state.  Boot-path callers ignore the return; depth is tracked so
 * tests can verify balanced Enter/Exit pairs. */
int func_80072714(void)
{
    return g_irq_lock_depth++;
}

void func_80072724(void)
{
    if (g_irq_lock_depth > 0) {
        g_irq_lock_depth--;
    }
}

int PE_Irq_LockDepth(void)
{
    return g_irq_lock_depth;
}

int PE_Event_Open(uint32_t cls, uint32_t spec, uint32_t mode, pe_addr_t handler)
{
    (void)cls; (void)spec; (void)mode; (void)handler;
    return (int)g_next_event_handle++;
}

int PE_Event_Enable(int handle)
{
    (void)handle;
    return 1;
}

void PE_Sdk_ResetState(void)
{
    g_irq_lock_depth = 0;
    g_next_event_handle = 0x100;
    g_pe_gte.ofx = g_pe_gte.ofy = 0;
    g_pe_gte.h = 0;
    g_pe_gte.dqa = g_pe_gte.dqb = 0;
    g_pe_gte.zsf3 = g_pe_gte.zsf4 = 0;
}
