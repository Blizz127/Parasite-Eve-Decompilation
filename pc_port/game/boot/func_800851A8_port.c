/*
 * Phase 6E-B46 — func_800851A8: SPU DMA upload (prefix-only)
 * (58 retail words / 0xE8 bytes, exe 0x800851A8–0x8008528F,
 * file offset 0x759A8, live split asm/disc1/74FB0.s:800–863;
 * all 58 instruction words verified exact against the SHA-exact
 * retail executable).
 *
 * Retail signature:
 *   int func_800851A8(pe_addr_t buffer, int transfer_count)
 *
 * Single call site in the executable:
 *   func_80087090 @0x800870B4 (jal, addu $a1,$s1,$zero delay slot):
 *   a0 = buffer, a1 = count; return consumed (compared against 1 for
 *   retry in func_80087090).
 *
 * ROM-order operation map:
 *   Prologue: save $s0/$s1/$s2/$s3/$ra on 0x28-frame.
 *   $s0 = a0 (buffer), $s3 = a1 (count).
 *
 *   1. func_80085174() — completion barrier (spin while D_8009D24C == 1).
 *      Inlined: D_8009D24C is 0 after boot init, terminates immediately.
 *
 *   2. func_80085084(buffer) — magic number check (5 instructions):
 *      v0 = *buffer + 0xB0BEB4BF.  Returns 0 if buffer starts with
 *      0x4F414B41 (the two's-complement complement), nonzero otherwise.
 *      Inlined.
 *
 *   3. If v0 != 0: store v0 to D_8009D24C, return -1 (error path).
 *      The delay slot of the bnez sets v0 = -1 before the branch.
 *
 *   4. If v0 == 0 (success path): read transfer params from buffer+0x10,
 *      call func_80085EB4 (SPU heap alloc), func_800850F4 (DMA transfer
 *      issue), copy data to D_800B2900, call func_80085174 again if
 *      count != 0, return 0.
 *
 *   The success path (steps 4+) directly requires SPU DMA register
 *   programming via func_80085E54 → func_8007D9F8.  This is a genuine
 *   hardware boundary.  The success path is routed through the
 *   centralized bootstrap boundary.
 *
 * Classification: 1 — translated retail logic (prefix).  The magic
 * number check and error path are independently proven retail code.
 * The success path's hardware dependencies (func_80085EB4, func_800850F4,
 * func_80085E54, func_8007D9F8) are routed through the centralized
 * bootstrap boundary.  Strict mode stops at the genuine hardware
 * provider.
 *
 * Dependency boundary:
 *   func_80085174  TRANSLATED (inlined: spin on D_8009D24C)
 *   func_80085084  TRANSLATED (inlined: magic number check)
 *   func_80085EB4  UNRESOLVED (SPU heap allocation) →
 *                  Bootstrap_ReturnInt1 (success path only)
 */
#include "psx_compat.h"
#include "pe_bootstrap.h"

/* ── Guest globals ──────────────────────────────────────────────────── */
#define GA_D_8009D24C  0x8009D24Cu   /* transfer completion flag          */
#define GA_D_800B2900  0x800B2900u   /* transfer data buffer              */

/* Magic number constant for buffer validation.
 * func_80085084 computes *buffer + 0xB0BEB4BF; returns 0 when the
 * buffer's first word is 0x4F414B41 (the two's-complement complement). */
#define PE_851A8_MAGIC  0xB0BEB4BFu

/* ── func_800851A8: SPU DMA upload (prefix-only) ─────────────────────── */
int func_800851A8(pe_addr_t buffer, int count)
{
    uint32_t check;

    /* 1. func_80085174 — completion barrier (spin while D_8009D24C == 1).
     *    After boot init, D_8009D24C is 0; this terminates immediately. */
    while (PE_LoadU32(GA_D_8009D24C) == 1) { }

    /* 2. func_80085084 — magic number check.
     *    v0 = *buffer + 0xB0BEB4BF.  Returns 0 if valid. */
    check = PE_LoadU32(buffer) + PE_851A8_MAGIC;

    /* 3. Error path: if check != 0, store -1 to D_8009D24C and return -1.
     *    Retail delay slot sets v0 = -1 before the branch, and the error
     *    path stores that v0 (-1) to D_8009D24C — not the check value. */
    if (check != 0) {
        PE_StoreU32(GA_D_8009D24C, 0xFFFFFFFFu);
        return -1;
    }

    /* 4. Success path: hardware-dependent (SPU DMA transfer).
     *    This requires func_80085EB4 (SPU heap alloc) and
     *    func_800850F4 (DMA transfer issue via func_80085E54 →
     *    func_8007D9F8).  Routed through the centralized bootstrap
     *    boundary.  Strict mode stops here. */
    return Bootstrap_ReturnInt1("func_800851A8", "func_80087090", 0,
                                buffer);
}
