/* Club effect constructor, target origin, spark initialization and drawing.
 * Original BE74C/BE96C/BE9FC/BEBB4.s; CE144 is matching src/ C.
 * Shared storage, VM, rendering and matched update leaves remain native. */
#include "psx_compat.h"
#include "pe_port_compat.h"

int func_800CE084(pe_addr_t slot)
{
    unsigned i;
    PE_StoreU32(func_800C22F8(slot),0x800E0FFCu);
    for (i=0;i<3;i++) PE_StoreU32(0x800E22B8u+i*4u,0x300u);
    PE_StoreU8(0x800E22CDu,5u);PE_StoreU16(0x800E22CEu,(uint16_t)-100);
    for (i=0;i<3;i++) PE_StoreU16(0x800E22B0u+i*2u,0u);
    for (i=0;i<3;i++) PE_StoreU8(0x800E22C8u+i,128u);
    return 0;
}

int func_800CE16C(pe_addr_t slot)
{
    int result=func_800C251C(slot,0x800E0FCCu);
    result|=func_800C2758(slot,0x800E0FB4u,0x800E0FD8u);
    if (result==-1) PE_StoreU8(slot,4u); /* Matching CE1DC. */
    return 0;
}

void func_800CE1FC(void)
{
    pe_addr_t body=PE_LoadU32(0x800E2248u);
    uint32_t index=PE_LoadU32(body+60u);
    pe_addr_t list=PE_LoadU32(body+76u),actor=PE_LoadU32(list+index*4u);
    unsigned i;
    PE_StoreU32(0x800E2848u,actor);
    if (!PE_LoadU32(list+(index+1u)*4u)) PE_StoreU32(PE_LoadU32(0x800E2248u)+64u,1u);
    PE_StoreU32(PE_LoadU32(0x800E2248u)+60u,index+1u);
    actor=PE_LoadU32(0x800E2848u);
    for (i=0;i<3;i++) PE_StoreU16(0x800E2808u+i*2u,PE_LoadU16(actor+616u+i*2u));
    func_800CEDA8(0);
}

void func_800CE2B4(pe_addr_t data)
{
    uint32_t random=func_80071A54();
    PE_StoreU16(data+6u,(uint16_t)(PE_LoadU16(0x800E2808u)-40u+(uint32_t)((int32_t)random%80)));
    random=func_80071A54();
    PE_StoreU16(data+8u,(uint16_t)(PE_LoadU16(0x800E280Au)-40u+(uint32_t)((int32_t)random%80)));
    random=func_80071A54();
    uint32_t z=PE_LoadU16(0x800E280Cu);
    PE_StoreU8(data+1u,127u);PE_StoreU16(data+4u,0u);PE_StoreU8(data+3u,0u);
    PE_StoreU16(data+10u,(uint16_t)(z-40u+(uint32_t)((int32_t)random%80)));
}

void func_800CE3B4(pe_addr_t data)
{
    unsigned i;
    func_800C2EAC(3u);func_800C3098(16);func_800C2FF0(32u,32u);func_800C3238(2u);
    for (i=0;i<3;i++) PE_StoreU16(0x800E22A8u+i*2u,PE_LoadU16(data+6u+i*2u));
    PE_StoreU16(0x800E22D0u,(uint16_t)(int16_t)(int8_t)PE_LoadU8(data+1u));
    PE_StoreU8(0x800E22CCu,(uint8_t)(PE_LoadU8(data+3u)*2u-96u));
    func_800C3B04(0x800E22A8u);func_800C3098(16);
}
