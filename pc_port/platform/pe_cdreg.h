/* ── pe_cdreg: minimal CD-controller register shadow ──────────────── *
 *
 * Retail's libcd driver dereferences the D_8009B27C/B280/B284/B288/B28C
 * pointer tables, which the EXE image pre-initializes to CD hardware
 * registers ([B27C] = 0x1F801800, [B280] = 0x1F801801, [B284] =
 * 0x1F801802, [B288] = 0x1F801803, [B28C] = 0x1F801020 — read back
 * from the SHA-1-exact Disc 1 EXE).  In-port there is no CD
 * controller, so this file is the single authority for that address
 * range (DICR precedent: peripheral state stays in one authority,
 * translations call it explicitly, no second copy).
 *
 * Semantics (Phase 6E-CD0, evidence in
 * docs/evidence/pe-cd0-reg-shadow/REPORT.md):
 * - Power-on/reset value is 0 everywhere (quiescent: no command in
 *   flight, no interrupt pending — the BA14 tag test `[B288] & 7`
 *   reads clear, exactly the fresh-boot state).
 * - Writes latch per byte; reads return latched bytes.  No executed
 *   chain path reads back a shadow write on its taken arm (proven by
 *   walking every table deref in 7B9EC/7B010/7B558: the tag tests
 *   only ever observe the reset/zeroed state), so the shadow is
 *   behaviorally transparent — any quiescent completion model
 *   produces identical guest-RAM traces.
 * - U32 access is little-endian over the bytes (the 0x1325 result
 *   mailbox word at [B28C]).
 * - The PE_CdLoad/PE_CdStore wrappers replicate address decoding:
 *   CD-range addresses hit the shadow, anything else falls through
 *   to guest RAM.  Translations use the wrappers for table-derived
 *   addresses only; fixed RAM addresses keep direct access.
 */
#ifndef PE_CDREG_H
#define PE_CDREG_H

#include <stdint.h>

#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CD controller registers + result mailbox. */
#define PE_CDREG_BASE 0x1F801800u
#define PE_CDREG_SIZE 4u
#define PE_CDREG_MAILBOX 0x1F801020u
#define PE_CDREG_MAILBOX_SIZE 4u

int PE_CdReg_IsMmio(pe_addr_t address, uint32_t size);
void PE_CdReg_Reset(void);
uint8_t PE_CdReg_ReadU8(pe_addr_t address);
void PE_CdReg_WriteU8(pe_addr_t address, uint8_t value);
uint32_t PE_CdReg_ReadU32(pe_addr_t address);
void PE_CdReg_WriteU32(pe_addr_t address, uint32_t value);

/* Address-decoding access for table-derived addresses. */
uint8_t PE_CdLoadU8(pe_addr_t address);
void PE_CdStoreU8(pe_addr_t address, uint8_t value);
uint32_t PE_CdLoadU32(pe_addr_t address);
void PE_CdStoreU32(pe_addr_t address, uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* PE_CDREG_H */
