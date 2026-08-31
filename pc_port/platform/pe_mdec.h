#ifndef PE_MDEC_H
#define PE_MDEC_H

#include <stdint.h>

#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PE_MDEC_UPLOAD_HISTORY 4u

typedef struct {
    uint32_t command;
    pe_addr_t source;
    uint32_t word_count;
    uint32_t bcr;
    uint64_t payload_fnv1a64;
} PeMdecUpload;

typedef struct {
    uint32_t control_last_write;
    uint32_t command_last_write;
    pe_addr_t dma0_madr;
    uint32_t dma0_bcr;
    uint32_t dma0_chcr;
    uint32_t dma1_chcr;
    uint32_t control_write_count;
    uint32_t reset_count;
    uint32_t input_wait_count;
    uint32_t upload_count;
    uint32_t completed_upload_count;
    int dma0_active;
    PeMdecUpload uploads[PE_MDEC_UPLOAD_HISTORY];
} PeMdecState;

void PE_MDEC_Init(void);
void PE_MDEC_BeginReset(void);
void PE_MDEC_WriteControl(uint32_t value);
void PE_MDEC_ClearDmaChannels(void);
int PE_MDEC_SubmitInputTable(pe_addr_t command_block, uint32_t size_words);
void PE_MDEC_GetState(PeMdecState *out);

#ifdef __cplusplus
}
#endif

#endif
