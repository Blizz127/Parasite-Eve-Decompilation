/*
 * PE-CH2 — func_80066800: opcode 0x82 52-byte view-record apply.
 *
 * Complete retail body (99 words / 0x18C, exe 0x80066800–0x8006698C,
 * file offset 0x57000).  Callers are opcode handler func_80018E58
 * (jal 0x80018E6C) and func_800677FC (jal 0x80067834).
 *
 * Record = *(D_800B1624) + offset_at_+0x1C + index*52.
 * Retail publishes H through *D_800BCFA8 and SetGeomScreen, copies the
 * nine rotation halfwords and three translation words through
 * *D_800BCFA4, records the byte index, and sets bit 0x80 in D_800BCF88.
 * Bytes at record+0x20 and beyond are not read.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800B1624 0x800B1624u
#define GA_D_800BCF88 0x800BCF88u
#define GA_D_800BCFA4 0x800BCFA4u
#define GA_D_800BCFA8 0x800BCFA8u
#define GA_D_800BCFFD 0x800BCFFDu

int func_80066800(unsigned int index)
{
    pe_addr_t container = PE_LoadU32(GA_D_800B1624);
    pe_addr_t record = container + PE_LoadU32(container + 0x1Cu)
                     + index * 52u;
    pe_addr_t h_dest = PE_LoadU32(GA_D_800BCFA8);
    uint16_t h;
    unsigned int i;

    h = PE_LoadU16(record);
    PE_StoreU32(h_dest, h);
    func_80079024((int)h);

    for (i = 0; i < 9u; i++) {
        pe_addr_t matrix = PE_LoadU32(GA_D_800BCFA4);
        PE_StoreU16(matrix + i * 2u, PE_LoadU16(record + 2u + i * 2u));
    }
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x14u,
                PE_LoadU32(record + 0x14u));
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x18u,
                PE_LoadU32(record + 0x18u));
    PE_StoreU32(PE_LoadU32(GA_D_800BCFA4) + 0x1Cu,
                PE_LoadU32(record + 0x1Cu));

    PE_StoreU8(GA_D_800BCFFD, (uint8_t)index);
    PE_StoreU32(GA_D_800BCF88, PE_LoadU32(GA_D_800BCF88) | 0x80u);
    return 0;
}

/*
 * PE-BTL64 — func_800661EC camera-request leaf.
 *
 * 31 words 0x800661EC..0x80066268, SHA-256
 * 095ed0474107c1b7124d63d5dc36c7fbc97f82ac409ee06746741734bc0bba28.
 * Zero jal. Callers 17C54 (0x03, a3=0) and 17C8C (0x46, a3=8).
 * If BCF88 bit 0x40 is clear, return -19 with no stores.
 * Else BCFA0=1, BCF9C/9E/A2=a0/a1/a2, BCF98=*BCF8C,
 * BCF88 = (flags & 0xFFF0) | (a3==8 ? 1 : 9), return 0.
 */
int func_800661EC(int a0, int a1, unsigned int a2, unsigned int a3)
{
    uint32_t flags;
    uint32_t low;

    flags = PE_LoadU32(GA_D_800BCF88);
    if ((flags & 0x40u) == 0u)
        return -19;
    PE_StoreU16(0x800BCFA0u, 1u);
    PE_StoreU16(0x800BCF9Cu, (uint16_t)a0);
    PE_StoreU16(0x800BCF9Eu, (uint16_t)a1);
    PE_StoreU16(0x800BCFA2u, (uint16_t)a2);
    PE_StoreU32(0x800BCF98u, PE_LoadU32(0x800BCF8Cu));
    low = (a3 == 8u) ? 1u : 9u;
    PE_StoreU32(GA_D_800BCF88, (flags & 0xFFF0u) | low);
    return 0;
}

#define GA_RSIN_589C  0x8009589Cu
#define GA_RSIN_509C  0x8009509Cu
#define GA_RSIN_489C  0x8009489Cu
#define GA_RSIN_409C  0x8009409Cu
#define GA_D_800BD000 0x800BD000u
#define GA_D_800BD020 0x800BD020u
#define GA_D_800BD022 0x800BD022u
#define GA_D_800BD028 0x800BD028u
#define GA_D_800BE9A0 0x800BE9A0u
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D2E8 0x8009D2E8u
#define GA_D_800BCF8C 0x800BCF8Cu
#define GA_D_800BCF8E 0x800BCF8Eu
#define GA_D_800BCF90 0x800BCF90u
#define GA_D_800BCF92 0x800BCF92u

