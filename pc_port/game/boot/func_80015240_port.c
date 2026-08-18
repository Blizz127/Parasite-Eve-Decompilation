/*
 * PE-BTL27 — opcode 0x9B (15240) and live empty-+0x1B0 callees.
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80015240 — 239w 0x80015240..0x800155FC, SHA-256
 * fb261fcb…c9f8. D_800910A0[0x9B]. Live type-0 imms
 * 0x55/0xA0/0x28; type-2 0x5A/0x96/0x3C.
 *
 * func_80039B74 — 108w 0x80039B74..0x80039D24, SHA-256
 * 8c66e398…84e7. a1==0 returns immediately (live +0x1B0=0).
 * Nonzero a1 (39D24/39ED4/79754) is not this cut.
 *
 * func_800362B8 — 79w 0x800362B8..0x800363F4, SHA-256
 * c1d94293…b78d. Zero jal. Size-class bank allocator.
 *
 * func_8003A6A8 dest+0==0 jals 3E188 then returns.
 * 3E188 187w 0x8003E188..0x8003E474, SHA-256 dc4df605…abe7.
 * dest+0x24==0 loads *(0+0x84): BTL-RAM-LOW APPROXIMATION.
 *
 * +0x98 bit 0x10000000 (35038) takes the no-yield return-1 arm.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_8009D300 0x8009D300u
#define GA_D_8009CDDC 0x8009CDDCu
#define GA_D_800B0E4C 0x800B0E4Cu
#define GA_D_800A7620 0x800A7620u
#define GA_D_800A7624 0x800A7624u
#define GA_B89F8      0x800B89F8u
#define GA_BEA40      0x800BEA40u
#define CMD_RTIR      0x049E012u
#define CMD_RTV0      0x0480012u

pe_addr_t func_800362B8(unsigned int size)
{
    unsigned int bank;
    unsigned int slot;
    unsigned int i;
    pe_addr_t heap;
    pe_addr_t found;

    if (size < 18401u)
        bank = 1u;
    else if (size <= 0x8FC0u)
        bank = 2u;
    else if (size <= 0x11F80u)
        bank = 4u;
    else
        bank = 8u;

    slot = 0u;
    while (slot < 16u) {
        unsigned int end = slot + bank;

        found = 0u;
        if (slot < end) {
            for (i = slot; i < end; i++) {
                if (PE_LoadU32(GA_D_800A7624 + i * 8u) != 0u) {
                    unsigned int used;

                    used = PE_LoadU32(GA_D_800A7620 + i * 8u);
                    slot = (bank < used) ? (slot + used) : (slot + bank);
                    found = 1u;
                    break;
                }
            }
        }
        if (found != 0u)
            continue;
        heap = PE_LoadU32(GA_D_800B0E4C);
        PE_StoreU32(GA_D_800A7620 + slot * 8u, bank);
        found = heap + slot * 18400u;
        PE_StoreU32(GA_D_800A7624 + slot * 8u, found);
        return found;
    }
    return 0u;
}

void func_80039B74(pe_addr_t dest, pe_addr_t clip, int a2, int a3)
{
    (void)dest;
    (void)a2;
    (void)a3;
    if (clip == 0u)
        return;
    /* Nonzero clip: 39ED4/39D24/79754 not this cut. */
}

static pe_addr_t pe_15240_kseg0(pe_addr_t addr)
{
    return 0x80000000u | (addr & 0x1FFFFFu);
}

void func_8003E188(pe_addr_t dest)
{
    pe_addr_t parent;
    pe_addr_t mats;
    int16_t idx;

    /* dest+0x24==0 is the live empty-+0x1AC path. Retail loads
     * KUSEG 0x84 (same physical as 0x80000084). Host has no
     * KUSEG mirror; map like 12574/1A918. Not a NULL skip. */
    parent = pe_15240_kseg0(PE_LoadU32(dest + 0x24u));
    idx = (int16_t)PE_LoadU16(dest + 0x32u);
    mats = pe_15240_kseg0(PE_LoadU32(parent + 0x84u)
                          + (pe_addr_t)idx * 32u);
    PE_GTE_LoadRT(mats);
    PE_GTE_SetV0((int16_t)PE_LoadU16(dest + 0x2Cu),
                 (int16_t)PE_LoadU16(dest + 0x2Eu),
                 (int16_t)PE_LoadU16(dest + 0x30u));
    PE_GTE_MVMVA(CMD_RTV0);
    PE_StoreU16(dest + 0xA0u, (uint16_t)g_pe_gte.ir[0]);
    PE_StoreU16(dest + 0xA2u, (uint16_t)g_pe_gte.ir[1]);
    PE_StoreU16(dest + 0xA4u, (uint16_t)g_pe_gte.ir[2]);
    PE_StoreU16(dest + 0x68u, (uint16_t)g_pe_gte.ir[0]);
    PE_StoreU16(dest + 0x6Au, (uint16_t)g_pe_gte.ir[1]);
    PE_StoreU16(dest + 0x6Cu, (uint16_t)g_pe_gte.ir[2]);
    PE_StoreU16(dest + 0xB4u, (uint16_t)g_pe_gte.ir[0]);
    PE_StoreU16(dest + 0xB6u, (uint16_t)g_pe_gte.ir[1]);
    PE_StoreU16(dest + 0xB8u, (uint16_t)g_pe_gte.ir[2]);
    PE_StoreU16(dest + 0x7Cu, PE_LoadU16(parent + 0x7Cu));
    PE_StoreU16(dest + 0x74u, PE_LoadU16(parent + 0x74u));
    PE_StoreU16(dest + 0x76u, PE_LoadU16(parent + 0x76u));
    PE_StoreU16(dest + 0x78u, PE_LoadU16(parent + 0x78u));
}

