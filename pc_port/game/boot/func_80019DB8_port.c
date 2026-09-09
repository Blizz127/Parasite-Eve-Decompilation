/* Script D2/D3,83 original words19DB8..19F04.
 * SHA256 f684e1f70afe44e8a2195316e020111fecf4c6ad3f99e2e1e3b0ad297662effe. */
#include "pe_port_compat.h"
int func_80019DB8(pe_addr_t args)
{
    uint32_t index=PE_LoadU32(PE_LoadU32(args));
    pe_addr_t destination=PE_LoadU32(args+4u);
    PE_StoreU32(destination,PE_LoadU32(0x800A76A4u+index*12u));
    return 1;
}
int func_80019DF4(pe_addr_t args)
{
    /* Original signed multiply-high division sequences truncate toward zero.
     * Reload both the source pointer and its value after each store. */
    int32_t ticks=(int32_t)PE_LoadU32(PE_LoadU32(args));
    PE_StoreU32(PE_LoadU32(args+4u),(uint32_t)(ticks/216000));
    ticks=(int32_t)PE_LoadU32(PE_LoadU32(args));
    PE_StoreU32(PE_LoadU32(args+8u),(uint32_t)((ticks%216000)/3600));
    ticks=(int32_t)PE_LoadU32(PE_LoadU32(args));
    PE_StoreU32(PE_LoadU32(args+12u),(uint32_t)((ticks%3600)/60));
    return 1;
}
