/* Original transition bounds 8018F92C..8018FFF4 and SDK792D4/791D0.
 * Four transformed corners define four cross-product planes. All dot products
 * and squared lengths retain their low 32 bits, including negative results. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

uint32_t func_800792D4(pe_addr_t point,pe_addr_t output,pe_addr_t flag_output)
{
    uint32_t flags=0;
    PE_GTE_SetV0((int16_t)PE_LoadU16(point),(int16_t)PE_LoadU16(point+2u),(int16_t)PE_LoadU16(point+4u));
    for(unsigned row=0;row<3;row++) {
        int64_t total=(int64_t)g_pe_gte.tr[row]*4096;
        for(unsigned col=0;col<3;col++) {
            total+=(int64_t)g_pe_gte.rt[row][col]*g_pe_gte.v0[col];
            if(total>=INT64_C(0x80000000000))flags|=1u<<(30-row);
            if(total< -INT64_C(0x80000000000))flags|=1u<<(27-row);
            uint64_t bits=(uint64_t)total&UINT64_C(0xFFFFFFFFFFF);
            total=(bits&UINT64_C(0x80000000000))?(int64_t)bits-INT64_C(0x100000000000):(int64_t)bits;
        }
        int32_t value=(int32_t)(total>>12);
        g_pe_gte.mac[row]=value;
        g_pe_gte.ir[row]=value< -32768?-32768:value>32767?32767:value;
        if(value< -32768 || value>32767)flags|=1u<<(24-row);
    }
    if(flags&0x7F87E000u)flags|=0x80000000u;
    for(unsigned i=0;i<3;i++)PE_StoreU32(output+i*4u,(uint32_t)g_pe_gte.mac[i]);
    PE_StoreU32(flag_output,flags);return flags;
}

void func_800791D0(pe_addr_t a,pe_addr_t b,pe_addr_t output)
{
    int16_t saved[5]={g_pe_gte.rt[0][0],g_pe_gte.rt[0][1],g_pe_gte.rt[1][1],g_pe_gte.rt[1][2],g_pe_gte.rt[2][2]};
    uint32_t x=PE_LoadU32(a),y=PE_LoadU32(a+4u),z=PE_LoadU32(a+8u);
    g_pe_gte.rt[0][0]=(int16_t)x;g_pe_gte.rt[0][1]=(int16_t)(x>>16);
    g_pe_gte.rt[1][1]=(int16_t)y;g_pe_gte.rt[1][2]=(int16_t)(y>>16);g_pe_gte.rt[2][2]=(int16_t)z;
    PE_GTE_SetIR((int16_t)PE_LoadU32(b),(int16_t)PE_LoadU32(b+4u),(int16_t)PE_LoadU32(b+8u));
    PE_GTE_OP(0,0);
    for(unsigned i=0;i<3;i++)PE_StoreU32(output+i*4u,(uint32_t)g_pe_gte.mac[i]);
    g_pe_gte.rt[0][0]=saved[0];g_pe_gte.rt[0][1]=saved[1];g_pe_gte.rt[1][1]=saved[2];g_pe_gte.rt[1][2]=saved[3];g_pe_gte.rt[2][2]=saved[4];
}

static uint32_t dot(const uint32_t a[3],const uint32_t b[3])
{return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];}

void func_8018F92C(pe_addr_t origin)
{
    const pe_addr_t scratch=0x1F800300u;
    static const pe_addr_t planes[]={0x8019CBB0u,0x8019CBD0u,0x8019CBF0u,0x8019CB50u};
    static const unsigned pairs[][2]={{2,0},{3,1},{3,2},{1,0}};
    uint8_t saved[112];uint32_t p[4][3],n[4][3],d[4],width[4];
    for(unsigned i=0;i<112;i++)saved[i]=PE_LoadU8(scratch+i);
    func_80078A94();func_80078E94(0x8019CDF0u);func_80078E04(0x8019CDF0u);
    for(unsigned j=0;j<4;j++) {
        func_800792D4(0x8019BFD0u+j*8u,scratch+j*16u,scratch+96u);
        for(unsigned i=0;i<3;i++)p[j][i]=PE_LoadU32(scratch+j*16u+i*4u);
    }
    for(unsigned j=0;j<4;j++) {
        for(unsigned i=0;i<3;i++) {
            PE_StoreU32(scratch+64u+i*4u,p[pairs[j][0]][i]-PE_LoadU32(origin+i*4u));
            PE_StoreU32(scratch+80u+i*4u,p[pairs[j][1]][i]-PE_LoadU32(origin+i*4u));
        }
        func_800791D0(scratch+64u,scratch+80u,planes[j]);
    }
    for(unsigned j=0;j<4;j++)for(unsigned i=0;i<3;i++)n[j][i]=PE_LoadU32(planes[j]+i*4u);
    d[0]=0u-dot(n[0],p[2]);d[1]=0u-dot(n[1],p[3]);d[2]=0u-dot(n[2],p[2]);d[3]=0u-dot(n[3],p[1]);
    width[0]=dot(n[0],p[1])+d[0];width[1]=dot(n[1],p[0])+d[1];width[2]=dot(n[2],p[1])+d[2];width[3]=dot(n[3],p[2])+d[3];
    PE_StoreU32(0x8019CB48u,d[0]);PE_StoreU32(0x8019CA90u,d[3]);PE_StoreU32(0x8019CB4Cu,d[1]);
    PE_StoreU32(0x8019CC04u,width[0]);PE_StoreU32(0x8019CC0Cu,width[1]);
    PE_StoreU32(0x8019CBA8u,d[2]);PE_StoreU32(0x8019CC10u,width[2]);PE_StoreU32(0x8019CBC4u,width[3]);
    static const pe_addr_t lengths[]={0x8019CBC8u,0x8019CC00u,0x8019CC08u,0x8019CBACu};
    for(unsigned j=0;j<4;j++)PE_StoreU32(lengths[j],func_80078004(dot(n[j],n[j])));
    func_80078B38();
    for(unsigned i=0;i<112;i++)PE_StoreU8(scratch+i,saved[i]);
}
