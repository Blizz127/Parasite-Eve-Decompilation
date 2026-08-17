/*
 * PE-BTL6 — func_800794C4 first leaf (RotMatrix, 163 words, zero callees).
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3d050_remainder_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * Exclusive 0x800794C4..0x80079750 (jr 0x80079748, delay nop).
 * Live 3D050 site is jal 794C4(dest+0x2C, dest+0x34) after those
 * halfwords were zeroed. Looks up D_800966EC[(angle&0xFFF)*4].
 * Later packed leaves / EnterCriticalSection tail are not this cut.
 * Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_SINCOS 0x800966ECu

static uint32_t pe_794c4_sincos_word(int16_t angle)
{
    unsigned idx;

    if (angle >= 0)
        idx = (unsigned)angle & 0xFFFu;
    else
        idx = (unsigned)(-(int)angle) & 0xFFFu;
    return PE_LoadU32(GA_SINCOS + idx * 4u);
}

void func_800794C4(pe_addr_t angles, pe_addr_t out)
{
    int16_t ax, ay, az;
    uint32_t word;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;

    if (angles == 0u || out == 0u)
        return;

    ax = (int16_t)PE_LoadU16(angles + 0u);
    ay = (int16_t)PE_LoadU16(angles + 2u);
    az = (int16_t)PE_LoadU16(angles + 4u);

    word = pe_794c4_sincos_word(ax);
    if (ax >= 0) {
        t3 = (int16_t)word;
        t0 = (int16_t)(word >> 16);
    } else {
        t3 = -(int16_t)word;
        t0 = (int16_t)(word >> 16);
    }

    word = pe_794c4_sincos_word(ay);
    if (ay >= 0) {
        t6 = (int16_t)word;
        t4 = -t6;
        t1 = (int16_t)(word >> 16);
    } else {
        t4 = (int16_t)word;
        t6 = -t4;
        t1 = (int16_t)(word >> 16);
    }
    PE_StoreU16(out + 4u, (uint16_t)t6);
    t8 = (int)((uint32_t)t1 * (uint32_t)t3);
    t6 = (-t8) >> 12;
    PE_StoreU16(out + 0xAu, (uint16_t)t6);
    t8 = (int)((uint32_t)t1 * (uint32_t)t0);

    word = pe_794c4_sincos_word(az);
    t6 = t8 >> 12;
    PE_StoreU16(out + 0x10u, (uint16_t)t6);
    if (az >= 0) {
        t5 = (int16_t)word;
        t2 = (int16_t)(word >> 16);
    } else {
        t5 = -(int16_t)word;
        t2 = (int16_t)(word >> 16);
    }

    t7 = (int)((uint32_t)t2 * (uint32_t)t1);
    PE_StoreU16(out + 0u, (uint16_t)(t7 >> 12));
    t7 = (int)((uint32_t)t5 * (uint32_t)t1);
    PE_StoreU16(out + 2u, (uint16_t)((-t7) >> 12));

    t7 = (int)((uint32_t)t2 * (uint32_t)t4);
    t8 = t7 >> 12;
    t6 = ((int)((uint32_t)t8 * (uint32_t)t3)) >> 12;
    t9 = ((int)((uint32_t)t5 * (uint32_t)t0)) >> 12;
    PE_StoreU16(out + 6u, (uint16_t)(t9 - t6));

    t7 = ((int)((uint32_t)t8 * (uint32_t)t0)) >> 12;
    t9 = ((int)((uint32_t)t5 * (uint32_t)t3)) >> 12;
    PE_StoreU16(out + 0xCu, (uint16_t)(t9 + t7));

    t7 = (int)((uint32_t)t5 * (uint32_t)t4);
    t8 = t7 >> 12;
    t6 = ((int)((uint32_t)t8 * (uint32_t)t3)) >> 12;
    t9 = ((int)((uint32_t)t2 * (uint32_t)t0)) >> 12;
    PE_StoreU16(out + 8u, (uint16_t)(t9 + t6));

    t7 = ((int)((uint32_t)t8 * (uint32_t)t0)) >> 12;
    t9 = ((int)((uint32_t)t2 * (uint32_t)t3)) >> 12;
    PE_StoreU16(out + 0xEu, (uint16_t)(t9 - t7));
}
