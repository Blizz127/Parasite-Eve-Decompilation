/*
 * Phase 6E-MV1d — func_8010C89C (overlay, 0x8010C89C..0x8010CBF8):
 * resumable VLC frame decoder.  Pure guest-RAM worker, no calls.
 * Transcribed from the authenticated 38-sector movie-module carve
 * (PE.IMG [0x039F,0x03C5) -> 0x8010BCF8, SHA-256 d0a22a1a...0b40).
 *
 * Arguments (801924F8 got_frame tail): a0 = frame-stream cursor
 * (0 = resume from saved state), a1 = output/halfword arena, a2 =
 * [0x801D0DF8] table word (used as table base minus 0x800: the
 * entry does a2 += 0x800, then a3 = a2+0x10000).  Entry-a3 is dead
 * (recomputed before first use) and passed as 0.  Static state is
 * 11 words at 0x8011EB8C..0x8011EBB4 (NOT 0x8012: the lui is
 * 0x8012 but every addiu is negative), seeded by the module image
 * ([EB8C] = 0x00FFFFFF: effectively no loop bound, so frames end
 * through the pad exit).  Exits: pad (CB84, returns 0, the normal
 * end-of-frame) and bound (CBC8, returns 1).  The caller ignores
 * the return.  COP0 Status bit17 set (CBAC) is not modeled by this RAM worker.
 * This port does not establish CPU cache/status fidelity.
 *
 * Shift-count care: MIPS sllv/srlv consume the low 5 bits, so
 * counts above 31 are masked (&31) — exact, not a change.  All
 * wrapping adds use uint32_t; signed branch tests cast to int32_t.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

#define GA_VLC_STATE      0x8011EB8Cu   /* saved t1 (bound half-count) */
#define GA_VLC_SAVED      0x8011EB90u   /* 9 saved regs (resume image) */
#define GA_VLC_BOUND      0x8011EBB4u   /* saved output bound t1 */

/* Host-side entry/exit telemetry (never guest authority: the MDEC
 * payload-fingerprint precedent). Lets tests observe the entry
 * triple and the exit code for oracle cross-checks. */
static PeC89CTelemetry g_c89c_telemetry;

void PE_C89C_GetTelemetry(PeC89CTelemetry *out)
{
    out->a0 = g_c89c_telemetry.a0;
    out->a1 = g_c89c_telemetry.a1;
    out->a2 = g_c89c_telemetry.a2;
    out->ret = g_c89c_telemetry.ret;
}

/* Bit-buffer refill (retail C9CC/CAA0/CAE4/CA4C, identical shape):
 * v1_low = v1 & 0xF always (branch delay); when v1 & 0x10 the next
 * stream halfword is appended at bit v1_low. */
static void c89c_refill(uint32_t *v0, int32_t *v1, pe_addr_t *a0,
                        uint32_t *dst)
{
    int32_t low = *v1 & 0xF;
    if ((*v1 & 0x10) == 0) {
        *v1 = low;
        return;
    }
    *dst = PE_LoadU16(*a0);
    *a0 += 2u;
    *v0 |= *dst << (uint32_t)low;
    *v1 = low;
}

