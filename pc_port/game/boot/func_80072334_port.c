/* Original formatter byte-copy helper,72334..723A0 (27 words).
 * Direction follows unsigned guest addresses, even across physical aliases.
 * Forward copying returns the advanced destination; backward copying does not. */
#include "psx_compat.h"
#include "pe_port_compat.h"
static pe_addr_t copy_access(pe_addr_t address)
{
    /* The port RAM API uses KSEG0; retain raw addresses in copy control flow. */
    if(address<0x200000u || (address&0xFFE00000u)==0xA0000000u)
        return 0x80000000u|(address&0x1FFFFFu);
    return address;
}
pe_addr_t func_80072334(pe_addr_t destination,pe_addr_t source,uint32_t count)
{
    if((int32_t)count<=0)return destination;
    if(destination>=source) {
        do {--count;PE_StoreU8(copy_access(destination+count),PE_LoadU8(copy_access(source+count)));} while(count);
    } else {
        do {uint8_t byte=PE_LoadU8(copy_access(source++));PE_StoreU8(copy_access(destination++),byte);} while(--count);
    }
    return destination;
}
