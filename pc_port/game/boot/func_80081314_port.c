/*
 * Phase 6E-B54K-AM — authenticated registration prefix of func_80081314.
 *
 * Complete retail function: [0x80081314,0x800813E8), 53 words, SHA-256
 * fe43d63bd26dd2279dbdc1c8858998cace3f0da9d7cd16172aa61b8d32a6ce42.
 * Translated production prefix: [0x80081314,0x8008138C), 30 words,
 * SHA-256 ec6afdca38a2662a48bf9f6fa9f97113a48c2ed75c9e8b814c69d69a0d77da35.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_sdk.h"

pe_addr_t func_800824C8(pe_addr_t callback)
{
    pe_addr_t old = PE_LoadU32(0x800B8AB4u);
    PE_StoreU32(0x800B8AB4u, callback);
    return old;
}

pe_addr_t func_800824F0(pe_addr_t callback)
{
    return func_80073CF4(3u, callback);
}

int func_80081314(pe_addr_t location, uint32_t mode)
{
    (void)location;

    if ((mode & 0x100u) != 0u) {
        PE_StoreU32(0x800A8020u, (mode & 0x20u) != 0u ? 0u : 1u);
        (void)func_800824F0(0x8007C214u);
        (void)func_800824C8(0x800813E8u);
    }

    /* The next retail operation is the generic four-command queue issue
     * through func_8007F0C8.  CdlReadS completion/DMA delivery is not yet
     * represented, so registration must not be mistaken for delivery. */
    Bootstrap_ReturnVoid("func_80081314_func_8007F0C8_cut",
                         "func_80081314");
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
