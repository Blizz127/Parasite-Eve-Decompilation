/*
 * PE-BTL6 — func_8003B97C empty early-out cut.
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d834_callees_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Retail 217 words 0x8003B97C..0x8003BCE0, zero callees. Live 3D834
 * is jal 3B97C(dest, D_800BEA40). Returns when dest+0==0, dest+0xBA==0,
 * or lbu(obj+2) is blez. The lighting GTE loop (NCLIP / scratchpad
 * 0x1F800004 / tables 0x800B1638 and 0x800A6360) is not this cut.
 * Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8003B97C_empty_cut(pe_addr_t dest, pe_addr_t bea40)
{
    pe_addr_t obj;

    (void)bea40;
    if (dest == 0u)
        return;
    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    if ((int16_t)PE_LoadU16(dest + 0xBAu) == 0)
        return;
    if (PE_LoadU8(obj + 2u) == 0u)
        return;
    /* Non-empty lighting walk is GTE; not this cut. */
}
