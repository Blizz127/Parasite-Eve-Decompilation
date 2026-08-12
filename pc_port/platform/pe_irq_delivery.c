/*
 * Phase 6E-B53I-B2 — source-3 CPU IRQ service and retail DMA dispatcher.
 *
 * Retail executable SHA-1:
 *   452fb033f2eaa4b18aa20a5bca60b8125af3a37b
 *
 * func_80073F00: 0x80073F00..0x800740CF, 116 words, semantic void ABI.
 * func_80074520: 0x80074520..0x8007469F,  96 words, semantic void ABI.
 */
#include "pe_irq_delivery.h"

#include <stdio.h>
#include <string.h>

#include "game_port.h"
#include "pe_bootstrap.h"
#include "pe_gpu.h"
#include "pe_guest_ram.h"
#include "pe_spu_dma.h"
#include "psx_compat.h"

#define GA_IRQ_RESET_GUARD        0x800945E4u
#define GA_IRQ_DISPATCH_ACTIVE    0x800945E6u
#define GA_IRQ_CPU_CALLBACK_TABLE 0x800945E8u
#define GA_IRQ_REGISTERED_MASK    0x80094614u
#define GA_IRQ_WATCHDOG           0x8009567Cu
#define PE_IRQ_CPU_SOURCE_COUNT   11u
#define PE_IRQ_DMA_SOURCE_BIT     0x0008u

#define GA_CPU_DMA_HANDLER        0x80074520u
#define GA_DMA_CALLBACK_TABLE     0x800956C0u
#define GA_DMA2_PUMP_HANDLER      0x80076EE4u
#define PE_DMA_CHANNEL_COUNT      7u

static PeIrqDeliveryTrace g_trace;
static uint64_t g_trace_order;

static uint64_t TraceNext(void)
{
    g_trace_order++;
    if (g_trace_order == 0u) {
        g_trace_order = 1u;
    }
    return g_trace_order;
}

void PE_IRQ_DeliveryTraceReset(void)
{
    memset(&g_trace, 0, sizeof(g_trace));
    g_trace_order = 0u;
}

void PE_IRQ_GetDeliveryTrace(PeIrqDeliveryTrace *out)
{
    if (out != NULL) {
        *out = g_trace;
    }
}

PeIrqEdgeResult PE_IRQ_BridgeDICRRisingEdge(PeIrqGeneration generation)
{
    /* Check before consuming: a stale checkpoint must not steal a fresh
     * post-reset edge belonging to the current generation. */
    if (generation != PE_IRQ_Generation()) {
        return PE_IRQ_EDGE_STALE;
    }
    if (!PE_GPU_TakeDICRRisingEdge()) {
        return PE_IRQ_EDGE_NONE;
    }
    if (!PE_IRQ_AssertSourcesForGeneration(PE_IRQ_DMA_SOURCE_BIT,
                                           generation)) {
        return PE_IRQ_EDGE_STALE;
    }
    g_trace.edge_assert_order = TraceNext();
    return PE_IRQ_EDGE_ASSERTED;
}

