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
 *   - I_MASK = 0 at 0x80073E60, its zero readback is written to I_STAT at
 *     0x80073E6C, and DPCR = 0x33333333 at 0x80073E7C.
 *   - func_80074330 clears 0x41A words starting at D_800945E4, the exact
 *     0x1068-byte range ending immediately before D_8009564C.  This includes
 *     the CPU callback table and registered-source mask.
 *   - sh 1 -> D_800945E4, then source 0 and source 3 registration in that
 *     order.  The authentic completed state has I_MASK/registered mask 0x9.
 * The guest-backed callback slot model (pe_callback.h, Phase 6E-B6)
 * replaces the retail callback queue: ResetCallback's first guard-passing
 * call zeroes the 8 guest slots at D_8009568C and the dispatch counter.
 */
#include "psx_compat.h"
#include "pe_sdk.h"
#include "pe_spu_dma.h"
#include "pe_gpu.h"
#include "pe_mdec.h"
#include "pe_irq.h"
#include "pe_irq_delivery.h"
#include "game_port.h"

static int      g_irq_lock_depth;
static uint32_t g_next_event_handle = 0x100;

#define GA_IRQ_RESET_BLOCK          0x800945E4u
#define GA_IRQ_RESET_BLOCK_BYTES    0x00001068u
#define GA_IRQ_RESET_GUARD          0x800945E4u
#define GA_IRQ_DISPATCH_ACTIVE      0x800945E6u
#define GA_IRQ_CPU_CALLBACK_TABLE   0x800945E8u
#define GA_IRQ_REGISTERED_MASK      0x80094614u
#define GA_IRQ_WATCHDOG             0x8009567Cu
#define PE_IRQ_CPU_SOURCE_COUNT     11u
#define GA_DMA_CALLBACK_TABLE       0x800956C0u
#define PE_DMA_CALLBACK_SLOTS       8u

#define GA_IRQ_SOURCE0_HANDLER      0x8007440Cu
#define GA_IRQ_SOURCE3_HANDLER      0x80074520u

static PeIrqSource0BiosState g_source0_bios = {
    .pad_clear_mode = 0u,
    .vblank_clear_mode = 0u,
};
static uint64_t g_source0_setup_order;

static void ResetSource0BiosState(void)
{
    memset(&g_source0_bios, 0, sizeof(g_source0_bios));
    g_source0_setup_order = 0u;
}

/* Exact B(5Bh) ChangeClearPAD host equivalent needed by source 0.  Value 0
 * passes VBlank processing to the lower-priority module; value 1 completes
 * it in the Pad/Card driver.  The retail ABI is void. */
static void ChangeClearPadB1(uint32_t flag)
{
    g_source0_bios.pad_clear_mode = flag;
    g_source0_bios.pad_calls++;
    g_source0_bios.pad_argument = flag;
    g_source0_bios.mask_at_pad_call = PE_IRQ_GetMask();
    g_source0_bios.source0_slot_at_pad_call =
        (pe_addr_t)PE_LoadU32(GA_IRQ_CPU_CALLBACK_TABLE);
    g_source0_bios.registered_mask_at_pad_call =
        PE_LoadU16(GA_IRQ_REGISTERED_MASK);
    g_source0_bios.pad_call_order = ++g_source0_setup_order;
}

/* Exact C(0Ah) ChangeClearRCnt(3, flag) host equivalent needed by source 0.
 * Counter 3 is the BIOS VBlank lane.  No timer/IRQ service is implemented. */
static uint32_t ChangeClearVBlankB1(uint32_t counter, uint32_t flag)
{
    uint32_t previous = g_source0_bios.vblank_clear_mode;
    g_source0_bios.vblank_clear_mode = flag;
    g_source0_bios.vblank_calls++;
    g_source0_bios.vblank_counter = counter;
    g_source0_bios.vblank_argument = flag;
    g_source0_bios.mask_at_vblank_call = PE_IRQ_GetMask();
    g_source0_bios.source0_slot_at_vblank_call =
        (pe_addr_t)PE_LoadU32(GA_IRQ_CPU_CALLBACK_TABLE);
    g_source0_bios.registered_mask_at_vblank_call =
        PE_LoadU16(GA_IRQ_REGISTERED_MASK);
    g_source0_bios.vblank_call_order = ++g_source0_setup_order;
    return previous;
}

void PE_Irq_GetSource0BiosState(PeIrqSource0BiosState *out)
{
    if (out != NULL) {
        *out = g_source0_bios;
    }
}

/* B53I-B1 translates only the execution-proven source-0/source-3 paths of
 * retail func_800740D0.  Other sources have additional BIOS side effects
 * (4,5,6) or unchecked table arithmetic and remain an explicit boundary. */
