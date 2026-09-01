/*
 * Phase 6E-B54K-AI — complete record-pool initializer reached by the movie
 * path. No SDK name is assigned without symbol or string evidence.
 *
 * Entry [0x8007A214,0x8007A240), 11 words, SHA-256
 * 71f85f940c5697918f4902822149f70490cfa4f66d388530e7fee3c3945ce986.
 * Worker [0x8007A244,0x8007A2A4), 24 words, SHA-256
 * be9001c4af959d678340c4d233f6f8d169420493ddde920f7e86e483c284f78d.
 * Clear helper [0x8007C444,0x8007C478), 13 words, SHA-256
 * 04b4534bd4721b1dfdc3bf42335c54cd9d4b3342d5387c2f52da9ffb26e847c4.
 */

#include "psx_compat.h"

#define GA_RECORD_BASE       0x800C0DC8u
#define GA_RECORD_COUNT      0x800C20C4u
#define GA_ACTIVE_INDEX      0x800BE9ECu
#define GA_RANGE_BEGIN       0x800BE9E4u
#define GA_RANGE_END         0x800BE998u
#define GA_ACTIVE_FLAG       0x800B89F4u
#define GA_STATE_0CD0        0x800B0CD0u
#define GA_STATE_8018        0x800A8018u
#define GA_STATE_5D54        0x800A5D54u
#define RECORD_SHIFT         5u

static void clear_record_words(uint32_t first, uint32_t count)
{
    uint32_t i;

    for (i = 0u; i < count; i++) {
        uint32_t index = first + i;
        pe_addr_t base = (pe_addr_t)PE_LoadU32(GA_RECORD_BASE);
        pe_addr_t record = (pe_addr_t)(base + (index << RECORD_SHIFT));
        PE_StoreU32(record, 0u);
    }
}

static void func_8007A244(void)
{
    uint32_t count = PE_LoadU32(GA_RECORD_COUNT);

    PE_StoreU32(GA_ACTIVE_INDEX, 0u);
    PE_StoreU32(GA_RANGE_BEGIN, 0u);
    PE_StoreU32(GA_RANGE_END, 0u);
    PE_StoreU32(GA_ACTIVE_FLAG, 0u);
    clear_record_words(0u, count);
    PE_StoreU32(GA_STATE_0CD0, 0u);
    PE_StoreU16(GA_STATE_8018, 0u);
    PE_StoreU32(GA_STATE_5D54, 0u);
}

void func_8007A214(pe_addr_t record_base, uint32_t record_count)
{
    PE_StoreU32(GA_RECORD_BASE, (uint32_t)record_base);
    PE_StoreU32(GA_RECORD_COUNT, record_count);
    func_8007A244();
}
