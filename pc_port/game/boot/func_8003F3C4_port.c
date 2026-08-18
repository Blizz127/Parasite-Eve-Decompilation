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
 * jal E01BC @ 3F578. Live E21A4<=0 early-outs. E026C/E03A0
 * are not this cut.
 * 70E54 live prefix @ 3F590: DrawSync(0), 42FE8 out
 * (gp+0x168!=6), VSync(2), ResetGraph(1),
 * PutDispEnv(BCE80+20*CDDC), 6EC08 status.
 * Live 6EC08==0 and B0CD8&0x200==0 runs 754E4
 * software (75EE0 + 71A34 memcpy) then flips
 * guest CDDC. 76C34(76B98) is not this cut.
 * 3F5EC VSync(2) then 6A0E8 @ 3F640. Live D1A0&0x10
 * early-out. 66C7C/6A25C are not live.
 * 3F684 D1C4==D280 loops to 3EB04 in retail; this cut
 * is one pass per 1220C tick. Dest-change (D1C4!=D280)
 * takes 74DC0 / 87024 / 3DFC8(1) / 696F0 live tail
 * then D1A0|=0x40 / B0CD8|=2.
 *
 * PE-BTL102: andi 0x100 after 35558 @ 3F500 is not a
 * function return. bnez 3F5F4 skips overlay/draw/6A0E8
 * and still falls into dest-change when D280 moved
 * (player-death 6A25C). 3F624 pad-combo 6A25C is not
 * this cut. 1220C then VSync(0)/SetDispMask(0),
 * B0CD8 &= ~0x100, j 1224C (outer 6A5BC restart).
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
extern unsigned int D_8009D1A0;
extern void func_80068CE0(void);
extern void func_80087024(void);
extern void func_800661A4(void);
extern void func_800661CC(void);
extern int func_80074A44(int mode);
extern void func_800754E4(pe_addr_t ot, pe_addr_t env);

void func_8003DFC8(int a0)
{
    (void)a0;
}

void func_800696F0(void)
{
    unsigned int i;
    pe_addr_t table;
    pe_addr_t slot;
    pe_addr_t p;

    /* D1A0&0x80 first-half jalr walk is not this cut.
     * Live 0x4000 skips it and still runs the 6984C tail. */
    table = PE_LoadU32(0x800942E0u);
    if (table != 0u && PE_RangeIsRam(table, 0x55u * 4u)) {
        for (i = 8u; i < 0x55u; i++) {
            slot = PE_LoadU32(table + i * 4u);
            if (slot != 0u)
                PE_StoreU32(slot, 0u);
        }
    }
    p = 0x800E10BCu;
    if (PE_RangeIsRam(p, (0x68u - 0x1Eu) * 4u)) {
        for (i = 0x1Eu; i < 0x68u; i++) {
            slot = PE_LoadU32(p);
            if (slot != 0u)
                PE_StoreU32(slot, 0u);
            p += 4u;
        }
    }
}

void func_8006A0E8(void)
{
    if ((PE_LoadU32(0x800B0CD8u) & 0x200u) != 0u)
        return;
    if ((D_8009D1A0 & 0x10u) == 0u)
        return;
    /* PutDrawEnv / SetDrawArea body is not this cut. */
}

/*
 * PE-BTL92 — func_800E01BC record walk.
 * 44 words 0x800E01BC..0x800E026C, SHA-256 87f953d4…7cd8.
 * lh E21A4, lw E2800, blez return. Live count is 0.
 * E026C/E03A0 are not this cut.
 */
void func_800E01BC(void)
{
    if ((int16_t)PE_LoadU16(0x800E21A4u) <= 0)
        return;
    /* count>0 walk is not this cut. */
}

int func_8006EC08(void)
{
    int8_t a;
    int16_t h;
    int8_t b;

    a = (int8_t)PE_LoadU8(0x800B0DBAu);
    if (a == 0)
        return 0;
    h = (int16_t)PE_LoadU16(0x800B0DBCu);
    if (h <= 0)
        return 0;
    b = (int8_t)PE_LoadU8(0x800B0DBBu);
    if (b == 0)
        return 1;
    return 2;
}

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
 * 3F074 @ 3F088 jals 6B4F8(D280) every tick. Host still
 * change-gates (not a retail skip). Dest-ready is 6B35C +
 * 6B4F8 + 6BECC==0 + 6C5BC==0 + 125E0. Does not write
 * mode 7/9/10. Type-1 clip is not manufactured.
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
    if (func_8003F074_dest_ready_cut(D_8009D280) != 0)
        pe_3f3c4_loaded_dest = D_8009D280;
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
    uint32_t cddc;
    uint32_t dest0;
    uint32_t d1a0;

    dest0 = D_8009D280;
    pe_3f3c4_ce90_once();
    pe_3f3c4_dest_change();
    pe_3f3c4_host_pad();
    func_8003EB04();
    func_80065400();
    func_80035558_walk_cut();
    bits = PE_LoadU32(0x800B0CD8u);
    /* 3F500 andi 0x100 / bnez 3F5F4: skip draw, not jr. */
    if ((bits & 0x100u) == 0u) {
        /* 3F50C: live 6EC08==0 skips overlay 122040/121A00/6E60C. */
        (void)func_8006EC08();
        bits = PE_LoadU32(0x800B0CD8u);
        if ((bits & 0x200u) != 0u)
            return;
        func_80068CE0();
        func_80037870();
        func_800661A4();
        func_800E01BC();
        func_800661CC();
        func_80068E24();
        func_80074DC0(0);
        func_80073A44(2);
        func_80074A44(1);
        func_800755F0(PE_Translate(
            0x800BCE80u + PE_LoadU32(0x8009CDDCu) * 20u, 0x14u));
        if (func_8006EC08() == 0 &&
            (PE_LoadU32(0x800B0CD8u) & 0x200u) == 0u) {
            /* 754E4 DrawOTagEnv(B0E38[CDDC]+0x3FFC, BCDC8+92*CDDC).
             * 76C34 GPU enqueue is not this cut. */
            cddc = PE_LoadU32(0x8009CDDCu);
            func_800754E4(PE_LoadU32(0x800B0E38u + cddc * 4u) + 0x3FFCu,
                          0x800BCDC8u + 92u * cddc);
            PE_StoreU32(0x8009CDDCu, cddc == 0u);
        }
        func_80073A44(2);
        func_8006A0E8();
    }
    /* 1220C stores D1C4=D280 before the jal. Tests that call
     * 3F3C4 directly may have a stale D1C4; dest0 is that entry
     * snapshot. Exit epilogue only if D280 changed this tick. */
    if (dest0 != D_8009D280) {
        func_80074DC0(0);
        func_80087024();
        func_8003DFC8(1);
        func_800696F0();
        /* 3F6F0: D1A0 = (D1A0|0x40) & ~0x3800; B0CD8 |= 2, &=~0x800. */
        d1a0 = (D_8009D1A0 | 0x40u) & ~0x3800u;
        D_8009D1A0 = d1a0;
        PE_StoreU32(0x8009D1A0u, d1a0);
        bits = (PE_LoadU32(0x800B0CD8u) | 2u) & ~0x800u;
        PE_StoreU32(0x800B0CD8u, bits);
        if ((bits & 0x200u) != 0u)
            PE_StoreU32(0x800B0CD8u, (bits | 2u) & 0xFFFF7DFFu);
    }
}
