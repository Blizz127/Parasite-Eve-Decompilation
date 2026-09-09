#include "pe_spu_synth.h"
#include "pe_spu_voice.h"
#include "pe_spu_dma.h"
#include "host_framebuffer.h"

/* 128-byte ADPCM block: shift=0 (12), filter=0, alternating nibbles 1/0. */
static const uint8_t k_test_adpcm_block[128] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
};

static void seed_test_tone(void)
{
    unsigned i;
    ResetTestState();
    PE_SpuDma_Reset();
    for (i = 0; i < sizeof(k_test_adpcm_block); i++)
        PE_SpuRam_StoreU8(0x1010u + i, k_test_adpcm_block[i]);
    PE_SpuRegister_StoreU16(0x00u, 0x3FFFu); /* vol L */
    PE_SpuRegister_StoreU16(0x02u, 0x3FFFu); /* vol R */
    PE_SpuRegister_StoreU16(0x04u, 0x1000u); /* pitch 1.0 */
    PE_SpuRegister_StoreU16(0x06u, 0x0202u); /* start 0x1010 bytes >> 3 */
}

static void test_DAY2_spu_voice_bridge(void)
{
    pe_addr_t voice = 0x800BC000u;
    TEST("DAY2_spu_voice_bridge");
    seed_test_tone();
    PE_StoreU32(voice, 0x80010000u);
    PE_StoreU32(voice + 0xF4u, 0x1FF80u);
    PE_StoreU16(voice + 0x76u, 0x3FFFu);
    PE_StoreU16(voice + 0x78u, 0x3FFFu);
    PE_StoreU32(voice + 0x44u, 0x10000000u);
    PE_StoreU32(voice + 0xF8u, 0x1010u);
    PE_StoreU16(voice + 0x10Eu, 0x00C0u);
    PE_StoreU16(voice + 0x110u, 0x0000u);
    PE_StoreU32(0x800BCD50u, 0x1000u);
    PE_StoreU32(0x8009D2C4u, 0x100u);
    PE_SpuScore_ApplyDirtyVoices();
    ASSERT(PE_LoadU32(voice + 0xF4u) == 0u, "85F74 clears applied flags");
    ASSERT(PE_SpuRegister_LoadU16(0x04u) == 0x1000u, "pitch published");
    ASSERT(PE_SpuRegister_LoadU16(0x06u) == 0x0202u, "start published");
    ASSERT(PE_SpuSynth_ActiveVoiceCount() == 1u, "key-on reaches synthesis");
    ASSERT(PE_SpuSynth_HashMix(128u) != UINT64_C(0x8857F8C2912F2615),
           "bridged voice produces non-silent mix");
    PASS();
}

static void test_DAY2_spu_synth(void)
{
    uint64_t hash;
    TEST("DAY2_spu_synth");
    seed_test_tone();
    ASSERT(PE_SpuSynth_ActiveVoiceCount() == 0u, "idle before key-on");
    PE_SpuRegister_StoreU16(0x188u, 1u);
    ASSERT(PE_SpuSynth_ActiveVoiceCount() == 1u, "voice 0 keyed on");
    hash = PE_SpuSynth_HashMix(256u);
    ASSERT(hash == UINT64_C(0xA1C1894DE517A255),
           "keyed ADPCM mix hash");
    PE_SpuRegister_StoreU16(0x18Au, 1u);
    ASSERT(PE_SpuSynth_ActiveVoiceCount() == 0u, "voice 0 keyed off");
    ASSERT(PE_SpuSynth_HashMix(64u) == UINT64_C(0x8857F8C2912F2615),
           "silent mix after key-off");
    seed_test_tone();
    PE_SpuRegister_StoreU16(0x188u, 1u);
    HostFB_VSync(0);
    ASSERT(PE_SpuSynth_FramesRendered() >= 1u,
           "HostFB_VSync advances synthesis");
    PASS();
}

/* Akao_Tick (8DB7C) advances score state beyond the 8CA84 command drain.
 * Callees 87AA8/8E8D0/89328/8D844/89784 remain host stubs — this proves the
 * bank-walk / tempo / slide scaffolding, not sample-advancing bytecode. */
