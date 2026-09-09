/* Original E7 two-stage menu entry and its constructor.
 * 15AF0..15BAC and4D18C..4D27C; callback execution is a separate graph. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

extern void func_8005C1EC(int enabled);
extern void func_80042538(void);

void func_8004D18C(void)
{
    pe_addr_t owner=func_80062CC4();
    pe_addr_t window=func_80062D2C(36u,owner,0u,0u);
    pe_addr_t list=func_8006322C(36u,window,window);
    PE_StoreU32(window+44u,0x8004D2DCu);
    PE_StoreU32(list+48u,0x8004FDE8u);PE_StoreU32(list+140u,0x8004FDA4u);
    if(PE_LoadU32(list+72u)==2u)PE_StoreU32(list+72u,0u);
    func_80062CB8(list);
    if(!func_80062A34(1u,19u)) {
        pe_addr_t help=func_80062D2C(19u,0u,0u,0u);
        PE_StoreU32(help+48u,0x8004C608u);
    }
    PE_StoreU32(func_80062A34(1u,19u)+56u,36u);
    PE_StoreU32(0x8009CF50u,1u);
    func_8005C1EC(1);func_80042538();func_8005DE88();
}

int func_80015AF0(pe_addr_t args)
{
    (void)args;
    if(PE_LoadU32(0x800B0CD8u)&0x1000u)return 1;
    pe_addr_t task=PE_LoadU32(0x8009D300u);
    uint16_t flags=PE_LoadU16(task+8u);
    if(!(flags&0x20u)) {
        PE_StoreU16(task+8u,flags|0x20u);
        uint32_t next=PE_LoadU32(0x8009CE00u);
        PE_StoreU32(task+16u,1u);
        PE_StoreU32(0x8009CE00u,next-8u);
        return 0;
    }
    unsigned epoch=PE_Port_StopEpoch();
    func_80067CBC();func_8004D18C();
    if(PE_Port_StopEpoch()!=epoch)return 0;
    uint32_t scene=PE_LoadU32(0x800B0CD8u);
    task=PE_LoadU32(0x8009D300u);
    PE_StoreU32(0x800B0CD8u,scene|0x9000u);
    uint32_t input=PE_LoadU32(0x8009D1A0u);
    flags=PE_LoadU16(task+8u);
    PE_StoreU32(0x8009D1A0u,input|4u);
    PE_StoreU16(task+8u,flags&0xFFDFu);
    return 1;
}

uint32_t func_80042770(uint32_t index)
{
    return PE_LoadU8(0x800A0ED4u+index*0x418u)&1u;
}

int func_8004FDA4(uint32_t index)
{
    return index==2u || func_80042770(index)!=0u;
}

void func_80050C08(int32_t index)
{
    func_8005E8A4(-2,-2);
    func_8005EB64(index<2?(uint32_t)index+0x84u:0x62u);
}

void func_8004FDE8(pe_addr_t list)
{
    unsigned epoch=PE_Port_StopEpoch();
    PE_StoreU32(0x8009CEF4u,list);
    func_800638D8(list,0x80050C08u);
    if(PE_Port_StopEpoch()!=epoch)return;
    func_8005EB58(1);
    uint32_t count=PE_LoadU32(list+56u);
    while(count) {
        func_8005EB64(0x68u);
        if(PE_Port_StopEpoch()!=epoch)return;
        func_8005E8A4(0,16);count--;
    }
}

uint32_t func_800428C4(void) { return PE_LoadU32(0x800A1860u)-1u; }
void func_80042910(void)
{ PE_StoreU32(0x800A1860u,0);PE_StoreU32(0x800A1868u,0); }
uint32_t func_80042B28(void) { return PE_LoadU32(0x800A1838u); }
uint32_t func_80042AD8(uint32_t index)
{
    uint32_t state=PE_LoadU8(0x800A0ED5u+index*0x418u);
    return state==3u || state==8u || state==10u;
}
void func_80062CD0(pe_addr_t selection)
{
    uint32_t previous=PE_LoadU32(0x8009D15Cu);
    PE_StoreU32(0x8009D15Cu,selection);PE_StoreU32(0x8009D160u,previous);
}
void func_8004298C(uint32_t index,uint32_t mode)
{
    pe_addr_t record=0x800A0ED4u+index*0x418u;
    uint32_t state=PE_LoadU8(record+1u);
    if(state!=0u && state!=12u)return;
    PE_StoreU8(record+1u,1);PE_StoreU8(record+11u,2);PE_StoreU16(record+22u,10);
    func_80062CD0(0);PE_StoreU32(0x800A186Cu,mode);
}
void func_80042A10(void)
{
    /* Identical two-record cleanup loop to42798, followed by these resets. */
    unsigned epoch=PE_Port_StopEpoch();
    func_80042798();
    if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU8(0x800A12EDu,0);PE_StoreU8(0x800A0ED5u,0);PE_StoreU32(0x800A1838u,0);
}

