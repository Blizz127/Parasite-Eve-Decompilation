/*
 * PE-BTL38 — func_8003F3C4 field-tick named cut (translated
 * retail sites, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 227 words 0x8003F3C4..0x8003F750. Sole TEXT caller 1220C @
 * 0x800123D8. 1220C stores D1C4=D280 immediately before the
 * jal, so the 3F3E8 equality holds on entry.
 *
 * This cut is the live mailbox/fade/message sites only:
 *   3F074 371B0 CE90 once when CE90==0 (not 34FC4/125E0)
 *   host pad idle / state-2 Cross before 3EB04
 *   jal 3EB04 @ 0x8003F40C
 *   jal 65400 @ 0x8003F4E8
 *   jal 35558 @ 0x8003F4F0
 *   jal 68CE0 @ 0x8003F560
 *   jal 37870 @ 0x8003F568
 *   jal 661A4 @ 0x8003F570 (GTE OFX/OFY from BCF94/96)
 *   jal 661CC @ 0x8003F580 (SetGeomOffset 160,112)
 *   jal 68E24 @ 0x8003F588
 *   when (B0CD8&0x100)==0 and (B0CD8&0x200)==0
 *
 * E01BC overlay between 661A4 and 661CC is not this cut.
 * 70E54 live prefix @ 3F590: DrawSync(0), 42FE8 out
 * (gp+0x168!=6), VSync(2), ResetGraph(1),
 * PutDispEnv(BCE80+20*CDDC). 6EC08+ is not this cut.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BE9A2 0x800BE9A2u
#define GA_D_800BCEA8 0x800BCEA8u
#define GA_D_8009CE90 0x8009CE90u
#define GA_D_800B162C 0x800B162Cu
#define GA_OVERLAY    0x800B0CD8u
#define REC_STRIDE    56u

extern unsigned int D_8009D280;
extern void func_80068CE0(void);
extern void func_800661A4(void);
extern void func_800661CC(void);
extern int func_80074A44(int mode);

static uint32_t pe_3f3c4_loaded_dest;

static int pe_3f3c4_kseg(pe_addr_t p)
{
    return p >= 0x80000000u && p < 0x80200000u;
}

static int pe_3f3c4_message_state2(void)
{
    unsigned int i;

    for (i = 0; i < 4u; i++) {
        if (PE_LoadU8(GA_D_800BCEA8 + i * REC_STRIDE) == 2u)
            return 1;
    }
    return 0;
}

/*
 * 3F074 @ 3F3D4 is the first jal. Full 3F074 also jals 34FC4
 * (wipes D20C) and 125E0 every call, so this cut is the 3F244
 * 371B0 site only, and only while CE90 is still 0.
 * 6B4F8_12574_publish_cut runs first when overlay+0x18C is
 * already a KSEG chunk2 dest and B162C is still 0.
 */
static void pe_3f3c4_ce90_once(void)
{
    pe_addr_t chunk;
    pe_addr_t stream;

    if (PE_LoadU32(GA_D_8009CE90) != 0u)
        return;
    chunk = PE_LoadU32(GA_OVERLAY + 0x18Cu);
    if (pe_3f3c4_kseg(chunk) && PE_LoadU32(GA_D_800B162C) == 0u)
        func_8006B4F8_12574_publish_cut();
    stream = func_8003F074_371b0_a0();
    if (pe_3f3c4_kseg(stream))
        func_800371B0(stream);
    pe_3f3c4_loaded_dest = D_8009D280;
}

/*
 * 3F074 @ 3F088 jals 6B4F8(D280) every tick. This cut runs that
 * dest-token load only when D280 changes after the CE90-once
 * publish, and only when the three overlay dest pointers are
 * already KSEG. Dest-change then runs the 3F074 pool
 * rebuild (34FC4 / 1266C / 125E0). Not every tick.
 */
static void pe_3f3c4_dest_change(void)
{
    pe_addr_t dest0;
    pe_addr_t dest1;
    pe_addr_t dest2;

    if (D_8009D280 == 0u || D_8009D280 == pe_3f3c4_loaded_dest)
        return;
    dest0 = PE_LoadU32(GA_OVERLAY + 0x194u);
    dest1 = PE_LoadU32(GA_OVERLAY + 0x168u);
    dest2 = PE_LoadU32(GA_OVERLAY + 0x18Cu);
    if (!pe_3f3c4_kseg(dest0) || !pe_3f3c4_kseg(dest1) ||
        !pe_3f3c4_kseg(dest2))
        return;
    if (PE_LoadU32(0x800B0DD8u) == 0u)
        return;
    if (func_8006B4F8_dest_load_cut(D_8009D280) != 0) {
        pe_3f3c4_loaded_dest = D_8009D280;
        func_80034FC4();
        func_8001266C();
        func_800125E0();
    }
}

/*
 * Host pad before 3EB04. Retail idle raw is 0xFFFF. Host RAM
 * zero after reset is uninitialized, not all-buttons-pressed.
 * State-2 Cross is deterministic confirm (Watch cursor 0), not
 * a planted D1F4 for type-5 0x0D.
 */
static void pe_3f3c4_host_pad(void)
{
    uint16_t raw;

    if (pe_3f3c4_message_state2())
        raw = 0xBFFFu;
    else {
        raw = PE_LoadU16(GA_D_800BE9A2);
        if (raw == 0u)
            raw = 0xFFFFu;
    }
    PE_StoreU16(GA_D_800BE9A2, raw);
}

void func_8003F3C4(void)
{
    uint32_t bits;

    pe_3f3c4_ce90_once();
    pe_3f3c4_dest_change();
    pe_3f3c4_host_pad();
    func_8003EB04();
    func_80065400();
    func_80035558_walk_cut();
    bits = PE_LoadU32(0x800B0CD8u);
    if ((bits & 0x100u) != 0u)
        return;
    if ((bits & 0x200u) != 0u)
        return;
    func_80068CE0();
    func_80037870();
    func_800661A4();
    func_800661CC();
    func_80068E24();
    func_80074DC0(0);
    func_80073A44(2);
    func_80074A44(1);
    func_800755F0(PE_Translate(
        0x800BCE80u + PE_LoadU32(0x8009CDDCu) * 20u, 0x14u));
}
