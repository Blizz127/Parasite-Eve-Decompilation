/*
 * Guest voice attribute publish path (partial retail 85F74 / 87798).
 */
#include "pe_spu_voice.h"
#include "pe_spu_dma.h"
#include "psx_compat.h"

static int guest_voice_index(pe_addr_t voice)
{
    if (voice >= 0x800BC000u && voice < 0x800BC000u + 12u * 0x11Cu)
        return (int)((voice - 0x800BC000u) / 0x11Cu);
    if (voice >= 0x800B8AC0u && voice < 0x800B8AC0u + 24u * 0x11Cu)
        return (int)((voice - 0x800B8AC0u) / 0x11Cu) + 12;
    return -1;
}

void func_80087798(uint32_t voice_index, uint32_t vol_left, uint32_t vol_right)
{
    uint32_t reg = voice_index * 0x10u;
    PE_SpuRegister_StoreU16(reg, (uint16_t)(vol_left & 0x7FFFu));
    PE_SpuRegister_StoreU16(reg + 2u, (uint16_t)(vol_right & 0x7FFFu));
}

void func_80085F74(pe_addr_t voice)
{
    uint32_t flags, applied = 0;
    int idx;
    uint32_t reg;

    if (PE_LoadU32(voice) < 1u)
        return;
    flags = PE_LoadU32(voice + 0xF4u);
    if (!flags)
        return;
    idx = guest_voice_index(voice);
    if (idx < 0 || idx >= 24)
        return;
    reg = (uint32_t)idx * 0x10u;

    if (flags & 0x1u) {
        func_80087798((uint32_t)idx,
                      PE_LoadU16(voice + 0x76u),
                      PE_LoadU16(voice + 0x78u));
        applied |= 0x1u;
    }
    if (flags & 0x2u) {
        func_80087798((uint32_t)idx,
                      PE_LoadU16(voice + 0x76u),
                      PE_LoadU16(voice + 0x78u));
        applied |= 0x2u;
    }
    if (flags & 0x40u) {
        PE_SpuRegister_StoreU16(reg + 4u,
                                  (uint16_t)(PE_LoadU32(voice + 0x44u) >> 16));
        applied |= 0x40u;
    }
    if (flags & 0x80u) {
        PE_SpuRegister_StoreU16(reg + 4u,
                                  (uint16_t)(PE_LoadU32(voice + 0x44u) >> 16));
        applied |= 0x80u;
    }
    if (flags & 0x400u) {
        PE_SpuRegister_StoreU16(reg + 6u,
                                  (uint16_t)(PE_LoadU32(voice + 0xF8u) >> 3));
        applied |= 0x400u;
    }
    if (flags & 0x800u) {
        PE_SpuRegister_StoreU16(reg + 8u, PE_LoadU16(voice + 0x10Eu));
        PE_SpuRegister_StoreU16(reg + 0xAu, PE_LoadU16(voice + 0x110u));
        applied |= 0x800u;
    }
    /* 0x2000 is key-off in the retail apply graph; instrument init sets it
     * alongside 0x1000 in the 0x1FF80 cluster but 85F74 finishes on key-on. */
    if ((flags & 0x2000u) && !(flags & 0x1000u)) {
        if (idx < 16)
            PE_SpuRegister_StoreU16(0x18Au,
                                    (uint16_t)(PE_SpuRegister_LoadU16(0x18Au) |
                                               (1u << idx)));
        else
            PE_SpuRegister_StoreU16(0x18Cu,
                                    (uint16_t)(PE_SpuRegister_LoadU16(0x18Cu) |
                                               (1u << (idx - 16))));
        applied |= 0x2000u;
    }
    if (flags & 0x1000u) {
        if (idx < 16)
            PE_SpuRegister_StoreU16(0x188u,
                                    (uint16_t)(PE_SpuRegister_LoadU16(0x188u) |
                                               (1u << idx)));
        else
            PE_SpuRegister_StoreU16(0x18Au,
                                    (uint16_t)(PE_SpuRegister_LoadU16(0x18Au) |
                                               (1u << (idx - 16))));
        applied |= 0x1000u;
    }
    /* Clear the instrument-init cluster once core playback fields are
     * published; loop/extended ADSR bits are deferred, not re-queued. */
    if (flags & 0x1FF80u)
        applied |= flags & 0x1FF80u;
    PE_StoreU32(voice + 0xF4u, flags & ~applied);
}

void PE_SpuScore_ApplyDirtyVoices(void)
{
    uint32_t dirty = PE_LoadU32(0x8009D2C4u);
    unsigned i;

    if (!(dirty & 0x100u))
        return;

    {
        uint32_t active = PE_LoadU32(0x800BCD50u);
        for (i = 0; i < 12u; i++) {
            if (active & (0x1000u << i))
                func_80085F74(0x800BC000u + i * 0x11Cu);
        }
    }
    {
        pe_addr_t state = PE_LoadU32(0x8009D2C8u);
        if (state) {
            uint32_t selected = PE_LoadU32(state + 4u);
            for (i = 0; i < 24u; i++) {
                if (selected & (1u << i))
                    func_80085F74(0x800B8AC0u + i * 0x11Cu);
            }
        }
    }
    PE_StoreU32(0x8009D2C4u, dirty & ~0x100u);
}
