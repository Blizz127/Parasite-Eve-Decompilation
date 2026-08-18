/*
 * PE-BTL121 — BE834 publisher.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * TEXT has no `li 406` store to BE834. The two absolute `sh`
 * sites (20F7C, 26FA0) write zero. Live publication is:
 *
 *   512AC(1, &index) → D010 = index+387
 *   299CC 29A68      → D2A4 = 5C498() / D010
 *   26824(1)         → lh D2A4; tid 406 takes jtbl[13]
 *                      @ 0x800108AC+13*4 = 2692C
 *                      sh D2A4, slot+4 (BE834 + CE3C*8)
 *
 * 57B70(index) is the 512AC(1) caller (57BB8 li a0,1 /
 * 57BBC a1=sp+16 / 57BD4 sw s1,16(sp) with s1=a0).
 *
 * 26FD0 / 71A54 / AE000 actor bind stay deferred. Do not
 * plant 406 on BE834.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D010 0x8009D010u
#define GA_D_8009D2A4 0x8009D2A4u
#define GA_D_8009CE3C 0x8009CE3Cu
#define GA_T_800BE830 0x800BE830u

void func_80057B70(uint32_t cmd_index)
{
    pe_addr_t buf = 0x8010B400u;

    PE_StoreU32(buf, cmd_index);
    func_800512AC(1, buf);
}

void func_80026824(int gate)
{
    int16_t tid;
    uint8_t slot_i;
    pe_addr_t slot;

    if (gate != 1)
        return;
    tid = (int16_t)PE_LoadU16(GA_D_8009D2A4);
    if (tid <= 0)
        return;
    if (tid < 387 || tid >= 407)
        return;
    if ((int)tid - 393 != 13)
        return;

    slot_i = PE_LoadU8(GA_D_8009CE3C);
    slot = GA_T_800BE830 + ((uint32_t)slot_i << 3);
    PE_StoreU16(slot + 4u, (uint16_t)tid);
    PE_StoreU8(GA_D_8009CE3C, (uint8_t)(slot_i + 1u));
}
