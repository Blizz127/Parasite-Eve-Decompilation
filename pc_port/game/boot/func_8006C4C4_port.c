/*
 * PE-BTL5 — overlay +0xE wait producers.
 *
 * Native translations, not matching src/ C. Authority is
 * pc_port/tools/pe_btl5_overlay_wait_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_8006C4C4 (62 words, 0x8006C4C4..0x8006C5BC exclusive) is the
 * field-transition setter: ori bits 0/1/3 of D_800B0CD8+0xE according
 * to a0, D_8009D1A0 bit 1, and overlay word bit 1. For a0 in 1..8 it
 * also writes a0 to +0xC/+0xD. Always returns 0.
 *
 * func_8006C5BC is the 427-word CD poll (0x8006C5BC..0x8006CC68).
 * Exclusive end is func_8006CC68. TEXT has exactly three jal sites:
 * 3F074@3F22C (field-tick poll), 35558@35B24 (after actor walk),
 * 6C1CC state 6@6C358. 209F0 @ 0x80020C5C jals it
 * (lh(*(D278+0x68)+6); slot-body +0x68=0 from 109B0.
 * APPROXIMATION: host a0=lh(6)==0).
 * 144FC/29810/6914C/6D60C have no direct jal.
 *
 * Named cut implemented here: CE2 in [10,14], +0xEE JT gates,
 * EE=0 bit0/bit1 advance, EE=11 ori 1, EE=12→13, EE 8/9/10 return 0.
 * EE 1-7 return 1 without clearing +0xE. EE=13 runs the proven
 * package-walk prefix then the 0x8006CC20..0x8006CC38 epilogue
 * (andi 0xFC / sb +0xE / sb 0 → +0xEE) after jal 3D834 returns.
 * Blanket +0xE&=0xFC from 0x55 without that EE=13 fall-through
 * is still forbidden. EE=0 bits-clear jals func_8006CC68 (always
 * v0=0). 3F074 then continues to 1A918 / 371B0 / 125E0 /
 * EXE-resident 0x800E0060 (not loaded, not M2).
 *
 * 144FC 0x3A oris D_8009D1A0 bit 1; the next field tick's 3F074
 * jals 6C4C4(CE4) which copies that into +0xE bit 1. Native invokes
 * that pair once per parked 0x55 tick, not 3F074's tight beq poll
 * (that would hang on unported EE=13). Do not auto-clear +0xE,
 * stub 6914C, set mode 7, or complete 0x55.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define GA_OVERLAY  0x800B0CD8u
#define GA_D2E8     0x8009D2E8u
#define GA_D254     0x8009D254u

extern unsigned int D_8009D1A0;

int func_8006C4C4(int a0)
{
    unsigned int status;
    unsigned int overlay_word;
    int slot;
    int current;

    if (a0 == -1) {
        status = PE_LoadU8(GA_OVERLAY + 0x0Du);
        PE_StoreU8(GA_OVERLAY + 0x0Cu, (uint8_t)status);
        PE_StoreU8(GA_OVERLAY + 0x0Eu,
                   (uint8_t)(PE_LoadU8(GA_OVERLAY + 0x0Eu) | 3u));
    }

    overlay_word = PE_LoadU32(GA_OVERLAY);
    if (((D_8009D1A0 & 2u) != 0u) || ((overlay_word & 2u) != 0u)) {
        PE_StoreU8(GA_OVERLAY + 0x0Eu,
                   (uint8_t)(PE_LoadU8(GA_OVERLAY + 0x0Eu) | 2u));
        PE_StoreU32(GA_D2E8, PE_LoadU32(GA_D2E8) & ~2u);
    }

    status = PE_LoadU8(GA_OVERLAY + 0x0Eu);
    if ((status & 4u) != 0u) {
        PE_StoreU8(GA_OVERLAY + 0x0Eu, (uint8_t)((status | 3u) & ~4u));
    }

    if ((unsigned int)(a0 - 1) < 8u) {
        current = (int)(int8_t)PE_LoadU8(GA_OVERLAY + 0x0Du);
        if (a0 != current) {
            slot = a0;
            PE_StoreU8(GA_OVERLAY + 0x0Du, (uint8_t)slot);
            PE_StoreU8(GA_OVERLAY + 0x0Cu, (uint8_t)slot);
            PE_StoreU8(GA_OVERLAY + 0x0Eu,
                       (uint8_t)(PE_LoadU8(GA_OVERLAY + 0x0Eu) | 1u));
        }
    }

    return 0;
}

void func_8006C5BC_ee13_epilogue_cut(void)
{
    uint8_t status;

    /* 0x8006CC20..0x8006CC38 exclusive: after jal 3D834 @ 0x8006CC18.
     * v0=1 is the EE=13 return; the j 0x8006CC3C skips the v0=0 join. */
    status = PE_LoadU8(GA_OVERLAY + 0x0Eu);
    PE_StoreU8(GA_OVERLAY + 0xEEu, 0u);
    PE_StoreU8(GA_OVERLAY + 0x0Eu, (uint8_t)(status & 0xFCu));
}