static PeIrqServiceResult GuestIndirectBoundary(const char *symbol,
                                                const char *caller,
                                                pe_addr_t handler)
{
    Bootstrap_ReturnVoid4Indirect(symbol, caller, (uintptr_t)handler,
                                  0u, 0u, 0u, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return PE_IRQ_SERVICE_BOUNDARY;
}

static PeIrqServiceResult GuestDirectBoundary(const char *symbol,
                                              const char *caller)
{
    Bootstrap_ReturnVoid(symbol, caller);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return PE_IRQ_SERVICE_BOUNDARY;
}

static uint32_t ReadRepresentedDmaMadr(uint32_t channel)
{
    /* Retail reads DMA_BASE + channel*0x10 for all channels 0..6.  B2
     * routes the two DMA register authorities that the port represents;
     * all other channel register blocks remain in their reset-zero state.
     * This is read-only routing, not a callback or DMA-state mirror. */
    if (channel == 2u) {
        return PE_GPU_ReadDMA2MADR();
    }
    if (channel == 4u) {
        return PE_SpuDma_ReadMADR();
    }
    return 0u;
}

static PeIrqServiceResult DispatchDmaCallback(pe_addr_t handler)
{
    int retail_returned;

    if (handler == GA_DMA2_PUMP_HANDLER) {
        g_trace.dma_callback_istat = PE_IRQ_ReadStatus();
        g_trace.dma_callback_dicr = PE_GPU_ReadDICR();
        g_trace.dma_callback_chcr = PE_GPU_ReadDMA2CHCR();
        (void)PE_func_80076EE4_Pump(&retail_returned);
        if (!retail_returned) {
            return PE_IRQ_SERVICE_BOUNDARY;
        }
        return PE_IRQ_SERVICE_RETURNED;
    }
    return GuestIndirectBoundary("func_80074520_dma_indirect_call",
                                 "func_80074520", handler);
}

PeIrqServiceResult PE_func_80074520_Dispatch(void)
{
    uint32_t pending;

    g_trace.dma_dispatch_entries++;

    /* 0x80074548..54: physical DICR high byte, excluding derived bit 31
     * from the seven-channel pending mask. */
    pending = (PE_GPU_ReadDICR() >> 24) & 0x7Fu;
    while (pending != 0u) {
        uint32_t snapshot = pending;
        uint32_t channel = 0u;

        /* 0x80074574..D4: low-to-high channels 0..6. */
        while (snapshot != 0u && channel < PE_DMA_CHANNEL_COUNT) {
            if ((snapshot & 1u) != 0u) {
                uint32_t flag = 1u << (24u + channel);
                uint32_t physical = PE_GPU_ReadDICR();
                uint32_t acknowledge =
                    physical & (0x00FFFFFFu | flag);
                pe_addr_t handler;

                /* 0x80074594..AC: fresh DICR read and exact W1C write
                 * precede the live callback-slot lookup. */
                PE_GPU_WriteDICR(acknowledge);
                g_trace.last_dma_channel = channel;
                g_trace.last_dma_ack_write = acknowledge;
                g_trace.dma_ack_order = TraceNext();
                if (g_trace.dma_ack_count <
                    sizeof(g_trace.dma_ack_channels)) {
                    g_trace.dma_ack_channels[g_trace.dma_ack_count] =
                        (uint8_t)channel;
                }
                g_trace.dma_ack_count++;

                handler = (pe_addr_t)PE_LoadU32(
                    (pe_addr_t)(GA_DMA_CALLBACK_TABLE + channel * 4u));
                g_trace.last_dma_handler = handler;
                if (handler != 0u) {
                    PeIrqServiceResult result;
                    g_trace.dma_callback_order = TraceNext();
                    result = DispatchDmaCallback(handler);
                    if (result != PE_IRQ_SERVICE_RETURNED) {
                        /* Retail never reaches 0x800745C8 after a nested
                         * non-return.  Acknowledgement stays committed. */
                        return result;
                    }
                }
            }
            snapshot >>= 1;
            channel++;
        }

        /* 0x800745D8..F8: live resample only after the current snapshot
         * has returned normally. */
        pending = (PE_GPU_ReadDICR() >> 24) & 0x7Fu;
    }

    /* 0x800745FC..78: retain both volatile DICR reads and the exact retail
     * diagnostic strings.  MADRs route to the existing live DMA2/DMA4
     * authorities; unrepresented channel register blocks are reset zero.
     * printf return values are ignored by retail. */
    {
        uint32_t physical = PE_GPU_ReadDICR();
        int diagnostic = (physical & 0xFF000000u) == 0x80000000u;
        if (!diagnostic) {
            diagnostic = (PE_GPU_ReadDICR() & PE_GPU_DICR_FORCE) != 0u;
        }
        if (diagnostic) {
            uint32_t diagnostic_dicr = PE_GPU_ReadDICR();
            g_trace.dma_diagnostic_count++;
            g_trace.dma_diagnostic_dicr = diagnostic_dicr;
            fprintf(stderr, "DMA bus error: code=%08x\n",
                    (unsigned)diagnostic_dicr);
            for (uint32_t channel = 0u;
                 channel < PE_DMA_CHANNEL_COUNT; channel++) {
                uint32_t madr = ReadRepresentedDmaMadr(channel);
                g_trace.dma_diagnostic_madr[channel] = madr;
                fprintf(stderr, "MADR[%d]=%08x\n", (int)channel,
                        (unsigned)madr);
            }
        }
    }
    return PE_IRQ_SERVICE_RETURNED;
}

void func_80074520(void)
{
    (void)PE_func_80074520_Dispatch();
}

static PeIrqServiceResult DispatchCpuCallback(pe_addr_t handler)
{
    if (handler == GA_CPU_DMA_HANDLER) {
        return PE_func_80074520_Dispatch();
    }
    return GuestIndirectBoundary("func_80073F00_cpu_indirect_call",
                                 "func_80073F00", handler);
}

PeIrqServiceResult PE_IRQ_ServicePendingForGeneration(
    PeIrqGeneration generation)
{
    uint16_t pending;

    /* Stale service must be completely inert, including dispatch-active
     * and acknowledgements. */
    if (generation != PE_IRQ_Generation()) {
        return PE_IRQ_SERVICE_STALE;
    }
    /* Masked status is hardware-pending but does not enter the retail CPU
     * exception dispatcher. */
    if ((uint16_t)(PE_IRQ_ReadStatus() & PE_IRQ_GetMask()) == 0u) {
        return PE_IRQ_SERVICE_RETURNED;
    }
    g_trace.cpu_service_entries++;

    /* 0x80073F24..54 diagnoses an uninitialised SDK controller and calls
     * func_80074384, the BIOS B(17h) ReturnFromException trampoline.  That
     * exception-context transfer is not represented; stop before retail's
     * later scan join rather than inventing a fallthrough. */
    if (PE_LoadU16(GA_IRQ_RESET_GUARD) == 0u) {
        fprintf(stderr, "unexpected interrupt(%04x)\n",
                (unsigned)PE_IRQ_ReadStatus());
        return GuestDirectBoundary("func_80074384", "func_80073F00");
    }

    /* 0x80073F60..84: registered & I_STAT & I_MASK, with active published
     * before scanning. */
    PE_StoreU16(GA_IRQ_DISPATCH_ACTIVE, 1u);
    pending = (uint16_t)(PE_LoadU16(GA_IRQ_REGISTERED_MASK) &
                         PE_IRQ_ReadStatus() & PE_IRQ_GetMask());
    while (pending != 0u) {
        uint16_t snapshot = pending;
        uint32_t source = 0u;

        while (snapshot != 0u && source < PE_IRQ_CPU_SOURCE_COUNT) {
            if ((snapshot & 1u) != 0u) {
                pe_addr_t handler;

                /* 0x80073FC0: W0C source acknowledgement precedes the
                 * live slot lookup at 0x80073FC4. */
                PE_IRQ_WriteStatus((uint16_t)~(uint16_t)(1u << source));
                g_trace.last_cpu_source = source;
                g_trace.last_cpu_ack_write =
                    (uint16_t)~(uint16_t)(1u << source);
                g_trace.cpu_ack_order = TraceNext();
                if (g_trace.cpu_ack_count <
                    sizeof(g_trace.cpu_ack_sources)) {
                    g_trace.cpu_ack_sources[g_trace.cpu_ack_count] =
                        (uint8_t)source;
                }
                g_trace.cpu_ack_count++;

                handler = (pe_addr_t)PE_LoadU32(
                    (pe_addr_t)(GA_IRQ_CPU_CALLBACK_TABLE + source * 4u));
                g_trace.last_cpu_handler = handler;
                if (handler != 0u) {
                    PeIrqServiceResult result;
                    g_trace.cpu_callback_order = TraceNext();
                    result = DispatchCpuCallback(handler);
                    if (result != PE_IRQ_SERVICE_RETURNED) {
                        /* Do not emulate unwinding cleanup: retail has not
                         * reached 0x80073FDC or terminal 0x800740AC. */
                        return result;
                    }
                }
            }
            snapshot = (uint16_t)(snapshot >> 1);
            source++;
        }

        /* 0x80073FF0..0x80074018: resample all three live authorities. */
        pending = (uint16_t)(PE_LoadU16(GA_IRQ_REGISTERED_MASK) &
                             PE_IRQ_ReadStatus() & PE_IRQ_GetMask());
    }

    /* 0x80074020..A0: registered sources have drained, but raw enabled
     * unregistered sources remain visible to the retail watchdog. */
    if ((uint16_t)(PE_IRQ_ReadStatus() & PE_IRQ_GetMask()) != 0u) {
        uint32_t old = PE_LoadU32(GA_IRQ_WATCHDOG);
        PE_StoreU32(GA_IRQ_WATCHDOG, old + 1u);
        if (old >= 0x801u) {
            fprintf(stderr, "[IRQ] unregistered I_STAT=%04X I_MASK=%04X\n",
                    PE_IRQ_ReadStatus(), PE_IRQ_GetMask());
            PE_StoreU32(GA_IRQ_WATCHDOG, 0u);
            PE_IRQ_WriteStatus(0u);
        }
    } else {
        PE_StoreU32(GA_IRQ_WATCHDOG, 0u);
    }

    /* Exact normal terminal delay-slot effect at 0x800740AC. */
    PE_StoreU16(GA_IRQ_DISPATCH_ACTIVE, 0u);
    return PE_IRQ_SERVICE_RETURNED;
}
