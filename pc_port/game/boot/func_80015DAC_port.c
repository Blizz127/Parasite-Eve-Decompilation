/*
 * PE-BTL15 / SEW19 — 15DAC room sound and default cases,
 * op 0xA (173F4), op 0x1D (17E20).
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80015DAC — 727 words 0x80015DAC..0x80016908, SHA-256
 * bd05f480…a978. D_800910A0[0xEA]. Jump table on *arg0-100,
 * sltiu 311, table at 0x800101B0. Out-of-range and table
 * targets 0x800168F4 are the epilogue `v0=1` (no stores).
 * Live type-6 first 0xEA is key 0x194; type-1 first is 0x193.
 * Both are that nop. Key 0x190 → 0x80016658 is the overlay
 * 12-byte table walk (BTL18). SEW19 restores keys300 and350..353
 * with their complete native sound call graphs. DAY1-15 wires
 * EA200/201/203/204 through original 6D2B8 and 86464/86498/864F8/86770.
 *
 * func_800173F4 — 7 words 0x800173F4..0x80017410. Zero jal.
 * *arg0 = *arg1; v0=1.
 *
 * func_80017E20 — 18 words 0x80017E20..0x80017E68. Zero jal.
 * if *arg0 != *arg1: gp+0x90 = *(D2F0)+0x9C + (*arg2)<<1.
 * Always v0=1.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern void func_80087024(void);

#define GA_EA_TABLE   0x800101B0u
#define GA_EA_NOP     0x800168F4u
#define GA_EA_190     0x80016658u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_800B0E64 0x800B0E64u
#define GA_D_800B0DB8 0x800B0DB8u
#define GA_D_800B0DB9 0x800B0DB9u
#define GA_D_800B0DFC 0x800B0DFCu
#define GA_D_800B0E00 0x800B0E00u
#define GA_D_8009D300 0x8009D300u
#define GA_EA_SLOT    0x80120FC0u

static uint32_t ea_value(pe_addr_t args,unsigned index)
{return PE_LoadU32(PE_LoadU32(args+index*4u));}

/* SEW21: two signed-byte music-channel records, 6DB48..6DC18. */
int func_8006DB48(uint32_t slot,uint32_t id,uint32_t handle,uint32_t volume)
{
    PE_StoreU8(0x800B0DB4u+slot*2u,(uint8_t)id);
    PE_StoreU8(0x800B0DB5u+slot*2u,(uint8_t)handle);
    PE_StoreU8(0x800B0DD6u+slot,(uint8_t)volume);
    if (slot<2u)
        PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)|(0x40u<<slot));
    return 0;
}

int func_8006DB9C(int32_t id)
{
    for (unsigned i=0;i<2;i++)
        if ((int8_t)PE_LoadU8(0x800B0DB4u+i*2u)==id)
            return (int8_t)PE_LoadU8(0x800B0DB5u+i*2u);
    return -1;
}

int func_8006DBE0(int32_t id)
{
    for (unsigned i=0;i<2;i++)
        if ((int8_t)PE_LoadU8(0x800B0DB4u+i*2u)==id) return (int)i;
    return -1;
}

/* Original EA205/206/207 share the channel-record update at16150. */
static int ea_music_volume(pe_addr_t args,uint32_t key)
{
    int handle=func_8006DB9C((int32_t)ea_value(args,1));
    int slot;
    if (handle==-1) return 1;
    if (key==205u)
        func_80086C1C(handle,(int)ea_value(args,2));
    else if (key==206u)
        func_80086C5C(handle,ea_value(args,2)<<1u,ea_value(args,3));
    else
        func_80086CA4(handle,ea_value(args,2)<<1u,ea_value(args,3),ea_value(args,4));
    slot=func_8006DBE0((int32_t)ea_value(args,1));
    handle=func_8006DB9C((int32_t)ea_value(args,1));
    func_8006DB48((uint32_t)slot,ea_value(args,1),(uint32_t)handle,
                  ea_value(args,key-203u));
    return 1;
}

/* SEW19: original sound requests at 162B0 and 164DC..16658.
 * The result goes through the script's output argument, including -1 when
 * the room package has no sound with the requested ID. */
