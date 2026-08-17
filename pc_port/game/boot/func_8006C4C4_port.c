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
 * 6C1CC state 6@6C358. 144FC/29810/6914C/6D60C do not jal it.
 *
 * Named cut implemented here: CE2 in [10,14], +0xEE JT gates,
 * EE=0 bit0/bit1 advance, EE=11 ori 1, EE=12→13, EE 8/9/10 return 0.
 * EE 1-7 return 1 without clearing +0xE. EE=13 runs the proven
 * package-walk prefix (overlay+0x158 → +0x1C0, zeros, D254 gate)
 * and still returns 1; jal 3D050/6698C/3D834 and andi 0xFC are
 * not this cut. 6CC68 is EE 0/1-7 only — not on the live bit1 path.
 *
 * 144FC 0x3A oris D_8009D1A0 bit 1; the next field tick's 3F074
 * jals 6C4C4(CE4) which copies that into +0xE bit 1. Native invokes
 * that pair once per parked 0x55 tick, not 3F074's tight beq poll
 * (that would hang on unported EE=13). Do not auto-clear +0xE,
 * stub 6914C, set mode 7, or complete 0x55.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

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

void func_8006C5BC_clear_wait_cut(void)
{
    uint8_t status;

    status = PE_LoadU8(GA_OVERLAY + 0x0Eu);
    PE_StoreU8(GA_OVERLAY + 0xEEu, 0u);
    PE_StoreU8(GA_OVERLAY + 0x0Eu, (uint8_t)(status & 0xFCu));
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
            return 0;
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