void func_8006C5BC_clear_wait_cut(void)
{
    func_8006C5BC_ee13_epilogue_cut();
}

/*
 * EE=13 body 0x8006C9F8..0x8006CB98 exclusive, before jal 3D050.
 * Walks overlay+0x158 like Writer B into +0x1C0, zeros +0x10 and
 * +0x134[3], optionally fills +0x134 from section+0x2C, then the
 * D254 / D1A0 a1 select. Does not jal 3D050/6698C/3D834 and does
 * not andi 0xFC. pkg==0 is a host guard (ROM would deref).
 * Returns the 3D050 a1, or 0 on the D254==0 early-out.
 */
pe_addr_t func_8006C5BC_ee13_prefix_cut(void)
{
    pe_addr_t pkg;
    pe_addr_t section;
    unsigned int packed;
    unsigned int count;
    unsigned int i;
    pe_addr_t rec;
    unsigned int word2c;
    pe_addr_t actor;

    pkg = PE_LoadU32(GA_OVERLAY + 0x158u);
    section = 0u;
    if (pkg != 0u) {
        section = pkg + PE_LoadU32(pkg + 4u);
        packed = PE_LoadU32(section + 0x10u);
        count = packed >> 22;
        rec = pkg + (packed & 0x3FFFFFu);
        for (i = 0; i < count; i++) {
            unsigned int idb = PE_LoadU8(rec + 7u);
            unsigned int ptr = PE_LoadU32(rec + 4u) & 0x00FFFFFFu;
            PE_StoreU32(GA_OVERLAY + 0x1C0u + idb * 4u, pkg + ptr);
            rec += 12u;
        }
        PE_StoreU8(GA_OVERLAY + 0x10u, 0u);
        for (i = 0; i < 3u; i++)
            PE_StoreU32(GA_OVERLAY + 0x134u + i * 4u, 0u);
        word2c = PE_LoadU32(section + 0x2Cu);
        if ((word2c >> 22) != 0u) {
            pe_addr_t rec2 = pkg + (word2c & 0x3FFFFFu);
            unsigned int n2 = word2c >> 22;
            for (i = 0; i < n2; i++) {
                unsigned int ptr = PE_LoadU32(rec2 + 4u) & 0x00FFFFFFu;
                PE_StoreU32(GA_OVERLAY + 0x134u + i * 4u, pkg + ptr);
                rec2 += 12u;
            }
            PE_StoreU8(GA_OVERLAY + 0x10u, PE_LoadU8(pkg + (word2c & 0x3FFFFFu) + 8u));
        }
    }

    actor = PE_LoadU32(GA_D254);
    if (actor == 0u)
        return 0u;
    if (pkg == 0u || section == 0u)
        return PE_LoadU32(GA_OVERLAY + 0x11Cu);
    if ((D_8009D1A0 & 2u) != 0u) {
        pe_addr_t rec_c = pkg + (PE_LoadU32(section + 0x0Cu) & 0x3FFFFFu);
        unsigned int ptr = PE_LoadU32(rec_c + 4u) & 0x00FFFFFFu;
        return pkg + ptr;
    }
    return PE_LoadU32(GA_OVERLAY + 0x11Cu);
}

