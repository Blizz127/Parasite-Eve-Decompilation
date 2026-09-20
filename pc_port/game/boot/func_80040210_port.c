/*
 * PE-SAVEWRITE — save/load buffer formatters (hand-translated, nonmatching).
 *
 * Authority: retail Disc1 EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b,
 * spans in asm/disc1/307CC.s and asm/disc1/4C974.s:
 *   func_8004006C  0x8004006C..0x80040210 (0x1A4, 105w)  fullwidth %d/%D/%s
 *   func_80040210  0x80040210..0x80040354 (0x144,  81w)  play-time fields
 *   func_8005DE08  0x8005DE08..0x8005DE70 (0x68)         name-table skip
 *   func_80043474  0x80043474..0x800434C0 (0x4C)         play-time rank
 *   func_8005D940  0x8005D940..0x8005D970 (0x30)         current name index
 *
 * These are translated from the retail bytes; none is a matching leaf.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_PLAYTIME_FRAMES 0x800A1708u
#define GA_PLAYTIME_HOURS  0x800A170Cu
#define GA_PLAYTIME_MINS   0x800A1710u
#define GA_PLAYTIME_SECS   0x800A1714u
#define GA_PLAYTIME_NAME1  0x800A1718u
#define GA_PLAYTIME_NAME2  0x800A171Cu
#define GA_NAME_TABLE_BASE 0x800A8028u
#define GA_NAME_TABLE_LEN  0x800A8054u
#define GA_NAME_TABLE_IDX  0x800A804Cu

/* Host scratch for func_8005D940's 8-byte name buffer (retail uses an
 * 8-byte stack local; the port has no guest stack).  Chosen in the free
 * 0x801FF040..0x801FFE00 band used by other port scratch. */
#define GA_NAME_SCRATCH 0x801FF680u

/* func_8005D940 — write the current room name for D_8009D280 and return its
 * func_8006E454 index.  Declared void in the matching leaf, but every retail
 * caller consumes $v0 (the value func_8006E454 leaves), so the host
 * implementation returns it. */
int func_8005D940(void)
{
    func_8006E2D0(GA_NAME_SCRATCH, PE_LoadU32(0x8009D280u));
    return func_8006E454(GA_NAME_SCRATCH);
}

/* func_8005DE08(a0) — advance past a0 NUL-terminated strings in the name
 * table; a0 < 0 selects 0xF (the retail "last access" sentinel). */
pe_addr_t func_8005DE08(int a0)
{
    pe_addr_t v1 = PE_LoadU32(GA_NAME_TABLE_LEN) + GA_NAME_TABLE_BASE;

    if (a0 < 0)
        a0 = 0xF;
    else
        a0 = (int)PE_LoadU8((pe_addr_t)((uint32_t)a0 +
                                        PE_LoadU32(GA_NAME_TABLE_IDX) +
                                        GA_NAME_TABLE_BASE));
    if (a0 == 0)
        return 0u;
    for (;;) {
        uint8_t c = PE_LoadU8(v1);
        v1++;
        if (c == 0u)
            a0--;
        if (a0 <= 0)
            break;
    }
    return v1;
}

/* func_80043474(a0) — play-time rank 1..6 by elapsed-frame thresholds. */
int func_80043474(int a0)
{
    if (a0 < 0x80)  return 1;
    if (a0 < 0x138) return 2;
    if (a0 < 0x1B0) return 3;
    if (a0 < 0x218) return 4;
    if (a0 < 0x2D3) return 5;
    return 6;
}

/* Signed /10 exactly as the retail mult-high + sra + sign-subtract sequence. */
static int32_t pe_st_div10(int32_t v)
{
    int32_t hi = (int32_t)(((int64_t)v * 0x66666667LL) >> 32);
    return (hi >> 2) - (v >> 31);
}

/* Two Shift-JIS bytes for a fullwidth digit 0..9. */
static void pe_st_wide_digit(pe_addr_t dst, int32_t digit)
{
    uint32_t g = (uint32_t)(digit + 0x824F);
    PE_StoreU8(dst, (uint8_t)(g >> 8));
    PE_StoreU8(dst + 1u, (uint8_t)g);
}

