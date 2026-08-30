/*
 * Phase 6E-B54K-AA — authenticated prefix of overlay func_801924F8.
 *
 * Complete retail function: [0x801924F8,0x80192934), 271 words, SHA-256
 * ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a.
 * Translated prefix: [0x801924F8,0x80192584), 35 words, SHA-256
 * 9f6476d633f517cd6e17fee8a76167180a9f87d320ecf0e62ef4e4f3b45114b1.
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

    func_801918F8(0, (int8_t)kind);
    func_801918F8(1, (int8_t)kind);
    Bootstrap_ReturnVoid("func_801924F8_80192584_cut", "func_801924F8");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