static int ea_sound(pe_addr_t args,uint32_t key)
{
    pe_addr_t actor=0;
    int32_t result;
    if (key==350u) {
        PE_StoreU16(0x800B0DD0u,(uint16_t)ea_value(args,1));
        PE_StoreU8(0x800B0DCEu,(uint8_t)ea_value(args,2));
        PE_StoreU16(0x800B0DD2u,(uint16_t)ea_value(args,3));
        PE_StoreU8(0x800B0DCFu,(uint8_t)ea_value(args,4));
        return 1;
    }
    if (key==300u) {
        result=func_8006DF50(PE_LoadU32(GA_D_800B0E64),ea_value(args,1),
                            ea_value(args,2),ea_value(args,3),ea_value(args,4));
    } else {
        int x,y,z;
        if (key==353u) {
            x=(int16_t)(ea_value(args,2)>>16u);
            y=(int16_t)(ea_value(args,3)>>16u);
            z=(int16_t)(ea_value(args,4)>>16u);
        } else {
            if (key==351u) actor=PE_LoadU32(GA_D_8009D2F0);
            else if (ea_value(args,2)==0u) {
                actor=PE_LoadU32(0x8009D254u);
                if (!actor) return 1;
            } else {
                for (actor=PE_LoadU32(0x8009D20Cu);actor;actor=PE_LoadU32(actor+4u))
                    if (PE_LoadU8(actor+12u)==ea_value(args,2) &&
                        PE_LoadU8(actor+13u)==ea_value(args,3) &&
                        !(PE_LoadU32(actor+0x98u)&16u)) break;
                if (!actor) return 1;
            }
            x=(int16_t)PE_LoadU16(actor+42u);
            y=(int16_t)PE_LoadU16(actor+46u);
            z=(int16_t)PE_LoadU16(actor+50u);
        }
        result=func_8006DCE4(ea_value(args,1),0u,x,y,z);
    }
    PE_StoreU32(PE_LoadU32(args+20u),(uint32_t)result);
    return 1;
}

/* Original 16758: rewind gp+0x90 by 0x20 and store the loader
 * return at task+0x10 so the same EA word retries next tick. */
static int ea_music_yield(uint32_t loader_ret)
{
    PE_StoreU32(GA_D_8009CE00, PE_LoadU32(GA_D_8009CE00) - 0x20u);
    PE_StoreU32(PE_LoadU32(GA_D_8009D300) + 0x10u, loader_ret);
    return 0;
}

/* EA200 at 15E30; EA203 at 15ED0. */
static int ea_music_start(pe_addr_t args, uint32_t key)
{
    uint32_t blocking = ea_value(args, key == 203u ? 3u : 2u) < 1u;
    int second = key == 203u;
    int ret;
    uint32_t slot;
    int handle;

    ret = func_8006D2B8((int)ea_value(args, 1), 1, second, GA_EA_SLOT,
                        (int)blocking);
    if (ret == 1)
        return ea_music_yield((uint32_t)ret);
    slot = PE_LoadU32(GA_EA_SLOT);
    if (slot + 2u < 2u)
        return 1;
    if (key == 200u) {
        handle = func_80086464(PE_LoadU32(GA_D_800B0E00 + slot * 4u));
        PE_StoreU32(PE_LoadU32(args + 20u), (uint32_t)handle);
        func_80086C1C(0, 0x7F);
        (void)func_8006DB48(slot, ea_value(args, 1), (uint32_t)handle, 0x7Fu);
    } else {
        handle = func_800864F8(PE_LoadU32(GA_D_800B0E00 + slot * 4u),
                               ea_value(args, 2));
        PE_StoreU32(PE_LoadU32(args + 20u), (uint32_t)handle);
        (void)func_8006DB48(slot, ea_value(args, 1), (uint32_t)handle,
                            ea_value(args, 2));
    }
    return 1;
}

/* EA201 at 15F78. */
static int ea_music_stop(pe_addr_t args)
{
    int handle = func_8006DB9C((int32_t)ea_value(args, 1));
    int ret;

    if (handle == -1)
        return 1;
    func_80086498((pe_addr_t)handle);
    ret = func_8006D2B8((int)ea_value(args, 1), 0, 0, GA_EA_SLOT, 1);
    if (ret == 1)
        return ea_music_yield((uint32_t)ret);
    return 1;
}

