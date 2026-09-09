/* Original card-operation dispatch and paths through their first unresolved
 * callee. The post-call directory/transfer/retry paths remain unported.
 * Boundary payload: known-register mask, then fifth argument when present.
 * Unknown guest-stack arguments are not assigned fabricated guest addresses. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

static void operation_boundary(const char *caller,pe_addr_t target,uint32_t a0,uint32_t a1,
                               uint32_t a2,uint32_t a3,uint32_t known,uint32_t fifth)
{
    uint32_t payload[2]={known,fifth};
    (void)Bootstrap_ReturnInt4Indirect("card operation unresolved call",caller,0,
        target,a0,a1,a2,a3,payload,sizeof(payload));
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

/* Optional explicit guest context; NULL retains the unresolved live boundary. */
static void card_operation(uint32_t index,pe_addr_t frame,const uint32_t *incoming)
{
    pe_addr_t record=0x800A0ED4u+index*0x418u;
    uint32_t state=PE_LoadU8(record+1u),selected;
    if(state==0u || state>=15u)return;
    if(state==13u) {
        if(PE_LoadU8(record)!=5u) {
            PE_StoreU32(0x800A1864u,0xFFFFFFFFu);PE_StoreU8(record+1u,0u);return;
        }
        goto ready;
    }
    if(state==14u) {
        if(incoming) {
            uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
            saved[0]=record;saved[2]=index;saved[8]=0x80041284u;
            unsigned epoch=PE_Port_StopEpoch();
            (void)PE_FormatterFrame(frame+24u,0x80010F60u,index,incoming[7],frame,saved);
            if(PE_Port_StopEpoch()!=epoch)return;
            operation_boundary("PE_CardOperationFrame",0x80072784u,frame+24u,0u,0u,0u,1u,0u);return;
        }
        /* a0 is the unresolved original frame's sp+18h, not a RAM scratch. */
        operation_boundary("func_80041108",0x80071A84u,0u,0x80010F60u,index,0u,6u,0u);return;
    }
    if(PE_LoadU8(record)!=1u) {
        if(incoming) {
            uint32_t cleanup[32];for(unsigned i=0;i<32;i++)cleanup[i]=incoming[i];
            cleanup[16]=record;cleanup[18]=index;cleanup[31]=0x80042000u;
            if(state>=4u && state<=10u)cleanup[17]=PE_LoadU8(record);
            if(state==11u)cleanup[19]=PE_LoadU8(record);
            PE_CardCleanupFrame(record,frame,cleanup);
        } else func_80040F80(record);
        return;
    }
    switch(state) {
    case 1:
    ready: {
        if(PE_LoadU8(record+8u)!=4u)return;
        uint32_t other=PE_LoadU8(0x800A0EDCu+(index==0u?0x418u:0u));
        if(other!=0u && other!=4u)return;
        PE_StoreU32(0x800A1838u,1u);
        PE_StoreU8(record+1u,state==13u?14u:PE_LoadU8(record+11u));return;
    }
    case 2:
        PE_StoreU8(PE_LoadU32(0x80092230u)+2u,index+0x30u);
        PE_StoreU8(record+2u,0u);PE_StoreU8(record+3u,0u);PE_StoreU8(record+6u,0u);
        PE_StoreU8(record+4u,0u);PE_StoreU8(record+7u,0u);PE_StoreU8(record+10u,0u);
        for(int i=14;i>=0;i--)PE_StoreU8(record+(uint32_t)i*0x44u+28u,2u);
        /* a1 is the unresolved original frame's sp+20h. */
        operation_boundary("func_80041108",0x800727B4u,PE_LoadU32(0x80092230u),incoming?frame+32u:0u,0u,0u,incoming?3u:1u,0u);return;
    case 3: {
        uint32_t cursor=PE_LoadU8(record+6u),limit=PE_LoadU8(record+2u)*2u;
        if(cursor<limit) {
            uint32_t center=PE_LoadU8(record+5u);
            do {
                uint32_t slot=center+(((cursor&1u)*2u-1u)*(((cursor&255u)+1u)>>1));
                if(slot<15u && !PE_LoadU8(record+slot*0x44u+29u))break;
                cursor++;PE_StoreU8(record+6u,cursor);
            } while((cursor&255u)<limit);
        }
        cursor=PE_LoadU8(record+6u);limit=PE_LoadU8(record+2u)*2u;
        selected=cursor<limit?PE_LoadU8(record+5u)+(((cursor&1u)*2u-1u)*((cursor+1u)>>1)):255u;
        PE_StoreU8(record+3u,selected);selected&=255u;
        if(selected==255u) {
            PE_StoreU8(record+1u,12u);PE_StoreU32(0x800A1838u,0u);return;
        }
        goto format_name;
    }
    case 4:
        selected=PE_LoadU8(record+3u);
        PE_StoreU8(record+selected*0x44u+69u,PE_LoadU32(0x800A1704u));
        selected=PE_LoadU8(record+3u);goto format_name;
    case 5:case 6:case 11:
        selected=PE_LoadU8(record+3u);
    format_name:
        if(incoming) {
            uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
            saved[0]=record;saved[2]=index;
            if(state==4u || state==5u || state==6u)saved[1]=1u;
            if(state==11u) {saved[2]=record>0x800A0ED4u;saved[3]=1u;}
            saved[8]=state==3u?0x800416D8u:state==4u?0x80041834u:
                state==5u?0x80041908u:state==6u?0x800419D4u:0x80041E98u;
            pe_addr_t format=PE_LoadU32(0x80092224u);
            uint32_t variant=PE_LoadU8(record+selected*0x44u+69u)+0x30u;
            PE_StoreU32(frame+16u,selected+0x41u);
            unsigned epoch=PE_Port_StopEpoch();
            (void)PE_FormatterFrame(0x8009EE70u,format,record>0x800A0ED4u,variant,frame,saved);
            if(PE_Port_StopEpoch()!=epoch)return;
            operation_boundary("PE_CardOperationFrame",0x80072734u,0x8009EE70u,
                state==4u?0x10200u:state==6u?2u:1u,0u,0u,3u,0u);return;
        }
        operation_boundary("func_80041108",0x80071A84u,0x8009EE70u,PE_LoadU32(0x80092224u),
            record>0x800A0ED4u,PE_LoadU8(record+selected*0x44u+69u)+0x30u,15u,selected+0x41u);return;
    case 7:case 8:case 9: {
        int32_t remaining=(int16_t)PE_LoadU16(record+20u),cap=state==8u?128:1024;
        operation_boundary("func_80041108",state==9u?0x80072764u:0x80072754u,
            PE_LoadU32(record+12u),PE_LoadU32(record+24u),
            (uint32_t)(remaining<cap+1?remaining:cap),0u,7u,0u);return;
    }
    case 10:
        operation_boundary("func_80041108",0x80072774u,PE_LoadU32(record+12u),0u,0u,0u,1u,0u);return;
    case 12:return;
    }
}

