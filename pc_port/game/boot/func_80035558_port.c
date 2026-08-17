/*
 * PE-BTL11 — func_80035558 D20C walk cut, type!=0 vtable 35E04,
 * and 361F4 slot publish (translated retail, not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80035558 — 459 words 0x80035558..0x80035C84, SHA-256
 * 7352fc04…ba83. Sole TEXT caller 3F3C4 @ 0x8003F4F0 (field
 * tick; 3F3C4 sole caller 0x800123D8). This cut is the prologue
 * only:
 *
 *   if (D_8009D1A0 & 4) return;   # ROM skips walk and continues
 *   for actor in D_8009D20C via +4:
 *       jalr actor+0x190 (a0=actor)
 *
 * Known jalr target 0x80035E04 (types 1–9) is ported below.
 * Type0 0x80035C84 and the remaining 29 jals (including 6C5BC)
 * are not this cut. Not M2: this is the field-tick actor walk.
 *
 * func_80035E04 — 83 words 0x80035E04..0x80035F50. If D1A0 bit
 * 0x100, only 361F4. Else snapshot +0x28/+0x38 into +0x40/+0x50,
 * jal 361F4, then if +0x98 bit 1 integrate motion. Live 35038
 * empty-+0x1AC leaves +0x98 bit 1 clear.
 *
 * func_800361F4 — 24 words 0x800361F4..0x80036254. sw actor →
 * gp+0x580 (D_8009D2F0), then three words at +0xA0 into
 * D_8009D300. Nonempty slots jal 17018. Live 35038 zeros
 * +0xA0/+0xA4 and stores the 12700 task at +0xA8.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009D300 0x8009D300u
#define GA_VT_35E04   0x80035E04u
#define GA_VT_35C84   0x80035C84u

extern unsigned int D_8009D1A0;

void func_800361F4(pe_addr_t actor)
{
    unsigned int i;
    uint32_t slot;

    PE_StoreU32(GA_D_8009D2F0, actor);
    for (i = 0; i < 3u; i++) {
        slot = PE_LoadU32(actor + 0xA0u + i * 4u);
        PE_StoreU32(GA_D_8009D300, slot);
        if (slot != 0u)
            func_80017018();
    }
}

void func_80035E04(pe_addr_t actor)
{
    if (D_8009D1A0 & 0x100u) {
        func_800361F4(actor);
        return;
    }

    PE_StoreU32(actor + 0x40u, PE_LoadU32(actor + 0x28u));
    PE_StoreU32(actor + 0x44u, PE_LoadU32(actor + 0x2Cu));
    PE_StoreU32(actor + 0x48u, PE_LoadU32(actor + 0x30u));
    PE_StoreU16(actor + 0x50u, PE_LoadU16(actor + 0x38u));
    PE_StoreU16(actor + 0x52u, PE_LoadU16(actor + 0x3Au));
    PE_StoreU16(actor + 0x54u, PE_LoadU16(actor + 0x3Cu));
    func_800361F4(actor);

    if ((PE_LoadU32(actor + 0x98u) & 2u) == 0u)
        return;

    PE_StoreU32(actor + 0x68u,
                PE_LoadU32(actor + 0x68u) + PE_LoadU32(actor + 0x88u));
    PE_StoreU32(actor + 0x6Cu,
                PE_LoadU32(actor + 0x6Cu) + PE_LoadU32(actor + 0x8Cu));
    PE_StoreU32(actor + 0x70u,
                PE_LoadU32(actor + 0x70u) + PE_LoadU32(actor + 0x90u));
    PE_StoreU32(actor + 0x68u,
                PE_LoadU32(actor + 0x68u) + PE_LoadU32(actor + 0x78u));
    PE_StoreU32(actor + 0x6Cu,
                PE_LoadU32(actor + 0x6Cu) + PE_LoadU32(actor + 0x7Cu));
    PE_StoreU32(actor + 0x70u,
                PE_LoadU32(actor + 0x70u) + PE_LoadU32(actor + 0x80u));
    PE_StoreU32(actor + 0x28u,
                PE_LoadU32(actor + 0x28u) + PE_LoadU32(actor + 0x68u));
    PE_StoreU32(actor + 0x2Cu,
                PE_LoadU32(actor + 0x2Cu) + PE_LoadU32(actor + 0x6Cu));
    PE_StoreU32(actor + 0x30u,
                PE_LoadU32(actor + 0x30u) + PE_LoadU32(actor + 0x70u));
    PE_StoreU32(actor + 0x28u,
                PE_LoadU32(actor + 0x28u) + PE_LoadU32(actor + 0x58u));
    PE_StoreU32(actor + 0x2Cu,
                PE_LoadU32(actor + 0x2Cu) + PE_LoadU32(actor + 0x5Cu));
    PE_StoreU32(actor + 0x30u,
                PE_LoadU32(actor + 0x30u) + PE_LoadU32(actor + 0x60u));
}

void func_80035558_walk_cut(void)
{
    pe_addr_t actor;
    pe_addr_t fn;

    if (D_8009D1A0 & 4u)
        return;

    actor = PE_LoadU32(GA_D_8009D20C);
    while (actor != 0u) {
        fn = PE_LoadU32(actor + 0x190u);
        if (fn == GA_VT_35E04)
            func_80035E04(actor);
        (void)GA_VT_35C84;
        actor = PE_LoadU32(actor + 4u);
    }
}
