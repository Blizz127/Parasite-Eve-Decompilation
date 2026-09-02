#include "pe_pad.h"

typedef struct { uint16_t buttons; uint32_t first, last; } PePadHold;

static int g_pad_enabled;
static pe_addr_t g_pad1 = 0x800BE9A0u;
static pe_addr_t g_pad2 = 0x800BE9C2u;
static PePadHold g_holds[PE_PAD_MAX_HOLDS];
static int g_hold_count;
static uint32_t g_deliveries;
static uint16_t g_live_buttons;

void PE_Pad_Reset(void)
{
    g_pad_enabled = 0;
    g_pad1 = 0x800BE9A0u;
    g_pad2 = 0x800BE9C2u;
    g_hold_count = 0;
    g_deliveries = 0;
    g_live_buttons = 0;
}

void PE_Pad_Enable(int enabled) { g_pad_enabled = enabled != 0; }
int  PE_Pad_Enabled(void) { return g_pad_enabled; }

void PE_Pad_SetBuffers(pe_addr_t pad1, pe_addr_t pad2)
{
    g_pad1 = pad1;
    g_pad2 = pad2;
}

int PE_Pad_ScheduleHold(uint16_t buttons, uint32_t first, uint32_t last)
{
    if (g_hold_count >= PE_PAD_MAX_HOLDS || last < first)
        return -1;
    g_holds[g_hold_count].buttons = buttons;
    g_holds[g_hold_count].first = first;
    g_holds[g_hold_count].last = last;
    g_hold_count++;
    return 0;
}

void PE_Pad_SetLiveButtons(uint16_t buttons) { g_live_buttons = buttons; }

uint16_t PE_Pad_ButtonsAt(uint32_t vsync_index)
{
    uint16_t buttons = g_live_buttons;
    int i;
    for (i = 0; i < g_hold_count; i++) {
        if (vsync_index >= g_holds[i].first && vsync_index <= g_holds[i].last)
            buttons |= g_holds[i].buttons;
    }
    return buttons;
}

void PE_Pad_Deliver(uint32_t vsync_index)
{
    uint16_t raw;
    if (!g_pad_enabled)
        return;
    raw = (uint16_t)~PE_Pad_ButtonsAt(vsync_index);
    PE_StoreU8(g_pad1, 0x00u);          /* connected */
    PE_StoreU8(g_pad1 + 1u, 0x41u);     /* digital pad, one halfword */
    PE_StoreU16(g_pad1 + 2u, raw);
    PE_StoreU8(g_pad2, 0xFFu);          /* no controller in port 2 */
    PE_StoreU8(g_pad2 + 1u, 0xFFu);
    PE_StoreU16(g_pad2 + 2u, 0xFFFFu);
    g_deliveries++;
}

uint32_t PE_Pad_DeliveryCount(void) { return g_deliveries; }
