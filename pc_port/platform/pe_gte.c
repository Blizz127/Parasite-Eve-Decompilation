/*
 * Phase 6E-A — libgte geometry-engine control state.
 *
 * func_80077F7C = PsyQ InitGeom (asm/disc1/68478.s, handwritten asm).
 * Classification: 2/3 (SDK host implementation with deterministic constants).
 *
 * Collapsed retail effects (hardware-only, no guest-RAM state):
 *   - func_8007A134: GTE kernel patch — EnterCriticalSection, BIOS B(56h),
 *     compare/patch 6 kernel words, FlushCache, ExitCriticalSection.
 *     Host kernel needs no GTE workaround patch: no-op.
 *   - SR |= 0x40000000 (cop2 enable): host has no coprocessor SR: no-op.
 *
 * Reproduced retail effects: the seven ctc2 control-register writes.
 * cop2 registers have no guest-RAM address, so they live in g_pe_gte.
 */
#include "pe_sdk.h"

PeGteState g_pe_gte;

void func_80077F7C(void)
{
    g_pe_gte.zsf3 = 0x155;      /* ctc2 $29 */
    g_pe_gte.zsf4 = 0x100;      /* ctc2 $30 */
    g_pe_gte.h    = 0x3E8;      /* ctc2 $26 */
    g_pe_gte.dqa  = -0x1062;    /* ctc2 $27 (0xFFFFEF9E) */
    g_pe_gte.dqb  = 0x1400000;  /* ctc2 $28 */
    g_pe_gte.ofx  = 0;          /* ctc2 $24 */
    g_pe_gte.ofy  = 0;          /* ctc2 $25 */
}

/* SetGeomOffset (asm leaf): OFX = a << 16, OFY = b << 16 */
void func_80079004(int a, int b)
{
    g_pe_gte.ofx = a << 16;
    g_pe_gte.ofy = b << 16;
}

/* SetGeomScreen (asm leaf): H = a */
void func_80079024(int a)
{
    g_pe_gte.h = a;
}

/* Phase 6E-B4 — exact 32-bit GTE LZCS/LZCR arithmetic (leading-sign-bit
 * count), factored out of the func_8003EAC8 translation.  No
 * __builtin_clz(0) undefined behavior, no floating point, no g_pe_gte
 * state: LZCS/LZCR are data registers with no caller-observable lifetime
 * past the instruction sequence that uses them. */
uint32_t PE_GTE_LZCR(uint32_t v)
{
    uint32_t sign = v & 0x80000000u;
    uint32_t n = 0;
    while (n < 32u && (v & 0x80000000u) == sign) {
        n++;
        v <<= 1;
    }
    return n;
}

static void pe_gte_load_m3(int16_t m[3][3], pe_addr_t matrix)
{
    m[0][0] = (int16_t)PE_LoadU16(matrix + 0u);
    m[0][1] = (int16_t)PE_LoadU16(matrix + 2u);
    m[0][2] = (int16_t)PE_LoadU16(matrix + 4u);
    m[1][0] = (int16_t)PE_LoadU16(matrix + 6u);
    m[1][1] = (int16_t)PE_LoadU16(matrix + 8u);
    m[1][2] = (int16_t)PE_LoadU16(matrix + 10u);
    m[2][0] = (int16_t)PE_LoadU16(matrix + 12u);
    m[2][1] = (int16_t)PE_LoadU16(matrix + 14u);
    m[2][2] = (int16_t)PE_LoadU16(matrix + 16u);
}

void PE_GTE_LoadRT(pe_addr_t matrix)
{
    pe_gte_load_m3(g_pe_gte.rt, matrix);
    g_pe_gte.tr[0] = (int32_t)PE_LoadU32(matrix + 20u);
    g_pe_gte.tr[1] = (int32_t)PE_LoadU32(matrix + 24u);
    g_pe_gte.tr[2] = (int32_t)PE_LoadU32(matrix + 28u);
}

