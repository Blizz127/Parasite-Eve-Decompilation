/*
 * PE-BTL6 — func_8003A088 live mode-0 empty cut.
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d834_callees_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Retail 392 words 0x8003A088..0x8003A6A8. Live EE=13 dest+0x28 is 0,
 * so jal 3DBE4 (+0x28==1) and jal 3DD08 (+0x28==3) are skipped, and
 * the +0x28==4 GTE multiply is skipped. Join at 0x8003A348 then
 * blez lhu(obj+0x18) returns at 0x8003A684. Scratchpad 0x1F800000
 * zeros and GTE mtc2 of dest+0x34 are not guest-RAM-observable and
 * are not invented. The bone-walk GTE loop is not this cut.
 * Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8003A088_mode0_empty_cut(pe_addr_t dest)
{
    pe_addr_t obj;
    int16_t mode;
    int16_t count;

    if (dest == 0u)
        return;

    mode = (int16_t)PE_LoadU16(dest + 0x28u);
    if (mode == 1 || mode == 3 || mode == 4)
        return;

    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    count = (int16_t)PE_LoadU16(obj + 0x18u);
    if (count <= 0)
        return;
    /* Non-empty walk is GTE MVMVA; not this cut. */
}
