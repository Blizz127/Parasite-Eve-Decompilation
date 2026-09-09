/*
 * PE-BTL28 — opcode 0xD9 (1A15C) and ratan2 79FB4.
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_8001A15C — 19 words 0x8001A15C..0x8001A1A8, SHA-256
 * f53fb5e6…9760. D_800910A0[0xD9].
 * *arg2 = 79FB4(*arg0, *arg1); v0=1.
 *
 * func_80079FB4 — 93 words 0x80079FB4..0x8007A128, SHA-256
 * e5b0edc7…f820. Zero jal. Signed ratan2 into
 * D_8009A6EC (4096-circle). Both-zero returns 0.
 * Live type-5 after 0x0C: ratan2(localE-localB, localD-localA).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009A6EC 0x8009A6ECu

int32_t func_80079FB4(int32_t a0, int32_t a1)
{
    int sign_y;
    int sign_x;
    int32_t angle;
    uint32_t idx;
    int32_t quot;
    int32_t denom;

    sign_y = 0;
    sign_x = 0;
    if (a1 < 0) {
        sign_y = 1;
        a1 = (int32_t)(0u - (uint32_t)a1);
    }
    if (a0 < 0) {
        sign_x = 1;
        a0 = (int32_t)(0u - (uint32_t)a0);
    }
    if (a1 == 0 && a0 == 0)
        return 0;

    if (a0 < a1) {
        if ((a0 & 0x7FE00000) != 0) {
            denom = a1 >> 10;
            quot = (denom != 0) ? (a0 / denom) : 0;
        } else {
            quot = (a1 != 0) ? ((int32_t)((uint32_t)a0 << 10) / a1) : 0;
        }
        idx = (uint32_t)quot << 1;
        angle = (int32_t)(int16_t)PE_LoadU16(GA_D_8009A6EC + idx);
    } else {
        if ((a1 & 0x7FE00000) != 0) {
            denom = a0 >> 10;
            quot = (denom != 0) ? (a1 / denom) : 0;
        } else {
            quot = (a0 != 0) ? ((int32_t)((uint32_t)a1 << 10) / a0) : 0;
        }
        idx = (uint32_t)quot << 1;
        angle = 1024 - (int32_t)(int16_t)PE_LoadU16(GA_D_8009A6EC + idx);
    }
    if (sign_y != 0)
        angle = 2048 - angle;
    if (sign_x != 0)
        angle = -angle;
    return angle;
}

int func_8001A15C(pe_addr_t args)
{
    int32_t a;
    int32_t b;

    a = (int32_t)PE_LoadU32(PE_LoadU32(args));
    b = (int32_t)PE_LoadU32(PE_LoadU32(args + 4u));
    PE_StoreU32(PE_LoadU32(args + 8u), (uint32_t)func_80079FB4(a, b));
    return 1;
}
