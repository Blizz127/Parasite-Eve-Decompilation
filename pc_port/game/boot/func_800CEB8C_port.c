/* Complete retail beam direction and collision helpers, BF0F0/B3390.s.
 * The original triangle test uses GTE OP results and writes scratch RAM. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

uint32_t func_80078004(uint32_t value)
{
    uint32_t leading=PE_GTE_LZCR(value),even,index,word;
    int32_t shift;
    if (leading==32u) return 0u;
    even=leading&~1u;shift=(int32_t)even-24;
    index=shift>=0?value<<(unsigned)shift:(uint32_t)((int32_t)value>>(24u-even));
    word=(uint32_t)(int32_t)(int16_t)PE_LoadU16(0x800960BCu+((index-64u)<<1u));
    return (word<<((31u-even)>>1u))>>12u;
}

void PE_EffectDirectionCFAA8(const int16_t from[3],const int16_t to[3],pe_addr_t out)
{
    int32_t dx=(int32_t)to[0]-from[0],dz=(int32_t)to[2]-from[2];
    int32_t yaw=1024-func_80079FB4(dz,dx);
    uint32_t square=(uint32_t)dx*(uint32_t)dx+(uint32_t)dz*(uint32_t)dz;
    uint32_t length;
    PE_StoreU16(out+2u,(uint16_t)yaw);
    length=func_80078004(square);
    PE_StoreU16(out,(uint16_t)(0u-(uint32_t)func_80079FB4((int32_t)to[1]-from[1],(int32_t)length)));
    PE_StoreU16(out+4u,0u);
    PE_StoreU16(out,(uint16_t)(PE_LoadU16(out)&4095u));
    PE_StoreU16(out+2u,(uint16_t)(PE_LoadU16(out+2u)&4095u));
}

void func_800CFAA8(pe_addr_t from,pe_addr_t to,pe_addr_t out)
{
    int16_t a[3],b[3];unsigned i;
    for (i=0;i<3;i++) {a[i]=(int16_t)PE_LoadU16(from+i*2u);b[i]=(int16_t)PE_LoadU16(to+i*2u);}
    PE_EffectDirectionCFAA8(a,b,out);
}

void PE_ActorJointDirectionCE9D4(pe_addr_t actor,uint32_t joint,int16_t out[3])
{
    int16_t origin[3],target[3];unsigned i;
    PE_GTE_LoadRT33(PE_LoadU32(actor+0x238u)+joint*32u);
    for (i=0;i<3;i++) {g_pe_gte.tr[i]=0;origin[i]=(int16_t)PE_LoadU16(0x800C2260u+i*2u);}
    PE_GTE_SetV0((int16_t)PE_LoadU16(0x800C2258u),(int16_t)PE_LoadU16(0x800C225Au),(int16_t)PE_LoadU16(0x800C225Cu));
    PE_GTE_MVMVA(0x80000u);
    for (i=0;i<3;i++) target[i]=(int16_t)g_pe_gte.mac[i];
    {
        int32_t dx=(int32_t)target[0]-origin[0],dz=(int32_t)target[2]-origin[2];
        uint32_t length=func_80078004((uint32_t)dx*(uint32_t)dx+(uint32_t)dz*(uint32_t)dz);
        out[1]=(int16_t)((1024-func_80079FB4(dz,dx))&4095);
        out[0]=(int16_t)((0u-(uint32_t)func_80079FB4((int32_t)target[1]-origin[1],(int32_t)length))&4095u);
        out[2]=0;
    }
}

void func_800CE9D4(pe_addr_t actor,uint32_t joint,pe_addr_t out)
{
    int16_t angles[3];unsigned i;
    PE_ActorJointDirectionCE9D4(actor,joint,angles);
    for (i=0;i<3;i++) PE_StoreU16(out+i*2u,(uint16_t)angles[i]);
}

void PE_EffectOffsetCFB7C(pe_addr_t angles,int32_t distance,int16_t out[3])
{
    int16_t rotation[9],a[3];int32_t result[3];unsigned i;
    PE_StoreU16(angles+4u,0u);
    for (i=0;i<3;i++) a[i]=(int16_t)PE_LoadU16(angles+i*2u);
    PE_RotMatrix79754_values(a,rotation);
    for (i=0;i<9;i++) g_pe_gte.rt[i/3u][i%3u]=rotation[i];
    for (i=0;i<3;i++) g_pe_gte.tr[i]=0;
    PE_GTE_SetV0((int16_t)PE_LoadU16(0x800C2260u),(int16_t)PE_LoadU16(0x800C2262u),(int16_t)distance);
    PE_GTE_MVMVA(0x80000u);
    for (i=0;i<3;i++) result[i]=g_pe_gte.mac[i];
    PE_GTE_LoadRT(PE_LoadU32(0x800BCFA4u));
    for (i=0;i<3;i++) out[i]=(int16_t)result[i];
}

void func_800CFB7C(pe_addr_t angles,int32_t distance,pe_addr_t out)
{
    int16_t v[3];unsigned i;
    PE_EffectOffsetCFB7C(angles,distance,v);
    for (i=0;i<3;i++) PE_StoreU16(out+i*2u,(uint16_t)v[i]);
}

static int effect_triangle(const int16_t point[3],const int16_t v[3][4])
{
    unsigned edge,i;
    uint32_t cross[3];
    for (i=0;i<6;i++) PE_StoreU32(0x1F800004u+i*16u,0u);
    PE_StoreU32(0x800E2844u,0x1F800000u);
    for (edge=0;edge<3;edge++) {
        int32_t dx=(int32_t)v[(edge+1u)%3u][0]-v[edge][0];
        int32_t dz=(int32_t)v[(edge+1u)%3u][2]-v[edge][2];
        int32_t px=(int32_t)point[0]-v[edge][0],pz=(int32_t)point[2]-v[edge][2];
        PE_StoreU32(0x1F800000u+edge*16u,(uint32_t)dx);
        PE_StoreU32(0x1F800008u+edge*16u,(uint32_t)dz);
        PE_StoreU32(0x1F800030u+edge*16u,(uint32_t)px);
        PE_StoreU32(0x1F800038u+edge*16u,(uint32_t)pz);
        /* CTC2 writes only control words 0,2,4; OP uses their diagonals. */
        g_pe_gte.rt[0][0]=(int16_t)dx;g_pe_gte.rt[0][1]=(int16_t)((uint32_t)dx>>16u);
        g_pe_gte.rt[1][1]=0;g_pe_gte.rt[1][2]=0;g_pe_gte.rt[2][2]=(int16_t)dz;
        PE_GTE_SetIR((int16_t)px,0,(int16_t)pz);PE_GTE_OP(0,0);
        for (i=0;i<3;i++) PE_StoreU32(0x1F800060u+edge*16u+i*4u,(uint32_t)g_pe_gte.mac[i]);
        cross[edge]=(uint32_t)g_pe_gte.mac[1];
    }
    return (int32_t)((cross[0]^cross[1])|(cross[1]^cross[2]))>0;
}

