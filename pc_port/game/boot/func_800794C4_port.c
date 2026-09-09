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

void PE_RotMatrix794C4_values(const int16_t angles[3], int16_t out[9])
{
    int16_t ax, ay, az;
    uint32_t word;
    int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;

    ax = angles[0];
    ay = angles[1];
    az = angles[2];

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
    out[2] = (int16_t)t6;
    t8 = (int)((uint32_t)t1 * (uint32_t)t3);
    t6 = (-t8) >> 12;
    out[5] = (int16_t)t6;
    t8 = (int)((uint32_t)t1 * (uint32_t)t0);

    word = pe_794c4_sincos_word(az);
    t6 = t8 >> 12;
    out[8] = (int16_t)t6;
    if (az >= 0) {
        t5 = (int16_t)word;
        t2 = (int16_t)(word >> 16);
    } else {
        t5 = -(int16_t)word;
        t2 = (int16_t)(word >> 16);
    }

    t7 = (int)((uint32_t)t2 * (uint32_t)t1);
    out[0] = (int16_t)(t7 >> 12);
    t7 = (int)((uint32_t)t5 * (uint32_t)t1);
    out[1] = (int16_t)((-t7) >> 12);

    t7 = (int)((uint32_t)t2 * (uint32_t)t4);
    t8 = t7 >> 12;
    t6 = ((int)((uint32_t)t8 * (uint32_t)t3)) >> 12;
    t9 = ((int)((uint32_t)t5 * (uint32_t)t0)) >> 12;
    out[3] = (int16_t)(t9 - t6);

    t7 = ((int)((uint32_t)t8 * (uint32_t)t0)) >> 12;
    t9 = ((int)((uint32_t)t5 * (uint32_t)t3)) >> 12;
    out[6] = (int16_t)(t9 + t7);

    t7 = (int)((uint32_t)t5 * (uint32_t)t4);
    t8 = t7 >> 12;
    t6 = ((int)((uint32_t)t8 * (uint32_t)t3)) >> 12;
    t9 = ((int)((uint32_t)t2 * (uint32_t)t0)) >> 12;
    out[4] = (int16_t)(t9 + t6);

    t7 = ((int)((uint32_t)t8 * (uint32_t)t0)) >> 12;
    t9 = ((int)((uint32_t)t2 * (uint32_t)t3)) >> 12;
    out[7] = (int16_t)(t9 - t7);
}

void func_800794C4(pe_addr_t angles, pe_addr_t out)
{
    int16_t input[3],rotation[9];unsigned i;
    if (!angles || !out) return;
    for (i=0;i<3;i++) input[i]=(int16_t)PE_LoadU16(angles+i*2u);
    PE_RotMatrix794C4_values(input,rotation);
    for (i=0;i<9;i++) PE_StoreU16(out+i*2u,(uint16_t)rotation[i]);
}

/* 79754, 163 words: alternate Euler order used by animation decoders.
 * Each multiply retains its low word before arithmetic >>12, as retail. */
static int32_t pe_79754_mul(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a * (uint32_t)b) >> 12;
}

void PE_RotMatrix79754_values(const int16_t angles[3], int16_t out[9])
{
    int32_t sn[3], cs[3], xy, yx;
    unsigned int i;
    for (i = 0; i < 3; i++) {
        uint32_t word = pe_794c4_sincos_word(angles[i]);
        sn[i] = (int16_t)word;
        if (angles[i] < 0) sn[i] = -sn[i];
        cs[i] = (int16_t)(word >> 16);
    }
    out[5] = (int16_t)-sn[0];
    out[2] = (int16_t)pe_79754_mul(sn[1], cs[0]);
    out[8] = (int16_t)pe_79754_mul(cs[1], cs[0]);
    out[3] = (int16_t)pe_79754_mul(sn[2], cs[0]);
    out[4] = (int16_t)pe_79754_mul(cs[2], cs[0]);
    xy = pe_79754_mul(sn[1], sn[0]);
    out[0] = (int16_t)(pe_79754_mul(cs[1], cs[2]) + pe_79754_mul(xy, sn[2]));
    out[1] = (int16_t)(-pe_79754_mul(cs[1], sn[2]) + pe_79754_mul(xy, cs[2]));
    yx = pe_79754_mul(cs[1], sn[0]);
    out[7] = (int16_t)(pe_79754_mul(sn[1], sn[2]) + pe_79754_mul(yx, cs[2]));
    out[6] = (int16_t)(-pe_79754_mul(sn[1], cs[2]) + pe_79754_mul(yx, sn[2]));
}

void PE_RotMatrix79754(const int16_t angles[3], pe_addr_t out)
{
    int16_t rotation[9];
    unsigned i;
    PE_RotMatrix79754_values(angles, rotation);
    for (i = 0; i < 9; i++) PE_StoreU16(out + i * 2u, (uint16_t)rotation[i]);
}
