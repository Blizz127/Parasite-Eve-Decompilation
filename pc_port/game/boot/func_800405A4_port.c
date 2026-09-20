/* Original card-status polling machine, 405A4..409B4 (260 words).
 *
 * The BIOS event drain (B(0Bh) TestEvent, func_800726F4) is a real host
 * adapter now (pe_libetc.c): the retail callers deliberately discard its
 * return value and only drain the four card event handles, so this path
 * returns instead of stopping.  The card kernel operations themselves
 * (A0 ABh/ACh and the B0 4Eh/50h wrappers under func_8007DD74) are real
 * host adapters too (pe_libcard.c): they model the documented empty-slot
 * timeout and deliver the F4000001h/F0000011h completion events, so the
 * status machine advances instead of stopping at a named boundary. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

static int card_status_events(pe_addr_t handles)
{
    /* Retail issues four TestEvent calls and discards every result
     * (307CC.s 80040644/54/64/74, 800406FC/70C/71C/72C, 80040780/90/FA0/FB0,
     * 8004093C/94C/95C/96C): the drain, not the value, is the effect. */
    for(unsigned i=0;i<4;i++)
        (void)func_800726F4((int)PE_LoadU32(handles+i*4u));
    return 1;
}

int func_8004D4A0(void) { return func_80062A34(1u,36u)!=0u; }

void func_800405A4(uint32_t index)
{
    pe_addr_t record=0x800A0ED4u+index*0x418u;
    unsigned epoch=PE_Port_StopEpoch();
    switch(PE_LoadU8(record+8u)) {
    case 0:
        PE_StoreU8(record,0u);goto poll;
    case 1:
        if(PE_LoadU32(0x800A1820u)) {
            PE_StoreU32(0x800A1820u,0u);
            if(PE_LoadU8(record)&1u) {PE_StoreU8(record+8u,4u);goto alternate;}
            goto begin_read;
        }
        if(PE_LoadU32(0x800A1824u)) {
            PE_StoreU32(0x800A1824u,0u);
            uint32_t flags=PE_LoadU8(record);
            PE_StoreU8(record+8u,0u);PE_StoreU8(record,flags&0xFBu);goto alternate;
        }
        if(!PE_LoadU32(0x800A1828u))return;
        PE_StoreU32(0x800A1828u,0u);
    begin_read:
        if(!card_status_events(0x800BCDB8u))return;
        PE_StoreU32(0x800A1834u,0u);PE_StoreU32(0x800A1830u,0u);PE_StoreU32(0x800A182Cu,0u);
        (void)func_8007DD74((int)(index<<4));
        PE_StoreU8(record+8u,2u);return;
    case 2:
        if(PE_LoadU32(0x800A182Cu)) {
            PE_StoreU32(0x800A182Cu,0u);
            if(!card_status_events(0x800BCDA8u))return;
            PE_StoreU32(0x800A1828u,0u);PE_StoreU32(0x800A1824u,0u);PE_StoreU32(0x800A1820u,0u);
            (void)func_8007DD54((int)(index<<4));
            PE_StoreU8(record+8u,3u);return;
        }
        if(!PE_LoadU32(0x800A1830u) && !PE_LoadU32(0x800A1834u))return;
        PE_StoreU32(0x800A1830u,0u);PE_StoreU32(0x800A1834u,0u);
        PE_StoreU8(record+8u,0u);goto alternate;
    case 3:
        if(PE_LoadU32(0x800A1820u)) {
            PE_StoreU32(0x800A1820u,0u);PE_StoreU8(record+8u,4u);
            if(!func_8004D4A0()) {
                PE_StoreU8(record,PE_LoadU8(record)|1u);func_8004298C(index,0u);
                if(PE_Port_StopEpoch()!=epoch)return;
            }
            goto alternate;
        }
        if(PE_LoadU32(0x800A1824u)) {
            PE_StoreU32(0x800A1824u,0u);PE_StoreU8(record+8u,0u);goto alternate;
        }
        if(!PE_LoadU32(0x800A1828u))return;
        PE_StoreU32(0x800A1828u,0u);
        {uint32_t flags=PE_LoadU8(record);PE_StoreU8(record+8u,4u);PE_StoreU8(record,flags|4u);}
        goto alternate;
    case 4:
        PE_StoreU8(record,PE_LoadU8(record)|1u);goto poll;
    default:return;
    }
    alternate: {
        uint32_t old=PE_LoadU32(0x800A183Cu);
        PE_StoreU32(0x800A1840u,0u);PE_StoreU32(0x800A183Cu,old==0u);return;
    }
    poll:
    if(PE_LoadU32(0x800A1838u) || index!=PE_LoadU32(0x800A183Cu))return;
    uint32_t timer=PE_LoadU32(0x800A1840u);
    PE_StoreU32(0x800A1840u,timer-1u);
    if((int32_t)timer>0)return;
    if(!card_status_events(0x800BCDA8u))return;
    PE_StoreU32(0x800A1828u,0u);PE_StoreU32(0x800A1824u,0u);PE_StoreU32(0x800A1820u,0u);
    (void)func_8007DD44((int)(index<<4));
    PE_StoreU8(record+8u,1u);
}