void func_8003A6A8(pe_addr_t dest, pe_addr_t unused)
{
    (void)unused;
    if (PE_LoadU32(dest) == 0u)
        func_8003E188(dest);
}

static void pe_15240_comp_matrix(pe_addr_t actor)
{
    int16_t scale;
    unsigned col;

    PE_GTE_LoadRT(actor + 0x1E8u);
    scale = (int16_t)PE_LoadU16(actor + 0x26u);
    for (col = 0u; col < 3u; col++) {
        int16_t ir[3] = { 0, 0, 0 };

        ir[col] = scale;
        PE_GTE_SetIR(ir[0], ir[1], ir[2]);
        PE_GTE_MVMVA(CMD_RTIR);
        PE_StoreU16(actor + 0x1E8u + col * 2u, (uint16_t)g_pe_gte.ir[0]);
        PE_StoreU16(actor + 0x1EEu + col * 2u, (uint16_t)g_pe_gte.ir[1]);
        PE_StoreU16(actor + 0x1F4u + col * 2u, (uint16_t)g_pe_gte.ir[2]);
    }
    PE_GTE_LoadRT(actor + 0x1E8u);
    PE_GTE_SetV0((int16_t)(PE_LoadU32(actor + 0x1FCu) >> 16),
                 (int16_t)(PE_LoadU32(actor + 0x200u) >> 16),
                 (int16_t)(PE_LoadU32(actor + 0x204u) >> 16));
    PE_GTE_MVMVA(CMD_RTV0);
    PE_StoreU32(actor + 0x1FCu, (uint32_t)g_pe_gte.mac[0]);
    PE_StoreU32(actor + 0x200u, (uint32_t)g_pe_gte.mac[1]);
    PE_StoreU32(actor + 0x204u, (uint32_t)g_pe_gte.mac[2]);
}

int func_80015240(pe_addr_t args)
{
    pe_addr_t actor;
    pe_addr_t dest;
    uint32_t flags;

    actor = PE_LoadU32(GA_D_8009D2F0);
    PE_StoreU32(actor + 0x1FCu, (uint32_t)(int32_t)(int16_t)PE_LoadU16(actor + 0x2Au));
    PE_StoreU32(actor + 0x200u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(actor + 0x2Eu));
    PE_StoreU32(actor + 0x204u, (uint32_t)(int32_t)(int16_t)PE_LoadU16(actor + 0x32u));
    PE_StoreU16(actor + 0x1E0u, PE_LoadU16(actor + 0x38u));
    PE_StoreU16(actor + 0x1E2u, PE_LoadU16(actor + 0x3Au));
    PE_StoreU16(actor + 0x1E4u, PE_LoadU16(actor + 0x3Cu));
    func_800794C4(actor + 0x1E0u, actor + 0x1E8u);
    pe_15240_comp_matrix(actor);

    PE_StoreU8(actor + 0x23Cu, (uint8_t)PE_LoadU32(PE_LoadU32(args)));
    PE_StoreU8(actor + 0x23Du, (uint8_t)PE_LoadU32(PE_LoadU32(args + 4u)));
    PE_StoreU8(actor + 0x23Eu, (uint8_t)PE_LoadU32(PE_LoadU32(args + 8u)));

    dest = actor + 0x1B4u;
    func_8006698C(dest);
    func_80039B74(dest, PE_LoadU32(actor + 0x1B0u), 0, 1);
    func_8003A088_mode0_walk_cut(dest);
    func_8003A6A8(dest, GA_B89F8);
    func_8003B97C_empty_cut(dest, GA_BEA40);
    func_8003BCE0(dest, 1, (int)(int16_t)PE_LoadU16(GA_D_8009CDDC));

    flags = PE_LoadU32(actor + 0x98u);
    if ((flags & 0x10000000u) != 0u) {
        uint32_t cddc = PE_LoadU32(GA_D_8009CDDC) ^ 1u;

        PE_StoreU32(GA_D_8009CDDC, cddc);
        func_8003B97C_empty_cut(dest, GA_BEA40);
        func_8003BCE0(dest, 1, (int)(int16_t)PE_LoadU16(GA_D_8009CDDC));
        PE_StoreU32(GA_D_8009CDDC, PE_LoadU32(GA_D_8009CDDC) ^ 1u);
        return 1;
    }
    if ((flags & 0x08000000u) == 0u) {
        PE_StoreU32(actor + 0x98u, flags | 0x08000000u);
        PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - 20u);
        PE_StoreU32(PE_LoadU32(GA_D_8009D300) + 0x10u, 1u);
        return 0;
    }
    PE_StoreU32(actor + 0x98u, flags & 0xF7FFFFFFu);
    return 1;
}