static void test_DAY2_akao_tick(void)
{
    pe_addr_t st = 0x800B6980u;
    pe_addr_t voice = 0x800B8AC0u;
    uint32_t acc_before;
    TEST("DAY2_akao_tick");
    ResetTestState();
    PE_SpuDma_Reset();
    PE_StoreU32(0x8009D2C8u, st);
    PE_StoreU32(0x8009D268u, 0u);
    PE_StoreU32(0x8009D22Cu, 0u);
    PE_StoreU32(0x8009D2DCu, 4u); /* force tempo-overflow tick path */
    PE_StoreU8(0x8009D2D2u, 0u);  /* scale_tempo identity */
    PE_StoreU32(st + 4u, 1u);     /* primary active mask bit0 */
    PE_StoreU32(st + 0x1Cu, 0u);
    PE_StoreU32(st + 0x6Cu, 0u);
    /* +0x20 is a word; tempo half sits at +0x22 — seed word first, then tempo. */
    PE_StoreU32(st + 0x20u, 0x10u);
    PE_StoreU32(st + 0x24u, 0x5u);
    PE_StoreU16(st + 0x22u, 0x100u);
    PE_StoreU32(st + 0x28u, 0u);
    PE_StoreU16(st + 0x52u, 3u);  /* pitch slide countdown */
    PE_StoreU16(st + 0x58u, 0u);
    PE_StoreU16(st + 0x60u, 0u);
    PE_StoreU16(voice + 0x56u, 5u);
    PE_StoreU16(voice + 0x58u, 9u);
    PE_StoreU32(0x800BCD50u, 0u);
    acc_before = PE_LoadU32(st + 0x28u);
    func_8008DB7C();
    ASSERT(PE_LoadU16(voice + 0x56u) == 4u, "primary voice note timer decrements");
    ASSERT(PE_LoadU16(voice + 0x58u) == 8u, "primary voice gate timer decrements");
    ASSERT(PE_LoadU16(st + 0x52u) == 2u, "pitch slide countdown decrements");
    ASSERT(PE_LoadU16(st + 0x20u) == 0x15u, "pitch slide applies delta");
    ASSERT(PE_LoadU16(st + 0x22u) == 0x100u, "tempo half preserved beside slide word");
    ASSERT((PE_LoadU32(st + 0x28u) & 0xFFFFu) ==
               ((acc_before + 0x100u) & 0xFFFFu),
           "tempo accumulator advances then masks low half");
    /* 87AA8: pitch-slide countdown on voice; cross 0xFFE00000 so F4 dirties. */
    PE_StoreU16(voice + 0x72u, 2u);
    PE_StoreU32(voice + 0x44u, 0x10000000u);
    PE_StoreU32(voice + 0x48u, 0x00200000u);
    PE_StoreU32(voice + 0xF4u, 0u);
    PE_StoreU32(0x8009D2C4u, 0u);
    func_80087AA8(voice, 1u);
    ASSERT(PE_LoadU16(voice + 0x72u) == 1u, "87AA8 decrements pitch slide");
    ASSERT(PE_LoadU32(voice + 0x44u) == 0x10200000u, "87AA8 applies pitch delta");
    ASSERT((PE_LoadU32(voice + 0xF4u) & 3u) != 0u, "87AA8 marks volume pending");
    ASSERT((PE_LoadU32(0x8009D2C4u) & 0x100u) != 0u, "87AA8 raises audio_dirty");
    /* E6 pitch bit 0x10 publishes through ApplyPending. */
    {
        pe_addr_t sv = 0x800BC000u;
        seed_test_tone();
        PE_StoreU32(sv, 0x80010000u);
        PE_StoreU32(sv + 0xF4u, 0x10u);
        PE_StoreU32(sv + 0x44u, 0x12340000u);
        PE_StoreU32(0x800BCD50u, 0x1000u);
        PE_StoreU32(0x8009D2C4u, 0x100u);
        PE_SpuScore_ApplyDirtyVoices();
        ASSERT(PE_LoadU32(sv + 0xF4u) == 0u, "pitch bit 0x10 cleared");
        ASSERT(PE_SpuRegister_LoadU16(0x04u) == 0x1234u, "pitch bit publishes");
    }
    /* 8900C: StepVoiceNote walks active mask and applies KeyOn-shaped publish. */
    {
        pe_addr_t key_scratch = 0x800BCD74u;
        PE_StoreU32(key_scratch, 0);
        PE_StoreU32(voice + 0x38u, 0x10u); /* pitch-flag path */
        PE_StoreU32(voice + 0x30u, 0x1000u);
        PE_StoreU16(voice + 0xE8u, 0);
        PE_StoreU16(voice + 0x36u, 0);
        PE_StoreU16(voice + 0x3Cu, 0); /* depth off */
        PE_StoreU32(voice + 0xF4u, 0);
        PE_StoreU32(0x8009D2C4u, 0);
        func_8008900C(0x800B8AC0u, 1u, 1u, key_scratch);
        ASSERT(PE_LoadU32(key_scratch) == 1u, "8900C records restart key-on");
        ASSERT((PE_LoadU32(voice + 0xF4u) & 0x1010u) != 0u,
               "8900C marks pitch+key pending");
        ASSERT(PE_LoadU16(voice + 0x10Cu) == 0x1000u, "8900C/89218 writes pitch");
    }
    ASSERT(!PE_Port_ShouldStop() && !g_stub_order_count,
           "Akao_Tick scaffolding runs without stub stops");
    PASS();
}