void PE_GTE_LoadRT33(pe_addr_t matrix)
{
    pe_gte_load_m3(g_pe_gte.rt, matrix);
}

void PE_GTE_LoadLCM(pe_addr_t matrix)
{
    pe_gte_load_m3(g_pe_gte.lcm, matrix);
}

void PE_GTE_LoadLLM_halfs(const int16_t *halfs)
{
    g_pe_gte.llm[0][0] = halfs[0];
    g_pe_gte.llm[0][1] = halfs[1];
    g_pe_gte.llm[0][2] = halfs[2];
    g_pe_gte.llm[1][0] = halfs[3];
    g_pe_gte.llm[1][1] = halfs[4];
    g_pe_gte.llm[1][2] = halfs[5];
    g_pe_gte.llm[2][0] = halfs[6];
    g_pe_gte.llm[2][1] = halfs[7];
    g_pe_gte.llm[2][2] = halfs[8];
}

void PE_GTE_SetIR(int16_t ir1, int16_t ir2, int16_t ir3)
{
    g_pe_gte.ir[0] = ir1;
    g_pe_gte.ir[1] = ir2;
    g_pe_gte.ir[2] = ir3;
}

void PE_GTE_SetV0(int16_t vx, int16_t vy, int16_t vz)
{
    g_pe_gte.v0[0] = vx;
    g_pe_gte.v0[1] = vy;
    g_pe_gte.v0[2] = vz;
}

void PE_GTE_SetV1(int16_t vx, int16_t vy, int16_t vz)
{
    g_pe_gte.v1[0] = vx;
    g_pe_gte.v1[1] = vy;
    g_pe_gte.v1[2] = vz;
}

void PE_GTE_SetV2(int16_t vx, int16_t vy, int16_t vz)
{
    g_pe_gte.v2[0] = vx;
    g_pe_gte.v2[1] = vy;
    g_pe_gte.v2[2] = vz;
}

void PE_GTE_SetRGBC(uint32_t rgbc)
{
    g_pe_gte.rgbc = rgbc;
}

void PE_GTE_SetBK(int32_t rbk, int32_t gbk, int32_t bbk)
{
    g_pe_gte.bk[0] = rbk;
    g_pe_gte.bk[1] = gbk;
    g_pe_gte.bk[2] = bbk;
}

static int32_t pe_gte_sat_ir(int64_t mac, int lm)
{
    if (lm) {
        if (mac < 0)
            return 0;
        if (mac > 0x7FFF)
            return 0x7FFF;
        return (int32_t)mac;
    }
    if (mac < -0x8000)
        return -0x8000;
    if (mac > 0x7FFF)
        return 0x7FFF;
    return (int32_t)mac;
}

void PE_GTE_MVMVA(uint32_t cmd)
{
    int sf = (int)((cmd >> 19) & 1u);
    int mx = (int)((cmd >> 17) & 3u);
    int vsel = (int)((cmd >> 15) & 3u);
    int cv = (int)((cmd >> 13) & 3u);
    int lm = (int)((cmd >> 10) & 1u);
    int16_t vx, vy, vz;
    int32_t tx, ty, tz;
    int row;
    int shift;

    /* Live 3A088 only issues mx=0 (RT). Other matrices are not invented. */
    if (mx != 0)
        return;

    if (vsel == 3) {
        vx = (int16_t)g_pe_gte.ir[0];
        vy = (int16_t)g_pe_gte.ir[1];
        vz = (int16_t)g_pe_gte.ir[2];
    } else if (vsel == 0) {
        vx = g_pe_gte.v0[0];
        vy = g_pe_gte.v0[1];
        vz = g_pe_gte.v0[2];
    } else {
        return;
    }

    if (cv == 0) {
        tx = g_pe_gte.tr[0];
        ty = g_pe_gte.tr[1];
        tz = g_pe_gte.tr[2];
    } else if (cv == 3) {
        tx = ty = tz = 0;
    } else {
        return;
    }

    shift = sf * 12;
    {
        const int32_t t[3] = { tx, ty, tz };
        for (row = 0; row < 3; row++) {
            int64_t mac;

            /* GTE translation is signed and scaled by 0x1000.  Multiply so
             * negative translations retain the hardware value without C's
             * undefined signed-left-shift behavior. */
            mac = (int64_t)t[row] * 4096LL
                + (int64_t)g_pe_gte.rt[row][0] * (int64_t)vx
                + (int64_t)g_pe_gte.rt[row][1] * (int64_t)vy
                + (int64_t)g_pe_gte.rt[row][2] * (int64_t)vz;
            mac >>= shift;
            g_pe_gte.mac[row] = (int32_t)mac;
            g_pe_gte.ir[row] = pe_gte_sat_ir(mac, lm);
        }
    }
}

