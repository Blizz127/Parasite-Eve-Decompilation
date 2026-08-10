/*
 * Phase 6E-B50 corrective — func_8006AD40: proven prefix only.
 *
 * Full retail body:
 *   391 words / 1564 bytes, exe 0x8006AD40–0x8006B35C (exclusive),
 *   file offset 0x5B540.
 *
 * Implemented prefix:
 *   68 words / 272 bytes, exe 0x8006AD40–0x8006AE50 (exclusive).
 *   The final two words are the call at 0x8006AE48 and its delay slot at
 *   0x8006AE4C:
 *
 *       jal  func_8006E1C0
 *        move a1,s4
 *
 * func_8006E1C0 is TRANSLATED (Phase 6E-B51), and B52 translates its two
 * func_8007506C (Psy-Q LoadImage) wrappers through the read-only validator.
 * B53C translates the func_80076C34 timeout/full-check prefix and exposes
 * func_80073E10 as its honest centralized boundary. No queue entry is yet
 * published, so non-strict execution still returns from func_8006AD40 as
 * soon as the translated callee returns. It must not execute the remaining
 * retail control flow with those effects missing.
 *
 * If the entry count is zero, the conditional retail call is bypassed.  The
 * host still returns at the end of the proven static prefix rather than
 * claiming the untranslated suffix.
 *
 * Classification: 1 — translated retail prefix with an honest unresolved
 * state-producing boundary.  No dependency is translated here.
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_D_800930EA  0x800930EAu
#define GA_D_800930EC  0x800930ECu
#define GA_D_800B0CD8  0x800B0CD8u
#define GA_D_800B0DD8  0x800B0DD8u

extern int func_8006E6A8(int lba, pe_addr_t dest, int sectors);
extern int func_8006E7E8(void);

int func_8006AD40(void)
{
    pe_addr_t lba_base;
    int status;

    /* 0x8006AD6C–0x8006AD7C: guarded zero return. */
    if (!(PE_LoadU32(GA_D_800B0CD8) & 1u))
        return 0;

    lba_base = PE_LoadU32(GA_D_800B0DD8);

    /* 0x8006AD88–0x8006ADD0: channel 1 issue/poll loop. */
    for (;;) {
        do {
            uint32_t start = PE_LoadU16(GA_D_800930EA);
            uint32_t end = PE_LoadU16(GA_D_800930EA + 2u);
            status = func_8006E6A8(
                (int)(lba_base + start),
                PE_LoadU32(GA_D_800B0CD8 + 0x160u),
                (int)(end - start));
        } while (status == -1);

        status = func_8006E7E8();
        if (status == 0)
            break;
        if (status != -1) {
            do {
                status = func_8006E7E8();
            } while (status != 0 && status != -1);
            if (status == 0)
                break;
        }
    }

    /* 0x8006ADD8–0x8006AE00: channel 2 issue loop. */
    do {
        uint32_t start = PE_LoadU16(GA_D_800930EC);
        uint32_t end = PE_LoadU16(GA_D_800930EC + 2u);
        status = func_8006E6A8(
            (int)(lba_base + start),
            PE_LoadU32(GA_D_800B0CD8 + 0x174u),
            (int)(end - start));
    } while (status == -1);

    /* 0x8006AE10–0x8006AE4C: first packet and unresolved boundary.
     * The header is at base + lw(base+4) + 0x28, not base + 0x28. */
    {
        pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x160u);
        pe_addr_t metadata = base + PE_LoadU32(base + 4u);
        uint32_t header = PE_LoadU32(metadata + 0x28u);
        uint32_t count = header >> 22;
        pe_addr_t entry = base + (header & 0x003FFFFFu);

        if (count != 0u)
            func_8006E1C0(entry, base);
    }

    /* Prefix boundary: B53E authentically issues the first LoadImage DMA and
     * leaves it pending.  The second request therefore selects the retail
     * enqueue path; B53F crosses func_80073CF4 and stops at its installed
     * func_800746A0 backend before queue publication.  Ask the host loop to
     * honor that already-requested unresolved boundary. */
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
