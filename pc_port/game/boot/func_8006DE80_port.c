/*
 * PE-BTL99 — func_8006DE80 ABI wrapper into func_8006DED4.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b. No matching src/ C.
 *
 * 6DE80: 21 words 0x8006DE80..0x8006DED4 exclusive,
 * SHA-256 860d94bcbf926fb977614f7d917d69f919c20aee54126f58fbba90dc426529d9.
 *
 *   6DED4(lw(D_800B0E08), a0, a1, (int16)a2, (int16)a3, (int16)a4)
 *
 * 1F814 jal @ 0x8001F970 after 1A680, before +0x98 bit 0x100:
 *   a0=0x46A, a1=0, a2=lh Aya+0x2A, a3=lh Aya+0x2E,
 *   stack=lh Aya+0x32.
 *
 * Death-arm jal @ 0x8001F430 uses a0=0x46B and is not this cut.
 *
 * 6DED4 packs the SVECTOR and jals 6DFA8 (GTE / 79244) then
 * 6DF50 (6E514 lookup + 86608). 79244 and 86608 are not ported.
 * Do not invent those stores. dest==0 is the live BTL99 fixture.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B0E08 0x800B0E08u

void func_8006DED4(pe_addr_t dest, int id, int a1, int x, int y, int z)
{
    (void)id;
    (void)a1;
    (void)x;
    (void)y;
    (void)z;
    (void)dest;
}

void func_8006DE80(int id, int a1, int x, int y, int z)
{
    func_8006DED4(PE_LoadU32(GA_D_800B0E08), id, a1,
                  (int)(int16_t)x, (int)(int16_t)y, (int)(int16_t)z);
}