int func_8010C89C(uint32_t a0, pe_addr_t a1, pe_addr_t a2, uint32_t a3)
{
    uint32_t t0, t1, t3, t4, v0;
    uint32_t t7, t8, t9;
    int32_t t2, t5, v1, at;
    pe_addr_t t6, tab, a3t;
    (void)a3; /* dead on entry: recomputed below before first use */
    g_c89c_telemetry.a0 = a0;
    g_c89c_telemetry.a1 = a1;
    g_c89c_telemetry.a2 = a2;

    /* C89C: t1 = [EB8C] (bnez delay slot: both paths). */
    t1 = PE_LoadU32(GA_VLC_STATE);
    a2 += 0x800u;               /* C8A4 */
    a3t = a2 + 0x10000u;        /* C8A8-AC: lui at,1; add a3,a2,at */
    if (a0 != 0u) {
        /* FRESH (C8F0): zero accumulators. */
        t5 = 0;
        t7 = 0u;
        t8 = 0u;
        t9 = 0u;
    } else {
        /* RESUME (C8B8): reload the saved image, skip the header. */
        a0 = PE_LoadU32(GA_VLC_SAVED);
        a1 = PE_LoadU32(GA_VLC_SAVED + 4u);
        v0 = PE_LoadU32(GA_VLC_SAVED + 8u);
        v1 = (int32_t)PE_LoadU32(GA_VLC_SAVED + 12u);
        t4 = PE_LoadU32(GA_VLC_SAVED + 16u);
        t5 = (int32_t)PE_LoadU32(GA_VLC_SAVED + 20u);
        t7 = PE_LoadU32(GA_VLC_SAVED + 24u);
        t8 = PE_LoadU32(GA_VLC_SAVED + 28u);
        t9 = PE_LoadU32(GA_VLC_SAVED + 32u);
        t1 += t1;               /* C8E4 */
        t6 = a1 + t1;           /* C8EC delay */
        goto main_loop;         /* C8E8: resume skips the header */
    }
    t1 += t1;                   /* C900 (fresh shares the bound setup) */
    t6 = a1 + t1;               /* C904 delay */
    /* HEADER (C908): stream word + four halfwords; t2 = count - 3. */
    t1 = PE_LoadU32(a0);
    t4 = PE_LoadU16(a0 + 4u);
    t2 = (int32_t)PE_LoadU16(a0 + 6u);
    v0 = PE_LoadU16(a0 + 8u);
    v1 = (int32_t)PE_LoadU16(a0 + 10u);
    t2 -= 3;
    t4 <<= 10;                  /* bltz delay: always executes */
    if (t2 >= 0)
        t5 = 1;
    a0 += 12u;                  /* C92C */
    v0 = (v0 << 16) | (uint32_t)v1;
    v1 = 0;
    PE_StoreU32(a1, t1);        /* C93C: raw stream word to output */
    t1 = ((t1 & 0xFFFFu) << 2) + 4u + a1;
    PE_StoreU32(GA_VLC_BOUND, t1);  /* C958: fresh output bound */
    a1 += 2u;                   /* C95C */
    goto t5_dispatch;

t5_dispatch:                    /* C960: t0 = v0 >> 22 (delay: always) */
    t0 = v0 >> 22;
    if (t5 == 0)
        goto ca38;
    at = (int32_t)(t0 ^ 0x3FFu);    /* C968 */
    a1 += 2u;                   /* C96C delay: always */
    if (at == 0)
        goto pad_exit;          /* CB84 */
    /* C974-80: tab = a2-0x400 (delay: always), minus 0x400 more
     * when t5 >= 3 (i.e. the passed table word). */
    tab = a2 - 0x400u;
    if (t5 >= 3)
        tab -= 0x400u;
    /* C984: table lookup through tab; t1 = base, t2 = count. */
    t0 = (v0 >> 24) << 2;
    t0 += tab;
    t1 = PE_LoadU16(t0);
    t2 = (int32_t)PE_LoadU16(t0 + 2u);
    t0 = 0u;
    v0 <<= (t1 & 31u);          /* C9A0 delay (sllv: low 5 bits) */
    if (t2 != 0) {
        int neg;
        at = 32 - t2;
        t0 = v0 >> (uint32_t)(at & 31);
        neg = (int32_t)v0 < 0;  /* bltz decides on the pre-shift v0 */
        v0 <<= (uint32_t)(t2 & 31); /* C9B4 delay */
        if (!neg) {
            uint32_t mask = 0xFFFFFFFFu;
            t3 = mask >> (uint32_t)(at & 31);   /* srlv -1,at */
            t0 -= t3;
        }
        v1 += t2;               /* C9C4 */
    }
    /* C9C8: */
    v1 += (int32_t)t1;
    c89c_refill(&v0, &v1, &a0, &t1);    /* C9CC */
    /* C9E8: accumulator select. Each arm computes t1 = tX+t0 AND
     * accumulates tX += t0 (the b-delay stores run on every arm). */
    at = t5 - 2;
    t1 = t9 + t0;               /* bgtz delay */
    if (at > 0) {
        t9 += t0;               /* CA10 */
    } else {
        t1 = t8 + t0;           /* beqz delay */
        if (at == 0) {
            t8 += t0;           /* CA0C delay */
        } else {
            t1 = t7 + t0;       /* C9FC */
            t7 += t0;           /* CA04 delay of b CA14 */
        }
    }
ca14:
    t1 <<= 2;
    t1 &= 0x3FFu;
    t1 |= t4;
    t5++;
    at = t5 - 7;
    PE_StoreU16(a1, (uint16_t)t1);  /* CA2C delay of bnez: always */
    if (at != 0)
        goto ca70;
    t5 -= 6;                    /* CA34 delay of b: 7 wraps to 1 */
    goto ca70;

ca38:                           /* t5 == 0 limb */
    at = (int32_t)(t0 ^ 0x1FFu);
    a1 += 2u;                   /* CA3C delay: always */
    if (at == 0)
        goto pad_exit;          /* CB84 */
    v0 <<= 10;                  /* CA44 */
    v1 += 10;
    c89c_refill(&v0, &v1, &a0, &t1);    /* CA4C */
    t0 |= t4;                   /* CA68 */
    PE_StoreU16(a1, (uint16_t)t0);  /* CA6C */
    /* fallthrough to ca70 */
ca70:
    at = (int32_t)(a1 - t6);
    a1 += 2u;                   /* CA74 delay: always, both outcomes */
    if (at >= 0)
        goto bound_exit;        /* CBC8 */
    /* fallthrough to main_loop */
main_loop:                      /* CA7C */
    t0 = (v0 >> 19) << 3;
    t0 += a2;
    t1 = PE_LoadU32(t0);
    at = (int32_t)(t1 & 0xFFu); /* CA94 delay: always */
    if (t1 == 0) {
        v0 <<= 8;               /* CA98 */
        v1 += 8;
        c89c_refill(&v0, &v1, &a0, &t0); /* CAA0 refill2 into t0 */
        t0 = (v0 >> 23) << 2;   /* CABC */
        t0 += a3t;
        t1 = PE_LoadU32(t0);
        t3 = 0u;                /* CACC */
        /* CAD4 delay (at = t1 & 0xFF) already equals at: t1 kept */
    } else {
        t3 = PE_LoadU32(t0 + 4u);   /* CAD8 */
    }
    v0 <<= (uint32_t)(at & 31); /* CADC (sllv: low 5 bits) */
    v1 += at;
    c89c_refill(&v0, &v1, &a0, &t0);    /* CAE4 refill3 into t0 */
    t1 >>= 16;                  /* CB00 */
    if (t1 == 0x7C1Fu)
        goto cb60;
    at = (int32_t)(t1 ^ 0xFE00u);   /* CB0C delay */
    PE_StoreU16(a1, (uint16_t)t1);  /* CB14 delay: always past here */
    if (at == 0)
        goto t5_dispatch;       /* CB10: inner restart at C960 */
    a1 += 2u;                   /* CB1C delay */
    if (t3 == 0u)               /* CB18 */
        goto main_loop;
    /* t3 extension halfwords: same 7C1F/FE00/store shape twice. */
    t2 = (int32_t)(t3 & 0xFFFFu);   /* CB20 */
    if (t2 == 0x7C1F)
        goto cb60;
    at = t2 ^ 0xFE00;           /* CB2C delay */
    PE_StoreU16(a1, (uint16_t)t2);  /* CB34 delay */
    if (at == 0)
        goto t5_dispatch;
    t2 = (int32_t)(t3 >> 16);   /* CB38 */
    a1 += 2u;                   /* CB40 delay: always */
    if (t2 == 0)                /* CB3C */
        goto main_loop;
    if (t2 == 0x7C1F)
        goto cb60;
    at = t2 ^ 0xFE00;           /* CB4C delay */
    PE_StoreU16(a1, (uint16_t)t2);  /* CB54 delay */
    if (at == 0)
        goto t5_dispatch;
    a1 += 2u;                   /* CB5C delay of b CA7C */
    goto main_loop;

cb60:
    t0 = v0 >> 16;              /* CB60 */
    PE_StoreU16(a1, (uint16_t)t0);
    a1 += 2u;
    t0 = PE_LoadU16(a0);
    a0 += 2u;
    v0 <<= 16;
    v0 |= t0 << (uint32_t)(v1 & 31);    /* sllv: low 5 bits */
    goto main_loop;             /* CB7C delay: or (folded above) */

pad_exit:                       /* CB84: pad with FE00 to the bound */
    t0 = GA_VLC_BOUND;
    t1 = PE_LoadU32(t0);        /* CB8C */
    while ((int32_t)(a1 - t1) < 0) {    /* CB94: subu + bgez */
        PE_StoreU16(a1, 0xFE00u);    /* CBA0 */
        a1 += 2u;               /* CBA8 delay */
    }
    /* CBAC-CBBC: mfc0 Status, OR 0x20000, mtc0. CPU status/cache
     * effects are not modeled here; this is not an IEc (bit0) write. */
    g_c89c_telemetry.ret = 0;
    return 0;                   /* CBC0 delay: v0 = 0 */

bound_exit:                     /* CBC8: save the 9-word image */
    PE_StoreU32(GA_VLC_SAVED, a0);
    PE_StoreU32(GA_VLC_SAVED + 4u, a1);
    PE_StoreU32(GA_VLC_SAVED + 8u, v0);
    PE_StoreU32(GA_VLC_SAVED + 12u, (uint32_t)v1);
    PE_StoreU32(GA_VLC_SAVED + 16u, t4);
    PE_StoreU32(GA_VLC_SAVED + 20u, (uint32_t)t5);
    PE_StoreU32(GA_VLC_SAVED + 24u, t7);
    PE_StoreU32(GA_VLC_SAVED + 28u, t8);
    PE_StoreU32(GA_VLC_SAVED + 32u, t9);
    g_c89c_telemetry.ret = 1;
    return 1;                   /* CBF8 delay: v0 = 1 */
}
