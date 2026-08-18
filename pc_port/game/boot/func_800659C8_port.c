/*
 * PE-CH2 — func_800659C8: opcode 0x7B camera-slot parameter.
 *
 * Complete retail body (12 words / 0x30, exe 0x800659C8–0x800659F8,
 * file offset 0x561C8).  The sole caller is opcode handler
 * func_80018C58 at jal 0x80018C70.
 *
 * The slot address is *(D_800B1624) + offset_at_+0x10 + index*16.
 * Retail shifts the parameter right by 8 and stores its low halfword
 * at slot+8.  The sh at 0x800659EC is the only store.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B1624 0x800B1624u

int func_800659C8(unsigned int index, unsigned int value)
{
    pe_addr_t container = PE_LoadU32(GA_D_800B1624);
    pe_addr_t slot = container + PE_LoadU32(container + 0x10u)
                   + (index << 4);

    PE_StoreU16(slot + 8u, (uint16_t)(value >> 8));
    return 0;
}
