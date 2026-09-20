/*
 * PE-SAVEWRITE — func_8005C25C (hand-translated, nonmatching).
 *
 * Authority: retail Disc1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b,
 * asm/disc1/4C974.s 0x8005C25C..0x8005C374 (0x118, 70w).
 *
 * Copies six 16-bit status words out of D_800A1E6E (0x20 stride) into
 * D_800C1EAC, gathers the play-time / rank / name fields, and installs them in
 * the save-record scratch block.  Called by func_80042020 before the buffer is
 * assembled and written through the card file API.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

void func_8005C25C(void)
{
    pe_addr_t dst = 0x800C1EACu;
    pe_addr_t src = 0x800A1E6Eu;
    uint32_t v0;
    uint32_t s0;

    for (int i = 0; i < 6; i++) {
        PE_StoreU16(dst, PE_LoadU16(src));
        src += 0x20u;
        dst += 2u;
    }

    v0 = (uint32_t)func_800614A0();
    PE_StoreU32(0x800C0E44u, v0);

    v0 = (uint32_t)func_800438E0();
    PE_StoreU16(0x800C0E40u, (uint16_t)v0);

    /* Divide by 100 via the retail 0x88888889 magic (unsigned). */
    {
        uint32_t a = PE_LoadU32(0x800A76BCu);
        uint32_t b = PE_LoadU32(0x800A76A4u);
        PE_StoreU32(0x800C0DE8u,
                    (uint32_t)(((uint64_t)a * 0x88888889ull) >> 32) >> 5);
        PE_StoreU32(0x800C0DECu,
                    (uint32_t)(((uint64_t)b * 0x88888889ull) >> 32) >> 5);
    }

    PE_StoreU8(0x800C0DFFu, (uint8_t)func_8005E884());

    s0 = (uint32_t)func_800527B4();
    v0 = (uint32_t)func_80064A48();
    s0 |= v0 << 2;
    PE_StoreU8(0x800C0DFDu, (uint8_t)s0);
    PE_StoreU8(0x800C0DFEu, 0u);

    PE_StoreU16(0x800C0E3Cu,
                (uint16_t)func_80043474((int32_t)PE_LoadU32(0x800A7918u)));
    PE_StoreU16(0x800C0E3Eu, (uint16_t)func_8005D940());
}
