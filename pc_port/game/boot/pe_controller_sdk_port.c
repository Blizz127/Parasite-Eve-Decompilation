/* Original controller queries/configuration used by 8003EB04.
 * Combined source SHA256 fa58951f31f09efabde626872321aeacd180bc41e70fdcafcec1eea1cc446918.
 * Dispatch only the callbacks installed by original 844E4/84B44; unknown
 * callback addresses retain an explicit boundary instead of inventing state. */
#include "pe_port_compat.h"
#include "game_port.h"

pe_addr_t func_80084B20(uint32_t port)
{ return 0x800A5B70u+((port&0xF0u)?0xF0u:0u); }
int func_80084F8C(pe_addr_t record)
{ return !PE_LoadU16(record+0xE6u) || PE_LoadU8(record+0x46u)!=255u; }
static pe_addr_t controller_record(uint32_t port)
{
    if(PE_LoadU32(0x8009B738u)!=0x80084B20u) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
    }
    return func_80084B20(port);
}
static int controller_busy(pe_addr_t record)
{
    if(PE_LoadU32(0x8009B740u)!=0x80084F8Cu) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 1;
    }
    return func_80084F8C(record);
}
int func_800825C0(uint32_t port)
{
    pe_addr_t p=controller_record(port);if(!p)return 0;
    if(!(PE_LoadU32(p+0x34u)&0xFFFF0000u)) {
        if(p==PE_LoadU32(p+0x10u) || !PE_LoadU8(p+0x38u)) {
            if(!PE_LoadU8(PE_LoadU32(p+0x30u)))return PE_LoadU8(p+0x49u);
        }
    }
    unsigned state=PE_LoadU8(p+0x49u);
    if(state==2u || state==3u)return 1;
    if(state==6u)return 4;
    return (int)state;
}
uint32_t func_80082680(uint32_t port,int32_t info,int32_t index)
{
    pe_addr_t p=controller_record(port);if(!p)return 0;
    switch(info) {
    case 1:return PE_LoadU8(p+0xE8u);
    case 2:return PE_LoadU16(p+0xE6u);
    case 3:return PE_LoadU8(p+0xE4u);
    case 4:
        if(index<0)return PE_LoadU8(p+0xE3u);
        if(index<PE_LoadU8(p+0xE3u))return PE_LoadU16(PE_LoadU32(p)+(uint32_t)index*2u);
        return 0;
    case 100:return PE_LoadU32(p+0x4Cu);
    default:return 0;
    }
}
void func_800835A4(pe_addr_t record,pe_addr_t motors,uint32_t count)
{
    PE_StoreU32(record+0x28u,motors);PE_StoreU8(record+0x34u,(uint8_t)count);
}
void func_80082974(uint32_t port,pe_addr_t motors,uint32_t count)
{
    pe_addr_t p=controller_record(port);if(!p)return;
    func_800835A4(p,motors,count);
}
int func_80083BB8(pe_addr_t record,pe_addr_t alignment)
{
    if(controller_busy(record))return 0;
    PE_StoreU8(record+0x46u,1);
    PE_StoreU32(record+0x14u,0x80083C20u);
    PE_StoreU32(record+0x20u,alignment);
    PE_StoreU32(record+0x18u,0x80083C3Cu);
    return 1;
}
int func_800828F4(uint32_t port,pe_addr_t alignment)
{
    pe_addr_t p=controller_record(port);if(!p)return 0;
    return func_80083BB8(p,alignment);
}
int func_80083D04(pe_addr_t record,uint32_t mode,uint32_t lock)
{
    if(controller_busy(record))return 0;
    unsigned current=PE_LoadU8(record+0xE4u);
    PE_StoreU8(record+0x46u,1);
    PE_StoreU32(record+0x14u,0x80083D9Cu);
    PE_StoreU32(record+0x18u,0x80083DF0u);
    PE_StoreU8(record+0x51u,(uint8_t)mode);
    PE_StoreU8(record+0x52u,(uint8_t)lock);
    PE_StoreU8(record+0x53u,(uint8_t)((mode&255u)==current));
    return 1;
}
int func_8008292C(uint32_t port,uint32_t mode,uint32_t lock)
{
    pe_addr_t p=controller_record(port);if(!p)return 0;
    return func_80083D04(p,mode&255u,lock&255u);
}
