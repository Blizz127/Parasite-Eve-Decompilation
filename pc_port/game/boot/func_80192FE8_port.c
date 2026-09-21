/*
 * Title draw leaf func_80192FE8 (installed at title node +0xC).
 *
 * Retail overlay: [0x80192FE8, 0x80193084), 39 words / 0x9C.
 * Updates y from (+0x24<<4); writes +0x1C = 256±acc; bumps +0x24 when
 * nonzero; when +0x24 == +0x28 invokes +0x14; marks +0x30 when acc>=16.
 */
#include "psx_compat.h"
#include "game_port.h"

extern void func_8019319C(pe_addr_t node);

void func_80192FE8(pe_addr_t node)
{
    int32_t acc = (int32_t)PE_LoadU32(node + 0x24u);
    int32_t value;
    pe_addr_t callback;
    int32_t target;

    PE_StoreU16(node + 0x06u, (uint16_t)(acc << 4));
    if (acc < 0)
        value = acc + 256;
    else
        value = 256 - acc;

    callback = PE_LoadU32(node + 0x14u);
    PE_StoreU32(node + 0x1Cu, (uint32_t)value);
    if (acc != 0)
        acc += 1;
    PE_StoreU32(node + 0x24u, (uint32_t)acc);
    target = (int32_t)PE_LoadU32(node + 0x28u);
    if (callback != 0u && acc == target) {
        if (callback == 0x8019319Cu)
            func_8019319C(node);
    }

    acc = (int32_t)PE_LoadU32(node + 0x24u);
    if (acc >= 16)
        PE_StoreU32(node + 0x30u, 1u);
}