static void pe_gte_mul3(const int16_t m[3][3], int32_t tx, int32_t ty,
                        int32_t tz, int16_t vx, int16_t vy, int16_t vz,
                        int sf, int lm)
{
    const int32_t t[3] = { tx, ty, tz };
    const int16_t v[3] = { vx, vy, vz };
    int row;
    int shift = sf * 12;

    for (row = 0; row < 3; row++) {
        int64_t mac;

        mac = (int64_t)t[row] * 4096LL
            + (int64_t)m[row][0] * (int64_t)v[0]
            + (int64_t)m[row][1] * (int64_t)v[1]
            + (int64_t)m[row][2] * (int64_t)v[2];
        mac >>= shift;
        g_pe_gte.mac[row] = (int32_t)mac;
        g_pe_gte.ir[row] = pe_gte_sat_ir(mac, lm);
    }
}

static uint8_t pe_gte_sat8(int32_t mac)
{
    int32_t x;

    if (mac < 0)
        return 0;
    x = mac >> 4; /* MAC/16 */
    if (x > 255)
        return 255;
    return (uint8_t)x;
}

void PE_GTE_NCCT(void)
{
    const int16_t *vs[3];
    uint8_t rgb[3];
    uint8_t code;
    int n;

    /* ROM word 0x118043F: sf=1 lm=1. Formula is psx-spx NCCT, not NCLIP. */
    vs[0] = g_pe_gte.v0;
    vs[1] = g_pe_gte.v1;
    vs[2] = g_pe_gte.v2;
    rgb[0] = (uint8_t)(g_pe_gte.rgbc & 0xFFu);
    rgb[1] = (uint8_t)((g_pe_gte.rgbc >> 8) & 0xFFu);
    rgb[2] = (uint8_t)((g_pe_gte.rgbc >> 16) & 0xFFu);
    code = (uint8_t)((g_pe_gte.rgbc >> 24) & 0xFFu);

    for (n = 0; n < 3; n++) {
        int row;

        pe_gte_mul3(g_pe_gte.llm, 0, 0, 0, vs[n][0], vs[n][1], vs[n][2], 1, 1);
        pe_gte_mul3(g_pe_gte.lcm, g_pe_gte.bk[0], g_pe_gte.bk[1],
                    g_pe_gte.bk[2], (int16_t)g_pe_gte.ir[0],
                    (int16_t)g_pe_gte.ir[1], (int16_t)g_pe_gte.ir[2], 1, 1);
        for (row = 0; row < 3; row++) {
            int64_t mac;

            mac = ((int64_t)rgb[row] * (int64_t)g_pe_gte.ir[row]) << 4;
            mac >>= 12;
            g_pe_gte.mac[row] = (int32_t)mac;
            g_pe_gte.ir[row] = pe_gte_sat_ir(mac, 1);
        }
        g_pe_gte.rgb_fifo[n] =
            (uint32_t)pe_gte_sat8(g_pe_gte.mac[0])
            | ((uint32_t)pe_gte_sat8(g_pe_gte.mac[1]) << 8)
            | ((uint32_t)pe_gte_sat8(g_pe_gte.mac[2]) << 16)
            | ((uint32_t)code << 24);
    }
}
