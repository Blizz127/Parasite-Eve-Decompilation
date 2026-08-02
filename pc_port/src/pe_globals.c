/*
 * Phase 6A — PE global variable allocations.
 *
 * These globals correspond to retail PE addresses, allocated as host globals.
 * On a 64-bit host, PE's 32-bit addresses are NOT used as raw pointers.
 * Instead we use typed host globals.
 */

#include "psx_compat.h"
#include <stddef.h>

/* ── func_8006E9A0 globals ──────────────────────────────────────────── */

DISP_ENV  D_800BCE80 = {{0}};
uint8_t  *D_80011614 = NULL;
int       D_8009CDDC = 0;
uint32_t  D_8009D280 = 0;

/* ── func_8001220C (main) globals ───────────────────────────────────── */

uint32_t  D_800B0CD8 = 0;
uint32_t  D_8009D1C4 = 0;
uint32_t  D_800A7918 = 0;

/* ── Resolver globals (for func_8006A8D4, func_8003E680) ────────────── */

/* Provide a buffer for the arena-pointer array so func_8006A8D4's
   D_80011614 = &lookup[...] expression compiles. */
static uint8_t arena_buffer[256];
uint8_t *lookup_init(void) { return arena_buffer; }
