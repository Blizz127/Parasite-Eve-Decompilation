/* Original menu list rendering, clipping, scroll thumb and draw tree.
 * 53CD4.s / 556B4.s / 52ABC.s / 5373C.s / 408A8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_gpu.h"
#include "game_port.h"
#include <stdio.h>

static pe_addr_t list_ram(pe_addr_t address)
{ return address<0x200000u?address|0x80000000u:address; }
static int32_t list_word(pe_addr_t node,uint32_t offset)
{ return (int32_t)PE_LoadU32(list_ram(node+offset)); }

static void menu_draw_callback(pe_addr_t fn,uint32_t value)
{
    switch (fn) {
    case 0x8004DA04u:func_8004DA04();break;
    case 0x8004CFD4u:func_8004CFD4();break;
    case 0x8004FDE8u:func_8004FDE8(value);break;
    case 0x80050C08u:func_80050C08((int32_t)value);break;
    case 0x8004F644u:func_8004F644();break;
    case 0x8004F798u:func_8004F798();break;
    case 0x8004F7D8u:func_8004F7D8();break;
    case 0x80051060u:func_80051060();break;
    case 0x80050204u:func_80050204(value);break;
    case 0x8005022Cu:func_8005022C(value);break;
    case 0x8004FFD0u:func_8004FFD0(value);break;
    case 0x8004F978u:func_8004F978(value);break;
    case 0x800509E0u:func_800509E0(value);break;
    case 0x8004FA10u:func_8004FA10(value);break;
    case 0x8004FB48u:func_8004FB48(value);break;
    case 0x800430A0u:func_800430A0((int32_t)value);break;
    case 0x80050D18u:func_80050D18();break;
    case 0x80045A98u:func_80045A98(value);break;
    case 0x8004F9A0u:func_8004F9A0(value);break;
    case 0x80050AD8u:func_80050AD8((int32_t)value);break;
    case 0x80047040u:func_80047040(value);break;
    case 0x800471BCu:func_800471BC(value);break;
    case 0x8004732Cu:func_8004732C(value);break;
    case 0x800474A8u:func_800474A8(value);break;
    case 0x80050280u:func_80050280(value);break;
    case 0x80050308u:func_80050308(value);break;
    case 0x8004FC80u:func_8004FC80(value);break;
    case 0x80050B48u:func_80050B48(value);break;
    case 0x800453E8u:func_800453E8(value);break;
    case 0x8004F910u:func_8004F910(value);break;
    case 0x80050878u:func_80050878(value);break;
    case 0x8004F950u:func_8004F950(value);break;
    case 0x800509A8u:func_800509A8(value);break;
    case 0x8004FFA8u:func_8004FFA8(value);break;
    case 0x80050CF8u:func_80050CF8();break;
    case 0x8004CDD4u:func_8004CDD4(value);break;
    case 0x80044E14u:func_80044E14(value);break;
    case 0x800447F0u:func_800447F0(value);break;
    case 0x8004F8D0u:func_8004F8D0(value);break;
    case 0x80050804u:func_80050804(value);break;
    case 0x80064EB4u:func_80064EB4(value);break;
    case 0x80043B0Cu:func_80043B0C(value);break;
    case 0x80043C64u:func_80043C64(value);break;
    case 0x8004905Cu:func_8004905C(value);break;
    case 0x80050748u:func_80050748(value);break;
    case 0x8004F838u:func_8004F838(value);break;
    case 0x8004DF74u:func_8004DF74(value);break;
    case 0x8004C608u:func_8004C608(value);break;
    case 0x80050F10u:func_80050F10(value);break;
    case 0x80050F64u:func_80050F64(value);break;
    case 0x80050FB8u:func_80050FB8(value);break;
    case 0x8005100Cu:func_8005100C(value);break;
    case 0x800500A8u:func_800500A8(value);break;
    case 0x8005010Cu:func_8005010C(value);break;
    case 0x80050178u:func_80050178(value);break;
    case 0x800501C8u:func_800501C8(value);break;
    default:
        fprintf(stderr,"[MENU] Unported drawing callback %08X\n",fn);
        Bootstrap_ReturnVoid("PE_MenuDrawCallback","menu drawing callback");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);break;
    }
}

static pe_addr_t list_parent(pe_addr_t child)
{
    pe_addr_t parent=PE_LoadU32(0x8009D154u);unsigned i;
    while (parent) {
        for (i=0;i<4;i++) if (PE_LoadU32(parent+8u+i*4u)==child) return parent;
        parent=PE_LoadU32(parent);
    }
    return 0u;
}

void func_80064EB4(pe_addr_t scrollbar)
{
    pe_addr_t list=PE_LoadU32(scrollbar+52u),parent;
    if (!list) return;
    parent=list_parent(scrollbar);
    func_8005EB58(PE_LoadU32(list_ram(parent+32u))==1u && PE_LoadU32(list_ram(parent+60u))!=0u);
    func_8005E8C4();
    func_8005E8A4((int32_t)(PE_LoadU32(list+60u)*PE_LoadU32(list+52u)+2u),
        (int32_t)(PE_LoadU32(scrollbar+56u)+2u));
    if (func_80062CC4()==scrollbar && (PE_GPU_VSyncQuery()&8u)) func_80062090(8u,PE_LoadU32(scrollbar+60u),0u);
    func_80061C34(8u,PE_LoadU32(scrollbar+60u),0u,1u);
    if (PE_LoadU32(list+92u)) {func_8005E8A4(0,-6);func_8005EB64(74u);func_8005E8A4(0,6);}
    if (list_word(list,92u)<(int32_t)(PE_LoadU32(list+88u)-PE_LoadU32(list+56u))) {
        func_8005E8A4(0,(int32_t)(PE_LoadU32(scrollbar+60u)+2u));func_8005EB64(75u);
    }
    func_8005E914();
}

void func_800634D4(pe_addr_t node,pe_addr_t draw,int32_t row,uint32_t dim)
{
    uint32_t index=PE_LoadU32(node+84u)*(PE_LoadU32(node+92u)+(uint32_t)row);
    uint32_t column=0u,bit=1u<<(index&31u);
    func_8005E8C4();func_8005E8A4(2,2);
    while ((int32_t)column<list_word(node,52u)) {
        uint32_t enabled=1u,predicate=PE_LoadU32(node+140u);
        if (predicate) {
            if (predicate==0x8004FC3Cu) enabled=(uint32_t)func_8004FC3C((int32_t)index++);
            else if (predicate==0x8004FDA4u) enabled=(uint32_t)func_8004FDA4(index++);
            else {
                fprintf(stderr,"[MENU] Unported cell predicate %08X index %u\n",predicate,index);
                Bootstrap_ReturnVoid("PE_MenuCellEnabled","menu cell predicate");
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return;
            }
            PE_StoreU32(node+116u,(PE_LoadU32(node+116u)&~bit)|(enabled?bit:0u));bit<<=1u;
        }
        func_8005EB58(!enabled || (dim && (column!=PE_LoadU32(node+68u) ||
            PE_LoadU32(node+92u)+(uint32_t)row!=PE_LoadU32(node+72u))));
        if (draw) menu_draw_callback(draw,PE_LoadU32(node+52u)*(PE_LoadU32(node+92u)+(uint32_t)row)+column);
        if (PE_Port_ShouldStop()) return;
        if ((PE_GPU_VSyncQuery()&8u) && column==PE_LoadU32(node+76u) &&
            PE_LoadU32(node+92u)+(uint32_t)row==PE_LoadU32(node+80u)) {
            func_8005E8A4(-2,-2);func_80062090(PE_LoadU32(node+60u),PE_LoadU32(node+64u),0u);func_8005E8A4(2,2);
        }
        func_8005E8A4(list_word(node,60u),0);column++;
    }
    func_8005E914();func_8005E8A4(0,list_word(node,64u));
}

void func_8006374C(pe_addr_t node)
{
    pe_addr_t p=node,packet;uint32_t x=0u,y=0u;
    int16_t rect[4];
    while (p) {x+=PE_LoadU32(p+24u);y+=PE_LoadU32(p+28u);p=list_parent(p);}
    rect[0]=(int16_t)x;rect[1]=(int16_t)(y+(PE_LoadU32(0x8009D108u)?224u:0u));
    rect[2]=(int16_t)((uint32_t)list_word(node,60u)*(uint32_t)list_word(node,52u));
    rect[3]=(int16_t)((uint32_t)list_word(node,64u)*(uint32_t)list_word(node,56u));
    packet=PE_MenuPacketAlloc(12u);
    if (packet) PE_SetDrawAreaValues75B84(packet,rect);
    PE_MenuPacketLink(packet);
}

static int32_t list_divide(uint32_t numerator,uint32_t denominator)
{
    if (!denominator || (numerator==0x80000000u && denominator==UINT32_MAX)) {
        Bootstrap_ReturnVoid("func_80065260_divide","menu scrollbar arithmetic trap");
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
    }
    return (int32_t)numerator/(int32_t)denominator;
}

void func_80065260(pe_addr_t scrollbar)
{
    pe_addr_t list;
    uint32_t row_height,visible,total,height,position;int32_t result;
    if (!scrollbar) return;
    list=PE_LoadU32(scrollbar+52u);visible=PE_LoadU32(list+56u);total=PE_LoadU32(list+88u);
    if ((int32_t)visible>=(int32_t)total) return;
    row_height=PE_LoadU32(list+64u);height=row_height*visible;
    result=list_divide(height*visible,total);
    if (PE_Port_ShouldStop()) return;
    PE_StoreU32(scrollbar+60u,(uint32_t)result);
    position=row_height*PE_LoadU32(list+92u)-PE_LoadU32(list+96u);
    result=list_divide(height*position,total*row_height);
    if (!PE_Port_ShouldStop()) PE_StoreU32(scrollbar+56u,(uint32_t)result);
}

void func_800638D8(pe_addr_t node,pe_addr_t draw)
{
    uint32_t dim=(uint32_t)list_word(list_parent(node),60u),row=0u;
    int16_t full[4]={0,0,320,224};pe_addr_t packet;int32_t scroll,step;
    PE_StoreU32(0x8009D164u,PE_LoadU32(node+60u));PE_StoreU32(0x8009D168u,PE_LoadU32(node+64u));
    if (PE_LoadU32(0x8009D108u)) full[1]=224;
    packet=PE_MenuPacketAlloc(12u);
    if (packet) PE_SetDrawAreaValues75B84(packet,full);
    PE_MenuPacketLink(packet);func_8005E8C4();func_8005E8A4(0,list_word(node,96u));
    if (list_word(node,96u)>0) {
        func_8005E8A4(0,(int32_t)(0u-PE_LoadU32(node+64u)));func_800634D4(node,draw,-1,dim);
    }
    while ((int32_t)row<list_word(node,56u)) {
        func_800634D4(node,draw,(int32_t)row++,dim);if (PE_Port_ShouldStop()) return;
    }
    if (list_word(node,96u)<0) func_800634D4(node,draw,(int32_t)row,dim);
    if (PE_Port_ShouldStop()) return;
    func_8005E914();func_8006374C(node);func_80065260(PE_LoadU32(node+128u));
    scroll=list_word(node,96u);step=list_word(node,64u)/2;
    if (scroll>0) {scroll=(int32_t)((uint32_t)scroll-(uint32_t)step);if (scroll<0) scroll=0;}
    else if (scroll<0) {scroll=(int32_t)((uint32_t)scroll+(uint32_t)step);if (scroll>0) scroll=0;}
    PE_StoreU32(node+96u,(uint32_t)scroll);
    if (!scroll && list_word(node,68u)>=0 && list_word(node,72u)>=list_word(node,92u) &&
        list_word(node,72u)<(int32_t)(PE_LoadU32(node+92u)+PE_LoadU32(node+56u))) {
        uint32_t pressed=0u,focused=func_80062CC4()==node;
        func_8005E8C4();
        func_8005E8A4((int32_t)(PE_LoadU32(node+60u)*PE_LoadU32(node+68u)),
            (int32_t)(PE_LoadU32(node+64u)*(PE_LoadU32(node+72u)-PE_LoadU32(node+92u))));
        if (!(PE_LoadU32(node+100u)&128u) && focused && PE_LoadU32(0x8009D0E8u) && (func_8005E038()&32u)) pressed=1u;
        func_800622BC(PE_LoadU32(node+60u),PE_LoadU32(node+64u),pressed,focused);func_8005E914();
    }
}

void func_800500A8(pe_addr_t node)
{
    pe_addr_t group=func_80062A34(2u,23u);
    PE_StoreU32(0x8009CF54u,func_8005DC4C(group?PE_LoadU32(group+72u)+115u:117u));
    func_800638D8(node,0x80050F10u);
}

static void name_control_list(pe_addr_t node,pe_addr_t draw,unsigned rows)
{
    unsigned i;
    PE_StoreU32(0x8009CEF4u,node);func_800638D8(node,draw);func_8005EB58(1u);
    for (i=0;i<rows;i++) {if (i) func_8005E8A4(0,16);func_8005EB64(104u);}
}
void func_8005010C(pe_addr_t node) { name_control_list(node,0x80050F64u,3); }
void func_80050178(pe_addr_t node) { name_control_list(node,0x80050FB8u,2); }
void func_800501C8(pe_addr_t node) { name_control_list(node,0x8005100Cu,1); }

void func_80062830(pe_addr_t node)
{
    unsigned i;
    func_8005E8C4();func_8005E8A4(list_word(node,24u),list_word(node,28u));
    if (PE_LoadU32(node+48u)) {
        func_8005E8C4();menu_draw_callback(PE_LoadU32(node+48u),node);func_8005E914();
        if (PE_Port_ShouldStop()) return;
    }
    for (i=0;i<4;i++) if (PE_LoadU32(node+8u+i*4u)) {
        func_80062830(PE_LoadU32(node+8u+i*4u));if (PE_Port_ShouldStop()) return;
    }
    func_8005E914();
}

void func_80062FEC(void)
{
    pe_addr_t node=PE_LoadU32(0x8009D154u);uint32_t modal=0u,ancestor=0u;
    while (node) {
        if (PE_LoadU32(node+32u)==1u) {PE_StoreU32(node+60u,0u);modal|=PE_LoadU32(node+68u);}
        node=PE_LoadU32(node);
    }
    node=func_80062CC4();
    while (node) {
        if (PE_LoadU32(node+32u)==1u) {PE_StoreU32(node+60u,ancestor);ancestor=1u;}
        node=PE_LoadU32(node+4u);
    }
    node=PE_LoadU32(0x8009D154u);
    while (node) {
        if (PE_LoadU32(node+32u)==1u && !PE_LoadU32(node+72u)) {
            uint32_t dim=PE_LoadU32(node+64u)?0u:PE_LoadU32(node+60u)&1u;
            if (modal && !PE_LoadU32(node+68u)) dim|=1u;
            PE_StoreU32(node+60u,dim);func_8005EB58(dim);PE_StoreU32(0x8009D124u,0u);PE_StoreU32(0x8009D128u,0u);
            PE_StoreU32(0x8009D138u,PE_LoadU32(node+52u));func_80062830(node);
            if (PE_Port_ShouldStop()) return;
            PE_StoreU32(0x8009D124u,PE_LoadU32(node+24u));PE_StoreU32(0x8009D128u,PE_LoadU32(node+28u));
            func_8005EB58(PE_LoadU32(node+60u));
            func_80061C34(PE_LoadU32(node+52u),PE_LoadU32(node+56u),PE_LoadU32(node+76u),PE_LoadU32(node+68u));
        }
        node=PE_LoadU32(node);
    }
}