void func_8004D9D8(void) { func_80062F1C(func_80062A34(1u,39u)); }
void func_8004DC84(void) { func_80062F3C(42u); }
void func_8004CDAC(void)
{
    unsigned epoch=PE_Port_StopEpoch();func_80062F3C(40u);
    if(PE_Port_StopEpoch()!=epoch)return;
    func_80062F3C(61u);
}
void func_80042928(void)
{
    unsigned epoch=PE_Port_StopEpoch();func_8004298C(PE_LoadU32(0x800A1860u)-1u,1u);
    if(PE_Port_StopEpoch()!=epoch)return;
    PE_StoreU32(0x800A1860u,0u);PE_StoreU32(0x800A1868u,0u);
}

void func_800425DC(void)
{
    unsigned epoch=PE_Port_StopEpoch();
    func_800405A4(1u);if(PE_Port_StopEpoch()!=epoch)return;
    func_800405A4(0u);if(PE_Port_StopEpoch()!=epoch)return;
    if(PE_LoadU32(0x800A1864u)) {
        uint32_t index=PE_LoadU32(0x800A1860u)-1u;
        uint32_t status=PE_LoadU8(0x800A0EDCu+index*0x418u);
        if(status!=4u && status!=1u)PE_StoreU32(0x800A1864u,0xFFFFFFFEu);
        uint32_t timer=PE_LoadU32(0x800A1864u);
        timer-=(int32_t)timer>0;PE_StoreU32(0x800A1864u,timer);
        if((int32_t)timer<=0) {
            func_8004D9D8();if(PE_Port_StopEpoch()!=epoch)return;
            timer=PE_LoadU32(0x800A1864u);
            if(timer==0u || timer==0xFFFFFFFFu) {
                func_8004CC50(timer==0u?0x52u:0x3Cu,0u);
                if(PE_Port_StopEpoch()!=epoch)return;
                func_8004D024(timer==0u?0x80042928u:0x80042910u);
                PE_StoreU32(0x800A1868u,1u);
            } else func_80042910();
            PE_StoreU32(0x800A1864u,0u);
        }
    }
    for(int index=1;index>=0;index--) {
        if(PE_LoadU32(0x800A1860u)==(uint32_t)index+1u &&
           !(PE_LoadU8(0x800A0ED4u+(uint32_t)index*0x418u)&1u)) {
            func_8004DC84();if(PE_Port_StopEpoch()!=epoch)return;
            func_8004CDAC();if(PE_Port_StopEpoch()!=epoch)return;
            func_80042910();
        }
        func_80041108((uint32_t)index);
        if(PE_Port_StopEpoch()!=epoch)return;
    }
}
