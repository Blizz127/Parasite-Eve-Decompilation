/* Save/load page support: the D_800C0E00 status-screen builder and the two
 * list-rendering callbacks the 0x21/0x23 pages install. 4E194.s / 42664.s /
 * 41470.s. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"

/* Status/name-entry setup for the file menu.  D_800C0E0A holds the table
 * index (0x62) used to pick the D_800C0E00 descriptor, and the seven u16
 * slots at D_800C0E28 receive 0 for lanes 1/2 when a nonzero mode is passed
 * (retail stores 1000 for every other lane). */
void func_8005D994(int32_t mode)
{
    pe_addr_t descriptor;
    uint32_t value;
    unsigned i;
    PE_StoreU8(0x800C0E0Cu,0x32u);
    if (PE_LoadU32(0x8009D048u)==0x800C0E48u)
        PE_StoreU32(0x8009D050u,func_80052F70());
    PE_StoreU8(0x800C0E0Au,0x62u);
    descriptor=func_8005DBF8()+(uint32_t)PE_LoadU8(0x800C0E0Au)*4u;
    PE_StoreU32(0x800C0E00u,PE_LoadU32(descriptor));
    value=(uint32_t)PE_LoadU16(func_8005DBAC(0x62));
    PE_StoreU32(0x800C0E24u,0x000FFFFFu);
    PE_StoreU16(0x800C0E08u,(uint16_t)value);
    PE_StoreU16(0x800C0E06u,(uint16_t)value);
    for (i=0;i<7;i++)
        PE_StoreU16(0x800C0E28u+i*2u,(uint16_t)((mode!=0&&(i==1u||i==2u))?0u:0x3E8u));
    func_8005247C();
}

/* Copy the record's +0x1C halfword over its +0x0C/+0x0E cursor pair. */
void func_8005247C(void)
{
    pe_addr_t record;
    func_8005218C();
    record=PE_LoadU32(0x8009D254u);
    if (!record) return;
    record=PE_LoadU32(record);
    if (!record) return;
    PE_StoreU16(record+0x0Eu,PE_LoadU16(record+0x1Cu));
    PE_StoreU16(record+0x0Cu,PE_LoadU16(record+0x1Cu));
}

void func_80050C70(int32_t slot)
{
    func_80064C54((uint32_t)(slot+0x2E));
    if (func_800527B4()==slot) func_80064C80();
}

void func_80050CB4(int32_t slot)
{
    func_80064C54((uint32_t)(slot+0x31));
    if (func_80064A48()==slot) func_80064C80();
}
