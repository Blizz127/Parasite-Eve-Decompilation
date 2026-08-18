/*
 * PE-BTL54 — func_80037548 message-record poll (translated
 * retail, not matching src/). Authority:
 * build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * 27 words 0x80037548..0x800375B4, SHA-256 from oracle.
 * Zero jal. Scan four 56-byte D_800BCEA8 records for
 * +0x10 == needle (lh), return signed byte0. No match → 0.
 * Matching leaf exists under src/; this is the host translation.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_D_800BCEA8 0x800BCEA8u
#define REC_STRIDE    56u

int func_80037548(int needle)
{
    unsigned int i;
    pe_addr_t rec;
    int result;

    needle = (int)(int16_t)needle;
    result = 0;
    for (i = 0; i < 4u; i++) {
        rec = GA_D_800BCEA8 + i * REC_STRIDE;
        if ((int)(int16_t)PE_LoadU16(rec + 0x10u) == needle) {
            result = (int)(int8_t)PE_LoadU8(rec);
            break;
        }
    }
    return result;
}
