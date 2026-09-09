/*
 * PE-BTL21 — live type-1 0xE1 / 0x84 / 0x88 (translated retail,
 * not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_8001A374 — 7 words 0x8001A374..0x8001A390. sb *arg0 →
 * D_800BCFFC. v0=1. Live imm 0x54.
 *
 * func_80018E84 — 12 words 0x80018E84..0x80018EB4. sh *arg0 →
 * D_800BD020, sh *arg1 → D_800BD022. v0=1. Live 0x800, 0x800.
 *
 * func_80018F54 — 8 words 0x80018F54..0x80018F74.
 * D_800BCFEE &= ~0x40. v0=1.
 *
 * After these, type-1 0x14/0x02 are already ported and yield.
 * Next visit is 0x08/1735C (type-0 spawn) — not this cut.
 *
 * PE-BTL31 — opcode 0x86 / 18EE0 / 66C7C.
 * 18EE0 11 words 0x80018EE0..0x80018F0C, SHA-256 d042e2ad…0c.
 * D_800910A0[0x86]. jal 66C7C(lhu *arg0); v0=1.
 * Live type-1 persist[0x4A]!=39: imm 0x1E.
 *
 * 66C7C 27 words 0x80066C7C..0x80066CE8, SHA-256 ce26b53b…ef0.
 * Zero jal. v0=0. Snapshot CFE8/EA/EC, sb 6→CFEE, zero those
 * three, CFF6=a0, CFF8=0, CFF0/F2/F4=snapshot.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCFE8 0x800BCFE8u
#define GA_D_800BCFEA 0x800BCFEAu
#define GA_D_800BCFEC 0x800BCFECu
#define GA_D_800BCFEE 0x800BCFEEu
#define GA_D_800BCFF0 0x800BCFF0u
#define GA_D_800BCFF2 0x800BCFF2u
#define GA_D_800BCFF4 0x800BCFF4u
#define GA_D_800BCFF6 0x800BCFF6u
#define GA_D_800BCFF8 0x800BCFF8u

int func_8001A374(pe_addr_t args)
{
    PE_StoreU8(0x800BCFFCu, (uint8_t)PE_LoadU32(PE_LoadU32(args)));
    return 1;
}

/* SEW22: E2, original1A390..1A3FC (27 words). Test a fixed-point
 * position against an embedded script polygon; arg3 is a halfword offset
 * from the current actor's script base, arg2 is the vertex count. */
int func_8001A390(pe_addr_t args)
{
    pe_addr_t actor=PE_LoadU32(0x8009D2F0u);
    pe_addr_t polygon=PE_LoadU32(actor+0x9Cu)+
                     (PE_LoadU32(PE_LoadU32(args+12u))<<1u);
    int result=func_8001CAB0((int32_t)PE_LoadU32(PE_LoadU32(args)),
                            (int32_t)PE_LoadU32(PE_LoadU32(args+4u)),
                            polygon,PE_LoadU16(PE_LoadU32(args+8u)));
    PE_StoreU32(PE_LoadU32(args+16u),(uint32_t)result);
    return 1;
}

int func_80018E84(pe_addr_t args)
{
    PE_StoreU16(0x800BD020u, (uint16_t)PE_LoadU32(PE_LoadU32(args)));
    PE_StoreU16(0x800BD022u, (uint16_t)PE_LoadU32(PE_LoadU32(args + 4u)));
    return 1;
}

int func_80018F54(pe_addr_t args)
{
    (void)args;
    PE_StoreU8(0x800BCFEEu, (uint8_t)(PE_LoadU8(0x800BCFEEu) & 0xBFu));
    return 1;
}

int func_80066C7C(unsigned int a0)
{
    uint16_t s0;
    uint16_t s1;
    uint16_t s2;

    s0 = PE_LoadU16(GA_D_800BCFE8);
    s1 = PE_LoadU16(GA_D_800BCFEA);
    s2 = PE_LoadU16(GA_D_800BCFEC);
    PE_StoreU8(GA_D_800BCFEE, 6u);
    PE_StoreU16(GA_D_800BCFE8, 0u);
    PE_StoreU16(GA_D_800BCFEA, 0u);
    PE_StoreU16(GA_D_800BCFEC, 0u);
    PE_StoreU16(GA_D_800BCFF6, (uint16_t)a0);
    PE_StoreU16(GA_D_800BCFF8, 0u);
    PE_StoreU16(GA_D_800BCFF0, s0);
    PE_StoreU16(GA_D_800BCFF2, s1);
    PE_StoreU16(GA_D_800BCFF4, s2);
    return 0;
}

int func_80018EE0(pe_addr_t args)
{
    func_80066C7C(PE_LoadU16(PE_LoadU32(args)));
    return 1;
}

/*
 * PE-BTL33 — opcode 0xAA overlay bit 19618.
 * 8 words 0x80019618..0x80019638, SHA-256 5eb9bc4a…c537.
 * D_800910A0[0xAA]. Zero jal. D_800B0CD8 |= 0x2000; v0=1.
 * Live type-0 after 0xFF 0x04 / 0x01.
 */
int func_80019618(pe_addr_t args)
{
    (void)args;
    PE_StoreU32(0x800B0CD8u, PE_LoadU32(0x800B0CD8u) | 0x2000u);
    return 1;
}
