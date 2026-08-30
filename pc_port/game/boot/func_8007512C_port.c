/*
 * Phase 6E-B54K-R — PsyQ MoveImage and its exact GP0(80h) issue worker.
 *
 * func_8007512C is the complete 46-word wrapper at
 * 0x8007512C..0x800751E3.  D_80095744 points at 0x80095704; the wrapper
 * therefore reads target +8 = func_80076C34 and worker +0x18 =
 * func_80076B98.  It does not traverse the neighboring func_80076C10 shim.
 *
 * func_80076B98 is the complete 18-word linked-list DMA worker at
 * 0x80076B98..0x80076BDF.  Native resolves only the one-packet MoveImage
 * list (tag 0x04FFFFFF, command 0x80000000).  General DrawOTag lists remain
 * an explicit packet boundary; no guest instruction stream is interpreted.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_gpu.h"

#define GA_GPU_NAME_MOVEIMAGE  0x800118ECu
#define GA_GPU_JTB_PTR         0x80095744u
#define GA_GPU_DISPATCH        0x80076C34u
#define GA_GPU_MOVE_PACKET     0x800957E4u
#define GPU_MOVE_WORKER        0x80076B98u

int func_80076B98(pe_addr_t packet, uint32_t auxiliary)
{
    uint32_t tag;
    uint32_t command;
    uint32_t source;
    uint32_t destination;
    uint32_t size;

    (void)auxiliary; /* retail worker never reads a1 */
    if (!PE_RangeIsRam(packet, 0x14u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_packet_span", "func_80076B98", 0, 0u,
            packet, auxiliary, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    tag = PE_LoadU32(packet);
    command = PE_LoadU32(packet + 4u);
    source = PE_LoadU32(packet + 8u);
    destination = PE_LoadU32(packet + 12u);
    size = PE_LoadU32(packet + 16u);
    if (tag != 0x04FFFFFFu || command != 0x80000000u) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_packet_cut", "func_80076B98", 0, 0u,
            packet, tag, command, auxiliary, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    /* Retail writes GP1(04h)=2, DMA2 MADR=packet, BCR=0, then
     * CHCR=0x01000401.  The represented one-packet list is synchronous in
     * the native GPU authority, so it creates no request-mode DMA token. */
    if (!PE_GPU_WriteGP1(0x04000002u) ||
        !PE_GPU_MoveImage(source, destination, size)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_gpu_move_cut", "func_80076B98", 0, 0u,
            packet, source, destination, size, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
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
