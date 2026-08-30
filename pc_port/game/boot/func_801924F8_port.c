/*
 * Phase 6E-B54K-AA — authenticated prefix of overlay func_801924F8.
 *
 * Complete retail function: [0x801924F8,0x80192934), 271 words, SHA-256
 * ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a.
 * Translated prefix: [0x801924F8,0x8019256C), 29 words, SHA-256
 * 2e6352856b04f1eee0ab1bae00324a36306fc63e14cb3101d030e073914b9ab2.
 * The next instruction is the first call to overlay-local func_801918F8.
 */
#include "psx_compat.h"
#include "game_port.h"

int func_801924F8(int index)
{
    uint16_t record_index = (uint16_t)index;
    pe_addr_t record;
    uint8_t kind;

    if (record_index >= 47u)
        return 0;

    PE_StoreU8(0x800B0DBFu, (uint8_t)index);
    record = 0x801D0E00u + (uint32_t)record_index * 20u;
    PE_StoreU32(0x801D11ACu, record);
    kind = PE_LoadU8(record + 4u);
    PE_StoreU8(0x800B0DBBu, kind);

    Bootstrap_ReturnVoid4(
        "func_801918F8", "func_801924F8",
        0u, (uintptr_t)(int32_t)(int8_t)kind, 0u, 0u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