pe_addr_t func_800740D0(uint32_t source, pe_addr_t handler)
{
    pe_addr_t slot;
    pe_addr_t previous;
    uint16_t registered;
    uint16_t restored_mask;
    uint16_t source_bit;

    if (source != 0u && source != 3u) {
        pe_addr_t result = (pe_addr_t)Bootstrap_ReturnInt4Indirect(
            "func_800740D0_source_cut", "func_800740D0", 0,
            0x800740D0u, source, handler, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return result;
    }

    slot = (pe_addr_t)(GA_IRQ_CPU_CALLBACK_TABLE + (source << 2));
    previous = (pe_addr_t)PE_LoadU32(slot);

    /* 0x8007410C: equality returns before the guard or mask is touched. */
    if (handler == previous) {
        return previous;
    }

    /* 0x80074114..1C: registration is unavailable before ResetCallback has
     * published its guard. */
    if (PE_LoadU16(GA_IRQ_RESET_GUARD) == 0u) {
        return previous;
    }

    restored_mask = PE_IRQ_ExchangeMask(0u);
    source_bit = (uint16_t)(1u << source);

    if (handler != 0u) {
        PE_StoreU32(slot, handler);
        registered = PE_LoadU16(GA_IRQ_REGISTERED_MASK);
        restored_mask = (uint16_t)(restored_mask | source_bit);
        registered = (uint16_t)(registered | source_bit);
    } else {
        PE_StoreU32(slot, 0u);
        registered = PE_LoadU16(GA_IRQ_REGISTERED_MASK);
        restored_mask = (uint16_t)(restored_mask & (uint16_t)~source_bit);
        registered = (uint16_t)(registered & (uint16_t)~source_bit);
    }
    PE_StoreU16(GA_IRQ_REGISTERED_MASK, registered);

    if (source == 0u) {
        uint32_t removing = handler == 0u;
        ChangeClearPadB1(removing);
        (void)ChangeClearVBlankB1(3u, removing);
    }

    (void)PE_IRQ_ExchangeMask(restored_mask);
    if (source == 0u) {
        g_source0_bios.source0_mask_restore_order = ++g_source0_setup_order;
    }
    return previous;
}

/* Complete canonical installed-target behavior of the 12-word wrapper at
 * 0x80073CC4..0x80073CF3.  As with the already-translated func_80073CF4,
 * pre-install/dirty SDK jump-table behavior is outside the proven path. */
pe_addr_t func_80073CC4(uint32_t source, pe_addr_t handler)
{
    return func_800740D0(source, handler);
}

void func_80073C94(void)
{
    if (PE_LoadU16(GA_IRQ_RESET_GUARD) != 0) {
        return;                     /* one-time guard */
    }

    /* 0x80073E60..7C: controller reset order is I_MASK, I_STAT, DPCR. */
    (void)PE_IRQ_ExchangeMask(0u);
    PE_IRQ_WriteStatus(PE_IRQ_GetMask());
    PE_GPU_WriteDPCR(0x33333333u);

    /* 0x80073E80: func_80074330(D_800945E4, 0x41A) clears words, not
     * bytes.  The exclusive end is exactly the SDK jump table D_8009564C. */
    PE_Fill(GA_IRQ_RESET_BLOCK, GA_IRQ_RESET_BLOCK_BYTES, 0u);

    /* 0x80073EC0 delay slot publishes the guard immediately before the
     * source-0 initializer. */
    PE_StoreU16(GA_IRQ_RESET_GUARD, 1u);
    PE_Callback_ResetTable();

    /* func_800743B4 -> func_80073CC4(0, func_8007440C). */
    (void)func_80073CC4(0u, GA_IRQ_SOURCE0_HANDLER);

    /* func_800744D4 clears all eight guest DMA identities, then writes
     * literal zero to DICR in the delay slot of the source-3 registration
     * call.  DICR zero clears controls but W1C-retains existing flags. */
    for (uint32_t channel = 0u; channel < PE_DMA_CALLBACK_SLOTS; channel++) {
        PE_StoreU32((pe_addr_t)(GA_DMA_CALLBACK_TABLE + channel * 4u), 0u);
    }
    PE_GPU_WriteDICR(0u);
    (void)func_80073CC4(3u, GA_IRQ_SOURCE3_HANDLER);
}

/* Phase 6E-B53D — func_80073E10 (asm/disc1/64610.s @ file 0x64610, 6 words,
 * 0x18 bytes, exclusive end 0x80073E28): the retail 16-bit I_MASK exchange
 * helper.
 *   lui  v1,0x8009 ; lw v1,0x5674(v1)   v1 = D_80095674 = 0x1F801074
 *   nop            ; lhu v0,0(v1)       v0 = previous I_MASK (zero-extended)
 *   jr   ra        ; sh  a0,0(v1)       I_MASK = a0 low 16 bits (delay slot)
 * True ABI: uint16_t func_80073E10(uint16_t new_mask) — v0 is the
 * zero-extended previous mask; only a0's low halfword is stored.  Nine
 * executable call sites (func_80076C34 x3, func_80076EE4 x2, func_80077144
 * x2, func_80077404 x2) all use it as mask-off / mask-restore pairs.
 * Classification: 3 (deterministic platform provider, PE_IRQ authority). */
uint16_t func_80073E10(uint16_t new_mask)
{
    return PE_IRQ_ExchangeMask(new_mask);
}

/* Phase 6E-B6 — func_80073D24 (asm/disc1/5F3E4.s @ 0x80073D24, 13 words):
 * libetc callback-slot wrapper, reached through the jump table D_8009564C
 * field 0x14 with the slot forced to 4 in the jal delay slot.  Field 0x14
 * is installed by ResetCallback (func_80073E28) as func_800743B4's return
 * value = func_80074478 (asm/disc1/645F8.s @ 0x80074478):
 *   addr = D_8009568C + (slot << 2); prev = *addr;
 *   if (handler != prev) *addr = handler;  return prev;
 * Both exe-wide call sites (func_8003E680 @ 0x8003E6E0 / 0x8003E6F0)
 * discard the return.  Classification: 2 (SDK host implementation). */
uint32_t func_80073D24(pe_addr_t handler)
{
    return PE_Callback_SetSlot(4u, handler);
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

/* Retail is the three-word BIOS A0(44h) FlushCache veneer at 0x800726C4.
 * Native host code is never emitted into the emulated R3000 instruction
 * cache, so there is no corresponding cache authority to mutate. */
void func_800726C4(void)
{
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
    PE_IRQ_Reset();
    PE_IRQ_DeliveryTraceReset();
    ResetSource0BiosState();

    /* Coherent host reset of the guest-backed CPU registration authority.
     * This is intentionally narrower than retail func_80074330's 0x1068-byte
     * initialization block: PE_Sdk_ResetState must not erase unrelated guest
     * program state.  ResetCallback performs the authentic full clear. */
    PE_StoreU16(GA_IRQ_RESET_GUARD, 0u);
    PE_StoreU16(GA_IRQ_DISPATCH_ACTIVE, 0u);
    for (uint32_t source = 0u; source < PE_IRQ_CPU_SOURCE_COUNT; source++) {
        PE_StoreU32((pe_addr_t)(GA_IRQ_CPU_CALLBACK_TABLE + source * 4u), 0u);
    }
    PE_StoreU16(GA_IRQ_REGISTERED_MASK, 0u);
    PE_StoreU32(GA_IRQ_WATCHDOG, 0u);
    for (uint32_t channel = 0u; channel < PE_DMA_CALLBACK_SLOTS; channel++) {
        PE_StoreU32((pe_addr_t)(GA_DMA_CALLBACK_TABLE + channel * 4u), 0u);
    }

    PE_SpuDma_Reset();
    PE_GPU_Init();
    PE_MDEC_Init();
    /* Host/platform reset cancels an IRQ that can no longer be delivered. */
    PE_StoreU32(0x8009D24Cu, 0);
    PE_StoreU32(0x8009B434u, 0);
    PE_StoreU32(0x8009B3ECu, 0);
    PE_StoreU32(0x8009B384u, 0);
    g_pe_gte.ofx = g_pe_gte.ofy = 0;
    g_pe_gte.h = 0;
    g_pe_gte.dqa = g_pe_gte.dqb = 0;
    g_pe_gte.zsf3 = g_pe_gte.zsf4 = 0;
    memset(g_pe_gte.rt, 0, sizeof(g_pe_gte.rt));
    memset(g_pe_gte.tr, 0, sizeof(g_pe_gte.tr));
    memset(g_pe_gte.llm, 0, sizeof(g_pe_gte.llm));
    memset(g_pe_gte.lcm, 0, sizeof(g_pe_gte.lcm));
    memset(g_pe_gte.bk, 0, sizeof(g_pe_gte.bk));
    memset(g_pe_gte.ir, 0, sizeof(g_pe_gte.ir));
    memset(g_pe_gte.mac, 0, sizeof(g_pe_gte.mac));
    memset(g_pe_gte.v0, 0, sizeof(g_pe_gte.v0));
    memset(g_pe_gte.v1, 0, sizeof(g_pe_gte.v1));
    memset(g_pe_gte.v2, 0, sizeof(g_pe_gte.v2));
    g_pe_gte.rgbc = 0;
    memset(g_pe_gte.rgb_fifo, 0, sizeof(g_pe_gte.rgb_fifo));
    /* func_80052C6C state globals */
    D_8009D018 = 0;
    D_8009D03C = 0;
    D_8009D048 = 0;
    D_8009D04C = 0;
    D_8009D050 = 0;
    D_8009D054 = 0;
    D_8009D058 = 0;
    D_8009D064 = 0;
}