static int32_t pe_rsin_77d30(uint32_t a0)
{
    if (a0 < 0x401u)
        return (int16_t)PE_LoadU16(GA_RSIN_589C + a0 * 2u);
    if (a0 < 0x801u)
        return (int16_t)PE_LoadU16(GA_RSIN_589C + (0x800u - a0) * 2u);
    if (a0 < 0xC01u)
        return -(int16_t)PE_LoadU16(GA_RSIN_589C + (0x1000u - a0) * 2u);
    return -(int16_t)PE_LoadU16(GA_RSIN_489C + a0 * 2u);
}

int32_t func_80077CF4(int32_t angle)
{
    int32_t s;

    if (angle < 0) {
        s = pe_rsin_77d30((uint32_t)(-angle) & 0xFFFu);
        return -s;
    }
    return pe_rsin_77d30((uint32_t)angle & 0xFFFu);
}

int32_t func_80077DC4(int32_t angle)
{
    uint32_t a0;

    if (angle < 0)
        angle = -angle;
    a0 = (uint32_t)angle & 0xFFFu;
    if (a0 < 0x401u)
        return (int16_t)PE_LoadU16(GA_RSIN_589C + (0x400u - a0) * 2u);
    if (a0 < 0x801u)
        return -(int16_t)PE_LoadU16(GA_RSIN_509C + a0 * 2u);
    if (a0 < 0xC01u)
        return (int16_t)PE_LoadU16(GA_RSIN_409C + a0 * 2u);
    return -(int16_t)PE_LoadU16(GA_RSIN_589C + (0xC00u - a0) * 2u);
}

/*
 * PE-BTL69 — func_80066CE8 walk heading matrix.
 *
 * 158 words 0x80066CE8..0x80066F60. Sole TEXT jal from 68CE0
 * @ 68CE8; 68CE0 is 3F3C4 @ 3F560 (same gate as 37870).
 * Digital yaw is BD020; analog BD022 is not this cut.
 * rsin/rcos build Ry; GTE column-extract writes BD000 and
 * zeros BD014/18/1C. 67A78/67B74/67D18 are not this cut.
 */
void func_80066CE8(void)
{
    uint16_t yaw;
    int32_t s;
    int32_t c;
    pe_addr_t rec;

    if ((PE_LoadU16(GA_D_800BE9A0) & 0xF000u) == 0x7000u)
        yaw = PE_LoadU16(GA_D_800BD022);
    else
        yaw = PE_LoadU16(GA_D_800BD020);
    rec = PE_LoadU32(GA_D_8009D254);
    if (rec != 0u && (PE_LoadU32(GA_D_8009D2E8) & 0x10u) != 0u) {
        rec = PE_LoadU32(rec);
        yaw = (uint16_t)(yaw + 0x800u);
        if (rec != 0u)
            yaw = (uint16_t)(yaw
                             + ((PE_LoadU32(rec + 0x4Cu) >> 7) & 0xC00u));
    }
    yaw &= 0xFFFu;
    c = func_80077DC4((int32_t)yaw);
    s = func_80077CF4((int32_t)yaw);
    PE_StoreU16(GA_D_800BD000, (uint16_t)c);
    PE_StoreU16(GA_D_800BD000 + 2u, 0u);
    PE_StoreU16(GA_D_800BD000 + 4u, (uint16_t)s);
    PE_StoreU16(GA_D_800BD000 + 6u, 0u);
    PE_StoreU16(GA_D_800BD000 + 8u, 0x1000u);
    PE_StoreU16(GA_D_800BD000 + 10u, 0u);
    PE_StoreU16(GA_D_800BD000 + 12u, (uint16_t)-s);
    PE_StoreU16(GA_D_800BD000 + 14u, 0u);
    PE_StoreU16(GA_D_800BD000 + 16u, (uint16_t)c);
    PE_StoreU32(GA_D_800BD000 + 0x14u, 0u);
    PE_StoreU32(GA_D_800BD000 + 0x18u, 0u);
    PE_StoreU32(GA_D_800BD000 + 0x1Cu, 0u);
}

extern unsigned int D_8009D1A0;

/*
 * PE-BTL74 — 65674 / 68CE0.
 *
 * 65674 — 166 words 0x80065674..0x8006590C, SHA-256
 * 9b325f33… . Zero jal. First lw is D1A0; bne skips the
 * body. Live 3E974 leaves D1A0|=0x4000, so this cut is
 * the early-out. The B1624 walk is not this cut.
 *
 * 68CE0 — 18 words 0x80068CE0..0x80068D28, SHA-256 below.
 * 3F3C4 @ 3F560. jal 66CE8, 65674, 67E1C, 67A78, 67B74,
 * 67D18; v0=0.
 */
void func_80065674(void)
{
    if (D_8009D1A0 != 0u)
        return;
}

