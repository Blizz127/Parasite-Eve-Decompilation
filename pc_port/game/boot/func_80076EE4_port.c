/*
 * Phase 6E-B53H — execution-proven busy-DMA prefix of the libgpu command
 * queue pump func_80076EE4.
 *
 * Retail func_80076EE4 is 152 words / 0x260 bytes at
 * 0x80076EE4..0x80077143 (exclusive end 0x80077144, executable file offset
 * 0x676E4, live split asm/disc1/66B54.s).  Its body SHA-256 is
 * a124857ab6fd91a3b68ea3e5c2efa5337bf6ed5528ef94ca23bc4a781337a78c.
 *
 * ABI: `int func_80076EE4(void)`.  No instruction in the body reads a0..a3
 * before writing it, and the four direct callers pass no arguments.
 *
 * B53H translates ONLY the entry busy-DMA fast path, which is the exact
 * canonical path: the first LoadImage DMA2 transfer is still in flight when
 * func_80076C34 opportunistically pumps at 0x80076EA4.
 *
 *   80076EE4  lui   v0,0x8009
 *   80076EE8  lw    v0,0x5860(v0)   ; D_80095860 = 0x1F8010A8 (DMA2 CHCR)
 *   80076EEC  addiu sp,sp,-0x20     ; private frame only
 *   80076EF0  sw    ra,0x18(sp)
 *   80076EF4  sw    s1,0x14(sp)
 *   80076EF8  sw    s0,0x10(sp)
 *   80076EFC  lw    v0,0(v0)        ; CHCR - first hardware state read
 *   80076F00  lui   s0,0x0100       ; 0x01000000 = CHCR bit 24 (start/busy)
 *   80076F04  and   v0,v0,s0
 *   80076F08  bnez  v0,0x80077130   ; busy -> straight to the epilogue
 *   80076F0C   addiu v0,zero,1      ; DELAY SLOT: the return value is 1
 *   ...
 *   80077130  lw    ra,0x18(sp)
 *   80077134  lw    s1,0x14(sp)
 *   80077138  lw    s0,0x10(sp)
 *   8007713C  jr    ra
 *   80077140   addiu sp,sp,0x20
 *
 * On this path the function therefore:
 *
 *   - reads DMA2 CHCR and nothing else;
 *   - never calls func_80073E10, so it performs NO I_MASK exchange and has
 *     no restore obligation (the exchange at 0x80076F10 is past the branch);
 *   - never reads the producer D_80095874 or the consumer D_80095878;
 *   - never reads or writes any ring entry;
 *   - never reads GPUSTAT (the first GPUSTAT read is at 0x80076FA4);
 *   - never calls func_80073CF4/func_800746A0, so callback slot 2 and DICR
 *     are untouched (the deregistration call is at 0x80076F90);
 *   - never invokes a queued worker (the jalr is at 0x80077034);
 *   - never advances the consumer (the store is at 0x80077054);
 *   - never clears the work marker or invokes the DrawSync callback
 *     (0x80077108 / 0x8007710C);
 *   - writes only its own stack frame;
 *   - returns exactly 1.
 *
 * The busy path executes 16 of the 152 words: the 11-word prefix
 * 0x80076EE4..0x80076F0C plus the 5-word shared epilogue
 * 0x80077130..0x80077140.  The remaining 136 words
 * (0x80076F10..0x8007712C) are the idle-DMA consumer path.  They are NOT
 * translated here: canonical execution cannot reach them without first
 * completing the in-flight DMA2 transfer, and completing it merely to
 * advance would fabricate hardware progress.  That path is documented in
 * docs/b53h_func_80076EE4.md for the next rung and is exposed as a
 * controlled boundary below.
 *
 * D_80095860 is a retail .data word holding the literal MMIO address
 * 0x1F8010A8.  CHCR itself stays in the single B53B pe_gpu authority; this
 * translation creates no second CHCR representation.
 */

#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"

/* Retail .data word holding the DMA2 CHCR MMIO address literal. */
#define GA_GPU_DMA2_CHCR_POINTER 0x80095860u
#define PE_DMA2_CHCR_ADDRESS     0x1F8010A8u

/* Value-only entry telemetry used to prove that hardware completion and the
 * DICR edge bridge never call the pump, including its otherwise silent busy
 * fast path.  It is not queue, callback, or run-control authority. */
static uint64_t g_pump_entry_count;

int PE_func_80076EE4_Pump(int *retail_returned)
{
    g_pump_entry_count++;
    if (retail_returned != NULL) {
        *retail_returned = 0;
    }

    /* 0x80076EFC..0x80076F08: the busy test is `CHCR & 0x01000000`, taken
     * before any queue or interrupt state is touched. */
    if ((PE_GPU_ReadDMA2CHCR() & PE_GPU_DMA2_CHCR_BUSY) != 0u) {
        /* 0x80076F0C delay slot, then the shared epilogue at 0x80077130. */
        if (retail_returned != NULL) {
            *retail_returned = 1;
        }
        return 1;
    }

    /* 0x80076F10 onwards: the idle-DMA consumer path exchanges I_MASK,
     * may deregister DMA callback slot 2, polls GPUSTAT, invokes a queued
     * worker, advances the consumer, and may clear the work marker and fire
     * the DrawSync callback.  B53H does not translate it and does not
     * complete DMA to reach it.
     *
     * `retail_returned` stays 0 so callers learn that retail control flow
     * did NOT return here.  A caller must not infer this from the stop
     * reason: PE_Port_RequestStop keeps the FIRST reason, so an earlier
     * frame-limit stop would otherwise mask this boundary. */
    (void)Bootstrap_ReturnInt(
        "func_80076EE4_idle_pump", "func_80076EE4", 0);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0; /* host prefix cut; not a claimed retail result */
}

/* Retail-ABI form, and the identity registered in DMA callback slot 2.
 *
 * CONTRACT: production callers must use PE_func_80076EE4_Pump and honour
 * `retail_returned`.  This form discards that signal, and on the
 * untranslated idle path it returns 0 — which is also a legal retail value
 * (an empty ring yields `(producer - consumer) & 0x3F == 0`).  A caller
 * that consumes this return without checking the signal would therefore be
 * consuming a fabricated result.  It always latches the boundary and
 * requests a stop, so the condition is visible, but B53I's DMA IRQ
 * dispatcher must bind to the _Pump form, not to this one. */
int func_80076EE4(void)
{
    return PE_func_80076EE4_Pump(NULL);
}

/* Read-only names for the retail authority this prefix uses.  They are
 * test/documentation evidence only: no production code consults the retail
 * pointer word D_80095860, and CHCR itself stays owned by B53B pe_gpu.
 * They deliberately do NOT carry the PE_GPU_ platform prefix, so nothing
 * here can be mistaken for a second CHCR authority. */
pe_addr_t PE_Pump_Dma2ChcrPointerAddress(void)
{
    return (pe_addr_t)GA_GPU_DMA2_CHCR_POINTER;
}

uint32_t PE_Pump_Dma2ChcrMmioAddress(void)
{
    return PE_DMA2_CHCR_ADDRESS;
}

void PE_Pump_TraceReset(void)
{
    g_pump_entry_count = 0u;
}

uint64_t PE_Pump_EntryCount(void)
{
    return g_pump_entry_count;
}