int func_80015DAC_key190_cut(pe_addr_t args)
{
    pe_addr_t base;
    uint32_t word;
    uint32_t count;
    uint32_t i;
    uint32_t want;
    pe_addr_t ent;

    base = PE_LoadU32(GA_D_800B0E64);
    word = PE_LoadU32(base + PE_LoadU32(base + 4u) + 0x30u);
    count = word >> 22;
    if (count == 0u)
        return 1;
    want = PE_LoadU32(PE_LoadU32(args + 4u));
    ent = base + (word & 0x3FFFFFu);
    for (i = 0u; i < count; i++) {
        if ((PE_LoadU8(ent + 3u) & 0x10u) != 0u &&
            (uint32_t)PE_LoadU16(ent + 10u) == want) {
            PE_StoreU8(GA_D_800B0DB8, (uint8_t)PE_LoadU16(ent + 10u));
            PE_StoreU8(GA_D_800B0DB9, PE_LoadU8(ent + 8u));
            PE_StoreU32(GA_D_800B0DFC,
                        base + (PE_LoadU32(ent + 4u) & 0x00FFFFFFu));
            return 1;
        }
        ent += 12u;
    }
    return 1;
}

/* Original16794..16870: EA406/407 append actor animation sound events.
 * Sixteen dynamic slots follow the four built-in Aya records. EA406 uses
 * the same sound in both banks; EA407 takes a separate second-bank sound. */
static int ea_animation_sound(pe_addr_t args, uint32_t key)
{
    uint32_t count = PE_LoadU8(0x800B0CE9u);
    pe_addr_t actor, record;
    if (count >= 16u)
        return 1;
    actor = PE_LoadU32(GA_D_8009D2F0);
    record = 0x800944A8u + count * 8u;
    PE_StoreU8(record, PE_LoadU8(actor + 12u));
    PE_StoreU8(record + 1u, PE_LoadU8(actor + 13u));
    PE_StoreU8(record + 2u, (uint8_t)ea_value(args, 1));
    PE_StoreU8(record + 3u, (uint8_t)ea_value(args, 2));
    PE_StoreU16(record + 4u, (uint16_t)ea_value(args, 3));
    PE_StoreU16(record + 6u, (uint16_t)ea_value(args, key == 406u ? 3u : 4u));
    PE_StoreU8(0x800B0CE9u, (uint8_t)(PE_LoadU8(0x800B0CE9u) + 1u));
    return 1;
}

/* Original6D24C..6D2B8, reached by EA217 at162A0. */
void func_8006D24C(void)
{
    for (unsigned i=0;i<6u;i++)
        PE_StoreU8(0x800B0DB2u+i,0xFFu);
    PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)&~0xF0u);
    func_80086FF8();
}

int func_80015DAC_default_cut(pe_addr_t args)
{
    uint32_t key;
    uint32_t idx;
    pe_addr_t target;

    key = PE_LoadU32(PE_LoadU32(args));
    if (key==200u || key==203u)
        return ea_music_start(args,key);
    if (key==201u)
        return ea_music_stop(args);
    if (key==204u) {
        (void)func_80086770(ea_value(args,1));
        return 1;
    }
    if (key>=205u && key<=207u)
        return ea_music_volume(args,key);
    /* Original16384/164CC: EA305 scales duration before the audio leaf. */
    if (key==305u) {
        (void)func_800868AC(ea_value(args,1)<<1u,ea_value(args,2));
        return 1;
    }
    if (key==314u) {
        func_80087024();
        return 1;
    }
    if (key==300u || (key>=350u && key<=353u))
        return ea_sound(args,key);
    if (key == 217u) {
        func_8006D24C();
        return 1;
    }
    if (key == 406u || key == 407u)
        return ea_animation_sound(args, key);
    idx = key - 100u;
    if (idx >= 311u)
        return 1;
    target = PE_LoadU32(GA_EA_TABLE + idx * 4u);
    if (target == GA_EA_NOP)
        return 1;
    if (target == GA_EA_190)
        return func_80015DAC_key190_cut(args);
    /* Other 15DAC cases are not this cut. */
    return 1;
}

int func_800173F4(pe_addr_t args)
{
    PE_StoreU32(PE_LoadU32(args), PE_LoadU32(PE_LoadU32(args + 4u)));
    return 1;
}

int func_80017E20(pe_addr_t args)
{
    uint32_t left;
    uint32_t right;
    pe_addr_t actor;
    uint32_t off;

    left = PE_LoadU32(PE_LoadU32(args));
    right = PE_LoadU32(PE_LoadU32(args + 4u));
    if (left != right) {
        actor = PE_LoadU32(GA_D_8009D2F0);
        off = PE_LoadU32(PE_LoadU32(args + 8u));
        PE_StoreU32(GA_D_8009CE00,
                    PE_LoadU32(actor + 0x9Cu) + (off << 1));
    }
    return 1;
}
