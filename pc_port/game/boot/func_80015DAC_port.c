/*
 * PE-BTL15 — 15DAC default/nop cut, op 0xA (173F4), op 0x1D (17E20).
 * Translated retail, not matching src/.
 *
 * Authority: build/disc1.candidate.exe SHA-1
 * 452fb033f2eaa4b18aa20a5bca60b8125af3a37b.
 *
 * func_80015DAC — 727 words 0x80015DAC..0x80016908, SHA-256
 * bd05f480…a978. D_800910A0[0xEA]. Jump table on *arg0-100,
 * sltiu 311, table at 0x800101B0. Out-of-range and table
 * targets 0x800168F4 are the epilogue `v0=1` (no stores).
 * Live type-6 first 0xEA is key 0x194; type-1 first is 0x193.
 * Both are that nop. Key 0x190 → 0x80016658 is the overlay
 * 12-byte table walk (BTL18). Other keys are not this cut.
 *
 * func_800173F4 — 7 words 0x800173F4..0x80017410. Zero jal.
 * *arg0 = *arg1; v0=1.
 *
 * func_80017E20 — 18 words 0x80017E20..0x80017E68. Zero jal.
 * if *arg0 != *arg1: gp+0x90 = *(D2F0)+0x9C + (*arg2)<<1.
 * Always v0=1.
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define GA_EA_TABLE   0x800101B0u
#define GA_EA_NOP     0x800168F4u
#define GA_EA_190     0x80016658u
#define GA_D_8009D2F0 0x8009D2F0u
#define GA_D_8009CE00 0x8009CE00u
#define GA_D_800B0E64 0x800B0E64u
#define GA_D_800B0DB8 0x800B0DB8u
#define GA_D_800B0DB9 0x800B0DB9u
#define GA_D_800B0DFC 0x800B0DFCu

int func_80015DAC_key190_cut(pe_addr_t args)
{
    pe_addr_t base;
    uint32_t word;
    uint32_t count;
    uint32_t i;
    uint32_t want;
    pe_addr_t ent;

    base = PE_LoadU32(GA_D_800B0E64);
    word = PE_LoadU32(base + PE_LoadU32(base + 4u) + 0x30u);
    count = word >> 22;
    if (count == 0u)
        return 1;
    want = PE_LoadU32(PE_LoadU32(args + 4u));
    ent = base + (word & 0x3FFFFFu);
    for (i = 0u; i < count; i++) {
        if ((PE_LoadU8(ent + 3u) & 0x10u) != 0u &&
            (uint32_t)PE_LoadU16(ent + 10u) == want) {
            PE_StoreU8(GA_D_800B0DB8, (uint8_t)PE_LoadU16(ent + 10u));
            PE_StoreU8(GA_D_800B0DB9, PE_LoadU8(ent + 8u));
            PE_StoreU32(GA_D_800B0DFC,
                        base + (PE_LoadU32(ent + 4u) & 0x00FFFFFFu));
            return 1;
        }
        ent += 12u;
    }
    return 1;
}

int func_80015DAC_default_cut(pe_addr_t args)
{
    uint32_t key;
    uint32_t idx;
    pe_addr_t target;

    key = PE_LoadU32(PE_LoadU32(args));
    idx = key - 100u;
    if (idx >= 311u)
        return 1;
    target = PE_LoadU32(GA_EA_TABLE + idx * 4u);
    if (target == GA_EA_NOP)
        return 1;
    if (target == GA_EA_190)
        return func_80015DAC_key190_cut(args);
    /* Other 15DAC cases are not this cut. */
    return 1;
}

int func_800173F4(pe_addr_t args)
{
    PE_StoreU32(PE_LoadU32(args), PE_LoadU32(PE_LoadU32(args + 4u)));
    return 1;
}

int func_80017E20(pe_addr_t args)
{
    uint32_t left;
    uint32_t right;
    pe_addr_t actor;
    uint32_t off;

    left = PE_LoadU32(PE_LoadU32(args));
    right = PE_LoadU32(PE_LoadU32(args + 4u));
    if (left != right) {
        actor = PE_LoadU32(GA_D_8009D2F0);
        off = PE_LoadU32(PE_LoadU32(args + 8u));
        PE_StoreU32(GA_D_8009CE00,
                    PE_LoadU32(actor + 0x9Cu) + (off << 1));
    }
    return 1;
}
