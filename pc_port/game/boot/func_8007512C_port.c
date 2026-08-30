/*
 * Phase 6E-B54K-R — PsyQ MoveImage and its exact GP0(80h) issue worker.
 *
 * func_8007512C is the complete 46-word wrapper at
 * 0x8007512C..0x800751E3.  D_80095744 points at 0x80095704; the wrapper
 * therefore reads target +8 = func_80076C34 and worker +0x18 =
 * func_80076B98.  It does not traverse the neighboring func_80076C10 shim.
 *
 * func_80076B98 is the complete 18-word linked-list DMA worker at
 * 0x80076B98..0x80076BDF. Native resolves a single, terminal GPU packet:
 * GP0(80h) keeps its synchronous MoveImage path, while drawing-environment
 * words traverse the generic GP0 parser. Multi-node ordering tables remain an
 * explicit boundary; no guest instruction stream is interpreted.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"

#define GA_GPU_NAME_MOVEIMAGE  0x800118ECu
#define GA_GPU_JTB_PTR         0x80095744u
#define GA_GPU_DISPATCH        0x80076C34u
#define GA_GPU_MOVE_PACKET     0x800957E4u
#define GPU_MOVE_WORKER        0x80076B98u

static int IsGp0EnvironmentWord(uint32_t word)
{
    uint32_t opcode = word >> 24;

    return word == 0u || (opcode >= 0xE1u && opcode <= 0xE6u);
}

int func_80076B98(pe_addr_t packet, uint32_t auxiliary)
{
    uint32_t tag;
    uint32_t count;
    uint32_t command;
    uint32_t i;
    size_t packet_bytes;
    PeGpuState gpu;

    (void)auxiliary; /* retail worker never reads a1 */
    if (!PE_RangeIsRam(packet, 4u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_packet_span", "func_80076B98", 0, 0u,
            packet, auxiliary, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    tag = PE_LoadU32(packet);
    count = tag >> 24;
    packet_bytes = ((size_t)count + 1u) * 4u;
    if ((tag & 0x00FFFFFFu) != 0x00FFFFFFu || count == 0u ||
        count > 15u || !PE_RangeIsRam(packet, packet_bytes)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_packet_cut", "func_80076B98", 0, 0u,
            packet, tag, count, auxiliary, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    command = PE_LoadU32(packet + 4u);

    /* Validate the whole represented node before the worker's GP1 write.
     * This keeps rejected packet shapes mutation-free. */
    if (!(count == 4u && command == 0x80000000u)) {
        for (i = 0u; i < count; i++) {
            if (!IsGp0EnvironmentWord(
                    PE_LoadU32(packet + 4u + i * 4u))) {
                (void)Bootstrap_ReturnInt4Indirect(
                    "func_80076B98_packet_cut", "func_80076B98", 0, 0u,
                    packet, tag, i, PE_LoadU32(packet + 4u + i * 4u),
                    NULL, 0u);
                PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
                return 0;
            }
        }
    }

    /* Retail writes GP1(04h)=2, DMA2 MADR=packet, BCR=0, then
     * CHCR=0x01000401.  The represented one-packet list is synchronous in
     * the native GPU authority, so it creates no request-mode DMA token. */
    if (!PE_GPU_WriteGP1(0x04000002u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_gp1_cut", "func_80076B98", 0, 0u,
            packet, tag, command, auxiliary, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    if (count == 4u && command == 0x80000000u) {
        if (!PE_GPU_MoveImage(PE_LoadU32(packet + 8u),
                              PE_LoadU32(packet + 12u),
                              PE_LoadU32(packet + 16u))) {
            (void)Bootstrap_ReturnInt4Indirect(
                "func_80076B98_gpu_move_cut", "func_80076B98", 0, 0u,
                packet, PE_LoadU32(packet + 8u),
                PE_LoadU32(packet + 12u), PE_LoadU32(packet + 16u),
                NULL, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        }
        return 0;
    }

    for (i = 0u; i < count; i++) {
        uint32_t word = PE_LoadU32(packet + 4u + i * 4u);

        if (!PE_GPU_WriteGP0(word)) {
            (void)Bootstrap_ReturnInt4Indirect(
                "func_80076B98_gp0_packet_cut", "func_80076B98", 0, 0u,
                packet, i, word, tag, NULL, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
    }
    PE_GPU_GetState(&gpu);
    if (gpu.gp0_state != PE_GPU_GP0_IDLE) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_incomplete_packet_cut", "func_80076B98", 0,
            0u, packet, tag, gpu.gp0_state, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    }
    return 0; /* worker v0 is ignored by both retail callers */
}

int func_8007512C(const RECT *rect, int destination_x, int destination_y)
{
    pe_addr_t jtb;
    pe_addr_t target;
    pe_addr_t worker;
    uint32_t source;
    uint32_t destination;
    uint32_t size;

    func_80074E28(GA_GPU_NAME_MOVEIMAGE, rect);
    if (rect->w == 0 || rect->h == 0) {
        return -1;
    }

    source = (uint32_t)(uint16_t)rect->x |
             ((uint32_t)(uint16_t)rect->y << 16);
    destination = (uint32_t)(uint16_t)destination_x |
                  ((uint32_t)(uint16_t)destination_y << 16);
    size = (uint32_t)(uint16_t)rect->w |
           ((uint32_t)(uint16_t)rect->h << 16);

    /* Retail store order at 0x800751A4..0x800751B4.  Header and command are
     * immutable executable data and are deliberately not synthesized. */
    PE_StoreU32(GA_GPU_MOVE_PACKET + 12u, destination);
    PE_StoreU32(GA_GPU_MOVE_PACKET + 8u, source);
    PE_StoreU32(GA_GPU_MOVE_PACKET + 16u, size);

    jtb = PE_LoadU32(GA_GPU_JTB_PTR);
    worker = PE_LoadU32(jtb + 0x18u);
    target = PE_LoadU32(jtb + 8u);
    if (target == GA_GPU_DISPATCH) {
        return func_80076C34(worker, GA_GPU_MOVE_PACKET, 0x14, 0u);
    }

    return Bootstrap_ReturnInt4Indirect(
        "func_80076C34", "func_8007512C", 0, target,
        worker, GA_GPU_MOVE_PACKET, 0x14u, 0u, rect, sizeof(*rect));
}
