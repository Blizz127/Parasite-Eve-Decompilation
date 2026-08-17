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
 * func_8001266C — 37 words 0x8001266C..0x80012700. Sole TEXT caller
 * 3F074 @ 0x8003F0B8, immediately after 34FC4. Links 72 task blocks
 * at D_8009D310 stride 0x2C through +0x24, publishes the head to
 * gp+0x8C (D_8009CDFC), clears gp+0x590, nulls the last +0x24,
 * zeros 64 words at D_800B6A80. 12700 pops this list.
 *
 * func_80035038 — 328 words 0x80035038..0x80035558. Live 125E0
 * arguments are a1=0, a2=1. Empty D_8009D2AC returns 0. Nonempty
 * a1=0 path: pop, insert at D_8009D20C, init fields, sb desc[0/1]
 * at +0x0C/+0x0D, +0x1AC = D_800B0E70[type] when type<10 (EXE BSS
 * 0 unless overlay published), jal 12700(entry from *D_800B161C +
 * type*4 + 8). If +0x1AC==0, OR 0xE0 into +0x98 and return the
 * actor. Type 0 publishes D254 and sets +0x20=0x10000; jal 2F76C
 * (5218C/51980/51E64) is not this cut. +0x1AC!=0 (1A680/362B8/
 * 3D050) is not this cut.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_8009CE04 0x8009CE04u
#define GA_D_8009CDFC 0x8009CDFCu
#define GA_D_8009D300 0x8009D300u
#define GA_D_8009D310 0x8009D310u
#define GA_D_8009DF68 0x8009DF68u
#define GA_D_800B6A80 0x800B6A80u
#define GA_D_8009D2AC 0x8009D2ACu
#define GA_D_8009D20C 0x8009D20Cu
#define GA_D_8009D254 0x8009D254u
#define GA_D_8009D224 0x8009D224u
#define GA_D_8009D2A6 0x8009D2A6u
#define GA_D_800915DC 0x800915DCu
#define GA_D_800B0E70 0x800B0E70u
#define GA_D_800B161C 0x800B161Cu
#define TASK_STRIDE   0x2Cu
#define TASK_COUNT    0x47u

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

void func_8001266C(void)
{
    unsigned int i;
    pe_addr_t next;

    PE_StoreU32(GA_D_8009D300, 0u);
    PE_StoreU32(GA_D_8009CDFC, GA_D_8009D310);
    next = GA_D_8009D310 + TASK_STRIDE;
    for (i = 0; i < TASK_COUNT; i++) {
        PE_StoreU32(GA_D_8009D310 + 0x24u + i * TASK_STRIDE, next);
        next += TASK_STRIDE;
    }
    PE_StoreU32(GA_D_8009DF68, 0u);
    for (i = 0; i < 0x40u; i++)
        PE_StoreU32(GA_D_800B6A80 + i * 4u, 0u);
}

