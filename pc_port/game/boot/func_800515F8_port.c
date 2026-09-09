/* Original PE reservations, costs and menu availability, 33A4C/41D10/467E0.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

int32_t PE_MenuPEValues(int32_t *maximum)
{
    pe_addr_t actor=PE_LoadU32(0x8009D254u),record,entry,end;
    uint32_t value;
    if (!actor || !(record=PE_LoadU32(actor))) return 0;
    value=(uint32_t)(int32_t)(int16_t)PE_LoadU16(record+10u);
    end=PE_LoadU32(0x8009D014u);
    for (entry=0x800A1AA0u;entry<end;entry+=36u)
        if (PE_LoadU32(entry)==1u) value-=PE_LoadU32(entry+8u);
    if (maximum) *maximum=(int16_t)PE_LoadU16(record+42u);
    return (int32_t)value;
}

int32_t func_800515F8(pe_addr_t maximum)
{
    int32_t value,limit;
    pe_addr_t actor=PE_LoadU32(0x8009D254u);
    if (!actor || !PE_LoadU32(actor)) return 0;
    value=PE_MenuPEValues(&limit);
    if (maximum) PE_StoreU32(maximum<0x200000u?maximum|0x80000000u:maximum,(uint32_t)limit);
    return value;
}

pe_addr_t func_8005DC10(void)
{return PE_LoadU32(0x800A8048u)+0x800A8028u;}

int func_800524D0(void)
{
    pe_addr_t actor=PE_LoadU32(0x8009D254u),record;
    return actor && (record=PE_LoadU32(actor))?((PE_LoadU32(record+76u)>>9u)&1u):0;
}

int32_t func_800579D4(int32_t ability,int32_t available)
{
    int32_t maximum=0,cost;
    pe_addr_t armor;
    unsigned i,count;
    if (ability==6 || ability==19) return available;
    if (ability==5) {PE_MenuPEValues(&maximum);return maximum/3;}
    cost=PE_LoadU16(func_8005DC10()+(uint32_t)ability*4u+2u);
    armor=func_8005332C((int8_t)PE_LoadU8(0x800C0E22u));
    if (!armor) return cost;
    count=PE_LoadU8(armor+20u);
    for (i=0;i<count;i++) if (PE_LoadU8(armor+21u+i)==14u) return cost*2/3;
    return cost;
}

int func_8004324C(int32_t ability)
{
    int32_t maximum=0,available=PE_MenuPEValues(&maximum);
    pe_addr_t item=func_8005DB44((uint32_t)ability+235u);
    int enabled=(PE_LoadU8(item+5u)>>((uint32_t)func_8005B89C()&31u))&1u;
    if (ability==5) {if (available<maximum/3) enabled=0;}
    else if (ability==6) {if (available<=0) enabled=0;}
    else if (ability==19) {if (available<maximum) enabled=0;}
    else if (available<func_800579D4(ability,0)) enabled=0;
    if (PE_LoadU32(0x8009CF3Cu) && ((uint32_t)ability-6u<3u || ability==10 || ability==19)) enabled=0;
    if (ability==5 && func_800524D0()) enabled=0;
    if (ability==6) {
        uint32_t kind;
        item=func_8005332C((int8_t)PE_LoadU8(0x800C0E20u));
        if (item<0x200000u) item|=0x80000000u;
        kind=PE_LoadU8(item+6u);
        if (kind==8u) enabled=0;
        else {
            if (kind && kind<8u) kind=kind>4u?kind-4u:1u;
            else {kind=PE_LoadU8(item+6u);kind=kind>=19u?kind-18u:0u;}
            if ((kind!=1u && kind!=3u) ||
                (int32_t)(func_80056C14(kind-1u)+PE_LoadU16(item+10u))<=0) enabled=0;
        }
    }
    if ((ability<3 || ability==18) && PE_LoadU16(0x800C0E08u)>=PE_LoadU16(0x800C0E06u)) enabled=0;
    return enabled;
}