int func_800C62DC(pe_addr_t point,pe_addr_t triangle)
{
    int16_t p[3],v[3][4]={{0}};unsigned i,j;
    for (i=0;i<3;i++) p[i]=(int16_t)PE_LoadU16(point+i*2u);
    for (i=0;i<3;i++) for (j=0;j<3;j++) v[i][j]=(int16_t)PE_LoadU16(triangle+i*8u+j*2u);
    return effect_triangle(p,v);
}

static int effect_quad(const int16_t v[4][4])
{
    pe_addr_t aya=PE_LoadU32(0x8009D254u);
    int16_t p[3];int first,second;unsigned i;
    for (i=0;i<3;i++) p[i]=(int16_t)PE_LoadU16(aya+42u+i*4u);
    first=effect_triangle(p,v);second=effect_triangle(p,v+1);
    return first|second;
}

int func_800C6B20(pe_addr_t quad)
{
    int16_t v[4][4]={{0}};unsigned i,j;
    for (i=0;i<4;i++) for (j=0;j<3;j++) v[i][j]=(int16_t)PE_LoadU16(quad+i*8u+j*2u);
    return effect_quad(v);
}

int func_800CEB8C(pe_addr_t from,pe_addr_t to,int32_t radius)
{
    int32_t heading=-func_80079FB4((int16_t)PE_LoadU16(to+4u)-(int16_t)PE_LoadU16(from+4u),
        (int16_t)PE_LoadU16(to)-(int16_t)PE_LoadU16(from));
    int32_t dx=(int32_t)((uint32_t)func_80077CF4(heading)*(uint32_t)radius)/4096;
    int32_t dz=(int32_t)((uint32_t)func_80077DC4(heading)*(uint32_t)radius)/4096;
    int16_t v[4][4]={{0}};unsigned i;
    for (i=0;i<4;i++) {
        pe_addr_t p=i<2u?from:to;
        v[i][0]=(int16_t)(PE_LoadU16(p)+((i&1u)?dx:-dx));
        v[i][2]=(int16_t)(PE_LoadU16(p+4u)+((i&1u)?dz:-dz));
    }
    return effect_quad(v);
}
