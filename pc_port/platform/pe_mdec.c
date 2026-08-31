/* Value-only MDEC/DMA0-1 substrate for the authenticated libpress path. */
#include "pe_mdec.h"

#include "pe_gpu.h"

#include <string.h>

#define MDEC_DMA_CHCR_INPUT 0x01000201u

static PeMdecState g_mdec;

static uint64_t Fnv1a64(const uint8_t *data, uint32_t size)
{
    uint64_t value = UINT64_C(0xCBF29CE484222325);
    uint32_t i;

    for (i = 0u; i < size; i++) {
        value ^= data[i];
        value *= UINT64_C(0x100000001B3);
    }
    return value;
}

void PE_MDEC_Init(void)
{
    memset(&g_mdec, 0, sizeof(g_mdec));
}

void PE_MDEC_BeginReset(void)
{
    uint32_t reset_count = g_mdec.reset_count + 1u;

    memset(&g_mdec, 0, sizeof(g_mdec));
    g_mdec.reset_count = reset_count;
    PE_MDEC_WriteControl(0x80000000u);
}

void PE_MDEC_WriteControl(uint32_t value)
{
    g_mdec.control_last_write = value;
    g_mdec.control_write_count++;
}

void PE_MDEC_ClearDmaChannels(void)
{
    g_mdec.dma0_madr = 0u;
    g_mdec.dma0_bcr = 0u;
    g_mdec.dma0_chcr = 0u;
    g_mdec.dma1_chcr = 0u;
    g_mdec.dma0_active = 0;
}

int PE_MDEC_SubmitInputTable(pe_addr_t command_block, uint32_t size_words)
{
    uint32_t blocks = size_words >> 5;
    uint32_t words = blocks << 5;
    uint32_t bytes = words << 2;
    const uint8_t *payload;
    PeMdecUpload *upload;

    if (g_mdec.dma0_active) {
        g_mdec.dma0_active = 0;
        g_mdec.completed_upload_count++;
    }
    g_mdec.input_wait_count++;

    payload = PE_TranslateConst((pe_addr_t)(command_block + 4u), bytes);
    PE_GPU_WriteDPCR(PE_GPU_ReadDPCR() | 0x88u);
    g_mdec.command_last_write = PE_LoadU32(command_block);
    g_mdec.dma0_madr = (pe_addr_t)(command_block + 4u);
    g_mdec.dma0_bcr = (blocks << 16) | 0x20u;
    g_mdec.dma0_chcr = MDEC_DMA_CHCR_INPUT;
    g_mdec.dma0_active = 1;

    upload = &g_mdec.uploads[g_mdec.upload_count % PE_MDEC_UPLOAD_HISTORY];
    upload->command = g_mdec.command_last_write;
    upload->source = g_mdec.dma0_madr;
    upload->word_count = words;
    upload->bcr = g_mdec.dma0_bcr;
    upload->payload_fnv1a64 = Fnv1a64(payload, bytes);
    g_mdec.upload_count++;
    return 0;
}

void PE_MDEC_GetState(PeMdecState *out)
{
    if (out != NULL)
        *out = g_mdec;
}
