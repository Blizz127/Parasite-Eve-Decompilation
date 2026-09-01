/*
 * Phase 6E-B54K-AF — authenticated prefix of overlay func_801924F8.
 *
 * Complete retail function: [0x801924F8,0x80192934), 271 words, SHA-256
 * ef825dccdbfd2a74941203d37739c713ad1e3bd8de48ca747f55d0e75a92f00a.
 * Translated prefix: [0x801924F8,0x80192750), 150 words, SHA-256
 * cddb140d8d927057b46a04a2e22afeafce3e24bc1d1faf13c6599302a035313c.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

int func_801924F8(int index)
{
    uint16_t record_index = (uint16_t)index;
    pe_addr_t record;
    pe_addr_t suffix;
    pe_addr_t active_pair;
    uint8_t kind;
    uint16_t movie_x;
    uint16_t movie_y;
    uint32_t display_buffer;
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

    PE_StoreU32(0x801D0DDCu, PE_LoadU32(0x801D0DC4u));
    record = PE_LoadU32(0x801D11ACu);
    movie_x = PE_LoadU16(record + 10u);
    movie_y = PE_LoadU16(record + 12u);
    display_buffer = PE_LoadU32(0x800ACDDCu);

    PE_StoreU8(0x801D148Au, (uint8_t)display_buffer);
    PE_StoreU32(0x801D1464u, PE_LoadU32(0x801D0DE8u));
    PE_StoreU8(0x801D146Cu, 0u);
    PE_StoreU32(0x801D1470u, PE_LoadU32(0x801D0DF0u));
    PE_StoreU32(0x801D1474u, PE_LoadU32(0x801D0DF4u));
    PE_StoreU8(0x801D1478u, 0u);
    PE_StoreU16(0x801D147Au, movie_x);
    PE_StoreU16(0x801D1482u, movie_x);
    PE_StoreU32(0x801D1468u, PE_LoadU32(0x801D0DECu));
    PE_StoreU16(0x801D147Cu, (uint16_t)(movie_y + 240u));
    PE_StoreU16(0x801D1484u, movie_y);

    active_pair = 0x801D1464u + ((display_buffer & 0xFFu) << 3);
    PE_StoreU16(0x801D148Cu, PE_LoadU16(active_pair + 22u));
    PE_StoreU16(0x801D148Eu, PE_LoadU16(active_pair + 24u));
    PE_StoreU16(0x801D1490u,
                PE_LoadU8(0x800B0DBBu) != 0u ? 24u : 16u);
    PE_StoreU8(0x801D1494u, 0u);

    func_8010BE3C(0);
    if (PE_Port_ShouldStop())
        return 0;

    func_8010C0D8(0x80191DC8u);

    func_8007A214(PE_LoadU32(0x801D0DFCu), 0x40u);

    Bootstrap_ReturnVoid("func_801924F8_80192750_cut", "func_801924F8");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