void func_8004DAA4(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    if(PE_LoadU32(0x8009CF50u)) {
        if(func_80062A34(1u,42u))return;
        pe_addr_t owner=func_80062A34(2u,36u);
        pe_addr_t text=func_8005DC4C(func_800428C4()+0x47u);
        pe_addr_t window=func_80062D2C(42u,owner,0u,1u);
        pe_addr_t list=func_8006322C(42u,window,window);
        PE_StoreU32(window+48u,0x80044E14u);PE_StoreU32(window+44u,0x80044E98u);
        PE_StoreU32(list+48u,0x8004F950u);PE_StoreU32(0x8009CF14u,0x6Cu);
        func_80062CD0(list);PE_StoreU32(list+68u,1u);
        func_80052E30(PE_LoadU32(0x8009CF10u));
        if(PE_Port_StopEpoch()!=epoch)return;
        if(text)func_80052BCC(0x800A19C0u,text);
        else PE_StoreU8(0x800A19C0u,255u);
        if(PE_Port_StopEpoch()!=epoch)return;
        func_80052C08(0x800A19C0u,func_8005DC4C(0x49u));
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFA4u,0x4Au);
        uint32_t width=func_8005F1A0(0x800A19C0u);
        if(PE_Port_StopEpoch()!=epoch)return;
        width=(int32_t)width<120?120u:func_8005F1A0(0x800A19C0u);
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(window+52u,width+20u);PE_StoreU32(window+56u,66u);
        uint32_t x=300u-width;
        uint32_t saved_width=PE_LoadU32(window+52u);
        PE_StoreU32(window+24u,(uint32_t)((int32_t)x>>1));
        PE_StoreU32(list+24u,(uint32_t)((int32_t)(saved_width-128u)>>1));
        uint32_t height=PE_LoadU32(window+56u);
        PE_StoreU32(0x8009CFA8u,0x80050580u);PE_StoreU32(list+28u,height-20u);
    } else {
        if(func_80062A34(1u,40u))return;
        func_8004CC50(func_800428C4()+0x47u,0x49u);
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFFCu,0x80042910u);
    }
}

int func_80042848(uint32_t index)
{
    uint32_t flagged=PE_LoadU8(0x800A0ED4u+index*0x418u)&4u;
    if(flagged && !PE_LoadU32(0x800A1860u)) {
        PE_StoreU32(0x800A1860u,index+1u);func_8004DAA4();
    }
    return flagged==0u;
}

int func_8004D2DC(pe_addr_t window,uint32_t event)
{
    unsigned epoch=PE_Port_StopEpoch();
    int32_t selection=func_8006346C(func_80062A20(window,0u));
    if(event&0x10000u) {
        if(selection<0 || selection>2) {func_800526C4();return 1;}
        if(selection<2) {
            int ready=func_80042848((uint32_t)selection);
            if(PE_Port_StopEpoch()!=epoch || !ready)return 1;
            if(!func_80042770((uint32_t)selection) || func_80042B28())return 1;
            func_80042A10();if(PE_Port_StopEpoch()!=epoch)return 1;
            func_8004298C((uint32_t)selection,1u);if(PE_Port_StopEpoch()!=epoch)return 1;
            func_800525EC();return 1;
        }
    } else if(!(event&0x40u))return 1;
    const uint32_t ids[]={37,38,36,19};
    for(unsigned i=0;i<4;i++) {
        func_80062F1C(func_80062A34(1u,ids[i]));
        if(PE_Port_StopEpoch()!=epoch)return 1;
    }
    func_8005C1EC(0);if(PE_Port_StopEpoch()!=epoch)return 1;
    func_800512AC(9,0);if(PE_Port_StopEpoch()!=epoch)return 1;
    PE_StoreU32(0x8009CFF8u,0);
    if(event&0x10000u)func_800525EC();else func_80052634();
    return 1;
}

