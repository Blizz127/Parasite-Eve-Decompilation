/* Original 488-word sprite renderer 800C3B04 and the three COP2 wrappers
 * it calls (79244 RTPS, 79E14 Y-compose, 78C34 ApplyMatrix). Authority:
 * B3390.s / BE50C.s and the pinned Disc1 executable. Native translation
 * only; not matching C. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

static int32_t hit_mul(int32_t a, int32_t b)
{
    return (int32_t)((uint32_t)a * (uint32_t)b);
}

int32_t func_80079244(pe_addr_t point, pe_addr_t screen, pe_addr_t ir0, pe_addr_t flags)
{
    uint32_t xy = 0, z = 0;
    PE_GTE_SetV0((int16_t)PE_LoadU16(point), (int16_t)PE_LoadU16(point + 2u),
                 (int16_t)PE_LoadU16(point + 4u));
    PE_GTE_RTPS_coordinates(&xy, &z);
    PE_StoreU32(screen, xy);
    PE_StoreU32(ir0, (uint32_t)g_pe_gte.ir0);
    PE_StoreU32(flags, 0u);
    return (int32_t)z >> 2;
}

/* SetRotMatrix: five words into GTE RT (ctc2 $0..$4). */
void func_80078E04(pe_addr_t matrix)
{
    PE_GTE_LoadRT33(matrix);
}

#define GA_D_800963E8 0x800963E8u
#define GA_D_800963EC 0x800963ECu
#define GA_D_800963DC 0x800963DCu
#define PE_MATRIX_STACK_MAX 0x280u

static void pe_gte_store_rt_tr(pe_addr_t dest)
{
    unsigned i, j;

    for (i = 0u; i < 3u; i++) {
        for (j = 0u; j < 3u; j++)
            PE_StoreU16(dest + (i * 3u + j) * 2u, (uint16_t)g_pe_gte.rt[i][j]);
    }
    /* Retail CFC2 RT33 + SW writes both halfwords, sign-extending RT33. */
    PE_StoreU32(dest + 16u, (uint32_t)(int32_t)g_pe_gte.rt[2][2]);
    PE_StoreU32(dest + 20u, (uint32_t)g_pe_gte.tr[0]);
    PE_StoreU32(dest + 24u, (uint32_t)g_pe_gte.tr[1]);
    PE_StoreU32(dest + 28u, (uint32_t)g_pe_gte.tr[2]);
}

/*
 * PushMatrix: 40 words 0x80078A94..0x80078B38. Depth D_800963E8 is a
 * byte offset into the 20-slot array at D_800963EC. sltiu 0x280 is
 * the full-stack gate; the overflow arm jals 80071A74 (printf) and
 * returns. Success is eight cfc2 words ($0..$7) then depth += 0x20.
 */
void func_80078A94(void)
{
    uint32_t depth = PE_LoadU32(GA_D_800963E8);

    if (depth >= PE_MATRIX_STACK_MAX) {
        PE_StoreU32(GA_D_800963DC, 0u);
        return;
    }
    pe_gte_store_rt_tr(GA_D_800963EC + depth);
    PE_StoreU32(GA_D_800963E8, depth + 0x20u);
}

/*
 * PopMatrix: 40 words 0x80078B38..0x80078BD8. bgtz depth; empty stack
 * jals 80071A74 and returns. Else depth -= 0x20, eight ctc2 $0..$7.
 */
void func_80078B38(void)
{
    uint32_t depth = PE_LoadU32(GA_D_800963E8);

    if ((int32_t)depth <= 0) {
        PE_StoreU32(GA_D_800963DC, 0u);
        return;
    }
    depth -= 0x20u;
    PE_StoreU32(GA_D_800963E8, depth);
    PE_GTE_LoadRT(GA_D_800963EC + depth);
}

/* 5 words 0x8006EC6C..0x8006EC80: sll 16 / sra 14 is (int16)a1 * 4. */
pe_addr_t func_8006EC6C(pe_addr_t base, int offset)
{
    int32_t delta = ((int32_t)offset) << 16;

    delta >>= 14;
    return base + PE_LoadU32(base + (uint32_t)delta);
}

