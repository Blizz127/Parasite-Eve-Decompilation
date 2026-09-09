/* Original inventory description and filtered-list lookups.
 * 4E44C.s / 43CE4.s / 44AA0.s / 486D8.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"

pe_addr_t func_8005DCEC(uint32_t id)
{
    pe_addr_t archive=0x800A8028u+PE_LoadU32(0x800A802Cu);
    pe_addr_t table=archive+PE_LoadU32(archive+12u);
    if (id>=PE_LoadU16(table)) return 0u;
    return table+(uint32_t)(int32_t)(int16_t)PE_LoadU16(table+id*2u+2u);
}

int32_t func_8005415C(int32_t index)
{
    pe_addr_t record=func_8005332C(index);
    return record?PE_LoadU8(record+6u):0;
}

int32_t func_80054288(void) {return (int32_t)PE_LoadU32(0x8009D040u);}

static int32_t filtered_item(int32_t index,pe_addr_t count,pe_addr_t table)
{
    if (index<0 || index>=(int32_t)PE_LoadU32(count)) return 0;
    return (int16_t)PE_LoadU16(table+(uint32_t)index*2u);
}
int32_t func_800556E8(int32_t index) {return filtered_item(index,0x8009D040u,0x800A1D9Cu);}
int32_t func_80058E08(int32_t index) {return filtered_item(index,0x8009D044u,0x800A1E00u);}
int32_t func_80057ED8(int32_t index) {return filtered_item(index,0x8009D078u,0x800A1FD4u);}

pe_addr_t func_80058BBC(int32_t index)
{
    int32_t value=(int16_t)PE_LoadU16(PE_LoadU32(0x8009D07Cu)+(uint32_t)index*2u);
    if ((uint32_t)(value-256)<128u) return 0x800BEEACu+(uint32_t)value*32u;
    if ((uint32_t)(value-1)<255u) return func_8005DB44((uint32_t)(value-1));
    if ((uint32_t)(value-512)<9u) return 0x8009DE64u+(uint32_t)value*32u;
    return 0u;
}

int32_t func_80059F08(uint32_t index)
{
    if (index>=2u) return -1;
    func_80052E30(PE_LoadU32(0x8009D098u+index*4u));
    return (int32_t)PE_LoadU32(0x8009D090u+index*4u);
}

void func_80055610(void)
{
    uint32_t bits=PE_LoadU32(0x800C0E24u),count=0u,i;
    for (i=0;i<20u;i++,bits>>=1u) if (bits&1u) PE_StoreU16(0x800A1D9Cu+count++*2u,(uint16_t)i);
    PE_StoreU32(0x8009D068u,0u);PE_StoreU32(0x8009D040u,count);
}
