/* Retail command-selection initialization (20F18, 374E8, 43240) and
 * effect cleanup (70064, 6FC18). Authority: 11718.s, 27C6C.s, 5F484.s.
 * The nine destruction callbacks used by this path are matching C leaves:
 * C7DC4, C8F08, C9C00, CA798, CD960, CCF80, CE1DC, CBFA4, D4850.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static pe_addr_t effect_slot(uint32_t index)
{
    return index<11u ? PE_LoadU32(0x800942E4u)+index*0xA0Cu :
                       PE_LoadU32(0x800942E8u)+(index-11u)*0x10Cu;
}

static void clear_effect_slot(pe_addr_t slot)
{
    if (PE_LoadU8(slot+1u)==0x72u) {
        unsigned i;
        for (i=0;i<7;i++) PE_StoreU32(0x800E10A0u+i*4u,0u);
        PE_StoreU32(0x800B0CD8u,PE_LoadU32(0x800B0CD8u)&~0x10000u);
    }
    PE_StoreU8(slot,0u);
    PE_StoreU8(slot+1u,255u); PE_StoreU8(slot+2u,255u); PE_StoreU8(slot+3u,255u);
    PE_StoreU32(slot+4u,0u); PE_StoreU32(slot+8u,0u);
}

int32_t func_8006FC18(uint32_t index, pe_addr_t owner, uint32_t force)
{
    pe_addr_t slot, entry, fn;
    uint32_t state, code;
    if (index>=22u) return -22;
    slot=effect_slot(index); state=PE_LoadU8(slot);
    if (!state || state==6u || (!force && PE_LoadU32(slot+8u)!=owner)) return 0;
    code=PE_LoadU8(slot+1u);
    if (code>=0xC0u) return -23;
    entry=PE_LoadU32(PE_LoadU32(0x800942E0u)+(code<0x55u?code:0x55u)*4u);
    if (!entry) return -24;
    fn=PE_LoadU32(entry+0x14u);
    if (!fn) return -1;
    switch (fn) {
    case 0x800C7DC4u: case 0x800C8F08u: case 0x800C9C00u:
    case 0x800CA798u: case 0x800CD960u: case 0x800CCF80u:
    case 0x800CE1DCu: case 0x800CBFA4u: case 0x800D4850u:
        PE_StoreU8(slot,4u); break;
    default:
        (void)Bootstrap_ReturnInt4Indirect("effect_destroy_callback", "func_8006FC18",
            -1,fn,slot,owner,force,0u,NULL,0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }
    clear_effect_slot(effect_slot(index));
    return 0;
}

int32_t func_80070064(void)
{
    unsigned index;
    int32_t result=0;
    for (index=0;index<11;index++) {
        pe_addr_t slot=effect_slot(index);
        uint32_t code=PE_LoadU8(slot+1u);
        if (code>=8u && (uint32_t)(code-0x55u)>=30u) continue;
        result=func_8006FC18(index,PE_LoadU32(0x8009D254u),1u);
        if (result) return result;
        clear_effect_slot(effect_slot(index));
    }
    return result;
}

/* 6FE14 releases every effect owned by an actor, across both pools. The
 * second clear is present in retail even when 6FC18 ignored a dormant slot. */
int32_t func_8006FE14(pe_addr_t owner)
{
    unsigned index;
    int32_t result=0;
    if (!owner) return -25;
    for (index=0;index<22;index++) {
        if (PE_LoadU32(effect_slot(index)+8u)!=owner) continue;
        result=func_8006FC18(index,owner,1u);
        if (result) return result;
        clear_effect_slot(effect_slot(index));
    }
    return result;
}

/* 702DC and 701B4: release each complete pool, including dormant headers. */
static int32_t clear_effect_pool(unsigned first)
{
    unsigned i;
    for (i=first;i<first+11u;i++) {
        int32_t result=func_8006FC18(i,0u,1u);
        if (result) return result;
        clear_effect_slot(effect_slot(i));
    }
    return 0;
}

int32_t func_800702DC(void) { return clear_effect_pool(0u); }
int32_t func_800701B4(void) { return clear_effect_pool(11u); }
void func_800703F4(void)
{
    (void)func_800702DC();
    (void)func_800701B4();
}

/* Script 98: find the selected live actor, release its effects and continue.
 * This is used by Eve's first-hit scene before the next animation/dialogue. */
int func_8001930C(pe_addr_t args)
{
    pe_addr_t actor;
    uint32_t kind=PE_LoadU32(PE_LoadU32(args));
    if (!kind) actor=PE_LoadU32(0x8009D254u);
    else {
        actor=PE_LoadU32(0x8009D20Cu);
        while (actor) {
            if (PE_LoadU8(actor+0xCu)==kind &&
                PE_LoadU8(actor+0xDu)==PE_LoadU32(PE_LoadU32(args+4u)) &&
                !(PE_LoadU32(actor+0x98u)&0x10u)) break;
            actor=PE_LoadU32(actor+4u);
        }
    }
    (void)func_8006FE14(actor);
    return 1;
}

void func_800374E8(void)
{
    unsigned i;
    for (i=0;i<4;i++) {
        pe_addr_t text=0x800BCEA8u+i*56u;
        PE_StoreU8(text,0u);
        PE_StoreU32(text+12u,PE_LoadU32(text+12u)&0xFDFFFFFFu);
    }
}

void func_80043240(int32_t value)
{
    PE_StoreU32(0x8009CF3Cu,(uint32_t)value);
}

void func_80020F18(void)
{
    pe_addr_t record, weapon;
    unsigned i;
    (void)func_80070064();
    PE_StoreU32(0x8009D200u,0xFFFFFFFFu);
    PE_StoreU32(0x8009D2FCu,0xFFFFFFFFu);
    PE_StoreU32(0x8009D258u,0xFFFFFFFFu);
    PE_StoreU32(0x8009D208u,0xFFFFFFFFu);
    for (i=0;i<45;i++) {
        pe_addr_t out=0x800BE830u+i*8u;
        PE_StoreU32(out,0u); PE_StoreU16(out+6u,0u); PE_StoreU16(out+4u,0u);
    }
    record=PE_LoadU32(0x8009D278u); weapon=PE_LoadU32(record+0x68u);
    PE_StoreU8(0x8009CE44u,0u); PE_StoreU8(0x8009CE40u,0u);
    PE_StoreU8(0x8009D294u,0u);
    PE_StoreU8(0x8009D2D8u,(uint8_t)((PE_LoadU32(weapon+0x10u)>>4u)&3u));
    PE_StoreU8(0x8009D1DCu,(uint8_t)(PE_LoadU32(weapon+0x10u)&15u));
    for (i=0;i<4;i++) PE_StoreU8(0x8009CE38u+i,PE_LoadU8(weapon+0x14u+i));
    func_800374E8();
    PE_StoreU8(0x8009D1CEu,0u);
    PE_StoreU32(0x8009D1ACu,PE_LoadU32(0x8009D1ACu)&~0x300u);
    func_80026FD0();
    PE_StoreU8(0x8009CE60u,0u);
}
