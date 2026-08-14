/*
 * Phase 6E-B54B — func_8006AD40: proven prefix through counted texture loop.
 *
 * Full retail body:
 *   391 words / 1564 bytes, exe 0x8006AD40–0x8006B35C (exclusive),
 *   file offset 0x5B540.
 *
 * Implemented prefix:
 *   74 words / 296 bytes, exe 0x8006AD40–0x8006AE68 (exclusive).
 *   B54B completes the counted func_8006E1C0 loop beginning with the call
 *   at 0x8006AE48 and ending after the delay slot at 0x8006AE64:
 *
 *       jal  func_8006E1C0
 *        move a1,s4
 *
 * func_8006E1C0 is TRANSLATED (Phase 6E-B51), and B52 translates its two
 * func_8007506C (Psy-Q LoadImage) wrappers through the read-only validator.
 * B53I-D completes the accepted two-LoadImage lifecycle for entry 0. B54A
 * proves that 0x8006AE50 is mid-loop rather than a dependency boundary, so
 * B54B issues the remaining entries through the same translated helper and
 * stops at the loop exit. If the initial entry count is zero, retail bypasses
 * the loop and reaches that same 0x8006AE68 boundary directly.
 *
 * Classification: 1 — translated retail prefix with an honest unresolved
 * boundary. No new dependency or hardware behavior is translated here.
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

    /* 0x8006AE10–0x8006AE64: complete first counted texture-entry loop.
     * The header is at base + lw(base+4) + 0x28, not base + 0x28. */
    {
        pe_addr_t base = PE_LoadU32(GA_D_800B0CD8 + 0x160u);
        pe_addr_t metadata = base + PE_LoadU32(base + 4u);
        uint32_t header = PE_LoadU32(metadata + 0x28u);
        uint32_t count = header >> 22;
        pe_addr_t entry = base + (header & 0x003FFFFFu);
        uint32_t issued = 0u;

        if (count != 0u) {
            for (;;) {
                uint32_t continue_loop;

                func_8006E1C0(entry, base);        /* 0x8006AE48 */
                header = PE_LoadU32(metadata + 0x28u); /* 0x8006AE50 */
                issued += 1u;                      /* 0x8006AE54 */
                count = header >> 22;              /* 0x8006AE58 */
                continue_loop = issued < count;    /* sltu 0x8006AE5C */
                entry += 0x14u;                    /* delay 0x8006AE64 */
                if (!continue_loop)
                    break;
            }
        }
    }

    /* B54B prefix cut at retail 0x8006AE68. The later D_80091648 packing,
     * archive lookup/table walk, CD poll, and all later suffix dependencies
     * remain unconsumed. Keep the established provider name so existing
     * strict/frontier tooling continues to identify this function's cut. */
    (void)Bootstrap_ReturnInt(
        "func_8006AD40_prefix_cut", "func_8006AD40", 0);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
    return 0;
}
