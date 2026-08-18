/*
 * Phase 6E-B53B — bounded deterministic GPU/DMA2 hardware substrate.
 *
 * This module owns hardware state only.  The retail libgpu command ring,
 * producer/consumer indices, copied packets, and callback slots remain in
 * guest RAM and are intentionally absent here.
 */
#ifndef PE_GPU_H
#define PE_GPU_H

#include <stdint.h>

#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PE_GPU_VRAM_WIDTH       1024u
#define PE_GPU_VRAM_HEIGHT       512u
#define PE_GPU_VRAM_PIXELS      (PE_GPU_VRAM_WIDTH * PE_GPU_VRAM_HEIGHT)

#define PE_GPU_STATUS_READY_GP0 0x04000000u

#define PE_GPU_DMA2_CHCR_IDLE   0x00000401u
#define PE_GPU_DMA2_CHCR_LOAD   0x01000201u
#define PE_GPU_DMA2_CHCR_BUSY   0x01000000u
#define PE_GPU_DMA2_DPCR_ENABLE 0x00000800u
#define PE_GPU_DMA2_DICR_ENABLE 0x00040000u
#define PE_GPU_DICR_MASTER      0x00800000u
#define PE_GPU_DMA2_DICR_FLAG   0x04000000u
#define PE_GPU_DICR_FORCE       0x00008000u
#define PE_GPU_DICR_FLAGS       0x7F000000u
#define PE_GPU_DICR_MASTER_FLAG 0x80000000u

typedef enum {
    PE_GPU_GP0_IDLE = 0,
    PE_GPU_GP0_EXPECT_POSITION,
    PE_GPU_GP0_EXPECT_SIZE,
    PE_GPU_GP0_IMAGE_DATA
} PeGpuGp0State;

/* Read-only snapshot.  Every address is a 32-bit guest address; no native
 * pointer is retained or exposed. */
typedef struct {
    uint32_t status;
    uint32_t gp1_dma_direction;
    PeGpuGp0State gp0_state;
    uint32_t image_x;
    uint32_t image_y;
    uint32_t image_width;
    uint32_t image_height;
    uint32_t image_current_pixel;
    uint32_t image_remaining_pixels;

    pe_addr_t dma2_madr;
    uint32_t dma2_bcr;
    uint32_t dma2_chcr;
    uint32_t dpcr;
    uint32_t dicr;

    int dma2_active;
    pe_addr_t dma2_source;
    uint32_t dma2_word_count;
    uint64_t dma2_event_token;

    uint32_t vsync_count;
    uint64_t dma_event_count;
    uint64_t dma_data_order;
    uint64_t dma_completion_order;
    int dicr_rising_edge_pending;
} PeGpuState;

/* Host lifecycle.  Init clears VRAM and hardware.  Reset cancels hardware
 * activity but deliberately preserves VRAM, matching the distinction
 * between platform initialization and a GP1/control reset. */
void PE_GPU_Init(void);
void PE_GPU_Reset(void);

uint32_t PE_GPU_ReadStatus(void);
void PE_GPU_SetReady(int ready);

/* Supported GP0 subset: 0x01000000 cache clear and the A0 image
 * command/position/size/data stream.  Supported GP1 subset: exact commands
 * 00, 01, 02, 04000000, and 04000002.  Return 1 on acceptance, 0 when the
 * command is unsupported or invalid for the current state. */
int PE_GPU_WriteGP0(uint32_t value);
int PE_GPU_WriteGP1(uint32_t value);

/* Native safety preflight for an A0 image-load command.  This is a pure
 * query: it never changes readiness, parser, DMA, VRAM, or event state. */
int PE_GPU_CanBeginImageLoad(int needs_dma);

/* LoadImage block/request DMA.  Validation precedes all register/state
 * mutation.  Issue never reads the payload or completes the transfer. */
int PE_GPU_DMA2Issue(pe_addr_t madr, uint32_t bcr, uint32_t chcr);
pe_addr_t PE_GPU_ReadDMA2MADR(void);
uint32_t PE_GPU_ReadDMA2BCR(void);
uint32_t PE_GPU_ReadDMA2CHCR(void);
int PE_GPU_DMA2Pending(void);
int PE_GPU_DMA2CompletionPending(void);
int PE_GPU_DMA2InterruptAsserted(void);
uint64_t PE_GPU_DMA2EventToken(void);
int PE_GPU_ServiceDMA2Completion(uint64_t event_token);

uint32_t PE_GPU_ReadDPCR(void);
void PE_GPU_WriteDPCR(uint32_t value);
void PE_GPU_EnableDMA2(void);

uint32_t PE_GPU_ReadDICR(void);
/* Stored state excludes the read-only, physically derived bit 31. */
uint32_t PE_GPU_ReadStoredDICR(void);
void PE_GPU_WriteDICR(uint32_t value);
void PE_GPU_SetDMA2InterruptEnabled(int enabled);
void PE_GPU_AcknowledgeDMA2Interrupt(void);

/* Every DICR mutation recomputes physical bit 31.  A false->true
 * transition latches a one-shot event until this separate bridge consumes
 * it.  Falling transitions never erase an unconsumed rise. */
int PE_GPU_DICRRisingEdgePending(void);
int PE_GPU_TakeDICRRisingEdge(void);

/* Explicit hardware completion-flag input for a represented DMA channel.
 * It has no data-transfer or callback behavior.  A normal completion flag
 * is created only when that channel and the DICR master are enabled. */
int PE_GPU_LatchDMACompletionFlag(uint32_t dma_channel);

/* Safe read-only access to the PSX VRAM authority.  The accessor itself
 * does not wrap; wrapping belongs to the GP0 image-transfer operation. */
int PE_GPU_ReadVRAM(uint32_t x, uint32_t y, uint16_t *pixel);

uint32_t PE_GPU_VSyncQuery(void);
void PE_GPU_VBlankStep(void);

void PE_GPU_GetState(PeGpuState *out);

#ifdef __cplusplus
}
#endif

#endif /* PE_GPU_H */