void func_80041108(uint32_t index)
{
    card_operation(index,0u,0);
}

/* Memory/output adapter for a supplied original call context. Cleanup and BIOS
 * remain explicit boundaries; no return/register state is synthesized for them. */
void PE_CardOperationFrame(uint32_t index,pe_addr_t caller_sp,const uint32_t incoming[32])
{
    pe_addr_t frame=caller_sp-0x78u;
    PE_StoreU32(frame+0x68u,incoming[18]);PE_StoreU32(frame+0x60u,incoming[16]);
    PE_StoreU32(frame+0x70u,incoming[31]);PE_StoreU32(frame+0x6Cu,incoming[19]);
    PE_StoreU32(frame+0x64u,incoming[17]);
    card_operation(index,frame,incoming);
}

void func_80042228(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    const uint32_t ids[]={38,37,36};
    for(unsigned i=0;i<3;i++) {func_80062F3C(ids[i]);if(PE_Port_StopEpoch()!=epoch)return;}
    func_800512AC(12,0u);
}

void func_8004D5CC(uint32_t index)
{
    unsigned epoch=PE_Port_StopEpoch();pe_addr_t list=func_80062A34(2u,36u);
    func_80062F3C(40u);if(PE_Port_StopEpoch()!=epoch)return;
    pe_addr_t callback=PE_LoadU32(0x8009CFFCu);
    if(callback) {
        switch(callback) {
        case 0x80042910u:func_80042910();break;
        case 0x80042928u:func_80042928();break;
        case 0x8005C488u:func_8005C488();break;
        case 0x80062F9Cu:func_80062F9C();break;
        default:operation_boundary("func_8004D5CC",callback,0u,0u,0u,0u,0u,0u);return;
        }
        if(PE_Port_StopEpoch()!=epoch)return;
        PE_StoreU32(0x8009CFFCu,0u);
    }
    if(index!=PE_LoadU32(0x8009CF44u))return;
    func_80062F3C(63u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(39u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F1C(func_80062A34(1u,41u));if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(31u);if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(index+37u);if(PE_Port_StopEpoch()!=epoch)return;
    if(list) {
        PE_StoreU32(list+68u,0u);
        if(!func_800631DC())func_80062CB8(list);
    }
}

void func_80040F80(pe_addr_t record)
{
    unsigned epoch=PE_Port_StopEpoch();
    if(PE_LoadU32(0x800A185Cu)) {func_80042228();return;}
    uint32_t handle=PE_LoadU32(record+12u);
    if((int32_t)handle>=0) {
        operation_boundary("func_80040F80",0x80072774u,handle,0u,0u,0u,1u,0u);return;
    }
    if(PE_LoadU8(record+1u)==9u) {
        uint32_t selected=PE_LoadU8(record+3u);
        operation_boundary("func_80040F80",0x80071A84u,0x8009EE70u,PE_LoadU32(0x80092224u),
            record>0x800A0ED4u,PE_LoadU8(record+selected*0x44u+69u)+0x30u,15u,selected+0x41u);return;
    }
    uint32_t product=(record-0x800A0ED4u)*0xC9484E2Bu;
    func_8004D5CC((uint32_t)((int32_t)product>>3));
    if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU8(record+1u,0u);PE_StoreU8(record+4u,0u);
    PE_StoreU32(0x800A1854u,0u);PE_StoreU32(0x800A1838u,0u);func_80062CE4();
}

/* Original40F80 frame through the first unresolved callee. The direct native
 * entry above continues to own returning cleanup without a guest frame. */
void PE_CardCleanupFrame(pe_addr_t record,pe_addr_t caller_sp,const uint32_t incoming[32])
{
    uint32_t override=PE_LoadU32(0x800A185Cu);
    pe_addr_t frame=caller_sp-0x50u;
    PE_StoreU32(frame+0x40u,incoming[18]);PE_StoreU32(frame+0x48u,incoming[31]);
    PE_StoreU32(frame+0x44u,incoming[19]);PE_StoreU32(frame+0x3Cu,incoming[17]);
    PE_StoreU32(frame+0x38u,incoming[16]);
    if(override) {
        operation_boundary("PE_CardCleanupFrame",0x80042228u,0u,0u,0u,0u,0u,0u);return;
    }
    uint32_t handle=PE_LoadU32(record+12u);
    if((int32_t)handle>=0) {
        operation_boundary("PE_CardCleanupFrame",0x80072774u,handle,0u,0u,0u,1u,0u);return;
    }
    uint32_t index=(uint32_t)((int32_t)((record-0x800A0ED4u)*0xC9484E2Bu)>>3);
    if(PE_LoadU8(record+1u)!=9u) {
        operation_boundary("PE_CardCleanupFrame",0x8004D5CCu,index,0u,0u,0u,1u,0u);return;
    }
    uint32_t saved[9];for(unsigned i=0;i<8;i++)saved[i]=incoming[16u+i];
    saved[0]=index;saved[1]=0u;saved[2]=record;saved[3]=0xFFFFFFFFu;saved[8]=0x80041048u;
    uint32_t selected=PE_LoadU8(record+3u);
    pe_addr_t format=PE_LoadU32(0x80092224u);
    uint32_t variant=PE_LoadU8(record+selected*0x44u+69u)+0x30u;
    PE_StoreU32(frame+16u,selected+0x41u);
    unsigned epoch=PE_Port_StopEpoch();
    (void)PE_FormatterFrame(0x8009EE70u,format,record>0x800A0ED4u,variant,frame,saved);
    if(PE_Port_StopEpoch()!=epoch)return;
    /* Reload original callee-saved memory for the next formatter call. */
    for(unsigned i=0;i<8;i++)saved[i]=PE_LoadU32(frame-0x250u+0x228u+i*4u);
    saved[8]=0x80041064u;
    (void)PE_FormatterFrame(frame+24u,0x80010F4Cu,saved[0],0x8009EE70u,frame,saved);
    if(PE_Port_StopEpoch()!=epoch)return;
    operation_boundary("PE_CardCleanupFrame",0x80072734u,frame+24u,1u,0u,0u,3u,0u);
}
