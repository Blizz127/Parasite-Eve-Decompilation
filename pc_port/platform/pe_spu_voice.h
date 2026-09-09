/*
 * Phase AUD1-E1..E8 — guest voice → SPU bridge + Psy-Q attr setters.
 *
 * Real retail symbols (matching guest leaves under src/ where carved):
 *   func_80085F74 = SpuSetCommonAttr
 *   func_800862F4 = SpuSetVoiceAttr
 *   func_800878F0 / func_80089F08 hosted in pe_akao_tick (WriteVoiceParam / ENVX)
 * PE_SpuVoice_ApplyPending publishes guest voice+0xF4 pending bits.
 */
#ifndef PE_SPU_VOICE_H
#define PE_SPU_VOICE_H

#include "pe_guest_ram.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void func_80087798(uint32_t voice_index, uint32_t vol_left, uint32_t vol_right);
void func_80085F74(pe_addr_t attr);
void func_800862F4(int voice, uint16_t left, uint16_t right, int16_t left_mode,
                   uint16_t right_mode);
void PE_SpuVoice_ApplyPending(pe_addr_t voice);
void PE_SpuScore_ApplyDirtyVoices(void);
void func_8008DB7C(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_SPU_VOICE_H */