/* SetTransMatrix: three words at +0x14 into GTE TR (ctc2 $5..$7). */
void func_80078E94(pe_addr_t matrix)
{
    g_pe_gte.tr[0] = (int32_t)PE_LoadU32(matrix + 20u);
    g_pe_gte.tr[1] = (int32_t)PE_LoadU32(matrix + 24u);
    g_pe_gte.tr[2] = (int32_t)PE_LoadU32(matrix + 28u);
}

int32_t func_80079274(pe_addr_t v0, pe_addr_t v1, pe_addr_t v2, pe_addr_t sxy0,
                      pe_addr_t sxy1, pe_addr_t sxy2, pe_addr_t ir0, pe_addr_t flags)
{
    uint32_t xy[3] = {0, 0, 0};
    uint32_t z[3] = {0, 0, 0};

    PE_GTE_SetV0((int16_t)PE_LoadU16(v0), (int16_t)PE_LoadU16(v0 + 2u),
                 (int16_t)PE_LoadU16(v0 + 4u));
    PE_GTE_SetV1((int16_t)PE_LoadU16(v1), (int16_t)PE_LoadU16(v1 + 2u),
                 (int16_t)PE_LoadU16(v1 + 4u));
    PE_GTE_SetV2((int16_t)PE_LoadU16(v2), (int16_t)PE_LoadU16(v2 + 2u),
                 (int16_t)PE_LoadU16(v2 + 4u));
    PE_GTE_RTPT_coordinates(xy, z);
    PE_StoreU32(sxy0, xy[0]);
    PE_StoreU32(sxy1, xy[1]);
    PE_StoreU32(sxy2, xy[2]);
    PE_StoreU32(ir0, (uint32_t)g_pe_gte.ir0);
    PE_StoreU32(flags, 0u);
    return (int32_t)z[2] >> 2;
}

void func_80079E14(int32_t angle, pe_addr_t dest)
{
    uint32_t word;
    int32_t cosine, sine, t7 = angle;
    int16_t m00, m01, m02, m10, m11, m12;
    if (t7 >= 0) {
        word = PE_LoadU32(0x800966ECu + ((unsigned)t7 & 0xFFFu) * 4u);
        sine = (int16_t)word;
        cosine = (int16_t)(word >> 16);
    } else {
        t7 = -t7;
        word = PE_LoadU32(0x800966ECu + ((unsigned)t7 & 0xFFFu) * 4u);
        sine = -(int16_t)word;
        cosine = (int16_t)(word >> 16);
    }
    m00 = (int16_t)PE_LoadU16(dest);
    m01 = (int16_t)PE_LoadU16(dest + 2u);
    m02 = (int16_t)PE_LoadU16(dest + 4u);
    m10 = (int16_t)PE_LoadU16(dest + 6u);
    m11 = (int16_t)PE_LoadU16(dest + 8u);
    m12 = (int16_t)PE_LoadU16(dest + 10u);
    PE_StoreU16(dest, (uint16_t)((hit_mul(cosine, m00) - hit_mul(sine, m10)) >> 12));
    PE_StoreU16(dest + 2u, (uint16_t)((hit_mul(cosine, m01) - hit_mul(sine, m11)) >> 12));
    PE_StoreU16(dest + 4u, (uint16_t)((hit_mul(cosine, m02) - hit_mul(sine, m12)) >> 12));
    PE_StoreU16(dest + 6u, (uint16_t)((hit_mul(sine, m00) + hit_mul(cosine, m10)) >> 12));
    PE_StoreU16(dest + 8u, (uint16_t)((hit_mul(sine, m01) + hit_mul(cosine, m11)) >> 12));
    PE_StoreU16(dest + 10u, (uint16_t)((hit_mul(sine, m02) + hit_mul(cosine, m12)) >> 12));
}

