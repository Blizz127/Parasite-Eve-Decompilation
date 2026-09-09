/*
 * Phase 6E-B54K-R — PsyQ MoveImage and its exact GP0(80h) issue worker.
 *
 * func_8007512C is the complete 46-word wrapper at
 * 0x8007512C..0x800751E3.  D_80095744 points at 0x80095704; the wrapper
 * therefore reads target +8 = func_80076C34 and worker +0x18 =
 * func_80076B98.  It does not traverse the neighboring func_80076C10 shim.
 *
 * func_80076B98 is the complete 18-word linked-list DMA worker at
 * 0x80076B98..0x80076BDF (Phase 6E-DRW1 extends it from a single terminal
 * packet to the multi-node ordering-table walk, per
 * pc_port/docs/drawotag_decision.md: the guest builds the table, the host
 * walks it read-only).  Retail programs GP1(04h)=2, DMA2 MADR=packet,
 * BCR=0, CHCR=0x01000401 once, and the channel walks 24-bit links to the
 * 0xFFFFFF terminator, forwarding each node's count words after its tag.
 * Native performs that walk synchronously: GP0(80h) keeps its MoveImage
 * path, drawing-environment words traverse the generic GP0 parser, and
 * any other shape stays a mutation-free named cut.  A next-address at or
 * above guest RAM ends the walk silently after its node's words are sent:
 * retail provably survives the stub-tail jump every cleared-OT draw, so
 * termination (not traversal) is the faithful observable, and size-0
 * stub nodes submit nothing either way.  A zero tag (uninitialized link,
 * which retail never writes) ends the walk the same way.  No other cycle
 * guard exists in retail and none is added: only corrupt nonzero links
 * could cycle, and retail hangs on those identically.
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

/* Validate one chain node without submitting it: representable span,
 * word count within the authenticated bound, and a MoveImage shape or
 * all-environment words.  Reports through the single-node boundary name
 * so earlier cuts keep their identity. */
static int DRW1_ValidateNode(pe_addr_t node, uint32_t tag, uint32_t count,
                             pe_addr_t previous_node)
{
    uint32_t command;
    uint32_t i;
    size_t node_bytes = ((size_t)count + 1u) * 4u;

    /* Size-0 nodes are the OTC empty-bucket links: nothing to send, just
     * follow the link.  (Single-node terminal size-0 was previously cut;
     * no test plants it, and hardware sends nothing either way.)
     * The record's a3 is the node whose link led here (0 for the head), so
     * a boundary report can name the writer of a corrupt link. */
    if (count > 15u || !PE_RangeIsRam(node, node_bytes)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_packet_cut", "func_80076B98", 0, 0u,
            node, tag, count, previous_node, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    if (count == 0u)
        return 1;
    command = PE_LoadU32(node + 4u);
    if (count == 4u && command == 0x80000000u)
        return 1;
    /* Structural walk over the node's command stream.  Each command word
     * must be one the GPU substrate can execute from idle, and its payload
     * words (accepted positionally as data) must fit the node — retail
     * emits complete primitives only, so a truncated one stays a cut.  The
     * length comes from the substrate itself (PE_GPU_GP0_PacketWords), so a
     * primitive is admitted here exactly when submission can execute it. */
    i = 0u;
    while (i < count) {
        uint32_t word = PE_LoadU32(node + 4u + i * 4u);
        uint32_t words = PE_GPU_GP0_PacketWords(word);

        if (words == 0u && IsGp0EnvironmentWord(word))
            words = 1u;
        if (words != 0u && i + words <= count) {
            i += words;
            continue;
        }
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_packet_cut", "func_80076B98", 0, 0u,
            node, tag, i, word, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }
    return 1;
}

/* Submit one validated node: MoveImage shape through its synchronous
 * path, otherwise word-by-word through the generic GP0 parser. */
static int DRW1_SubmitNode(pe_addr_t node, uint32_t tag, uint32_t count)
{
    uint32_t i;

    if (count == 4u && PE_LoadU32(node + 4u) == 0x80000000u) {
        if (!PE_GPU_MoveImage(PE_LoadU32(node + 8u),
                              PE_LoadU32(node + 12u),
                              PE_LoadU32(node + 16u))) {
            (void)Bootstrap_ReturnInt4Indirect(
                "func_80076B98_gpu_move_cut", "func_80076B98", 0, 0u,
                node, PE_LoadU32(node + 8u),
                PE_LoadU32(node + 12u), PE_LoadU32(node + 16u),
                NULL, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        }
        return 1;
    }

    for (i = 0u; i < count; i++) {
        uint32_t word = PE_LoadU32(node + 4u + i * 4u);

        if (!PE_GPU_WriteGP0(word)) {
            (void)Bootstrap_ReturnInt4Indirect(
                "func_80076B98_gp0_packet_cut", "func_80076B98", 0, 0u,
                node, i, word, tag, NULL, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return 0;
        }
    }
    return 1;
}

int func_80076B98(pe_addr_t packet, uint32_t auxiliary)
{
    pe_addr_t node;
    uint32_t tag;
    uint32_t count;
    PeGpuState gpu;

    (void)auxiliary; /* retail worker never reads a1 */
    if (!PE_RangeIsRam(packet, 4u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_packet_span", "func_80076B98", 0, 0u,
            packet, auxiliary, 0u, 0u, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    /* Head node validates before the worker's GP1 write, keeping
     * rejected shapes mutation-free exactly as before. */
    node = packet;
    tag = PE_LoadU32(node);
    count = tag >> 24;
    if (!DRW1_ValidateNode(node, tag, count, 0u))
        return 0;

    /* Retail writes GP1(04h)=2, DMA2 MADR=packet, BCR=0, then
     * CHCR=0x01000401.  The represented list is synchronous in
     * the native GPU authority, so it creates no request-mode DMA token. */
    if (!PE_GPU_WriteGP1(0x04000002u)) {
        (void)Bootstrap_ReturnInt4Indirect(
            "func_80076B98_gp1_cut", "func_80076B98", 0, 0u,
            node, tag, PE_LoadU32(node + 4u), auxiliary, NULL, 0u);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return 0;
    }

    /* Chain walk: submit, then follow the 24-bit link.  Later nodes
     * validate-then-submit progressively, as the hardware DMA would;
     * only the head keeps the validate-before-program order.  A zero tag
     * is an uninitialized link (retail never writes one: OTC, AddPrim,
     * and tail links are all nonzero); it ends the walk silently rather
     * than spinning on address zero, which no valid chain can name. */
    for (;;) {
        uint32_t next;
        pe_addr_t previous_node;

        if (!DRW1_SubmitNode(node, tag, count))
            return 0;
        next = tag & 0x00FFFFFFu;
        if (next == 0x00FFFFFFu || next >= 0x00200000u)
            break;
        previous_node = node;
        node = 0x80000000u | next;
        tag = PE_LoadU32(node);
        if (tag == 0u)
            break;
        count = tag >> 24;
        if (!DRW1_ValidateNode(node, tag, count, previous_node))
            return 0;
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
