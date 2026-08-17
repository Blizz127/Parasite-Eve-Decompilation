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

void PE_GTE_LoadRT(pe_addr_t matrix)
{
    g_pe_gte.rt[0][0] = (int16_t)PE_LoadU16(matrix + 0u);
    g_pe_gte.rt[0][1] = (int16_t)PE_LoadU16(matrix + 2u);
    g_pe_gte.rt[0][2] = (int16_t)PE_LoadU16(matrix + 4u);
    g_pe_gte.rt[1][0] = (int16_t)PE_LoadU16(matrix + 6u);
    g_pe_gte.rt[1][1] = (int16_t)PE_LoadU16(matrix + 8u);
    g_pe_gte.rt[1][2] = (int16_t)PE_LoadU16(matrix + 10u);
    g_pe_gte.rt[2][0] = (int16_t)PE_LoadU16(matrix + 12u);
    g_pe_gte.rt[2][1] = (int16_t)PE_LoadU16(matrix + 14u);
    g_pe_gte.rt[2][2] = (int16_t)PE_LoadU16(matrix + 16u);
    g_pe_gte.tr[0] = (int32_t)PE_LoadU32(matrix + 20u);
    g_pe_gte.tr[1] = (int32_t)PE_LoadU32(matrix + 24u);
    g_pe_gte.tr[2] = (int32_t)PE_LoadU32(matrix + 28u);
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

            mac = ((int64_t)t[row] << 12)
                + (int64_t)g_pe_gte.rt[row][0] * (int64_t)vx
                + (int64_t)g_pe_gte.rt[row][1] * (int64_t)vy
                + (int64_t)g_pe_gte.rt[row][2] * (int64_t)vz;
            mac >>= shift;
            g_pe_gte.mac[row] = (int32_t)mac;
            g_pe_gte.ir[row] = pe_gte_sat_ir(mac, lm);
        }
    }
}
