/*
 * Phase 6E-B54K-U — PsyQ DrawPrim wrapper func_80075358 and the exact
 * execution-proven command-word path through func_80076B58.
 *
 * func_80075358: [0x80075358,0x800753B4), 23 words, SHA-256
 * b16699f3b147f2e86daf0cb43680613cfe2893f8aa5c7cbe06ccb34a74d3920e
 * func_80076B58: [0x80076B58,0x80076B98), 16 words, SHA-256
 * c34c4cc1323c3d3ff00222500fd91dee13e581d3ebd75141953149a7e943646e
 *
 * The wrapper calls jump-table slot 15 (DrawSync), reads packet[3], then
 * calls slot 5 with packet+4 and that byte length. The worker always writes
 * GP1(04h)=DMA off, then writes exactly length words to GP0 in ascending
 * order. Native transients are consumed synchronously and never retained.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"

#define GA_GPU_JTB_PTR 0x80095744u
#define GA_DRAWSYNC_TARGET 0x80077294u
#define GA_DRAWPRIM_WORKER 0x80076B58u

static int PE_func_80076B58_Words(const uint32_t *words, uint32_t count,
                                  const char *caller)
{
    uint32_t i;

    /* Retail performs this store even when count is zero. */
    if (!PE_GPU_WriteGP1(0x04000000u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B58_gp1_cut", caller, -1, GA_DRAWPRIM_WORKER,
            count, PE_GPU_ReadStatus(), PE_GPU_ReadDMA2CHCR(), 0u,
            NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }

    for (i = 0u; i < count; i++) {
        if (!PE_GPU_WriteGP0(words[i])) {
            (void)Bootstrap_ReturnInt4Indirect(
                "func_80076B58_gp0_cut", caller, -1, GA_DRAWPRIM_WORKER,
                i, count, words[i], PE_GPU_ReadStatus(),
                &words[i], sizeof(words[i]));
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return -1;
        }
    }
    return 0;
}

static int PE_func_80075358_Words(const uint32_t *words, uint32_t count,
                                  const char *caller)
{
    pe_addr_t jtb = PE_LoadU32(GA_GPU_JTB_PTR);
    pe_addr_t drawsync = PE_LoadU32(jtb + 0x3Cu);
    pe_addr_t worker;

    if (drawsync != GA_DRAWSYNC_TARGET) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80077294", caller, -1, drawsync,
            0u, 0u, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }
    if (func_80074DC0(0) != 0 || PE_Port_ShouldStop())
        return -1;

    /* Retail re-reads D_80095744 after DrawSync. */
    jtb = PE_LoadU32(GA_GPU_JTB_PTR);
    worker = PE_LoadU32(jtb + 0x14u);
    if (worker != GA_DRAWPRIM_WORKER) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B58", caller, -1, worker,
            0u, count, 0u, 0u, words,
            count > 4u ? 16u : count * 4u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }
    return PE_func_80076B58_Words(words, count, caller);
}

int PE_func_80075358_Transient(const uint32_t *words, uint8_t count)
{
    if (count != 0u && words == NULL) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }
    return PE_func_80075358_Words(words, count, "func_80190660");
}

int func_80075358(pe_addr_t packet)
{
    uint32_t words[255];
    uint32_t count;
    uint32_t i;

    if (!PE_RangeIsRam(packet, 4u)) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }
    count = PE_LoadU8(packet + 3u);
    if (!PE_RangeIsRam(packet + 4u, (size_t)count * 4u)) {
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return -1;
    }
    for (i = 0u; i < count; i++)
        words[i] = PE_LoadU32(packet + 4u + i * 4u);
    return PE_func_80075358_Words(words, count, "func_80075358");
}
