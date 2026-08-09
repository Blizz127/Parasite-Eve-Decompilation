/*
 * Phase 6E-B53D — single 16-bit I_MASK (0x1F801074) platform authority.
 *
 * Retail evidence (SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b):
 * every libetc/libgpu I_MASK access is a 16-bit lhu/sh through the guest
 * pointer D_80095674, whose executable initial data is 0x1F801074.  This
 * module owns only that mask state.  It deliberately does not model
 * I_STAT (0x1F801070), IRQ dispatch, callback invocation, or any interrupt
 * delivery/timing: writes replace the mask, reads return it, and nothing
 * else happens.
 */
#ifndef PE_IRQ_H
#define PE_IRQ_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Host lifecycle initialization: I_MASK = 0, the value retail establishes
 * in func_80073E28 (ResetCallback's one-time interrupt-controller reset)
 * and the only value the boot path has ever depended on.  Called from
 * PE_Sdk_ResetState, the single host reset owner. */
void PE_IRQ_Reset(void);

uint16_t PE_IRQ_GetMask(void);

/* Exact func_80073E10 semantics: return the previous 16-bit mask
 * (zero-extended lhu), then replace it with the new 16-bit value (sh).
 * No IRQ dispatch, no I_STAT mutation, no callback invocation. */
uint16_t PE_IRQ_ExchangeMask(uint16_t new_mask);

#ifdef __cplusplus
}
#endif

#endif /* PE_IRQ_H */