pe_addr_t func_80035038(pe_addr_t desc, pe_addr_t parent, unsigned int a2)
{
    pe_addr_t actor;
    pe_addr_t old;
    pe_addr_t entry;
    pe_addr_t task;
    unsigned int type;
    unsigned int i;
    uint32_t id;

    (void)a2;
    actor = PE_LoadU32(GA_D_8009D2AC);
    if (actor == 0u)
        return 0;

    PE_StoreU32(GA_D_8009D2AC, PE_LoadU32(actor + 4u));

    if (parent == 0u) {
        /* Live 125E0 a1=0: insert at D_8009D20C head. */
        old = PE_LoadU32(GA_D_8009D20C);
        if (old != 0u)
            PE_StoreU32(old + 8u, actor);
        PE_StoreU32(actor + 8u, 0u);
        PE_StoreU32(GA_D_8009D20C, actor);
        PE_StoreU32(actor + 4u, old);
    } else {
        old = PE_LoadU32(parent + 4u);
        PE_StoreU32(actor + 8u, parent);
        PE_StoreU32(actor + 4u, old);
        PE_StoreU32(parent + 4u, actor);
        if (old != 0u)
            PE_StoreU32(old + 8u, actor);
    }

    PE_StoreU32(actor + 0x88u, 0u);
    PE_StoreU32(actor + 0x8Cu, 0x4000u);
    PE_StoreU32(actor + 0x90u, 0u);
    PE_StoreU32(actor + 0x68u, 0u);
    PE_StoreU32(actor + 0x6Cu, 0u);
    PE_StoreU32(actor + 0x70u, 0u);
    PE_StoreU32(actor + 0x78u, 0u);
    PE_StoreU32(actor + 0x7Cu, 0u);
    PE_StoreU32(actor + 0x80u, 0u);
    PE_StoreU32(actor + 0x58u, 0u);
    PE_StoreU32(actor + 0x5Cu, 0u);
    PE_StoreU32(actor + 0x60u, 0u);

    type = PE_LoadU8(desc);
    PE_StoreU32(actor + 0x190u, PE_LoadU32(GA_D_800915DC + type * 8u));
    PE_StoreU32(actor + 0x194u, PE_LoadU32(GA_D_800915DC + 4u + type * 8u));
    if (type == 0u) {
        PE_StoreU32(GA_D_8009D254, actor);
        PE_StoreU32(actor + 0x20u, 0x10000u);
        /* jal 2F76C: 5218C/51980/51E64 not this cut. */
    } else {
        PE_StoreU32(actor + 0x20u, 0x50000u);
        PE_StoreU32(actor, 0u);
    }

    PE_StoreU8(actor + 0x0Cu, (uint8_t)type);
    PE_StoreU8(actor + 0x0Du, PE_LoadU8(desc + 1u));
    if (type < 10u)
        PE_StoreU32(actor + 0x1ACu, PE_LoadU32(GA_D_800B0E70 + type * 4u));
    else
        PE_StoreU32(actor + 0x1ACu, 0u);

    id = PE_LoadU32(GA_D_8009D224);
    PE_StoreU32(actor + 0x98u, 0x10000000u);
    PE_StoreU32(actor + 0x1Cu, 0x10000u);
    PE_StoreU16(actor + 0x10u, 0xC8u);
    PE_StoreU32(actor + 0x1B0u, 0u);
    PE_StoreU32(actor + 0x14u, 0u);
    PE_StoreU32(actor + 0x18Cu, 0u);
    PE_StoreU32(actor + 0x1A4u, 0u);
    PE_StoreU32(actor + 0x19Cu, 0u);
    PE_StoreU32(actor + 0x1A0u, 0u);
    PE_StoreU32(actor + 0x198u, 0u);
    PE_StoreU16(actor + 0x26u, 0x1000u);
    PE_StoreU32(GA_D_8009D224, id + 1u);
    PE_StoreU16(actor + 0x24u, (uint16_t)id);
    PE_StoreU8(actor + 0x0Eu, 0u);
    PE_StoreU8(actor + 0x27Cu, 0u);
    PE_StoreU8(actor + 0x27Du, 0x80u);
    for (i = 0; i < 0x38u; i++)
        PE_StoreU32(actor + 0xACu + i * 4u, 0u);
    for (i = 0; i < 3u; i++)
        PE_StoreU32(actor + 0xA0u + i * 4u, 0u);

    entry = PE_LoadU32(pe_kseg0(PE_LoadU32(GA_D_800B161C) + type * 4u + 8u));
    PE_StoreU32(actor + 0x9Cu, entry);
    task = func_80012700(entry, 0u);
    PE_StoreU32(actor + 0xA8u, task);
    PE_StoreU16(actor + 0x38u, 0u);
    PE_StoreU16(actor + 0x3Au, 0u);
    PE_StoreU16(actor + 0x3Cu, 0u);
    PE_StoreU16(GA_D_8009D2A6, (uint16_t)(PE_LoadU16(GA_D_8009D2A6) + 1u));

    if (PE_LoadU32(actor + 0x1ACu) == 0u) {
        PE_StoreU32(actor + 0x98u, PE_LoadU32(actor + 0x98u) | 0xE0u);
        return actor;
    }
    /* +0x1AC != 0: 1A680 / 362B8 / 3D050 not this cut. */
    return actor;
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