void func_800144FC_state3A_d1a0_cut(void)
{
    D_8009D1A0 |= 2u;
}

/* 10 words. ctc2 OFX/OFY from D_800BCF94/CF96 (lhu << 16). */
void func_800661A4(void)
{
    g_pe_gte.ofx = (int32_t)((unsigned int)PE_LoadU16(0x800BCF94u) << 16);
    g_pe_gte.ofy = (int32_t)((unsigned int)PE_LoadU16(0x800BCF96u) << 16);
}

/* 8 words. SetGeomOffset(0xA0, 0x70); v0=0. */
void func_800661CC(void)
{
    func_80079004(0xA0, 0x70);
}

/*
 * func_8006CC68 — 79 words, SHA-256 262dcfc6…a74d.
 * EE=0 idle jal; every arm returns 0 (cd88 addu v0,0 or delay of
 * the +0x98-flag skip). Live 0x3A path: overlay[0]&0x000C0000==0,
 * D254!=0, actor+0x98&0x20000040==0, D1A0 bit1 set → publish
 * D_800B0D10=actor+0x1B4 / +0x3C=3 / +0x3E=0x12 once, then
 * 661A4, 3A088(overlay+0x14), 3AC90, 3AF14, 661CC.
 * 3AC90 (161w) and 3AF14 (140w) are not this cut.
 */
int func_8006CC68(void)
{
    pe_addr_t actor;
    unsigned int word;

    word = PE_LoadU32(GA_OVERLAY);
    if ((word & 0x000C0000u) != 0u)
        return 0;

    actor = PE_LoadU32(GA_D254);
    if (actor == 0u)
        return 0;

    if ((PE_LoadU32(actor + 0x98u) & 0x20000040u) != 0u)
        return 0;

    if ((D_8009D1A0 & 2u) == 0u && (PE_LoadU32(GA_D2E8) & 2u) != 0u)
        return 0;

    if (PE_LoadU32(0x800B0D10u) == 0u) {
        PE_StoreU32(0x800B0D10u, actor + 0x1B4u);
        PE_StoreU16(0x800B0D14u, 3u);
        PE_StoreU16(0x800B0D16u, 0x12u);
    }

    func_800661A4();
    func_8003A088_mode0_empty_cut(GA_OVERLAY + 0x14u);
    func_800661CC();
    return 0;
}

int func_8006C5BC(void)
{
    unsigned int ce2;
    unsigned int ee;
    unsigned int status;

    ce2 = PE_LoadU8(GA_OVERLAY + 0x0Au);
    if ((ce2 - 10u) >= 5u)
        return 0;

    for (;;) {
        ee = PE_LoadU8(GA_OVERLAY + 0xEEu);
        if (ee >= 14u || ee == 8u || ee == 9u || ee == 10u)
            return 0;
        if (ee == 0u) {
            status = PE_LoadU8(GA_OVERLAY + 0x0Eu);
            if ((status & 1u) != 0u) {
                PE_StoreU8(GA_OVERLAY + 0xEEu, 1u);
                continue;
            }
            if ((status & 2u) != 0u) {
                PE_StoreU8(GA_OVERLAY + 0xEEu, 11u);
                continue;
            }
            return func_8006CC68();
        }
        if (ee == 11u) {
            status = PE_LoadU8(GA_OVERLAY + 0x0Eu);
            PE_StoreU8(GA_OVERLAY + 0xEEu, 12u);
            PE_StoreU8(GA_OVERLAY + 0x0Eu, (uint8_t)(status | 1u));
            return 1;
        }
        if (ee == 12u) {
            PE_StoreU8(GA_OVERLAY + 0xEEu, 13u);
            return 1;
        }
        if (ee == 13u) {
            (void)func_8006C5BC_ee13_prefix_cut();
            func_8006C5BC_ee13_epilogue_cut();
            return 1;
        }
        return 1;
    }
}

