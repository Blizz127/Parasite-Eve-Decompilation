/*
 * DAY2-158 dig/91dc8 — title-overlay DecDCTout DMA1 callback.
 *
 * Retail [0x80191DC8,0x80191FB8), 124 words / 0x1F0,
 * SHA-256 d4d36c74fdae7ccb189a0d98c170806fc84b1fe24c700a447bee5eafbd941086.
 * Carve: PE.IMG overlay sectors [0x03D2,0x0457) @ 0x8018EFF0
 * (PE.IMG off 0x1EBDD8). Neighbor: 91B64 ends 91DC0; next leaf 91FB8.
 *
 * Structural twin of player func_801214D4 (same size/CFG), but title BSS
 * at 801D14xx / 801D0DC0 and display helper 918F8 (not 21004). Not a
 * Psy-Q leaf; khasinski has no matched C for this body.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_port_compat.h"
#include "pe_sdk.h"
#include "pe_mdec.h"

void func_80191DC8(void)
{
    RECT saved;
    uint32_t old_buffer, next_buffer, bank;
    int32_t right;

    if ((int8_t)PE_LoadU8(0x800B0DBBu) &&
        (int16_t)PE_LoadU16(0x800B0CD0u)) {
        func_8007C564();
        if (PE_Port_ShouldStop())
            return;
        PE_StoreU16(0x800B0CD0u, 0u);
    }

    saved.x = (int16_t)PE_LoadU16(0x801D148Cu);
    saved.y = (int16_t)PE_LoadU16(0x801D148Eu);
    saved.w = (int16_t)PE_LoadU16(0x801D1490u);
    saved.h = (int16_t)PE_LoadU16(0x801D1492u);

    old_buffer = PE_LoadU8(0x801D1478u);
    next_buffer = old_buffer ^ 1u;
    PE_StoreU8(0x801D1478u, (uint8_t)next_buffer);

    bank = PE_LoadU8(0x801D148Au);
    PE_StoreU16(0x801D148Cu,
                (uint16_t)((uint16_t)saved.x + (uint16_t)saved.w));
    right = (int16_t)PE_LoadU16(0x801D147Au + bank * 8u) +
            (int16_t)PE_LoadU16(0x801D147Eu + bank * 8u);

    if ((int16_t)PE_LoadU16(0x801D148Cu) < right) {
        int32_t product = (int32_t)saved.w * (int32_t)saved.h;
        int32_t words = (product + (product >> 31)) >> 1;
        /* Host-safety only (same shape as 1214D4); retail has no such cut. */
        if (!PE_MDEC_HasDecode()) {
            Bootstrap_ReturnVoid4("func_8010C01C", "func_80191DC8",
                                  PE_LoadU32(0x801D1470u + next_buffer * 4u),
                                  (uint32_t)words, 0u, 0u);
            PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
            return;
        }
        func_8010C01C(PE_LoadU32(0x801D1470u + next_buffer * 4u),
                      (uint32_t)words);
        if (PE_Port_ShouldStop())
            return;
    } else {
        PE_StoreU8(0x801D1494u, 1u);
        bank ^= 1u;
        PE_StoreU8(0x801D148Au, (uint8_t)bank);
        PE_StoreU16(0x801D148Cu, PE_LoadU16(0x801D147Au + bank * 8u));
        PE_StoreU16(0x801D148Eu, PE_LoadU16(0x801D147Cu + bank * 8u));
        if (PE_LoadU8(0x801D0DC0u) == 1u) {
            uint32_t wide = PE_LoadU8(0x800B0DBBu) ^ 1u;
            PE_StoreU8(0x800B0DBBu, (uint8_t)wide);
            PE_StoreU16(0x801D1490u, wide ? 24u : 16u);
            func_801918F8((int8_t)(PE_LoadU32(0x8009CDDCu) ^ 1u),
                          (int8_t)wide);
            if (PE_Port_ShouldStop())
                return;
            PE_StoreU8(0x801D0DC0u, 2u);
        }
    }

    func_8007506C(&saved,
                  PE_LoadU32(0x801D1470u +
                             (uint32_t)(int32_t)(int8_t)old_buffer * 4u));
}
