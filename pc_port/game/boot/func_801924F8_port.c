/*
 * Phase 6E-B54K-AA — authenticated prefix of overlay func_801924F8.
 *
 * Complete retail function: [0x801924F8,0x80192934), 271 words, SHA-256
 * ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a.
 * Translated prefix: [0x801924F8,0x80192614), 71 words, SHA-256
 * c5ac60f55bb880129bf6cb3363a84d1a3febec10fea7d81a78ea809658eb6c8c.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

int func_801924F8(int index)
{
    uint16_t record_index = (uint16_t)index;
    pe_addr_t record;
    pe_addr_t suffix;
    uint8_t kind;
    char filename[32] = "";
    int status;

    if (record_index >= 47u)
        return 0;

    PE_StoreU8(0x800B0DBFu, (uint8_t)index);
    record = 0x801D0E00u + (uint32_t)record_index * 20u;
    PE_StoreU32(0x801D11ACu, record);
    kind = PE_LoadU8(record + 4u);
    PE_StoreU8(0x800B0DBBu, kind);

    func_801918F8(0, (int8_t)kind);
    func_801918F8(1, (int8_t)kind);

    (void)func_800719F4(filename,
                        record_index < 21u ? "\\FMV1" : "\\FMV2");
    suffix = PE_LoadU32(record);
    (void)func_800719F4(
        filename, (const char *)PE_TranslateConst(suffix, 16u));

    do {
        status = 0;
        if (func_8007F72C() == 1 && func_8007F778() == 0)
            status = func_80081414(0x801D0DC4u, filename);
    } while (status == 0 || status == -1);

    Bootstrap_ReturnVoid("func_801924F8_80192614_cut", "func_801924F8");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
