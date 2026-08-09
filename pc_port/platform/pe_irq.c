/*
 * Phase 6E-B53D — single 16-bit I_MASK (0x1F801074) platform authority.
 *
 * This is the only native storage of the interrupt mask.  It implements
 * exactly the deterministic 16-bit read/write behavior retail performs
 * through D_80095674; it does not deliver interrupts, mutate I_STAT, or
 * invoke callbacks.
 */
#include "pe_irq.h"

static uint16_t g_i_mask;

void PE_IRQ_Reset(void)
{
    g_i_mask = 0;
}

uint16_t PE_IRQ_GetMask(void)
{
    return g_i_mask;
}

uint16_t PE_IRQ_ExchangeMask(uint16_t new_mask)
{
    uint16_t previous = g_i_mask;
    g_i_mask = new_mask;
    return previous;
}
