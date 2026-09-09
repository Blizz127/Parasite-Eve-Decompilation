/* Original ammunition transfer, confirmation and command history.
 * 467E0.s / 340EC.s / 41898.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

static pe_addr_t ammo_ram(pe_addr_t p) {return p<0x200000u?p|0x80000000u:p;}
int32_t func_8005E120(void) {return (int32_t)PE_LoadU32(0x8009D0F4u);}

pe_addr_t func_80051098(void)
{
    pe_addr_t entry=PE_LoadU32(0x8009D014u);
    if (entry<0x800A1B30u) PE_StoreU32(0x8009D014u,entry+36u);
    else {
        for (entry=0x800A1AA0u;entry<0x800A1B0Cu;entry+=36u) {
            uint32_t offset;
            for (offset=0;offset<32u;offset+=16u) {
                uint32_t values[4],i;
                for (i=0;i<4;i++) values[i]=PE_LoadU32(entry+36u+offset+i*4u);
                for (i=0;i<4;i++) PE_StoreU32(entry+offset+i*4u,values[i]);
            }
            PE_StoreU32(entry+32u,PE_LoadU32(entry+68u));
        }
    }
    return entry;
}

int func_80056B24(int32_t amount)
{
    pe_addr_t from=amount>0?0x800A1F94u:0x800A1FB4u,to=amount>0?0x800A1FB4u:0x800A1F94u;
    uint32_t count=amount>0?(uint32_t)amount:0u-(uint32_t)amount;
    uint32_t remaining=PE_LoadU16(from+10u)-count,next=PE_LoadU16(to+10u)+count;
    int32_t capacity;int result=0;
    if ((int32_t)remaining<0) {next+=remaining;remaining=0u;result=1;}
    capacity=PE_LoadU8(to+9u)+(int16_t)PE_LoadU16(to+18u);if (capacity>=1000) capacity=999;
    if (capacity<(int32_t)next) {remaining+=next-(uint32_t)capacity;next=(uint32_t)capacity;result=2;}
    PE_StoreU16(from+10u,(uint16_t)remaining);PE_StoreU16(to+10u,(uint16_t)next);
    if (!remaining && PE_LoadU16(from+12u)) {PE_StoreU16(from+10u,PE_LoadU16(from+12u));PE_StoreU16(from+12u,0u);}
    return result;
}

void func_80057094(void)
{
    pe_addr_t first=PE_LoadU32(0x8009D070u),second=PE_LoadU32(0x8009D074u),history,extra=0u,record;
    uint32_t reserve;
    if (PE_LoadU16(0x800A1F9Eu)==PE_LoadU16(ammo_ram(first+10u)) &&
        PE_LoadU16(0x800A1FBEu)==PE_LoadU16(ammo_ram(second+10u))) return;
    history=func_80051098();PE_StoreU32(ammo_ram(history),4u);PE_StoreU32(ammo_ram(history+12u),0u);
    PE_StoreU32(ammo_ram(history+4u),PE_LoadU32(0x8009D070u));
    PE_StoreU32(ammo_ram(history+8u),PE_LoadU32(0x8009D074u));
    reserve=PE_LoadU16(0x800A1FA0u);
    if (reserve) extra=0x800A1E44u+PE_LoadU8(0x800A1FB3u)*32u;
    else if ((reserve=PE_LoadU16(0x800A1FC0u))!=0u) extra=0x800A1E44u+PE_LoadU8(0x800A1FD3u)*32u;
    if (extra) {
        uint32_t amount=PE_LoadU16(extra+10u)+reserve;
        int32_t capacity=PE_LoadU8(extra+9u)+(int16_t)PE_LoadU16(extra+18u);
        PE_StoreU32(ammo_ram(history+12u),extra);PE_StoreU32(ammo_ram(history+32u),PE_LoadU16(extra+10u));
        if (capacity>=1000) capacity=999;
        if (capacity<(int32_t)amount) amount=(uint32_t)capacity;
        PE_StoreU16(extra+10u,(uint16_t)amount);
    }
    first=PE_LoadU32(0x8009D070u);second=PE_LoadU32(0x8009D074u);
    PE_StoreU32(ammo_ram(history+20u),PE_LoadU16(ammo_ram(first+10u)));
    PE_StoreU32(ammo_ram(history+28u),PE_LoadU16(ammo_ram(second+10u)));
    PE_CopyItemRecord(first,0x800A1F94u);PE_CopyItemRecord(PE_LoadU32(0x8009D074u),0x800A1FB4u);
    record=func_8005332C((int8_t)PE_LoadU8(0x800C0E20u));
    if (record==PE_LoadU32(0x8009D070u) || record==PE_LoadU32(0x8009D074u)) PE_MenuApplyAmmo(record);
    func_800512AC(7,0u);
}

void func_80056FB8(void)
{
    unsigned i;
    for (i=0;i<2;i++) {
        pe_addr_t record=0x800A1F94u+i*32u,name;
        if (i) func_8005E8A4(0,24);
        if (PE_LoadU8(record+5u)&16u) name=PE_LoadU8(record+6u)==9u?0x800C20B4u:0x800C20A4u;
        else name=func_8005DC9C(PE_LoadU8(record+4u)-1u);
        func_800534E4(record,name);
    }
}
void func_800453E8(pe_addr_t window)
{
    (void)window;func_8005E8A4(60,18);func_8005EB64(PE_LoadU32(0x8009CFACu)+77u);
    func_8005E8A4(-56,-14);func_80056FB8();
}

int func_800452C0(pe_addr_t window,uint32_t event)
{
    if (event&0x1000u) {
        int result=func_80056B24((int32_t)(0u-(uint32_t)func_8005E120()));
        if (result) func_800526C4();else func_8005267C();PE_StoreU32(0x8009CFACu,1u);
    } else if (event&0x4000u) {
        int result=func_80056B24(func_8005E120());
        if (result) func_800526C4();else func_8005267C();PE_StoreU32(0x8009CFACu,0u);
    } else if (event&0x10000u) {
        func_80057094();PE_StoreU32(0x8009CF94u,UINT32_MAX);PE_StoreU32(0x8009CF8Cu,UINT32_MAX);
        func_80055760();func_80062F1C(window);func_800525EC();
    }
    if (event&64u) {
        PE_StoreU32(0x8009CF94u,UINT32_MAX);PE_StoreU32(0x8009CF8Cu,UINT32_MAX);
        func_80055760();func_80062F1C(window);func_80052634();
    }
    return 1;
}

void func_80044274(int32_t index)
{
    int32_t count;pe_addr_t alternate,primary;
    PE_StoreU32(0x8009CF88u,func_80052F0C());PE_StoreU32(0x8009CF8Cu,(uint32_t)index);
    count=func_800562A4(index);
    if (!count) {PE_StoreU32(0x8009CF8Cu,UINT32_MAX);func_80055760();return;}
    alternate=func_80062A34(2u,13u);primary=func_80062A34(2u,14u);
    if (alternate && func_80063428(alternate) && primary) {
        int32_t i;
        PE_StoreU32(alternate+68u,UINT32_MAX);func_80052E30(0u);
        for (i=0;i<(int32_t)func_80052F70();i++) if (func_80055FE0(i)) break;
        PE_StoreU32(primary+68u,0u);PE_StoreU32(primary+72u,i<(int32_t)func_80052F70()?(uint32_t)i:0u);
        func_80062CB8(primary);
    }
    if (count==1) {
        PE_StoreU32(0x8009CF90u,PE_LoadU32(0x8009CF88u));func_8005600C(0x8009CF90u,0x8009CF94u);
        func_800451D0(func_80062CC4());
    }
}
