/* Arrange Items and shared storage sorting menus. Original 340EC.s,
 * 379E4.s, 37B54.s, 37CD0.s and 40A80.s; gp=8009CD70. */
#include "psx_compat.h"
#include "pe_port_compat.h"

extern unsigned int func_80052F70(void);

void func_80046DFC(pe_addr_t owner,uint32_t alternate)
{
    uint32_t id=58u+alternate;
    pe_addr_t window=func_80062D2C(id,owner,0u,0u),list=func_8006322C(id,window,window);
    PE_StoreU32(window+44u,0x800471E4u);PE_StoreU32(list+48u,0x800471BCu);
    func_80062CB8(list);func_800631AC(func_80062A34(1u,51u));
    PE_StoreU32(0x8009CFB8u,alternate?PE_LoadU32(0x8009CF0Cu):0u);
}

void func_80046EAC(uint32_t arranged)
{
    uint32_t mode,saved;pe_addr_t owner,window,list;
    func_80062F3C(60u);func_80062F3C(59u);func_80062F3C(58u);
    mode=PE_LoadU32(0x8009CFB8u);
    if (mode==1u || mode==2u) {func_80063198(func_80062A34(1u,51u));return;}
    if (mode) return;
    if (!arranged) {func_800439D8();return;}
    owner=func_80062CC4();PE_StoreU32(owner+72u,0u);
    window=func_80062D2C(27u,0u,0u,0u);PE_StoreU32(window+48u,0x800447F0u);
    window=func_80062D2C(1u,owner,0u,0u);list=func_8006322C(1u,window,window);
    PE_StoreU32(window+44u,0x80044444u);PE_StoreU32(list+48u,0x8004F8D0u);func_80062CB8(list);
    PE_StoreU32(list+132u,0x80057C54u);PE_StoreU32(list+136u,0x80050260u);func_80055760();
    PE_StoreU32(0x8009CF94u,UINT32_MAX);PE_StoreU32(0x8009CF8Cu,UINT32_MAX);
    func_800647D0(list,(int32_t)func_80052F70());PE_StoreU32(0x8009CF00u,0u);
    saved=PE_LoadU32(0x8009CF98u);
    if (saved) {
        saved--;PE_StoreU32(list+68u,saved&1u);PE_StoreU32(list+72u,(saved>>1u)&127u);
        PE_StoreU32(list+92u,(uint32_t)((int32_t)saved>>8));
    }
}

void func_80047040(uint32_t index)
{
    uint32_t mode=PE_LoadU32(0x8009CFB8u);
    index+=PE_LoadU8(0x800922B8u+mode);
    if (mode==1u) {
        func_8005E8A4(0,2);func_8005EB64(PE_LoadU8(0x800922BCu+index));
        func_8005E8A4(18,2);func_8005EB64(34u);func_8005E8A4(12,-2);
        func_8005EB64(PE_LoadU8(0x800922C4u+index));
        if (!PE_LoadU32(0x8009CFB8u)) {
            func_8005E8A4(18,2);func_8005EB64(34u);func_8005E8A4(12,-2);
            func_8005EB64(PE_LoadU8(0x800922CCu+index));
        }
    } else {
        func_8005EB64(PE_LoadU8(0x800922BCu+index));func_8005EB64(104u);
        func_8005E8A4(18,4);func_8005EB64(34u);func_8005E8A4(12,-4);
        func_8005EB64(PE_LoadU8(0x800922C4u+index));func_8005EB64(104u);
        if (!PE_LoadU32(0x8009CFB8u)) {
            func_8005E8A4(18,4);func_8005EB64(34u);func_8005E8A4(12,-4);
            func_8005EB64(PE_LoadU8(0x800922CCu+index));func_8005EB64(104u);
        }
    }
}

void func_800471BC(pe_addr_t list) {func_800638D8(list,0x80047040u);}
void func_8004732C(pe_addr_t list) {func_800638D8(list,0x80050280u);}
void func_800474A8(pe_addr_t list) {func_800638D8(list,0x80050308u);}

int func_800471E4(pe_addr_t window,uint32_t event)
{
    if (event&0x10000u) {
        pe_addr_t list=func_80062A20(window,0u);
        int32_t index=func_80063428(list);uint32_t mode=PE_LoadU32(0x8009CFB8u);
        if (!mode && index==2) {
            window=func_80062D2C(59u,list,0u,0u);list=func_8006322C(59u,window,window);
            PE_StoreU32(window+44u,0x80047354u);
            func_80063158(window,(int32_t)(112u-PE_LoadU32(window+24u)),(int32_t)(72u-PE_LoadU32(window+28u)));
            PE_StoreU32(list+48u,0x8004732Cu);func_80062CB8(list);
        } else if (!mode || mode==2u) func_800473E4(list,(uint32_t)index);
        else if (mode==1u) {func_8005B71C((uint32_t)index);func_80046EAC(1u);}
        func_800525EC();
    } else if (event&64u) {func_80046EAC(0u);func_80052634();}
    return 1;
}

int func_80047354(pe_addr_t window,uint32_t event)
{
    if (event&0x10000u) {
        int32_t index=func_80063428(func_80062A20(window,0u));
        if (PE_LoadU32(0x8009CFB8u)) func_8005B71C((uint32_t)index);
        else func_8005B500(2u,(uint32_t)index);
        func_80046EAC(1u);func_800525EC();
    } else if (event&64u) {func_80062F1C(window);func_80052634();}
    return 1;
}

void func_800473E4(pe_addr_t owner,uint32_t selection)
{
    pe_addr_t window=func_80062D2C(60u,owner,0u,0u),list=func_8006322C(60u,window,window);
    PE_StoreU32(window+44u,0x800474D0u);PE_StoreU32(list+48u,0x800474A8u);func_80062CB8(list);
    if (!PE_LoadU32(0x8009CFB8u))
        func_80063158(window,(int32_t)(112u-PE_LoadU32(window+24u)),(int32_t)(32u-PE_LoadU32(window+28u)));
    PE_StoreU32(0x8009CF18u,selection==0u);
    if (selection) func_80063158(window,0,20);
}

int func_800474D0(pe_addr_t window,uint32_t event)
{
    if (event&0x10000u) {
        int32_t index=func_80063428(func_80062A20(window,0u));uint32_t selection=PE_LoadU32(0x8009CF18u);
        if (PE_LoadU32(0x8009CFB8u)) func_8005B7D0(selection,(uint32_t)index);
        else func_8005B500(selection,(uint32_t)index);
        func_80046EAC(1u);func_800525EC();
    } else if (event&64u) {func_80062F1C(window);func_80052634();}
    return 1;
}

void func_80050280(uint32_t index)
{
    func_8005E8A4(0,2);func_8005EB64((uint32_t)(int32_t)(int16_t)PE_LoadU16(0x800922D0u+index*2u));
    func_8005E8A4(18,4);func_8005EB64(34u);func_8005E8A4(12,-4);
    func_8005EB64((uint32_t)(int32_t)(int16_t)PE_LoadU16(0x800922D0u+(index?0u:2u)));
}

void func_80050308(uint32_t index)
{func_8005EB64(index+(PE_LoadU32(0x8009CF18u)?124u:127u));}
