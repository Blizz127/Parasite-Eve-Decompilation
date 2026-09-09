/* Original EXE 80079384..800794C4 (80 words, including padding).
 * SHA256 418c0e10f86f636b682b2353689a5ce9a3f0719d39d63a9e371c6714f35e3028.
 * PsyQ triangle/quad projection and normal clipping. Guest write order matters.
 */
#include "pe_port_compat.h"
#include "pe_sdk.h"

static void projection_load(int16_t v[3], pe_addr_t p)
{
    uint32_t xy=PE_LoadU32(p), z=PE_LoadU32(p+4u);
    v[0]=(int16_t)xy; v[1]=(int16_t)(xy>>16); v[2]=(int16_t)z;
}

int32_t func_80079384(pe_addr_t a, pe_addr_t b, pe_addr_t c,
    pe_addr_t xy0, pe_addr_t xy1, pe_addr_t xy2,
    pe_addr_t cue, pe_addr_t depth, pe_addr_t flags)
{
    uint32_t xy[3],z[3];
    projection_load(g_pe_gte.v0,a); projection_load(g_pe_gte.v1,b); projection_load(g_pe_gte.v2,c);
    PE_GTE_RTPT_coordinates(xy,z);
    PE_StoreU32(flags,g_pe_gte.projection_flags);
    int32_t clip=PE_GTE_NCLIP();
    if (clip>0) {
        PE_StoreU32(xy0,xy[0]); PE_StoreU32(xy1,xy[1]); PE_StoreU32(xy2,xy[2]);
        PE_StoreU32(cue,(uint32_t)g_pe_gte.ir0);
        PE_GTE_AVSZ3(z); PE_StoreU32(depth,g_pe_gte.otz);
    }
    return clip;
}

int32_t func_80079414(pe_addr_t a, pe_addr_t b, pe_addr_t c, pe_addr_t d,
    pe_addr_t xy0, pe_addr_t xy1, pe_addr_t xy2, pe_addr_t xy3,
    pe_addr_t cue, pe_addr_t depth, pe_addr_t flags)
{
    uint32_t xy[3],z[4];
    projection_load(g_pe_gte.v0,a); projection_load(g_pe_gte.v1,b); projection_load(g_pe_gte.v2,c);
    PE_GTE_RTPT_coordinates(xy,z);
    uint32_t first_flags=g_pe_gte.projection_flags;
    PE_StoreU32(flags,first_flags);
    int32_t clip=PE_GTE_NCLIP();
    if (clip>0) {
        PE_StoreU32(xy0,xy[0]); PE_StoreU32(xy1,xy[1]); PE_StoreU32(xy2,xy[2]);
        projection_load(g_pe_gte.v0,d);
        PE_GTE_RTPS_coordinates(xy,z+3);
        PE_StoreU32(xy3,xy[0]); PE_StoreU32(cue,(uint32_t)g_pe_gte.ir0);
        PE_StoreU32(flags,first_flags|g_pe_gte.projection_flags);
        PE_GTE_AVSZ4(z); PE_StoreU32(depth,g_pe_gte.otz);
    }
    return clip;
}