pe_addr_t func_80078C34(pe_addr_t matrix, pe_addr_t vector, pe_addr_t out)
{
    PE_GTE_LoadRT33(matrix);
    PE_GTE_SetV0((int16_t)PE_LoadU16(vector), (int16_t)PE_LoadU16(vector + 2u),
                 (int16_t)PE_LoadU16(vector + 4u));
    PE_GTE_MVMVA(0x486012u);
    PE_StoreU16(out, (uint16_t)g_pe_gte.ir[0]);
    PE_StoreU16(out + 2u, (uint16_t)g_pe_gte.ir[1]);
    PE_StoreU16(out + 4u, (uint16_t)g_pe_gte.ir[2]);
    return out;
}

static uint32_t hit_scale_channel(uint8_t channel, uint16_t brightness)
{
    uint32_t product = (uint32_t)channel * (uint32_t)brightness;
    if (0x7FFFu < product) product = 0x7FFFu;
    return product >> 7;
}

void func_800C3B04(pe_addr_t style)
{
    pe_addr_t scratch = 0x1F800000u, packet, ot;
    int32_t width, height, half_w, half_h, depth, bias;
    uint32_t bank, scale, denom, product;
    unsigned i;
    PE_StoreU32(0x800E284Cu, scratch);
    PE_StoreU8(scratch + 0x11u, (uint8_t)(PE_LoadU8(style + 0x24u) & 0xF0u));
    PE_StoreU8(scratch + 0x10u, (uint8_t)((uint32_t)(PE_LoadU8(style + 0x24u) - PE_LoadU8(scratch + 0x11u)) << 4u));
    PE_StoreU8(scratch + 0x12u, (uint8_t)(PE_LoadU8(style + 0x25u) << 4u));
    PE_StoreU8(scratch + 0x13u, (uint8_t)(PE_LoadU8(style + 0x25u) >> 4u));
    width = (int32_t)((uint32_t)PE_LoadU8(0x800F345Cu) * PE_LoadU32(style + 0x10u)) >> 12;
    height = (int32_t)((uint32_t)PE_LoadU8(0x800F345Du) * PE_LoadU32(style + 0x14u)) >> 12;
    if (PE_LoadU16(style + 0x28u) != 0x80u) {
        uint32_t last = 0;
        for (i = 0; i < 3; i++) {
            last = hit_scale_channel(PE_LoadU8(style + 0x20u + i), PE_LoadU16(style + 0x28u));
            PE_StoreU8(scratch + 0x80u + i, (uint8_t)last);
        }
        PE_StoreU32(scratch + 0x18u, last);
    } else {
        for (i = 0; i < 3; i++) PE_StoreU8(scratch + 0x80u + i, PE_LoadU8(style + 0x20u + i));
    }
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    depth = func_80079244(style, scratch + 8u, scratch, scratch + 4u);
    PE_StoreU32(scratch + 0xCu, (uint32_t)depth);
    PE_StoreU16(scratch + 0x1Cu, PE_LoadU16(scratch + 8u));
    PE_StoreU16(scratch + 0x1Eu, PE_LoadU16(scratch + 0xAu));
    scale = PE_LoadU32(PE_LoadU32(0x800BCFA8u));
    denom = ((uint32_t)depth << 2u) + scale;
    if (!denom) denom = 1u;
    half_w = (int32_t)(((uint32_t)width << 5u) * scale / denom) >> 1;
    half_h = (int32_t)(((uint32_t)height << 5u) * scale / denom) >> 1;
    bank = PE_LoadU32(0x8009CDDCu);
    packet = PE_LoadU32(0x800B0E58u + bank * 4u) + PE_LoadU32(0x8009CDD8u);
    PE_StoreU8(packet + 3u, 9u);
    PE_StoreU8(packet + 7u, PE_LoadU8(0x800F337Au) ? 0x2Eu : 0x2Cu);
    if ((int16_t)PE_LoadU16(style + 0xCu)) {
        pe_addr_t matrix = scratch + 0x84u;
        PE_StoreU16(scratch + 0x40u, (uint16_t)(0 - half_w));
        PE_StoreU16(scratch + 0x42u, (uint16_t)(0 - half_h));
        PE_StoreU16(scratch + 0x48u, (uint16_t)half_w);
        PE_StoreU16(scratch + 0x4Au, (uint16_t)(0 - half_h));
        PE_StoreU16(scratch + 0x50u, (uint16_t)(0 - half_w));
        PE_StoreU16(scratch + 0x52u, (uint16_t)half_h);
        PE_StoreU16(scratch + 0x58u, (uint16_t)half_w);
        PE_StoreU16(scratch + 0x5Au, (uint16_t)half_h);
        PE_StoreU16(matrix, 0x1000u);
        PE_StoreU16(matrix + 8u, 0x1000u);
        PE_StoreU16(matrix + 16u, 0x1000u);
        PE_StoreU32(matrix + 20u, 0u);
        PE_StoreU32(matrix + 24u, 0u);
        PE_StoreU32(matrix + 28u, 0u);
        PE_StoreU16(matrix + 2u, 0u);
        PE_StoreU16(matrix + 4u, 0u);
        PE_StoreU16(matrix + 6u, 0u);
        PE_StoreU16(matrix + 10u, 0u);
        PE_StoreU16(matrix + 12u, 0u);
        PE_StoreU16(matrix + 14u, 0u);
        func_80079E14((int32_t)(int16_t)PE_LoadU16(style + 0xCu), matrix);
        func_80078C34(matrix, scratch + 0x40u, scratch + 0x60u);
        func_80078C34(matrix, scratch + 0x48u, scratch + 0x68u);
        func_80078C34(matrix, scratch + 0x50u, scratch + 0x70u);
        func_80078C34(matrix, scratch + 0x58u, scratch + 0x78u);
        for (i = 0; i < 4; i++) {
            PE_StoreU16(packet + 8u + i * 8u,
                        (uint16_t)(PE_LoadU16(scratch + 0x1Cu) + PE_LoadU16(scratch + 0x60u + i * 8u)));
            PE_StoreU16(packet + 10u + i * 8u,
                        (uint16_t)(PE_LoadU16(scratch + 0x1Eu) + PE_LoadU16(scratch + 0x62u + i * 8u)));
        }
    } else {
        PE_StoreU16(packet + 8u, (uint16_t)(PE_LoadU16(scratch + 0x1Cu) - half_w));
        PE_StoreU16(packet + 10u, (uint16_t)(PE_LoadU16(scratch + 0x1Eu) - half_h));
        PE_StoreU16(packet + 16u, (uint16_t)(PE_LoadU16(scratch + 0x1Cu) + half_w));
        PE_StoreU16(packet + 18u, (uint16_t)(PE_LoadU16(scratch + 0x1Eu) - half_h));
        PE_StoreU16(packet + 24u, (uint16_t)(PE_LoadU16(scratch + 0x1Cu) - half_w));
        PE_StoreU16(packet + 26u, (uint16_t)(PE_LoadU16(scratch + 0x1Eu) + half_h));
        PE_StoreU16(packet + 32u, (uint16_t)(PE_LoadU16(scratch + 0x1Cu) + half_w));
        PE_StoreU16(packet + 34u, (uint16_t)(PE_LoadU16(scratch + 0x1Eu) + half_h));
    }
    PE_StoreU8(packet + 4u, PE_LoadU8(scratch + 0x80u));
    PE_StoreU8(packet + 5u, PE_LoadU8(scratch + 0x81u));
    PE_StoreU8(packet + 6u, PE_LoadU8(scratch + 0x82u));
    PE_StoreU16(packet + 22u, PE_LoadU16(0x800E27ACu));
    PE_StoreU16(packet + 14u, (uint16_t)func_80077AA4((int32_t)(PE_LoadU16(0x800F341Cu) + PE_LoadU8(scratch + 0x12u)),
                                                      PE_LoadU16(0x800F341Eu) + PE_LoadU8(scratch + 0x13u)));
    PE_StoreU8(packet + 12u, (uint8_t)(PE_LoadU8(scratch + 0x10u) + 1u));
    PE_StoreU8(packet + 13u, (uint8_t)(PE_LoadU8(scratch + 0x11u) + 1u));
    PE_StoreU8(packet + 20u, (uint8_t)(PE_LoadU8(scratch + 0x10u) + PE_LoadU8(0x800F345Cu)));
    PE_StoreU8(packet + 21u, (uint8_t)(PE_LoadU8(scratch + 0x11u) + 1u));
    PE_StoreU8(packet + 28u, (uint8_t)(PE_LoadU8(scratch + 0x10u) + 1u));
    PE_StoreU8(packet + 29u, (uint8_t)(PE_LoadU8(scratch + 0x11u) + PE_LoadU8(0x800F345Du)));
    PE_StoreU8(packet + 36u, (uint8_t)(PE_LoadU8(scratch + 0x10u) + PE_LoadU8(0x800F345Cu)));
    PE_StoreU8(packet + 37u, (uint8_t)(PE_LoadU8(scratch + 0x11u) + PE_LoadU8(0x800F345Du)));
    bias = (int16_t)PE_LoadU16(style + 0x26u);
    ot = PE_LoadU32(0x800B0E38u + bank * 4u) + (uint32_t)(depth + bias) * 4u;
    product = (PE_LoadU32(packet) & 0xFF000000u) | (PE_LoadU32(ot) & 0xFFFFFFu);
    PE_StoreU32(packet, product);
    PE_StoreU32(0x8009CDD8u, PE_LoadU32(0x8009CDD8u) + 0x28u);
    PE_StoreU32(ot, (PE_LoadU32(ot) & 0xFF000000u) | (packet & 0xFFFFFFu));
}

