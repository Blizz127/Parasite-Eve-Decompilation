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
#include "game_port.h"

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
 * __builtin_clz(0) undefined behavior or floating point. This arithmetic
 * helper is pure; SetLZCS below also retains the register side effects. */
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

void PE_GTE_SetLZCS(uint32_t v)
{
    g_pe_gte.lzcs = v;
    g_pe_gte.lzcr = PE_GTE_LZCR(v);
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

/* Original transition-init SDK leaves: 78E34/78E64 ctc2 matrix words,
 * 78FC4/78FE4 background/far colors, and 77E64 SetFogNearFar.
 * Far-color storage alone does not implement missing depth-cue commands. */
void func_80078E34(pe_addr_t matrix) { pe_gte_load_m3(g_pe_gte.llm, matrix); }
void func_80078E64(pe_addr_t matrix) { pe_gte_load_m3(g_pe_gte.lcm, matrix); }

void func_80078FC4(uint32_t r, uint32_t g, uint32_t b)
{
    g_pe_gte.bk[0]=(int32_t)(r<<4);g_pe_gte.bk[1]=(int32_t)(g<<4);g_pe_gte.bk[2]=(int32_t)(b<<4);
}

void func_80078FE4(uint32_t r, uint32_t g, uint32_t b)
{
    g_pe_gte.fc[0]=(int32_t)(r<<4);g_pe_gte.fc[1]=(int32_t)(g<<4);g_pe_gte.fc[2]=(int32_t)(b<<4);
}

void func_80077E64(uint32_t near_z, uint32_t far_z, int32_t h)
{
    int32_t span=(int32_t)(far_z-near_z);
    if (span<100) return;
    /* Each original shift/product uses only the low 32 bits. */
    int32_t first=(int32_t)((0u-near_z)*far_z)/span;
    int32_t offset=(int32_t)(far_z<<12)/span;
    int32_t numerator=(int32_t)((uint32_t)first<<8);
    if (!h || (numerator==INT32_MIN && h==-1)) {
        /* Original BREAK 7 / BREAK 6, before either coefficient write. */
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    int32_t slope=numerator/h;
    if (slope<-32768) slope=-32768;
    if (slope>32767) slope=32767;
    g_pe_gte.dqa=slope; /* original 78FAC: ctc2 $27 */
    g_pe_gte.dqb=(int32_t)((uint32_t)offset<<12); /* 78FB8: ctc2 $28 */
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

static int64_t pe_gte_wrap44(int64_t value);

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

        /* GTE accumulates with signed 44-bit wrapping after each sum.
         * IR saturation uses the signed 32-bit post-shift MAC value. */
        mac = (int64_t)t[row] * 4096LL;
        for (int col = 0; col < 3; ++col)
            mac = pe_gte_wrap44(mac + (int64_t)m[row][col] * v[col]);
        mac >>= shift;
        g_pe_gte.mac[row] = (int32_t)mac;
        g_pe_gte.ir[row] = pe_gte_sat_ir(g_pe_gte.mac[row], lm);
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

/* RTPS/RTPT coordinate outputs (sf=1,lm=0).
 * Authority: psx-spx GTE coordinate commands / UNR division. This view
 * exposes SXY/SZ FIFOs, MAC0, IR0 and the accumulated projection FLAG. */
static uint32_t pe_gte_unr(uint32_t h, uint32_t sz)
{
    uint32_t shift = 0u, d, n, index, u;
    int table;
    uint64_t result;
    if (h >= sz * 2u) return 0x1FFFFu;
    while ((sz << shift) < 0x8000u) shift++;
    n = h << shift;
    d = sz << shift;
    index = (d - 0x7FC0u) >> 7;
    table = (int)((0x40000u / (index + 0x100u) + 1u) / 2u) - 0x101;
    u = (uint32_t)(table < 0 ? 0 : table) + 0x101u;
    d = (0x2000080u - d * u) >> 8;
    d = (0x80u + d * u) >> 8;
    result = ((uint64_t)n * d + 0x8000u) >> 16;
    return result > 0x1FFFFu ? 0x1FFFFu : (uint32_t)result;
}

static int64_t pe_gte_wrap44(int64_t value)
{
    uint64_t bits = (uint64_t)value & UINT64_C(0xFFFFFFFFFFF);
    return (bits & UINT64_C(0x80000000000))
        ? (int64_t)bits - INT64_C(0x100000000000) : (int64_t)bits;
}

/* GPF/GPL hardware arithmetic; see psx-spx GTE interpolation commands.
 * GPL restores the accumulator's pre-shift scale before its 44-bit sum. */
static void pe_gte_interpolate(int accumulate, int sf, int lm)
{
    unsigned i;
    uint32_t color = g_pe_gte.rgbc & 0xFF000000u;
    int shift = sf ? 12 : 0;
    for (i = 0; i < 3; i++) {
        int64_t base = accumulate ? (int64_t)g_pe_gte.mac[i] * (1 << shift) : 0;
        int64_t value = pe_gte_wrap44(base +
            (int64_t)(int16_t)g_pe_gte.ir[i] * (int16_t)g_pe_gte.ir0);
        g_pe_gte.mac[i] = (int32_t)(value >> shift);
        g_pe_gte.ir[i] = pe_gte_sat_ir(g_pe_gte.mac[i], lm);
        color |= (uint32_t)pe_gte_sat8(g_pe_gte.mac[i]) << (i * 8u);
    }
    g_pe_gte.rgb_fifo[0] = g_pe_gte.rgb_fifo[1];
    g_pe_gte.rgb_fifo[1] = g_pe_gte.rgb_fifo[2];
    g_pe_gte.rgb_fifo[2] = color;
}

void PE_GTE_GPF(int sf, int lm) { pe_gte_interpolate(0, sf, lm); }
void PE_GTE_GPL(int sf, int lm) { pe_gte_interpolate(1, sf, lm); }

void PE_GTE_OP(int sf, int lm)
{
    int16_t ir[3]={(int16_t)g_pe_gte.ir[0],(int16_t)g_pe_gte.ir[1],(int16_t)g_pe_gte.ir[2]};
    unsigned i;
    for (i=0;i<3;i++) {
        unsigned a=(i+1u)%3u,b=(i+2u)%3u;
        int64_t value=(int64_t)ir[b]*g_pe_gte.rt[a][a]-(int64_t)ir[a]*g_pe_gte.rt[b][b];
        g_pe_gte.mac[i]=(int32_t)(value>>(sf?12:0));
        g_pe_gte.ir[i]=pe_gte_sat_ir(g_pe_gte.mac[i],lm);
    }
}

static void pe_gte_mac0_flags(int64_t value, uint32_t *flags)
{
    if (value > INT32_MAX) *flags |= 1u << 16;
    if (value < INT32_MIN) *flags |= 1u << 15;
}

static void pe_gte_project_vertex(const int16_t vertex[3], uint32_t *xy, uint32_t *z,
                                  uint32_t *flags, int last)
{
    unsigned int row, col;
    uint32_t ratio;
    int64_t sx, sy, cue;
    for (row = 0; row < 3u; row++) {
        int64_t sum = (int64_t)g_pe_gte.tr[row] * 4096;
        for (col = 0; col < 3u; col++) {
            sum += (int64_t)g_pe_gte.rt[row][col] * vertex[col];
            if (sum >= INT64_C(0x80000000000)) *flags |= 1u << (30-row);
            if (sum < -INT64_C(0x80000000000)) *flags |= 1u << (27-row);
            sum = pe_gte_wrap44(sum);
        }
        g_pe_gte.mac[row] = (int32_t)(sum >> 12);
        g_pe_gte.ir[row] = pe_gte_sat_ir(g_pe_gte.mac[row], 0);
        if (g_pe_gte.mac[row] < -32768 || g_pe_gte.mac[row] > 32767) *flags |= 1u << (24-row);
    }
    *z = g_pe_gte.mac[2] < 0 ? 0u : g_pe_gte.mac[2] > 65535 ? 65535u : (uint32_t)g_pe_gte.mac[2];
    if (g_pe_gte.mac[2] < 0 || g_pe_gte.mac[2] > 65535) *flags |= 1u << 18;
    if ((uint16_t)g_pe_gte.h >= *z * 2u) *flags |= 1u << 17;
    ratio = pe_gte_unr((uint16_t)g_pe_gte.h, *z);
    pe_gte_mac0_flags((int64_t)ratio * g_pe_gte.ir[0] + g_pe_gte.ofx, flags);
    pe_gte_mac0_flags((int64_t)ratio * g_pe_gte.ir[1] + g_pe_gte.ofy, flags);
    sx = ((int64_t)ratio * g_pe_gte.ir[0] + g_pe_gte.ofx) >> 16;
    sy = ((int64_t)ratio * g_pe_gte.ir[1] + g_pe_gte.ofy) >> 16;
    if (sx < -1024 || sx > 1023) *flags |= 1u << 14;
    if (sy < -1024 || sy > 1023) *flags |= 1u << 13;
    if (sx < -1024) sx = -1024;
    if (sx > 1023) sx = 1023;
    if (sy < -1024) sy = -1024;
    if (sy > 1023) sy = 1023;
    *xy = (uint16_t)sx | ((uint32_t)(uint16_t)sy << 16);
    cue = (int64_t)ratio * (int16_t)g_pe_gte.dqa + g_pe_gte.dqb;
    g_pe_gte.mac0 = (int32_t)cue;
    if (last) {
        pe_gte_mac0_flags(cue, flags);
        if ((cue >> 12) < 0 || (cue >> 12) > 4096) *flags |= 1u << 12;
    }
    cue >>= 12;
    g_pe_gte.sxy[0] = g_pe_gte.sxy[1];
    g_pe_gte.sxy[1] = g_pe_gte.sxy[2];
    g_pe_gte.sxy[2] = *xy;
    for (row = 0; row < 3; row++) g_pe_gte.sz[row] = g_pe_gte.sz[row+1];
    g_pe_gte.sz[3] = *z;
    g_pe_gte.ir0 = cue < 0 ? 0 : cue > 4096 ? 4096 : (int32_t)cue;
}

void PE_GTE_RTPS_coordinates(uint32_t *xy, uint32_t *z)
{
    uint32_t flags = 0;
    pe_gte_project_vertex(g_pe_gte.v0, xy, z, &flags, 1);
    g_pe_gte.projection_flags = flags | ((flags & 0x7F87E000u) ? 0x80000000u : 0u);
}

void PE_GTE_RTPT_coordinates(uint32_t xy[3], uint32_t z[3])
{
    uint32_t flags = 0;
    pe_gte_project_vertex(g_pe_gte.v0, xy, z, &flags, 0);
    pe_gte_project_vertex(g_pe_gte.v1, xy + 1, z + 1, &flags, 0);
    pe_gte_project_vertex(g_pe_gte.v2, xy + 2, z + 2, &flags, 1);
    g_pe_gte.projection_flags = flags | ((flags & 0x7F87E000u) ? 0x80000000u : 0u);
}

void PE_GTE_AVSZ3(const uint32_t z[3])
{
    int64_t value=(int64_t)(int16_t)g_pe_gte.zsf3*(z[0]+z[1]+z[2]);
    g_pe_gte.mac0=(int32_t)value;
    value>>=12;
    g_pe_gte.otz=value<0?0u:value>65535?65535u:(uint32_t)value;
}

void PE_GTE_AVSZ4(const uint32_t z[4])
{
    int64_t value=(int64_t)(int16_t)g_pe_gte.zsf4*(z[0]+z[1]+z[2]+z[3]);
    g_pe_gte.mac0=(int32_t)value;
    value>>=12;
    g_pe_gte.otz=value<0?0u:value>65535?65535u:(uint32_t)value;
}

int32_t PE_GTE_NCLIP(void)
{
    int64_t value=0;
    for (unsigned i=0;i<3;i++)
        value+=(int64_t)(int16_t)g_pe_gte.sxy[i] *
            ((int32_t)(int16_t)(g_pe_gte.sxy[(i+1)%3]>>16)-
             (int32_t)(int16_t)(g_pe_gte.sxy[(i+2)%3]>>16));
    return g_pe_gte.mac0=(int32_t)value;
}