/* func_8004006C(dest, format) — Shift-JIS fullwidth "%d"/"%D"/"%s" formatter.
 * "%d" consumes one int from the D_800A1708 cursor and emits its two
 * fullwidth digits; "%D" emits one digit; "%s" consumes a pointer and copies
 * the NUL-terminated string.  Other characters are literal. */
void func_8004006C(pe_addr_t dest, pe_addr_t format)
{
    pe_addr_t a1 = format;
    pe_addr_t a2 = dest;
    pe_addr_t t0 = GA_PLAYTIME_FRAMES;
    uint8_t c = PE_LoadU8(a1);

    if (c == 0u) {
        PE_StoreU8(a2, 0u);
        return;
    }
    for (;;) {
        if (c != '%') {
            PE_StoreU8(a2, c);
            a2++;
            a1++;
            c = PE_LoadU8(a1);
        } else {
            a1++;
            c = PE_LoadU8(a1);
            a1++;
            if (c == 'd') {
                int32_t v = (int32_t)PE_LoadU32(t0);
                int32_t q10, q100;
                t0 += 4u;
                q10 = pe_st_div10(v);
                q100 = pe_st_div10(q10);
                pe_st_wide_digit(a2, q10 - q100 * 10);
                a2 += 2u;
                pe_st_wide_digit(a2, v - q10 * 10);
                a2 += 2u;
            } else if (c == 'D') {
                int32_t v = (int32_t)PE_LoadU32(t0);
                int32_t q10;
                t0 += 4u;
                q10 = pe_st_div10(v);
                pe_st_wide_digit(a2, v - q10 * 10);
                a2 += 2u;
            } else if (c == 's') {
                pe_addr_t s = PE_LoadU32(t0);
                uint8_t ch;
                t0 += 4u;
                if (s != 0u && PE_LoadU8(s) != 0u) {
                    ch = PE_LoadU8(s);
                    while (ch != 0u) {
                        s++;
                        PE_StoreU8(a2, ch);
                        a2++;
                        ch = PE_LoadU8(s);
                    }
                }
            } else {
                PE_StoreU8(a2, c);
                a2++;
            }
            c = PE_LoadU8(a1);
        }
        if (c == 0u)
            break;
    }
    PE_StoreU8(a2, 0u);
}

/* func_80040210(idx, time) — D_800A1708 = idx; D_800A170C/10/14 = H/M/S of
 * `time`; then format D_8009EE8C and return it. */
pe_addr_t func_80040210(int32_t idx, int32_t time)
{
    int32_t hours, minutes, seconds;
    int32_t sign = time < 0 ? -1 : 0;

    func_80071A24(0x8009EE8Cu, 0x44u);
    hours = (int32_t)((((int64_t)time * (int64_t)0x91A2B3C5LL) >> 32) + time);
    hours = (hours >> 11) - sign;
    PE_StoreU32(GA_PLAYTIME_FRAMES, (uint32_t)idx);
    PE_StoreU32(GA_PLAYTIME_HOURS, (uint32_t)hours);
    minutes = (int32_t)((((int64_t)time * (int64_t)0x88888889LL) >> 32) + time);
    minutes = (minutes >> 5) - sign;
    seconds = time - minutes * 60;
    PE_StoreU32(GA_PLAYTIME_MINS, (uint32_t)(minutes - hours * 60));
    PE_StoreU32(GA_PLAYTIME_SECS, (uint32_t)seconds);

    if (PE_LoadU32(0x800A1704u) != 0u) {
        uint32_t fmt = PE_LoadU32(0x8009222Cu);
        PE_StoreU32(GA_PLAYTIME_NAME1, (uint32_t)func_8005DE08(-1));
        func_8004006C(0x8009EE8Cu, (pe_addr_t)fmt);
    } else {
        uint32_t fmt;
        int32_t rank = func_80043474((int32_t)PE_LoadU32(0x800A7918u));
        PE_StoreU32(GA_PLAYTIME_NAME1, (uint32_t)rank);
        PE_StoreU32(GA_PLAYTIME_NAME2, (uint32_t)func_8005DE08(func_8005D940()));
        fmt = PE_LoadU32(0x80092228u);
        func_8004006C(0x8009EE8Cu, (pe_addr_t)fmt);
    }
    return 0x8009EE8Cu;
}