void func_800CDD0C(pe_addr_t slot, pe_addr_t rec, pe_addr_t data)
{
    unsigned i;
    uint32_t scale;
    (void)slot;
    (void)rec;
    func_800C2EAC(3u);
    func_800C3098(256);
    func_800C2FF0(32u, 32u);
    func_800C3238(2u);
    PE_StoreU16(0x800E27B0u, (uint16_t)(PE_LoadU16(data + 4u) + PE_LoadU16(data + 10u)));
    PE_StoreU16(0x800E27B2u, (uint16_t)(PE_LoadU16(data + 6u) + PE_LoadU16(data + 12u)));
    PE_StoreU16(0x800E27B4u, (uint16_t)(PE_LoadU16(data + 8u) + PE_LoadU16(data + 14u)));
    scale = (uint32_t)((int32_t)(int8_t)PE_LoadU8(data + 3u) << 3) + 0x20Cu;
    PE_StoreU32(0x800E27C0u, scale);
    PE_StoreU32(0x800E27C4u, scale);
    PE_StoreU32(0x800E27C8u, scale);
    PE_StoreU16(0x800E27D8u, (uint16_t)(int16_t)(int8_t)PE_LoadU8(data + 1u));
    func_800C3B04(0x800E27B0u);
    func_800C3098(16);
    for (i = 0; i < 8; i++) {
        PE_StoreU16(0x800E2770u, (uint16_t)(PE_LoadU16(data + 16u + i * 2u) + PE_LoadU16(data + 10u)));
        PE_StoreU16(0x800E2772u, (uint16_t)(PE_LoadU16(data + 32u + i * 2u) + PE_LoadU16(data + 12u)));
        PE_StoreU16(0x800E2774u, (uint16_t)(PE_LoadU16(data + 48u + i * 2u) + PE_LoadU16(data + 14u)));
        PE_StoreU16(0x800E2798u, (uint16_t)(int16_t)(int8_t)PE_LoadU8(data + 1u));
        func_800C3B04(0x800E2770u);
    }
}

void func_800CDE90(pe_addr_t slot, pe_addr_t rec, pe_addr_t data)
{
    (void)slot;
    (void)rec;
    func_800C2EAC(3u);
    func_800C3098(256);
    func_800C2FF0(32u, 32u);
    func_800C3238(3u);
    PE_StoreU16(0x800F33E8u, (uint16_t)(PE_LoadU16(data + 4u) + PE_LoadU16(data + 10u)));
    PE_StoreU16(0x800F33EAu, (uint16_t)(PE_LoadU16(data + 6u) + PE_LoadU16(data + 12u)));
    PE_StoreU16(0x800F33ECu, (uint16_t)(PE_LoadU16(data + 8u) + PE_LoadU16(data + 14u)));
    PE_StoreU16(0x800F3410u, (uint16_t)(int16_t)(int8_t)PE_LoadU8(data + 1u));
    func_800C3B04(0x800F33E8u);
}
