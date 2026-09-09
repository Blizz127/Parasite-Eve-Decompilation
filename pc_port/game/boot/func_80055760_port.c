/* Original item availability, item rearrangement and inventory list drawing.
 * 44AA0.s / 486D8.s / 467E0.s / 340EC.s / 40038.s / 40F48.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

int32_t func_8004E970(void) {return (int32_t)PE_LoadU32(0x8009CF0Cu);}

void func_80055760(void)
{
    uint32_t saved=func_80052F0C(),gun=0u,armor=0u,bank;
    int32_t i;
    for (bank=0;bank<(D_8009D04C?2u:1u);bank++) {
        func_80052E30(bank);
        for (i=0;i<(int32_t)D_8009D050;i++)
            if ((254u>>((uint32_t)func_8005415C(i)&31u))&1u) break;
        gun|=i<(int32_t)D_8009D050;
        for (i=0;i<(int32_t)D_8009D050;i++) if (func_8005415C(i)==9) break;
        armor|=i<(int32_t)D_8009D050;
    }
    for (bank=0;bank<(D_8009D04C?2u:1u);bank++) {
        int profile;
        func_80052E30(bank);
        for (i=0;i<(int32_t)D_8009D064;i++) PE_StoreU32(D_8009D058+(uint32_t)i*4u,0u);
        profile=func_8004E970();
        for (i=0;i<(int32_t)D_8009D050;i++) {
            pe_addr_t record=func_8005332C(i),bits;uint32_t enabled,subtype;
            if (!record) continue;
            enabled=(PE_LoadU8(record+5u)>>(func_8005B89C()?1u:0u))&1u;
            if (PE_LoadU8(record+6u)==10u) {
                subtype=PE_LoadU8(record+14u);
                if (profile) {if (subtype==2u) enabled=0u;}
                else if (subtype-4u<3u) enabled&=gun;
                else if (subtype-12u<3u) enabled&=armor;
            }
            if ((uint32_t)(PE_LoadU8(record+4u)-6u)<5u && PE_LoadU16(0x800C0E08u)>=PE_LoadU16(0x800C0E06u))
                enabled=0u;
            bits=D_8009D058+((uint32_t)i>>5u)*4u;
            PE_StoreU32(bits,PE_LoadU32(bits)|(enabled<<((uint32_t)i&31u)));
        }
    }
    func_80052E30(saved);
}

void func_80050260(void) {func_80055760();}

int func_80055FE0(int32_t index)
{return (PE_LoadU32(D_8009D058+(uint32_t)(index>>5)*4u)>>((uint32_t)index&31u))&1u;}

int func_80057C54(uint32_t first_list,int32_t first,uint32_t second_list,int32_t second)
{
    pe_addr_t a=D_8009D048+(uint32_t)first*2u,b=D_8009D048+(uint32_t)second*2u;
    uint16_t value;unsigned i;
    (void)first_list;(void)second_list;
    value=PE_LoadU16(a)^PE_LoadU16(b);PE_StoreU16(a,value);
    value=PE_LoadU16(b)^value;PE_StoreU16(b,value);
    value=PE_LoadU16(a)^value;PE_StoreU16(a,value);
    for (i=0;i<2;i++) {
        pe_addr_t equipped=0x800C0E20u+i*2u;int32_t selected=(int8_t)PE_LoadU8(equipped);
        if (selected==first) PE_StoreU8(equipped,(uint8_t)second);
        else if (selected==second) PE_StoreU8(equipped,(uint8_t)first);
    }
    func_80055760();return 1;
}

int32_t func_8005401C(void)
{
    uint32_t count=0u;pe_addr_t p,end;
    func_80052E30(0u);p=D_8009D048;end=p+D_8009D050*2u;
    while (p<end) {count+=PE_LoadU16(p)!=0u;p+=2u;}
    return (int32_t)count;
}

int func_80054240(int32_t index)
{
    return D_8009D048==0x800C0E48u && ((int8_t)PE_LoadU8(0x800C0E20u)==index ||
        (int8_t)PE_LoadU8(0x800C0E22u)==index);
}

void func_8005FA3C(int32_t value)
{
    int digits=2,divisor=10,i;
    if (value<0) {
        value=(int32_t)(0u-(uint32_t)value);func_8005EB64(82u);func_8005E8A4(5,0);digits=1;divisor=1;
    }
    for (i=0;i<digits;i++) {
        int32_t digit=value/divisor;
        func_8005F874(i<digits-1 && !digit?-1:digit);func_8005E8A4(5,0);divisor/=10;
    }
}

void func_80063158(pe_addr_t node,int32_t x,int32_t y)
{
    if (!node) return;
    PE_StoreU32(node+24u,PE_LoadU32(node+24u)+(uint32_t)x);
    PE_StoreU32(node+28u,PE_LoadU32(node+28u)+(uint32_t)y);func_8005E8A4(x,y);
}
void func_80062F1C(pe_addr_t node) {func_8006269C(node);}

void func_80064C80(void)
{
    PE_StoreU32(0x8009D114u,0x7F2000u);PE_StoreU32(0x8009D110u,0xFF4000u);
    func_8005E8A4(-2,-2);func_80062090(PE_LoadU32(0x8009D164u),PE_LoadU32(0x8009D168u),0u);
    PE_StoreU32(0x8009D114u,0x404040u);PE_StoreU32(0x8009D110u,0x808080u);func_8005E8A4(2,2);
}

void func_80050804(uint32_t index)
{
    func_8005EB58(PE_LoadU32(0x8009CEFCu) || !func_80055FE0((int32_t)index));
    func_800536B8((int32_t)index);if (func_80054240((int32_t)index)) func_80064C80();
}

void func_8004F8D0(pe_addr_t node)
{
    func_80052E30(0u);PE_StoreU32(0x8009CEF4u,node);func_800638D8(node,0x80050804u);
}

void func_800447F0(pe_addr_t window)
{
    pe_addr_t list=func_80062A34(2u,1u),spec=func_8005DA8C(1u);
    uint32_t y=PE_LoadU32(spec+4u)+PE_LoadU32(list+56u)*16u+4u-PE_LoadU32(window+28u);
    func_80063158(window,0,(int32_t)y);
    if (PE_LoadU32(list+104u)) {
        int32_t difference=(int32_t)(PE_LoadU32(list+88u)-PE_LoadU32(list+56u)-PE_LoadU32(list+92u));
        int32_t scroll=(int32_t)PE_LoadU32(list+96u);
        if (!difference) func_80063158(window,0,(int32_t)((uint32_t)scroll-16u));
        else if (difference==1 && scroll<0) func_80063158(window,0,scroll);
    }
    func_8005E8A4(10,2);func_8005F27C(func_8005DC4C(37u));func_8005E8A4(70,4);
    func_8005FA3C(func_8005401C());func_8005EB64(76u);func_8005E8A4(5,0);
    func_8005FA3C((int32_t)func_80052F70());
}

void func_80044174(pe_addr_t owner)
{
    pe_addr_t window=func_80062D2C(27u,0u,0u,0u),list;
    uint32_t saved;
    PE_StoreU32(window+48u,0x800447F0u);window=func_80062D2C(1u,owner,0u,0u);
    list=func_8006322C(1u,window,window);PE_StoreU32(window+44u,0x80044444u);
    PE_StoreU32(list+48u,0x8004F8D0u);func_80062CB8(list);
    PE_StoreU32(list+132u,0x80057C54u);PE_StoreU32(list+136u,0x80050260u);func_80055760();
    PE_StoreU32(0x8009CF94u,UINT32_MAX);PE_StoreU32(0x8009CF8Cu,UINT32_MAX);
    func_800647D0(list,(int32_t)func_80052F70());saved=PE_LoadU32(0x8009CF98u);
    PE_StoreU32(0x8009CF00u,0u);
    if (saved) {
        saved--;PE_StoreU32(list+68u,saved&1u);PE_StoreU32(list+72u,(saved>>1u)&127u);
        PE_StoreU32(list+92u,(uint32_t)((int32_t)saved>>8));
    }
}
