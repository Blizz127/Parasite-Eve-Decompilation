/* Complete PutDispEnv755F0..75AE8,318 words.
 * SHA256 c8dc390773e463aadf9158ed73f3cb4467a6c8b2af8f853c3950512ef0577ca1. */
#include "psx_compat.h"
#include "pe_gpu.h"
#include "game_port.h"
#include <stdio.h>
#define CHECK() do {if(PE_Port_StopEpoch()!=epoch)return env;} while(0)
static int32_t disp_clamp(int32_t v,int32_t lo,int32_t hi)
{return v<lo?lo:v>hi?hi:v;}
static void disp_command(uint32_t command)
{
    pe_addr_t table=PE_LoadU32(0x80095744u),target=0;
    if(PE_RangeIsRam(table,20))target=PE_LoadU32(table+16u);
    if(target!=0x80076B20u || !PE_GPU_WriteGP1(command)) {
        Bootstrap_ReturnVoid4Indirect("func_80076B20","func_800755F0",target,command,0,0,0);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }
    /* Original76B20 also caches the low command byte by GP1 opcode. */
    PE_StoreU8(0x800A3348u+(command>>24),command);
}
pe_addr_t func_800755F0(pe_addr_t env)
{
    uint32_t epoch=PE_Port_StopEpoch();
    if(PE_LoadU8(0x8009574Eu)>=2) {
        pe_addr_t target=PE_LoadU32(0x80095748u);
        if(target!=0x80071A74u) {
            Bootstrap_ReturnVoid4Indirect("func_80071A74","func_800755F0",target,0x80011970u,env,0,0);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return env;
        }
        fprintf(stderr,"PutDispEnv(%08x)...\n",env);
    }
    disp_command(0x05000000u|((PE_LoadU16(env+2u)&1023u)<<10)|(PE_LoadU16(env)&1023u));CHECK();
    int changed=PE_LoadU32(0x800957C8u)!=PE_LoadU32(env+16u);
    for(unsigned i=0;!changed && i<8;i+=2)changed=PE_LoadU16(0x800957B8u+i)!=PE_LoadU16(env+i);
    if(changed) {
        uint32_t mode=0x08000000u;
        PE_StoreU8(env+18u,PE_LoadU32(0x800956ECu));
        if(PE_LoadU8(env+18u)==1)mode|=8u;
        if(PE_LoadU8(env+17u))mode|=16u;
        if(PE_LoadU8(env+16u))mode|=32u;
        if(PE_LoadU8(0x8009574Fu))mode|=128u;
        int32_t w=(int16_t)PE_LoadU16(env+4u);
        mode|=w<281?0u:w<353?1u:w<401?64u:w<561?2u:3u;
        int32_t h=(int16_t)PE_LoadU16(env+6u);
        if(h>=(PE_LoadU8(env+18u)?289:257))mode|=36u;
        disp_command(mode);CHECK();
        PE_StoreU8(env+18u,8u);
    }
    changed=0;
    for(unsigned i=0;!changed && i<8;i+=2)changed=PE_LoadU16(0x800957C0u+i)!=PE_LoadU16(env+8u+i);
    if(changed || PE_LoadU8(env+18u)==8) {
        PE_StoreU8(env+18u,PE_LoadU32(0x800956ECu));
        int pal=PE_LoadU8(env+18u)!=0;
        int32_t top=(int16_t)PE_LoadU16(env+10u)+(pal?19:16);
        int32_t height=(int16_t)PE_LoadU16(env+14u);
        int32_t bottom=top+(height?height:240);
        int32_t w=(int16_t)PE_LoadU16(env+4u);
        unsigned index=w<281?0:w<353?1:w<401?2:w<561?3:4;
        uint32_t offset=(PE_LoadU8(env+18u)*5u+index)*4u;
        int32_t start=PE_LoadU16(0x80095820u+offset);
        uint32_t span=PE_LoadU16(0x80095822u+offset)-(uint32_t)start;
        start+=(int16_t)PE_LoadU16(env+8u)*PE_LoadU8(0x80095848u+index);
        int32_t width=(int16_t)PE_LoadU16(env+12u);
        if(width)span=(uint32_t)((int32_t)(span*(uint32_t)width)>>8);
        int32_t end=(int32_t)((uint32_t)start+span);
        start=disp_clamp(start,pal?540:500,pal?3220:3250);
        int32_t min_end=start+PE_LoadU8(0x80095848u+index)*4;
        end=end<min_end?min_end:end>(pal?3260:3290)?(pal?3260:3290):end;
        top=disp_clamp(top,pal?19:16,pal?303:257);
        bottom=bottom<top+2?top+2:bottom>(pal?305:258)?(pal?305:258):bottom;
        disp_command(0x06000000u|(((uint32_t)end&4095u)<<12)|((uint32_t)start&4095u));CHECK();
        disp_command(0x07000000u|(((uint32_t)bottom&1023u)<<10)|((uint32_t)top&1023u));CHECK();
    }
    /* Original memcpy copies forward; retain overlap behavior. */
    for(unsigned i=0;i<20;i++)PE_StoreU8(0x800957B8u+i,PE_LoadU8(env+i));
    HostFB_PresentDispEnv(env);
    return env;
}
#undef CHECK
