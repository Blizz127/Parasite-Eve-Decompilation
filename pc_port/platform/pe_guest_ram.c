/*
 * Phase 6D-S — Contiguous 2 MiB PS1 guest RAM implementation.
 */
#include "pe_guest_ram.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Internal state ─────────────────────────────────────────────────── */

static uint8_t *g_pe_ram = NULL;

/* ── Lifecycle ──────────────────────────────────────────────────────── */

void PE_RamInit(void)
{
    if (g_pe_ram) return;  /* already initialized */
    g_pe_ram = (uint8_t *)calloc(1, PE_RAM_SIZE);
    if (!g_pe_ram) {
        fprintf(stderr, "FATAL: PE_RamInit: cannot allocate %u bytes\n",
                PE_RAM_SIZE);
        abort();
    }
}

void PE_RamReset(void)
{
    if (!g_pe_ram) {
        PE_RamInit();
        return;
    }
    memset(g_pe_ram, 0, PE_RAM_SIZE);
}

void PE_RamDestroy(void)
{
    free(g_pe_ram);
    g_pe_ram = NULL;
}

/* ── Validation ─────────────────────────────────────────────────────── */

bool PE_AddressIsRam(pe_addr_t address)
{
    return address >= PE_RAM_BASE && address < PE_RAM_END;
}

bool PE_RangeIsRam(pe_addr_t address, size_t size)
{
    if (size == 0) return PE_AddressIsRam(address);
    /* Check for overflow: address + size must not wrap */
    if (address > PE_RAM_END) return false;
    if ((uint64_t)address + (uint64_t)size > (uint64_t)PE_RAM_END) return false;
    return address >= PE_RAM_BASE;
}

bool PE_AddAddress(pe_addr_t base, uint32_t delta, pe_addr_t *result)
{
    if (!result) return false;
    /* Check for overflow */
    if ((uint64_t)base + (uint64_t)delta > 0xFFFFFFFFu) {
        *result = 0;
        return false;
    }
    pe_addr_t r = base + delta;
    if (!PE_AddressIsRam(r)) {
        *result = 0;
        return false;
    }
    *result = r;
    return true;
}

/* ── Translation ────────────────────────────────────────────────────── */

static inline uint8_t *translate_impl(pe_addr_t address, size_t size,
                                       const char *caller)
{
    if (!g_pe_ram) {
        fprintf(stderr, "FATAL: %s: guest RAM not initialized\n", caller);
        abort();
    }
    if (!PE_RangeIsRam(address, size)) {
        fprintf(stderr, "FATAL: %s: invalid guest address 0x%08X size %zu\n",
                caller, address, size);
        abort();
    }
    return g_pe_ram + (address - PE_RAM_BASE);
}

void *PE_Translate(pe_addr_t address, size_t size)
{
    return translate_impl(address, size, "PE_Translate");
}

const void *PE_TranslateConst(pe_addr_t address, size_t size)
{
    return translate_impl(address, size, "PE_TranslateConst");
}

/* ── Little-endian loads ────────────────────────────────────────────── */

uint8_t PE_LoadU8(pe_addr_t address)
{
    uint8_t *p = translate_impl(address, 1, "PE_LoadU8");
    return *p;
}

uint16_t PE_LoadU16(pe_addr_t address)
{
    uint8_t *p = translate_impl(address, 2, "PE_LoadU16");
    /* Little-endian: first byte is LSB */
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

uint32_t PE_LoadU32(pe_addr_t address)
{
    uint8_t *p = translate_impl(address, 4, "PE_LoadU32");
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* ── Little-endian stores ───────────────────────────────────────────── */

void PE_StoreU8(pe_addr_t address, uint8_t value)
{
    uint8_t *p = translate_impl(address, 1, "PE_StoreU8");
    *p = value;
}

void PE_StoreU16(pe_addr_t address, uint16_t value)
{
    uint8_t *p = translate_impl(address, 2, "PE_StoreU16");
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
}

void PE_StoreU32(pe_addr_t address, uint32_t value)
{
    uint8_t *p = translate_impl(address, 4, "PE_StoreU32");
    p[0] = (uint8_t)(value & 0xFF);
    p[1] = (uint8_t)((value >> 8) & 0xFF);
    p[2] = (uint8_t)((value >> 16) & 0xFF);
    p[3] = (uint8_t)((value >> 24) & 0xFF);
}
