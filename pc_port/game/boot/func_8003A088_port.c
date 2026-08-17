/*
 * PE-BTL6 — func_8003A088 mode-0 empty cut + live GTE walk.
 *
 * Native translation, not matching src/ C. Authority is
 * pc_port/tools/pe_btl6_3a088_walk_oracle.py against EXE SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b and PE.IMG [428,434).
 *
 * Retail 392 words 0x8003A088..0x8003A6A8. Live dest+0x28 is 0.
 * Walk at 0x8003A3B4: parent bytes from dest+0x20; live [0,1].
 * cop2 0x049E012 = MVMVA sf=1 mx=RT v=IR cv=None lm=0
 * cop2 0x0480012 = MVMVA sf=1 mx=RT v=V0 cv=TR lm=0
 * Integer MAC only (pe_gte). Does not andi 0xFC.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"

#define CMD_RTIR  0x049E012u
#define CMD_RTV0  0x0480012u

void func_8003A088_mode0_empty_cut(pe_addr_t dest)
{
    pe_addr_t obj;
    int16_t mode;
    int16_t count;

    if (dest == 0u)
        return;

    mode = (int16_t)PE_LoadU16(dest + 0x28u);
    if (mode == 1 || mode == 3 || mode == 4)
        return;

    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    count = (int16_t)PE_LoadU16(obj + 0x18u);
    if (count <= 0)
        return;
}

static void pe_3a088_store_row(pe_addr_t out, unsigned col)
{
    PE_StoreU16(out + col, (uint16_t)g_pe_gte.ir[0]);
    PE_StoreU16(out + 6u + col, (uint16_t)g_pe_gte.ir[1]);
    PE_StoreU16(out + 12u + col, (uint16_t)g_pe_gte.ir[2]);
}

void func_8003A088_mode0_walk_cut(pe_addr_t dest)
{
    pe_addr_t obj;
    pe_addr_t parents;
    pe_addr_t src_base;
    pe_addr_t out;
    pe_addr_t stream;
    int16_t mode;
    int16_t count;
    int i;

    if (dest == 0u)
        return;

    mode = (int16_t)PE_LoadU16(dest + 0x28u);
    if (mode == 1 || mode == 3 || mode == 4)
        return;

    obj = PE_LoadU32(dest + 0x00u);
    if (obj == 0u)
        return;
    count = (int16_t)PE_LoadU16(obj + 0x18u);
    if (count <= 0)
        return;

    PE_GTE_LoadRT(dest + 0x34u);
    parents = PE_LoadU32(dest + 0x20u);
    src_base = PE_LoadU32(dest + 0x58u);
    out = PE_LoadU32(dest + 0x84u);
    stream = PE_LoadU32(dest + 0x80u);

    for (i = 0; i < count; i++) {
        int parent = (int)(int8_t)PE_LoadU8(parents + (pe_addr_t)i);
        pe_addr_t src;
        unsigned col;

        /* Live PE.IMG [428,434) parents are 0,1. Scratchpad ±1/±2 not this cut. */
        if (parent == -1 || parent == -2)
            return;

        src = src_base + (pe_addr_t)parent * 32u;
        for (col = 0; col < 3u; col++) {
            PE_GTE_SetIR((int16_t)PE_LoadU16(src + col),
                         (int16_t)PE_LoadU16(src + 6u + col),
                         (int16_t)PE_LoadU16(src + 12u + col));
            PE_GTE_MVMVA(CMD_RTIR);
            pe_3a088_store_row(out, col);
        }

        if (parent == 0) {
            /* ROM lhu src+20 / src+24 → VXY0; lwc2 VZ0 from src+28. */
            PE_GTE_SetV0((int16_t)PE_LoadU16(src + 20u),
                         (int16_t)PE_LoadU16(src + 24u),
                         (int16_t)(PE_LoadU32(src + 28u) & 0xFFFFu));
        } else {
            PE_GTE_SetV0(0, 0, 0);
        }
        PE_GTE_MVMVA(CMD_RTV0);
        PE_StoreU32(out + 20u, (uint32_t)g_pe_gte.mac[0]);
        PE_StoreU32(out + 24u, (uint32_t)g_pe_gte.mac[1]);
        PE_StoreU32(out + 28u, (uint32_t)g_pe_gte.mac[2]);
        PE_GTE_LoadRT(out);

        {
            pe_addr_t rec = PE_LoadU32(dest + 0x04u) + (pe_addr_t)parent * 12u;

            if (PE_LoadU8(rec + 4u) == 1u) {
                pe_addr_t slot = PE_LoadU32(dest + 0x18u)
                    + (pe_addr_t)parent * 16u;

                if ((int16_t)PE_LoadU16(slot + 0x0Eu) >= 0) {
                    uint32_t xy = PE_LoadU32(slot);
                    uint32_t z = PE_LoadU32(slot + 4u);

                    PE_GTE_SetV0((int16_t)(xy & 0xFFFFu),
                                 (int16_t)(xy >> 16),
                                 (int16_t)(z & 0xFFFFu));
                    PE_GTE_MVMVA(CMD_RTV0);
                    PE_StoreU16(stream + 0u, (uint16_t)g_pe_gte.ir[0]);
                    PE_StoreU16(stream + 2u, (uint16_t)g_pe_gte.ir[1]);
                    PE_StoreU16(stream + 4u, (uint16_t)g_pe_gte.ir[2]);
                    stream += 12u;
                }
            }
        }
        out += 32u;
    }
}
