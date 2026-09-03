/* ── pe_cdreg: minimal CD-controller register shadow ──────────────── */

#include "pe_cdreg.h"

#include "pe_guest_ram.h"

/* Four controller registers + one result-mailbox word, power-on 0. */
static uint8_t g_cdreg[PE_CDREG_SIZE];
static uint8_t g_cdreg_mailbox[PE_CDREG_MAILBOX_SIZE];

int PE_CdReg_IsMmio(pe_addr_t address, uint32_t size)
{
    uint64_t end = (uint64_t)address + (uint64_t)size;
    if (size == 0u || end > 0xFFFFFFFFu)
        return 0;
    if (address >= PE_CDREG_BASE &&
        end <= (uint64_t)PE_CDREG_BASE + PE_CDREG_SIZE)
        return 1;
    if (address >= PE_CDREG_MAILBOX &&
        end <= (uint64_t)PE_CDREG_MAILBOX + PE_CDREG_MAILBOX_SIZE)
        return 1;
    return 0;
}

void PE_CdReg_Reset(void)
{
    uint32_t i;
    for (i = 0u; i < PE_CDREG_SIZE; i++)
        g_cdreg[i] = 0u;
    for (i = 0u; i < PE_CDREG_MAILBOX_SIZE; i++)
        g_cdreg_mailbox[i] = 0u;
}

static uint8_t *cdreg_byte(pe_addr_t address)
{
    if (address >= PE_CDREG_BASE &&
        address < PE_CDREG_BASE + PE_CDREG_SIZE)
        return &g_cdreg[address - PE_CDREG_BASE];
    return &g_cdreg_mailbox[address - PE_CDREG_MAILBOX];
}

uint8_t PE_CdReg_ReadU8(pe_addr_t address)
{
    return *cdreg_byte(address);
}

void PE_CdReg_WriteU8(pe_addr_t address, uint8_t value)
{
    *cdreg_byte(address) = value;
}

uint32_t PE_CdReg_ReadU32(pe_addr_t address)
{
    uint32_t i;
    uint32_t v = 0u;
    for (i = 0u; i < 4u; i++)
        v |= (uint32_t)cdreg_byte(address + i)[0] << (i * 8u);
    return v;
}

void PE_CdReg_WriteU32(pe_addr_t address, uint32_t value)
{
    uint32_t i;
    for (i = 0u; i < 4u; i++)
        cdreg_byte(address + i)[0] = (uint8_t)((value >> (i * 8u)) & 0xFFu);
}

uint8_t PE_CdLoadU8(pe_addr_t address)
{
    if (PE_CdReg_IsMmio(address, 1u))
        return PE_CdReg_ReadU8(address);
    return PE_LoadU8(address);
}

void PE_CdStoreU8(pe_addr_t address, uint8_t value)
{
    if (PE_CdReg_IsMmio(address, 1u)) {
        PE_CdReg_WriteU8(address, value);
        return;
    }
    PE_StoreU8(address, value);
}

uint32_t PE_CdLoadU32(pe_addr_t address)
{
    if (PE_CdReg_IsMmio(address, 4u))
        return PE_CdReg_ReadU32(address);
    return PE_LoadU32(address);
}

void PE_CdStoreU32(pe_addr_t address, uint32_t value)
{
    if (PE_CdReg_IsMmio(address, 4u)) {
        PE_CdReg_WriteU32(address, value);
        return;
    }
    PE_StoreU32(address, value);
}
