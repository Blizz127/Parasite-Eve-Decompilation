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

/*
 * Stream-control initializer [0x8007C304,0x8007C388), 33 words,
 * SHA-256 b36aa720216a101d55e1f1fc01fdb4de5d469616649d35efb6e3637cc71992f2.
 * Its setter [0x8007C544,0x8007C560), 7 words, SHA-256
 * 0b85dee972b3ff9e4c01682f71ae2e5c63ed9c55d0934f20911104b6fb66f3db.
 */
#define GA_STREAM_MODE       0x800C0DC0u
#define GA_STREAM_START      0x800B6918u
#define GA_STREAM_END        0x800C0DBCu
#define GA_STREAM_ACTIVE     0x800C0DB8u
#define GA_STREAM_CALLBACK   0x800B0CC8u
#define GA_STREAM_OPTION     0x800A801Cu
#define GA_STREAM_STATE_A    0x800B8620u
#define GA_STREAM_STATE_B    0x800B6914u
#define GA_STREAM_AUXILIARY  0x800B0CCCu

static void func_8007C544(uint32_t mode, int32_t start, int32_t end)
{
    PE_StoreU32(GA_STREAM_MODE, mode);
    PE_StoreU32(GA_STREAM_START, (uint32_t)start);
    PE_StoreU32(GA_STREAM_END, (uint32_t)end);
}

void func_8007C304(uint32_t mode, int32_t start,
                   int32_t end, pe_addr_t callback, uint32_t auxiliary)
{
    func_8007C544(1u, start, end);
    PE_StoreU32(GA_STREAM_ACTIVE, 0u);
    PE_StoreU32(GA_STREAM_CALLBACK, (uint32_t)callback);
    PE_StoreU32(GA_STREAM_OPTION, mode & 1u);
    PE_StoreU32(GA_STREAM_STATE_A, 0u);
    PE_StoreU32(GA_STREAM_STATE_B, 0u);
    PE_StoreU16(GA_STATE_8018, 0u);
    PE_StoreU32(GA_STATE_5D54, 0u);
    PE_StoreU32(GA_STREAM_AUXILIARY, auxiliary);
}
