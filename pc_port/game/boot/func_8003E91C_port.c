/* Original VBlank game callback3E91C..3E944 and timer tick36F7C..3708C.
 * Combined with7440C dispatcher source SHA256 8b623f46317d202830636347024c7c3fb20712bc247b0e6869521c6c5a7c019f. */
#include "pe_port_compat.h"
#include "game_port.h"
extern int func_8006EC08(void);
void func_80036F7C(void)
{
    if(PE_LoadU32(0x8009D1A0u)&0x41u)return;
    uint32_t epoch=PE_Port_StopEpoch();
    int busy=func_8006EC08();
    if(PE_Port_StopEpoch()!=epoch || (busy&255))return;
    for(unsigned i=0;i<4;i++) {
        pe_addr_t p=0x800A76A0u+i*12u;
        uint32_t flags=PE_LoadU32(p);
        if((flags&1u) && !(flags&4u)) {
            /* At this point flags&5 is necessarily nonzero. The original
             * bounded-counter branches cannot be reached from this entry. */
            uint32_t value=PE_LoadU32(p+4u);
            PE_StoreU32(p+4u,value+((flags&2u)?UINT32_MAX:1u));
        }
    }
}
void func_8003E91C(void)
{
    uint32_t epoch=PE_Port_StopEpoch();
    (void)func_80070D6C();
    if(PE_Port_StopEpoch()!=epoch)return;
    func_80036F7C();
}
