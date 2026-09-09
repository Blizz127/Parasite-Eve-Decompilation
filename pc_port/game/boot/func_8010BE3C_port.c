/*
 * Phase 6E-B54K-AG — libpress DecDCTReset and internal hardware reset.
 *
 * Retail function: [0x8010BE3C,0x8010BE70), 13 words, SHA-256
 * d29b6f4ff5d6c611a9193862caffbce26eae005f5cca46747f4fc780ffdc4f9d.
 * The mode-zero arm calls ResetCallback, then every mode forwards unchanged
 * to the internal MDEC reset routine at 0x8010C0FC. B54K-AG translates its
 * complete mode-zero/mode-one hardware-init contract and both table uploads.
 */
#include "psx_compat.h"
#include "game_port.h"
#include "pe_mdec.h"
#include "pe_sdk.h"

static void func_8010C1EC(pe_addr_t command_block, uint32_t size_words)
{
    (void)PE_MDEC_SubmitInputTable(command_block, size_words);
}

static void func_8010C0FC(int mode)
{
    if (mode != 0 && mode != 1) {
        Bootstrap_ReturnVoid1("func_8010C0FC_bad_mode", "func_8010C0FC",
                              (uint32_t)mode);
        PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
        return;
    }

    PE_MDEC_BeginReset();
    PE_MDEC_ClearDmaChannels();
    PE_MDEC_WriteControl(0x60000000u);
    if (mode == 0) {
        func_8010C1EC(0x8010DA0Cu, 32u);
        func_8010C1EC(0x8010DA90u, 32u);
    }
}

void func_8010BE3C(int mode)
{
    if (mode == 0)
        func_80073C94();

    func_8010C0FC(mode);
}

/*
 * Retail DecDCToutCallback: [0x8010C0D8,0x8010C0FC), 9 words,
 * SHA-256 e74b3ac9231b36b93ae760b3c2cd8c018905679c0da656f07b8cbd164933d59b.
 * The wrapper registers only the guest callback identity in DMA channel 1;
 * it does not deliver an interrupt or invoke the callback.
 */
void func_8010C0D8(pe_addr_t callback)
{
    (void)func_80073CF4(1u, callback);
}

/* BFA0..C01C: DecDCTin edits only output-depth/high-bit command flags. */
void func_8010BFA0(pe_addr_t command_block,uint32_t mode)
{
    uint32_t command=PE_LoadU32(command_block);
    command=(mode&1u)?command&~0x08000000u:command|0x08000000u;
    PE_StoreU32(command_block,command);
    command=(mode&2u)?command|0x02000000u:command&~0x02000000u;
    PE_StoreU32(command_block,command);
    (void)PE_MDEC_SubmitInputTable(command_block,PE_LoadU16(command_block));
}
/* C01C wrapper plus C27C..C308: wait, round down to32-word blocks, issue. */
void func_8010C01C(pe_addr_t destination,uint32_t words)
{
    (void)PE_MDEC_SubmitOutput(destination,words);
}
