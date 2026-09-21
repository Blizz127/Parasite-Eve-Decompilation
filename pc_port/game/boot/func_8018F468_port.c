/*
 * Title object update/draw leaf func_8018F468.
 *
 * Retail overlay: [0x8018F468, 0x8018F7F0), 226 words / 0x388.
 * Walks D_801D1370: invokes each node's +0xC draw (guest 0x80192FE8),
 * accumulates a bounding box, clips against D_801D11C4 DRAWENV, invokes
 * each node's +0x10 update, and writes the clip rect back.
 *
 * Disassembly: PE.IMG LBA 1013 + 0x03D2*0x800, load VA 0x8018EFF0.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_TITLE_LIST          0x801D1370u
#define GA_ENVIRONMENT_CURRENT 0x801D11C4u
#define GA_TITLE_DRAW_FN       0x80192FE8u
#define GA_TITLE_UPDATE_FN     0x8018F7F0u

extern void func_80192FE8(pe_addr_t node);

static void title_call_draw(pe_addr_t node)
{
    pe_addr_t draw = PE_LoadU32(node + 0x0Cu);
    if (draw == GA_TITLE_DRAW_FN)
        func_80192FE8(node);
}

static void title_call_update(pe_addr_t node, int32_t a1, int32_t a2, int32_t a3)
{
    pe_addr_t update = PE_LoadU32(node + 0x10u);
    (void)a1;
    (void)a2;
    (void)a3;
    /* 8018F7F0 is the TIM/sprite updater; menu selection only needs draw
     * (+0x14 fire). Leave unbound update as a no-op until that leaf ports. */
    (void)update;
    (void)GA_TITLE_UPDATE_FN;
}

void func_8018F468(void)
{
    pe_addr_t node;
    pe_addr_t env;
    int32_t min_x = 0x7FFF;
    int32_t min_y = 0x7FFF;
    int32_t max_x = 0;
    int32_t max_y = 0;
    int16_t clip_x;
    int16_t clip_y;
    int16_t clip_w;
    int16_t clip_h;

    node = PE_LoadU32(GA_TITLE_LIST);
    while (node != 0u) {
        int16_t x;
        int16_t y;
        int16_t w;
        int16_t h;

        title_call_draw(node);
        x = (int16_t)PE_LoadU16(node + 0x04u);
        y = (int16_t)PE_LoadU16(node + 0x06u);
        w = (int16_t)PE_LoadU16(node + 0x08u);
        h = (int16_t)PE_LoadU16(node + 0x0Au);
        if (x < min_x)
            min_x = x;
        if (y < min_y)
            min_y = y;
        if (x + w > max_x)
            max_x = x + w;
        if (y + h > max_y)
            max_y = y + h;
        node = PE_LoadU32(node);
    }

    env = PE_LoadU32(GA_ENVIRONMENT_CURRENT);
    {
        int16_t env_w = (int16_t)PE_LoadU16(env + 0x7Cu);
        if (env_w > 0) {
            int16_t env_x = (int16_t)PE_LoadU16(env + 0x78u);
            int16_t env_y = (int16_t)PE_LoadU16(env + 0x7Au);
            int16_t env_h = (int16_t)PE_LoadU16(env + 0x7Eu);
            int32_t right = env_x + env_w;
            int32_t bottom = env_y + env_h;
            int32_t cx2;
            int32_t cy2;

            clip_x = env_x < min_x ? env_x : (int16_t)min_x;
            clip_y = env_y < min_y ? env_y : (int16_t)min_y;
            cx2 = max_x < right ? max_x : right;
            cy2 = max_y < bottom ? max_y : bottom;
            clip_w = (int16_t)(cx2 - clip_x);
            clip_h = (int16_t)(cy2 - clip_y);
        } else {
            clip_x = (int16_t)min_x;
            clip_y = (int16_t)min_y;
            clip_w = (int16_t)(max_x - min_x);
            clip_h = (int16_t)(max_y - min_y);
        }
    }

    if (clip_w > 0 && clip_h > 0) {
        node = PE_LoadU32(GA_TITLE_LIST);
        while (node != 0u) {
            int32_t nx = (int16_t)PE_LoadU16(node + 0x04u) - clip_x;
            int32_t nw = clip_w - (int16_t)PE_LoadU16(node + 0x08u);
            title_call_update(node, (nx * 3) >> 2, (nw * 3) >> 2,
                              (clip_w * 3) >> 2);
            node = PE_LoadU32(node);
        }
    }

    env = PE_LoadU32(GA_ENVIRONMENT_CURRENT);
    PE_StoreU16(env + 0x7Cu, (uint16_t)(max_x - min_x));
    PE_StoreU16(env + 0x78u, (uint16_t)min_x);
    PE_StoreU16(env + 0x7Au, (uint16_t)min_y);
    PE_StoreU16(env + 0x7Eu, (uint16_t)(max_y - min_y));
    PE_StoreU16(env + 0x70u, (uint16_t)clip_x);
    PE_StoreU16(env + 0x72u, (uint16_t)clip_y);
    PE_StoreU16(env + 0x74u, (uint16_t)clip_w);
    PE_StoreU16(env + 0x76u, (uint16_t)clip_h);
}
