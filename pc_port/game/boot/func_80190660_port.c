/*
 * Phase 6E-B54K-T — func_80190660 prefix through its first DrawPrim call.
 *
 * Retail function: [0x80190660,0x801909B4), 0x354 / 213 words.
 * Translated prefix: [0x80190660,0x80190860), 0x200 / 128 words.
 * The boundary instruction is jal func_80075358 at 0x80190860; its delay
 * slot stores the final RGB byte of the first SPRT before the call.
 *
 * The two caller-stack packet banks remain native transients.  No native
 * pointer is retained as guest authority.  At the unresolved DrawPrim
 * boundary, only the packet length and initialized command bytes are
 * snapshotted; retail leaves tag bytes 0..2 uninitialized on its stack.
 */
#include "psx_compat.h"
#include "game_port.h"

#include <string.h>

#define GA_OVERLAY_DATA_OFFSET  0x80193278u
#define GA_OVERLAY_DATA_ANCHOR  0x80193254u
#define GA_ENVIRONMENT_0        0x801D11BCu
#define GA_ENVIRONMENT_CURRENT  0x801D11C4u
#define GA_ENVIRONMENT_TOGGLE   0x801D11C8u

static RECT PE_OverlayLoadRect(pe_addr_t address)
{
    RECT rect;

    rect.x = (int16_t)PE_LoadU16(address + 0u);
    rect.y = (int16_t)PE_LoadU16(address + 2u);
    rect.w = (int16_t)PE_LoadU16(address + 4u);
    rect.h = (int16_t)PE_LoadU16(address + 6u);
    return rect;
}

static void PE_TransientStoreU16(uint8_t *destination, uint16_t value)
{
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8);
}

static void PE_TransientStoreU32(uint8_t *destination, uint32_t value)
{
    destination[0] = (uint8_t)value;
    destination[1] = (uint8_t)(value >> 8);
    destination[2] = (uint8_t)(value >> 16);
    destination[3] = (uint8_t)(value >> 24);
}

static void PE_OverlayBuildSprtPair(uint8_t *pair)
{
    uint8_t *second = pair + 0x14u;

    pair[3] = 4u;
    pair[7] = 0x64u;
    PE_TransientStoreU16(pair + 8u, 32u);
    PE_TransientStoreU16(pair + 10u, 88u);
    pair[12] = 0u;
    pair[13] = 0u;
    PE_TransientStoreU16(pair + 14u, 0x7800u);
    PE_TransientStoreU16(pair + 16u, 256u);
    PE_TransientStoreU16(pair + 18u, 64u);

    second[3] = 4u;
    second[7] = 0x64u;
    PE_TransientStoreU16(second + 8u, 284u);
    PE_TransientStoreU16(second + 10u, 80u);
    second[12] = 0u;
    second[13] = 0u;
    PE_TransientStoreU16(second + 14u, 0x7800u);
    PE_TransientStoreU16(second + 16u, 8u);
    PE_TransientStoreU16(second + 18u, 80u);
}

static void PE_OverlayBuildDrawModePair(uint8_t *pair)
{
    pair[3] = 1u;
    PE_TransientStoreU32(pair + 4u, 0xE1000018u);
    pair[11] = 1u;
    PE_TransientStoreU32(pair + 12u, 0xE1000019u);
}

int func_80190660(void)
{
    uint8_t sprites[0x50];
    uint8_t draw_modes[0x20];
    pe_addr_t record;
    pe_addr_t second;
    pe_addr_t environment;
    uint32_t next_toggle;
    uint32_t intensity;
    uint32_t draw_mode_command;
    uint32_t frame = 0u;
    RECT rect;
    unsigned parity;

    /* 0x80190698..0x801906DC: two overlay-resident image records. */
    record = GA_OVERLAY_DATA_ANCHOR + PE_LoadU32(GA_OVERLAY_DATA_OFFSET);
    rect = PE_OverlayLoadRect(record + 0x0Cu);
    (void)func_8007506C(&rect, record + 0x14u);
    if (PE_Port_ShouldStop())
        return -1;

    second = record + 8u + (PE_LoadU32(record + 8u) & ~3u);
    rect = PE_OverlayLoadRect(second + 4u);
    (void)func_8007506C(&rect, second + 0x0Cu);
    if (PE_Port_ShouldStop())
        return -1;
    if (func_80074DC0(0) != 0 || PE_Port_ShouldStop())
        return -1;

    /* 0x801906E8..0x8019079C: two parity banks of DR_MODE + SPRT pairs. */
    memset(sprites, 0, sizeof(sprites));
    memset(draw_modes, 0, sizeof(draw_modes));
    for (parity = 0u; parity < 2u; parity++) {
        PE_OverlayBuildDrawModePair(draw_modes + parity * 0x10u);
        PE_OverlayBuildSprtPair(sprites + parity * 0x28u);
    }

    /* Both display environments are disabled while the fade starts. */
    PE_StoreU8(PE_LoadU32(GA_ENVIRONMENT_0 + 4u) + 0x6Du, 0u);
    PE_StoreU8(PE_LoadU32(GA_ENVIRONMENT_0) + 0x6Du, 0u);
    func_80074D28(1);

    /* First iteration of the 480-frame loop.  The complete intensity shape
     * is retained even though this prefix reaches DrawPrim at frame zero. */
    parity = frame & 1u;
    next_toggle = PE_LoadU32(GA_ENVIRONMENT_TOGGLE) == 0u ? 1u : 0u;
    environment = PE_LoadU32(GA_ENVIRONMENT_0 + next_toggle * 4u);
    PE_StoreU32(GA_ENVIRONMENT_TOGGLE, next_toggle);
    PE_StoreU32(GA_ENVIRONMENT_CURRENT, environment);

    if (frame < 32u)
        intensity = frame * 4u;
    else if (frame < 392u)
        intensity = 128u;
    else if (frame < 424u)
        intensity = (424u - frame) * 4u;
    else
        intensity = 0u;

    sprites[parity * 0x28u + 4u] = (uint8_t)intensity;
    sprites[parity * 0x28u + 5u] = (uint8_t)intensity;
    sprites[parity * 0x28u + 6u] = (uint8_t)intensity;

    /* B54K-U: the first DrawPrim now traverses the exact wrapper/worker and
     * generic GP0(E1h) draw-mode state. */
    draw_mode_command = 0xE1000018u;
    if (PE_func_80075358_Transient(&draw_mode_command, 1u) != 0 ||
        PE_Port_ShouldStop())
        return -1;

    /* The next call is the four-word SPRT. Bytes 0..2 of the retail stack
     * tag are intentionally excluded; all 16 initialized command bytes and
     * the lbu p[3] length cross the diagnostic boundary by value. */
    (void)Bootstrap_ReturnInt4Indirect(
        "func_80075358", "func_80190660", -1, 0x80075358u,
        0u, sprites[parity * 0x28u + 3u], 0u, 0u,
        sprites + parity * 0x28u + 4u, 16u);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return -1;
}
