/* Phase 6D-S — PE global variables backed by contiguous 2 MiB guest RAM.
 *
 * Symbols that name bytes inside guest RAM (D_800B0CD8 block, D_80094488,
 * D_8009448C, D_800BCE80, ...) are typed lvalue macros in psx_compat.h and
 * have NO host definition here — one store, no split-brain.  Only
 * host-owned scalars and pe_addr_t guest-address holders live here. */
#include "psx_compat.h"
#include "pe_port_compat.h"
#include "pe_guest_ram.h"
#include <stddef.h>

/* ── Host-owned scalar globals (plain data, no pointer arithmetic) ──── */

int       D_8009CDDC = 0;
uint32_t  D_8009D280 = 0;
uint32_t  D_8009D1C4 = 0;
uint32_t  D_800A7918 = 0;
unsigned int D_8009D1A0, D_8009D250;
unsigned short D_80093164[4] = {0};

/* ── D_80011614 — retail guest pointer, held host-side as pe_addr_t ────
 *
 * No translated retail writer exists yet (the retail writer is an
 * earlier, untranslated init function).  The initial value is bootstrap
 * policy: the D_8010BD00 arena base, matching the arena anchor used by
 * func_8006A8D4's layout.  Real access sites translate via PE_Translate. */
pe_addr_t D_80011614 = 0x8010BD00u;

/* ── Arena pointer globals — pe_addr_t guest addresses ──────────────── */

pe_addr_t D_800B0E24, D_800B0E28, D_800B0E2C, D_800B0E30,
          D_800B0E34, D_800B0E38, D_800B0E3C, D_800B0E40,
          D_800B0E44, D_800B0E48, D_800B0E4C, D_800B0E50,
          D_800B0E54, D_800B0E58, D_800B0E5C, D_800B0E60,
          D_800B0E64, D_800B0E68, D_800B0E6C;
