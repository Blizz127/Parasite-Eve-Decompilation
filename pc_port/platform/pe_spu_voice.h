/*
 * Phase AUD1-E1 — guest voice → SPU register bridge (partial 85F74 / 87798).
 *
 * Publishes pending attributes from guest voice records to the host SPU
 * register window consumed by pe_spu_synth.c. This is not the full retail
 * score sequencer (func_8008DB7C bytecode ticks remain unported).
 */
#ifndef PE_SPU_VOICE_H
#define PE_SPU_VOICE_H

#include "pe_guest_ram.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Retail func_80087798 — per-voice volume pair at 0x1F801C00 + index*0x10. */
void func_80087798(uint32_t voice_index, uint32_t vol_left, uint32_t vol_right);

/* Partial func_80085F74 — apply guest voice+0xF4 pending bits to SPU regs. */
void func_80085F74(pe_addr_t voice);

/* Apply dirty voices when D_8009D2C4 bit 0x100 is set (audio_dirty). */
void PE_SpuScore_ApplyDirtyVoices(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_SPU_VOICE_H */