/* Card confirmation, progress display and delayed record transition. */
void func_800428D4(void)
{ PE_StoreU8(0x800A0ED5u+(PE_LoadU32(0x800A1860u)-1u)*0x418u,13u); }

void func_80042B50(pe_addr_t callback)
{ PE_StoreU32(0x800A1870u,callback);PE_StoreU32(0x800A1874u,1u); }

int func_8004DA9C(void) { return 1; }

int32_t func_80042464(void)
{
    pe_addr_t record=PE_LoadU32(0x800A1854u);
    if(!record)return 0;
    uint32_t value=PE_LoadU32(0x800A1858u)-(uint32_t)(int32_t)(int16_t)PE_LoadU16(record+20u)+0x400u;
    int32_t count=(int32_t)value>>10;
    return count<9?count:8;
}

void func_8004DA04(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    func_8005E8A4(0,10);func_80062A7C(PE_LoadU32(0x8009D000u));
    if(PE_Port_StopEpoch()!=epoch || PE_LoadU32(0x8009D000u)==0x40u)return;
    int32_t count=func_80042464();func_8005E8A4(16,20);
    for(int i=0;i<8;i++) {
        func_8005EB64(i<count?0x83u:0x82u);
        if(PE_Port_StopEpoch()!=epoch)return;
        func_8005E8A4(16,0);
    }
}

void func_8004CFD4(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    func_8005E8A4(0,10);func_8005F594(0x800A1A20u);
    if(PE_Port_StopEpoch()!=epoch)return;
    func_8005E8A4(0,14);func_8005F594(0x800A1A60u);
}

void func_8004CE28(uint32_t first,uint32_t second)
{
    unsigned epoch=PE_Port_StopEpoch();
    pe_addr_t window=func_80062D2C(40u,func_80062CC4(),0u,1u);
    pe_addr_t list=func_8006322C(40u,window,window);
    PE_StoreU32(window+48u,0x8004CFD4u);PE_StoreU32(window+44u,0x8004D030u);
    PE_StoreU32(list+48u,0x8004FFA8u);func_80062CB8(list);func_8004D024(0u);
    func_80052BCC(0x800A1A20u,func_8005DC4C(first));
    if(PE_Port_StopEpoch()!=epoch)return;
    func_80052BCC(0x800A1A60u,func_8005DC4C(second));
    if(PE_Port_StopEpoch()!=epoch)return;
    uint32_t a=func_8005F1A0(0x800A1A20u),b=func_8005F1A0(0x800A1A60u);
    if(PE_Port_StopEpoch()!=epoch)return;
    uint32_t width=func_8005F1A0((int32_t)b<(int32_t)a?0x800A1A20u:0x800A1A60u);
    if(PE_Port_StopEpoch()!=epoch)return;
    if((int32_t)width<100)width=100u;
    else {
        a=func_8005F1A0(0x800A1A20u);b=func_8005F1A0(0x800A1A60u);
        if(PE_Port_StopEpoch()!=epoch)return;
        width=func_8005F1A0((int32_t)b<(int32_t)a?0x800A1A20u:0x800A1A60u);
        if(PE_Port_StopEpoch()!=epoch)return;
    }
    PE_StoreU32(window+52u,width+20u);
    uint32_t height=PE_LoadU32(window+56u);
    PE_StoreU32(window+24u,(uint32_t)((int32_t)(300u-width)>>1));
    width=PE_LoadU32(window+52u);
    PE_StoreU32(window+56u,height+14u);
    uint32_t y=PE_LoadU32(list+28u);
    PE_StoreU32(list+24u,width-68u);PE_StoreU32(list+28u,y+14u);
    func_8005DE88();
}

void func_80050580(pe_addr_t unused,uint32_t confirmed)
{
    (void)unused;
    unsigned epoch=PE_Port_StopEpoch();
    if(confirmed) {
        pe_addr_t window=func_80062D2C(39u,func_80062CC4(),0u,1u);
        PE_StoreU32(window+48u,0x8004DA04u);PE_StoreU32(window+44u,0x8004DA9Cu);
        func_80062CB8(window);PE_StoreU32(0x8009D000u,0x40u);
        func_80042B50(0x800428D4u);
    } else {
        func_8004CE28(func_800428C4()+0x47u,0x4Bu);
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFFCu,0x80042910u);
    }
}