int func_8003F074_6C4C4_6C5BC_cut(void)
{
    int slot;

    slot = (int)(int8_t)PE_LoadU8(GA_OVERLAY + 0x0Cu);
    (void)func_8006C4C4(slot);
    return func_8006C5BC();
}

/* 3F074 @ 0x8003F22C: jal 6C5BC / beq v0,s0 until v0==0.
 * One 6C4C4, then the tight poll. Does not jal 1A918/E0060. */
int func_8003F074_poll_cut(void)
{
    int slot;
    int v0;
    int guard;

    slot = (int)(int8_t)PE_LoadU8(GA_OVERLAY + 0x0Cu);
    (void)func_8006C4C4(slot);
    guard = 0;
    do {
        v0 = func_8006C5BC();
        guard++;
    } while (v0 == 1 && guard < 16);
    return v0;
}

/* 3F074 @ 0x8003F0B0 / 0x8003F0B8: actor pool then task pool. */
void func_8003F074_pool_cut(void)
{
    func_80034FC4();
    func_8001266C();
}

/* 3F074 @ 0x8003F244: language bit selects 162C else 1628. */
pe_addr_t func_8003F074_371b0_a0(void)
{
    if ((PE_LoadU32(GA_OVERLAY) & 0x40000000u) != 0u)
        return PE_LoadU32(0x800B162Cu);
    return PE_LoadU32(0x800B1628u);
}

/* 3F074 after v0=0: 1A918, 371B0(selected a0), 125E0, E0060,
 * gp+0x34=0, sh 0 → D_800942EC, DrawSync(0), SetDispMask(1),
 * D1A0 &= ~0x40, D2E8 &= ~0xC, overlay[0] &= ~0x00000402.
 * Not M2. */
void func_8003F074_after_poll_cut(void)
{
    uint32_t word;

    func_8001A918();
    func_800371B0(func_8003F074_371b0_a0());
    func_800125E0();
    func_800E0060();
    PE_StoreU32(0x8009CDA4u, 0u);
    PE_StoreU16(0x800942ECu, 0u);
    func_80074DC0(0);
    func_80074D28(1);
    D_8009D1A0 &= ~0x40u;
    word = PE_LoadU32(GA_D2E8) & ~0xCu;
    PE_StoreU32(GA_D2E8, word);
    word = PE_LoadU32(GA_OVERLAY) & ~0x402u;
    PE_StoreU32(GA_OVERLAY, word);
}

/*
 * func_800E0060 — 27 words 0x800E0060..0x800E00CC,
 * SHA-256 cfa139eb…750e. EXE-resident (taddr 0x80010000 tsize
 * 0x1EE000). Zero jal/jalr. Sole TEXT caller 3F074 @ 0x8003F284.
 * REJECTED: loaded-overlay identity. Walks 0x14-stride slots
 * backward from D_800B0E5C-0x14, count D_800E21A4, sb 0 at
 * slot+0 when nonzero, then sh 0 → D_800E21A4.
 */
void func_800E0060(void)
{
    pe_addr_t cur;
    int16_t n;
    int i;

    cur = PE_LoadU32(0x800B0E5Cu) - 0x14u;
    n = (int16_t)PE_LoadU16(0x800E21A4u);
    PE_StoreU32(0x800E2800u, cur);
    i = 0;
    if (n > 0) {
        do {
            if (PE_LoadU8(cur) != 0u) {
                PE_StoreU8(cur, 0u);
                i++;
            }
            n = (int16_t)PE_LoadU16(0x800E21A4u);
            if (i >= n)
                break;
            cur -= 0x14u;
        } while (i < n);
    }
    PE_StoreU16(0x800E21A4u, 0u);
}
