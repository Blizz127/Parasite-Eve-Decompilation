/*
 * B54K-AT — host controller delivery into the retail PadInitDirect buffers.
 *
 * Retail's kernel pad driver rewrites the two 0x22-byte buffers registered
 * by func_8003E944 (0x800BE9A0 / 0x800BE9C2) on every VSync: byte 0 status
 * (0x00 ok, 0xFF absent), byte 1 id (0x41 digital, one halfword), bytes
 * 2..3 the active-low button halfword.  The port never modelled that, so
 * guest RAM held zeros, which func_8003EB04 decodes as every button held.
 * This host adapter delivers a connected digital pad in port 1 (nothing
 * pressed unless a hold is scheduled) and no controller in port 2.  It is
 * opt-in: production enables it; tests do not.
 */
#ifndef PE_PAD_H
#define PE_PAD_H

#include <stdint.h>
#include "pe_guest_ram.h"

#define PE_PAD_MAX_HOLDS 8

/* Retail raw button bits (active-high here; delivered inverted). */
#define PE_PAD_SELECT   0x0001u
#define PE_PAD_START    0x0008u
#define PE_PAD_UP       0x0010u
#define PE_PAD_RIGHT    0x0020u
#define PE_PAD_DOWN     0x0040u
#define PE_PAD_LEFT     0x0080u
#define PE_PAD_TRIANGLE 0x1000u
#define PE_PAD_CIRCLE   0x2000u
#define PE_PAD_CROSS    0x4000u
#define PE_PAD_SQUARE   0x8000u

void PE_Pad_Reset(void);
void PE_Pad_Enable(int enabled);
int  PE_Pad_Enabled(void);
void PE_Pad_SetBuffers(pe_addr_t pad1, pe_addr_t pad2);
/* Hold `buttons` for VSync indices [first,last] (inclusive).  Returns 0 on
 * success, -1 when the schedule table is full. */
int  PE_Pad_ScheduleHold(uint16_t buttons, uint32_t first, uint32_t last);
uint16_t PE_Pad_ButtonsAt(uint32_t vsync_index);
/* Write both buffers for this VSync.  No-op while disabled. */
void PE_Pad_Deliver(uint32_t vsync_index);
uint32_t PE_Pad_DeliveryCount(void);

#endif
