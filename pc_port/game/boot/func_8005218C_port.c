/*
 * Phase 6E-B43 — func_8005218C, prefix-only translation.
 *
 * The complete retail body is 155 instructions / 0x26C bytes at
 * 0x8005218C..0x800523F7 (exclusive end 0x800523F8), executable file
 * offset 0x4298C.  It is live in asm/disc1/42664.s:246-406 under the
 * configs/USA/disc1.yaml [0x42664, asm] split.  tools/b43_oracle.py holds
 * and executes the complete SHA-exact transcription.
 *
 * True top-level ABI: void func_8005218C(void).  All five executable
 * callers use a nop call delay slot and ignore the returned register.
 *
 * Architecture B is mandatory.  Retail's first dependency call is:
 *
 *   func_8005B91C(a0=0,
 *                 a1=(int32_t)(int16_t)load16(0x800C0E28),
 *                 a2=&frame_word_at_sp_plus_0x10,
 *                 a3=0)
 *
 * func_8005B91C does not provide a useful consumed return.  Instead, it
 * writes the 32-bit word designated by a2; 0x800521B8 immediately loads
 * that word and passes it to translated func_8005DBAC.  Continuing without
 * the unresolved dependency's state effect would fabricate the table index.
 * Therefore production executes exactly the independently proven prefix and
 * stops at func_8005B91C through the centralized boundary.  No persistent
 * guest or authoritative state is written before that call.
 *
 * The native local below represents retail's transient stack output word.
 * Its full-width host address exists only in the host diagnostic boundary
 * log; it is never truncated, stored in guest RAM, or retained after return.
 */
#include "psx_compat.h"

#define GA_B43_TARGET0 0x800C0E28u

void func_8005218C(void)
{
    int32_t table_index_out;
    int32_t target = (int32_t)(int16_t)PE_LoadU16(GA_B43_TARGET0);

    Bootstrap_ReturnVoid4("func_8005B91C", "func_8005218C",
                          (uintptr_t)0u, (uintptr_t)(uint32_t)target,
                          (uintptr_t)&table_index_out, (uintptr_t)0u);
}
