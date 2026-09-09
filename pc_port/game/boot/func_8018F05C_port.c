/* Complete original transition camera setup 8018F05C..8018F344.
 * Includes the SDK matrix concatenation and alternate Euler rotation leaves. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "game_port.h"

pe_addr_t func_800787D4(pe_addr_t a,pe_addr_t b,pe_addr_t out)
{
    int32_t r[3][3],t[3];
    PE_GTE_LoadRT33(a);
    for(unsigned col=0;col<3;col++) {
        PE_GTE_SetV0((int16_t)PE_LoadU16(b+col*2u),(int16_t)PE_LoadU16(b+6u+col*2u),(int16_t)PE_LoadU16(b+12u+col*2u));
        PE_GTE_MVMVA(0x86000u);
        for(unsigned row=0;row<3;row++)r[row][col]=g_pe_gte.ir[row];
    }
    /* Keep retail store/read ordering: callers may overlap matrices. */
    PE_StoreU32(out,(uint16_t)r[0][0]|((uint32_t)(uint16_t)r[0][1]<<16));
    PE_StoreU32(out+12u,(uint16_t)r[2][0]|((uint32_t)(uint16_t)r[2][1]<<16));
    PE_StoreU32(out+16u,(uint32_t)r[2][2]);
    PE_GTE_SetV0((int16_t)PE_LoadU16(b+20u),(int16_t)PE_LoadU16(b+24u),(int16_t)PE_LoadU16(b+28u));
    PE_GTE_MVMVA(0x86000u);
    PE_StoreU32(out+4u,(uint16_t)r[0][2]|((uint32_t)(uint16_t)r[1][0]<<16));
    PE_StoreU32(out+8u,(uint16_t)r[1][1]|((uint32_t)(uint16_t)r[1][2]<<16));
    for(unsigned i=0;i<3;i++) {
        int64_t value=(int64_t)g_pe_gte.mac[i]+(int32_t)PE_LoadU32(a+20u+i*4u);
        if(value<INT32_MIN || value>INT32_MAX) {
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);return 0;
        }
        t[i]=(int32_t)value;
    }
    for(unsigned i=0;i<3;i++)PE_StoreU32(out+20u+i*4u,(uint32_t)t[i]);
    return out;
}

static int32_t mul12(int32_t a,int32_t b)
{return (int32_t)((uint32_t)a*(uint32_t)b)>>12;}

pe_addr_t func_800799E4(pe_addr_t angles,pe_addr_t out)
{
    int32_t s[3],c[3];
    for(unsigned i=0;i<3;i++) {
        int32_t a=(int16_t)PE_LoadU16(angles+i*2u);
        uint32_t word=PE_LoadU32(0x800966ECu+((uint32_t)(a<0?-a:a)&4095u)*4u);
        s[i]=(int16_t)word;if(a<0)s[i]=-s[i];c[i]=(int16_t)(word>>16);
    }
    PE_StoreU16(out+12u,(uint16_t)-s[1]);
    PE_StoreU16(out+14u,(uint16_t)mul12(s[0],c[1]));
    PE_StoreU16(out+16u,(uint16_t)mul12(c[0],c[1]));
    PE_StoreU16(out,(uint16_t)mul12(c[1],c[2]));
    PE_StoreU16(out+6u,(uint16_t)mul12(s[2],c[1]));
    int32_t xy=mul12(s[0],s[1]),yx=mul12(s[1],c[0]);
    PE_StoreU16(out+2u,(uint16_t)(mul12(xy,c[2])-mul12(s[2],c[0])));
    PE_StoreU16(out+8u,(uint16_t)(mul12(xy,s[2])+mul12(c[0],c[2])));
    PE_StoreU16(out+4u,(uint16_t)(mul12(yx,c[2])+mul12(s[0],s[2])));
    PE_StoreU16(out+10u,(uint16_t)(mul12(yx,s[2])-mul12(s[0],c[2])));
    return out;
}

void func_8018F05C(void)
{
    const pe_addr_t scratch=0x1F800280u,eye=0x8019C330u,target=0x8019C810u;
    const pe_addr_t angles=0x8019BFC4u,matrix=0x8019CC30u;
    uint8_t saved[128];unsigned epoch=PE_Port_StopEpoch();
    for(unsigned i=0;i<128;i++)saved[i]=PE_LoadU8(scratch+i);
    /* Scratch offsets correspond to original stack locals at sp+0x10. */
    for(unsigned i=0;i<32;i++) {
        uint8_t b=PE_LoadU8(0x8018EFF4u+i);
        PE_StoreU8(scratch+32u+i,b);PE_StoreU8(scratch+64u+i,b);
    }
    for(unsigned i=0;i<3;i++)PE_StoreU32(scratch+i*4u,PE_LoadU32(target+i*4u)-PE_LoadU32(eye+i*4u));
    func_80078134(scratch,scratch+16u);
    if(PE_Port_StopEpoch()!=epoch)goto done;
    uint32_t x=PE_LoadU32(scratch+16u),z=PE_LoadU32(scratch+24u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(scratch+52u+i*4u,0u-PE_LoadU32(eye+i*4u));
    uint32_t length=func_80078004(x*x+z*z);
    PE_StoreU16(angles,(uint16_t)func_80079FB4((int32_t)PE_LoadU32(scratch+20u),(int32_t)length));
    PE_StoreU16(angles+2u,(uint16_t)(func_80079FB4((int32_t)z,(int32_t)x)-1024));
    PE_StoreU16(angles+4u,0);
    func_800794C4(angles,scratch+64u);
    func_800787D4(scratch+64u,scratch+32u,matrix);
    if(PE_Port_StopEpoch()!=epoch)goto done;
    for(unsigned i=0;i<3;i++) {
        PE_StoreU16(scratch+96u+i*2u,(uint16_t)PE_LoadU32(target+i*4u));
        PE_StoreU16(scratch+104u+i*2u,(uint16_t)PE_LoadU32(eye+i*4u));
        PE_StoreU32(scratch+112u+i*4u,i==1u?(uint32_t)-20000:0u);
    }
    func_8018F344(matrix,scratch+104u,scratch+96u,scratch+112u);
    if(PE_Port_StopEpoch()!=epoch)goto done;
    x=PE_LoadU32(scratch);z=PE_LoadU32(scratch+8u);
    length=func_80078004(x*x+z*z);
    PE_StoreU16(angles,(uint16_t)(1024u-(uint32_t)func_80079FB4((int32_t)length,(int32_t)(0u-PE_LoadU32(scratch+4u)))));
    PE_StoreU16(angles+2u,(uint16_t)(1024u-(uint32_t)func_80079FB4((int32_t)z,(int32_t)x)));
    PE_StoreU16(angles+4u,0);
    func_800799E4(angles,0x8019CDF0u);
    for(unsigned i=0;i<3;i++)PE_StoreU32(0x8019CE04u+i*4u,PE_LoadU32(eye+i*4u));
    func_80078E94(matrix);func_80078E04(matrix);
 done:
    for(unsigned i=0;i<128;i++)PE_StoreU8(scratch+i,saved[i]);
}