/*
 * PE-BTL75 — func_80067E1C camera-slot interpolate.
 *
 * 126 words 0x80067E1C..0x80068014, SHA-256
 * 99f90f7f78d13d6ac4ff441b07eaf674192eee1c5a94f7d743107aae93c83d6f.
 * Zero jal. Sole TEXT jal from 68CE0 @ 68CF8.
 * Runs when (D1A0&0x104)==0 (live 0x4000 passes).
 * Walks B1624 +0x14 records, stride 56, count at +6.
 * Bit 4: 8.8 step + remainder into +0xC/+0x20.
 * Bit 8: pull toward BCF8C vs BD028.
 * If BCF88&0x80: clear that bit and copy BCF8C/8E to BCF90/92.
 * Unpublished B1624==0 skips the walk (host; retail 6B4F8
 * publishes before 68CE0). 67A78 is the next 68CE0 tail.
 */
static int32_t pe_mips_div_hi(int32_t num, int32_t den)
{
    if (den == 0)
        return 0;
    if (den == -1 && num == (int32_t)0x80000000)
        return 0;
    return num % den;
}

void func_80067E1C(void)
{
    pe_addr_t container;
    pe_addr_t rec;
    uint16_t count;
    uint16_t i;
    uint32_t flags;

    if ((D_8009D1A0 & 0x104u) != 0u)
        return;
    container = PE_LoadU32(GA_D_800B1624);
    if (container == 0u)
        goto snapshot;
    count = PE_LoadU16(container + 6u);
    rec = container + PE_LoadU32(container + 0x14u);
    for (i = 0; i < count; i++, rec += 56u) {
        if ((PE_LoadU8(rec) & 4u) != 0u) {
            int32_t acc;
            int32_t den;

            acc = (((int32_t)(int16_t)PE_LoadU16(rec + 0xCu) << 8)
                   | (int32_t)PE_LoadU8(rec + 0x20u))
                  + (int32_t)(int16_t)PE_LoadU16(rec + 0x1Cu);
            den = (int32_t)PE_LoadU16(rec + 4u);
            PE_StoreU16(rec + 0x20u, (uint16_t)(acc & 0xFF));
            PE_StoreU16(rec + 0xCu,
                        (uint16_t)pe_mips_div_hi(acc >> 8, den));
            acc = (((int32_t)(int16_t)PE_LoadU16(rec + 0xEu) << 8)
                   | (int32_t)PE_LoadU8(rec + 0x22u))
                  + (int32_t)(int16_t)PE_LoadU16(rec + 0x1Eu);
            den = (int32_t)PE_LoadU16(rec + 6u);
            PE_StoreU16(rec + 0x22u, (uint16_t)(acc & 0xFF));
            PE_StoreU16(rec + 0xEu,
                        (uint16_t)pe_mips_div_hi(acc >> 8, den));
        }
        if ((PE_LoadU8(rec) & 8u) != 0u) {
            int32_t acc;
            int32_t delta;

            delta = (int32_t)(int16_t)PE_LoadU16(GA_D_800BCF8C)
                    - (int32_t)(int16_t)PE_LoadU16(GA_D_800BD028);
            acc = ((int32_t)(int16_t)PE_LoadU16(rec + 8u) << 8)
                  + delta * (int32_t)(int16_t)PE_LoadU16(rec + 0x1Cu);
            PE_StoreU16(rec + 0xCu, (uint16_t)(acc >> 8));
            PE_StoreU16(rec + 0x20u, (uint16_t)(acc & 0xFF));
            delta = (int32_t)(int16_t)PE_LoadU16(GA_D_800BCF8E)
                    - (int32_t)(int16_t)PE_LoadU16(GA_D_800BD028 + 2u);
            acc = ((int32_t)(int16_t)PE_LoadU16(rec + 0xAu) << 8)
                  + delta * (int32_t)(int16_t)PE_LoadU16(rec + 0x1Eu);
            PE_StoreU16(rec + 0xEu, (uint16_t)(acc >> 8));
            PE_StoreU16(rec + 0x22u, (uint16_t)(acc & 0xFF));
        }
    }
snapshot:
    flags = PE_LoadU32(GA_D_800BCF88);
    if ((flags & 0x80u) != 0u) {
        PE_StoreU32(GA_D_800BCF88, flags & ~0x80u);
        PE_StoreU16(GA_D_800BCF90, PE_LoadU16(GA_D_800BCF8C));
        PE_StoreU16(GA_D_800BCF92, PE_LoadU16(GA_D_800BCF8E));
    }
}

void func_80068CE0(void)
{
    func_80066CE8();
    func_80065674();
    func_80067E1C();
}
