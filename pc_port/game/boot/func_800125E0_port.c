/*
 * PE-BTL9 — func_80012574 list publish/rebase and func_800125E0
 * DrawSync + 35038 walk (translated retail, not matching src/).
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80012574 — 27 words 0x80012574..0x800125E0, SHA-256
 * bf4a0017…e124. Sole TEXT caller 6B4F8 @ 0x8006B8BC:
 *   a0 = s4 + (lw(s4 + (hdr+0x14 & 0x003FFFFF)) + 4 & 0x00FFFFFF)
 * then sw v0 → overlay+0x944. Always sw a0 → 0x94($gp)=D_8009CE04.
 * If *a0 > 0x80000000 the list is already relocated and a0 is
 * returned. Else *a0 += a0 and each of lw(+4) words at +8 is
 * likewise a0-relative. Guest 0 maps Kuseg 0x80000000 (host
 * exception-vector area is APPROXIMATION zeros).
 *
 * func_800125E0 — 35 words 0x800125E0..0x8001266C, SHA-256
 * 7c30399d…434c. Sole TEXT caller 3F074 @ 0x8003F27C after 371B0.
 *
 *   DrawSync(0)
 *   header = *(gp+0x94)
 *   desc = *header
 *   count = lbu(desc)
 *   for i in 0..count-1:
 *       func_80035038(desc + 1 + i*2, a1=0, a2=1)
 *
 * DrawSync is the already-shimmed HostFB_DrawSync. It is not battle
 * rendering and not M2.
 *
 * func_80035038 — 328 words 0x80035038..0x80035558. Live 125E0
 * arguments are a1=0, a2=1. First instruction loads 0x53C($gp) =
 * D_8009D2AC (freelist head; 34F10 zeros it). If that word is 0,
 * retail jumps to the epilogue with v0=0 and constructs nothing.
 * That empty-freelist return is the cut implemented here. A nonempty
 * pool is a later dependency (ctor body + 2F76C/12700/1A680/362B8/
 * 3D050/6698C/3D834). Do not invent actors when the pool is empty.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009CE04 0x8009CE04u
#define GA_D_8009D2AC 0x8009D2ACu

static pe_addr_t pe_kseg0(pe_addr_t addr)
{
    return 0x80000000u | (addr & 0x1FFFFFu);
}

pe_addr_t func_80012574(pe_addr_t a0)
{
    pe_addr_t base;
    uint32_t first;
    unsigned int i;
    unsigned int count;

    base = pe_kseg0(a0);
    first = PE_LoadU32(base);
    PE_StoreU32(GA_D_8009CE04, a0);
    if (first > 0x80000000u)
        return a0;

    count = PE_LoadU32(base + 4u);
    PE_StoreU32(base, a0 + first);
    for (i = 0; i < count; i++) {
        PE_StoreU32(base + 8u + i * 4u,
                    a0 + PE_LoadU32(base + 8u + i * 4u));
    }
    return a0;
}

pe_addr_t func_80035038(pe_addr_t desc, pe_addr_t parent, unsigned int a2)
{
    (void)desc;
    (void)parent;
    (void)a2;
    if (PE_LoadU32(GA_D_8009D2AC) == 0u)
        return 0;
    /* Nonempty freelist is not this cut. */
    return 0;
}

void func_800125E0(void)
{
    pe_addr_t header;
    pe_addr_t desc;
    unsigned int count;
    unsigned int i;

    func_80074DC0(0);
    header = pe_kseg0(PE_LoadU32(GA_D_8009CE04));
    desc = pe_kseg0(PE_LoadU32(header));
    count = PE_LoadU8(desc);
    for (i = 0; i < count; i++)
        (void)func_80035038(desc + 1u + i * 2u, 0u, 1u);
}
