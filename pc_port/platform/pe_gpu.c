/*
 * Phase 6E-B53B — minimal deterministic GPU/DMA2 platform authority.
 *
 * B53A proved the software queue separately.  This file therefore contains
 * no retail ring, worker identity, callback, or queue-pump state.  It models
 * only the hardware subset required by the LoadImage path: 1024x512x16 VRAM,
 * GPUSTAT ready bit 26, the GP0 A0 stream, the required GP1 commands, DMA2
 * block issue/completion, DPCR/DICR channel 2, and a deterministic VBlank
 * counter.  Hardware progress occurs only through explicit API calls.
 */
#include "pe_gpu.h"

#include <stddef.h>
#include <string.h>

typedef struct {
    PeGpuState state;
    uint64_t order_counter;
    uint64_t event_serial;
    uint16_t vram[PE_GPU_VRAM_PIXELS];
} PeGpuAuthority;

static PeGpuAuthority g_gpu = {
    .state = {
        .status = PE_GPU_STATUS_READY_GP0,
        .dma2_chcr = PE_GPU_DMA2_CHCR_IDLE
    }
};

static int DICRMasterFlag(uint32_t stored)
{
    return (stored & PE_GPU_DICR_FORCE) != 0u ||
           ((stored & PE_GPU_DICR_MASTER) != 0u &&
            (stored & PE_GPU_DICR_FLAGS) != 0u);
}

static void CommitStoredDICR(uint32_t stored)
{
    int old_level = DICRMasterFlag(g_gpu.state.dicr);
    int new_level;

    /* Physical/master flag bit 31 is derived and is never stored. */
    stored &= ~PE_GPU_DICR_MASTER_FLAG;
    new_level = DICRMasterFlag(stored);
    g_gpu.state.dicr = stored;
    if (!old_level && new_level) {
        /* Sticky until the separate CPU-source bridge consumes it.  A
         * later falling transition must not erase an already-created IRQ
         * edge. */
        g_gpu.state.dicr_rising_edge_pending = 1;
    }
}

static void ResetParser(void)
{
    g_gpu.state.gp0_state = PE_GPU_GP0_IDLE;
    g_gpu.state.image_x = 0;
    g_gpu.state.image_y = 0;
    g_gpu.state.image_width = 0;
    g_gpu.state.image_height = 0;
    g_gpu.state.image_current_pixel = 0;
    g_gpu.state.image_remaining_pixels = 0;
}

static void ResetHardwareState(void)
{
    uint64_t event_serial = g_gpu.event_serial;

    memset(&g_gpu.state, 0, sizeof(g_gpu.state));
    g_gpu.order_counter = 0;
    g_gpu.event_serial = event_serial;
    g_gpu.state.status = PE_GPU_STATUS_READY_GP0;
    g_gpu.state.dma2_chcr = PE_GPU_DMA2_CHCR_IDLE;
    ResetParser();
}

void PE_GPU_Init(void)
{
    memset(g_gpu.vram, 0, sizeof(g_gpu.vram));
    ResetHardwareState();
}

void PE_GPU_Reset(void)
{
    ResetHardwareState();
}

uint32_t PE_GPU_ReadStatus(void)
{
    return g_gpu.state.status;
}

void PE_GPU_SetReady(int ready)
{
    if (ready) {
        g_gpu.state.status |= PE_GPU_STATUS_READY_GP0;
    } else {
        g_gpu.state.status &= ~PE_GPU_STATUS_READY_GP0;
    }
}

static void FinishImage(void)
{
    g_gpu.state.gp0_state = PE_GPU_GP0_IDLE;
    g_gpu.state.image_remaining_pixels = 0;
}

static void WriteImagePixel(uint16_t pixel)
{
    uint32_t ordinal;
    uint32_t x;
    uint32_t y;

    if (g_gpu.state.gp0_state != PE_GPU_GP0_IMAGE_DATA ||
        g_gpu.state.image_remaining_pixels == 0 ||
        g_gpu.state.image_width == 0) {
        return;
    }

    ordinal = g_gpu.state.image_current_pixel;
    x = (g_gpu.state.image_x + ordinal % g_gpu.state.image_width) &
        (PE_GPU_VRAM_WIDTH - 1u);
    y = (g_gpu.state.image_y + ordinal / g_gpu.state.image_width) &
        (PE_GPU_VRAM_HEIGHT - 1u);
    g_gpu.vram[y * PE_GPU_VRAM_WIDTH + x] = pixel;
    g_gpu.state.image_current_pixel++;
    g_gpu.state.image_remaining_pixels--;
    if (g_gpu.state.image_remaining_pixels == 0) {
        FinishImage();
    }
}

static void WriteImageWord(uint32_t value)
{
    WriteImagePixel((uint16_t)value);
    if (g_gpu.state.gp0_state == PE_GPU_GP0_IMAGE_DATA) {
        WriteImagePixel((uint16_t)(value >> 16));
    }
}

int PE_GPU_WriteGP0(uint32_t value)
{
    uint64_t pixels;

    if ((g_gpu.state.status & PE_GPU_STATUS_READY_GP0) == 0) return 0;

    switch (g_gpu.state.gp0_state) {
    case PE_GPU_GP0_IDLE:
        if (value == 0x01000000u) {
            if (g_gpu.state.dma2_active) return 0;
            ResetParser();
            return 1;
        }
        if (value == 0xA0000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.gp0_state = PE_GPU_GP0_EXPECT_POSITION;
            return 1;
        }
        return 0;

    case PE_GPU_GP0_EXPECT_POSITION:
        g_gpu.state.image_x = value & 0xFFFFu;
        g_gpu.state.image_y = value >> 16;
        g_gpu.state.gp0_state = PE_GPU_GP0_EXPECT_SIZE;
        return 1;

    case PE_GPU_GP0_EXPECT_SIZE:
        g_gpu.state.image_width = value & 0xFFFFu;
        g_gpu.state.image_height = value >> 16;
        if (g_gpu.state.image_width == 0 ||
            g_gpu.state.image_width > PE_GPU_VRAM_WIDTH ||
            g_gpu.state.image_height == 0 ||
            g_gpu.state.image_height > PE_GPU_VRAM_HEIGHT) {
            ResetParser();
            return 0;
        }
        pixels = (uint64_t)g_gpu.state.image_width *
                 (uint64_t)g_gpu.state.image_height;
        if (pixels > UINT32_MAX) {
            ResetParser();
            return 0;
        }
        g_gpu.state.image_current_pixel = 0;
        g_gpu.state.image_remaining_pixels = (uint32_t)pixels;
        g_gpu.state.gp0_state = PE_GPU_GP0_IMAGE_DATA;
        return 1;

    case PE_GPU_GP0_IMAGE_DATA:
        if (g_gpu.state.dma2_active) return 0;
        WriteImageWord(value);
        return 1;
    }

    return 0;
}

int PE_GPU_WriteGP1(uint32_t value)
{
    switch (value) {
    case 0x00000000u: /* reset GPU: represented subset */
        if (g_gpu.state.dma2_active) return 0;
        ResetParser();
        g_gpu.state.gp1_dma_direction = 0;
        g_gpu.state.status |= PE_GPU_STATUS_READY_GP0;
        return 1;
    case 0x01000000u: /* reset command buffer only */
        if (g_gpu.state.dma2_active) return 0;
        ResetParser();
        g_gpu.state.status |= PE_GPU_STATUS_READY_GP0;
        return 1;
    case 0x02000000u: /* acknowledge GPU IRQ; no GPU IRQ state represented */
        return 1;
    case 0x04000000u: /* DMA direction off */
        g_gpu.state.gp1_dma_direction = 0;
        return 1;
    case 0x04000002u: /* DMA CPU -> GP0 */
        g_gpu.state.gp1_dma_direction = 2;
        return 1;
    default:
        return 0;
    }
}

int PE_GPU_CanBeginImageLoad(int needs_dma)
{
    if (g_gpu.state.gp0_state != PE_GPU_GP0_IDLE ||
        g_gpu.state.dma2_active) {
        return 0;
    }
    if (needs_dma &&
        (g_gpu.state.dpcr & PE_GPU_DMA2_DPCR_ENABLE) == 0u) {
        return 0;
    }
    return 1;
}

int PE_GPU_DMA2Issue(pe_addr_t madr, uint32_t bcr, uint32_t chcr)
{
    uint32_t blocks;
    uint32_t words;
    uint64_t bytes;
    uint32_t required_words;

    if (g_gpu.state.dma2_active ||
        g_gpu.state.gp0_state != PE_GPU_GP0_IMAGE_DATA ||
        g_gpu.state.gp1_dma_direction != 2u ||
        (g_gpu.state.dpcr & PE_GPU_DMA2_DPCR_ENABLE) == 0 ||
        chcr != PE_GPU_DMA2_CHCR_LOAD ||
        (madr & 3u) != 0 ||
        (bcr & 0xFFFFu) != 0x10u) {
        return 0;
    }

    blocks = bcr >> 16;
    if (blocks == 0) return 0;
    words = blocks * 16u;
    bytes = (uint64_t)words * 4u;
    required_words = (g_gpu.state.image_remaining_pixels + 1u) / 2u;
    if (words != required_words ||
        bytes > SIZE_MAX ||
        !PE_RangeIsRam(madr, (size_t)bytes)) {
        return 0;
    }

    g_gpu.state.dma2_madr = madr;
    g_gpu.state.dma2_bcr = bcr;
    g_gpu.state.dma2_chcr = chcr;
    g_gpu.state.dma2_active = 1;
    g_gpu.state.dma2_source = madr;
    g_gpu.state.dma2_word_count = words;
    g_gpu.event_serial++;
    if (g_gpu.event_serial == 0) g_gpu.event_serial++;
    g_gpu.state.dma2_event_token = g_gpu.event_serial;
    g_gpu.state.dma_data_order = 0;
    g_gpu.state.dma_completion_order = 0;
    return 1;
}

pe_addr_t PE_GPU_ReadDMA2MADR(void)
{
    return g_gpu.state.dma2_madr;
}

uint32_t PE_GPU_ReadDMA2BCR(void)
{
    return g_gpu.state.dma2_bcr;
}

uint32_t PE_GPU_ReadDMA2CHCR(void)
{
    return g_gpu.state.dma2_chcr;
}

int PE_GPU_DMA2Pending(void)
{
    return g_gpu.state.dma2_active;
}

int PE_GPU_DMA2CompletionPending(void)
{
    return (g_gpu.state.dicr & PE_GPU_DMA2_DICR_FLAG) != 0;
}

int PE_GPU_DMA2InterruptAsserted(void)
{
    return DICRMasterFlag(g_gpu.state.dicr);
}

uint64_t PE_GPU_DMA2EventToken(void)
{
    return g_gpu.state.dma2_event_token;
}

int PE_GPU_ServiceDMA2Completion(uint64_t event_token)
{
    uint32_t i;
    pe_addr_t source;

    if (!g_gpu.state.dma2_active || event_token == 0 ||
        event_token != g_gpu.state.dma2_event_token) {
        return 0;
    }

    source = g_gpu.state.dma2_source;
    for (i = 0; i < g_gpu.state.dma2_word_count; i++) {
        WriteImageWord(PE_LoadU32(source + i * 4u));
    }
    if (g_gpu.state.image_remaining_pixels != 0) {
        return 0;
    }

    g_gpu.state.dma_data_order = ++g_gpu.order_counter;
    g_gpu.state.dma2_active = 0;
    g_gpu.state.dma2_chcr &= ~PE_GPU_DMA2_CHCR_BUSY;
    if ((g_gpu.state.dicr &
         (PE_GPU_DICR_MASTER | PE_GPU_DMA2_DICR_ENABLE)) ==
        (PE_GPU_DICR_MASTER | PE_GPU_DMA2_DICR_ENABLE)) {
        CommitStoredDICR(g_gpu.state.dicr | PE_GPU_DMA2_DICR_FLAG);
    }
    g_gpu.state.dma_completion_order = ++g_gpu.order_counter;
    g_gpu.state.dma_event_count++;
    return 1;
}

uint32_t PE_GPU_ReadDPCR(void)
{
    return g_gpu.state.dpcr;
}

void PE_GPU_WriteDPCR(uint32_t value)
{
    g_gpu.state.dpcr = value;
}

void PE_GPU_EnableDMA2(void)
{
    g_gpu.state.dpcr |= PE_GPU_DMA2_DPCR_ENABLE;
}

uint32_t PE_GPU_ReadDICR(void)
{
    return g_gpu.state.dicr |
           (DICRMasterFlag(g_gpu.state.dicr) ?
            PE_GPU_DICR_MASTER_FLAG : 0u);
}

uint32_t PE_GPU_ReadStoredDICR(void)
{
    return g_gpu.state.dicr;
}

void PE_GPU_WriteDICR(uint32_t value)
{
    uint32_t flags = g_gpu.state.dicr & PE_GPU_DICR_FLAGS;
    flags &= ~(value & PE_GPU_DICR_FLAGS);
    CommitStoredDICR((value & 0x00FFFFFFu) | flags);
}

void PE_GPU_SetDMA2InterruptEnabled(int enabled)
{
    uint32_t stored = g_gpu.state.dicr;

    if (enabled) {
        stored |= PE_GPU_DICR_MASTER | PE_GPU_DMA2_DICR_ENABLE;
    } else {
        stored &= ~PE_GPU_DMA2_DICR_ENABLE;
        stored |= PE_GPU_DICR_MASTER;
    }
    CommitStoredDICR(stored);
}

void PE_GPU_AcknowledgeDMA2Interrupt(void)
{
    PE_GPU_WriteDICR((g_gpu.state.dicr & 0x00FFFFFFu) |
                     PE_GPU_DMA2_DICR_FLAG);
}

int PE_GPU_DICRRisingEdgePending(void)
{
    return g_gpu.state.dicr_rising_edge_pending;
}

int PE_GPU_TakeDICRRisingEdge(void)
{
    int pending = g_gpu.state.dicr_rising_edge_pending;
    g_gpu.state.dicr_rising_edge_pending = 0;
    return pending;
}

int PE_GPU_LatchDMACompletionFlag(uint32_t dma_channel)
{
    uint32_t enable_bit;
    uint32_t flag_bit;

    if (dma_channel >= 7u) {
        return 0;
    }
    enable_bit = 1u << (16u + dma_channel);
    flag_bit = 1u << (24u + dma_channel);
    if ((g_gpu.state.dicr & (PE_GPU_DICR_MASTER | enable_bit)) ==
        (PE_GPU_DICR_MASTER | enable_bit)) {
        CommitStoredDICR(g_gpu.state.dicr | flag_bit);
    }
    return 1;
}

int PE_GPU_ReadVRAM(uint32_t x, uint32_t y, uint16_t *pixel)
{
    if (!pixel || x >= PE_GPU_VRAM_WIDTH || y >= PE_GPU_VRAM_HEIGHT) {
        return 0;
    }
    *pixel = g_gpu.vram[y * PE_GPU_VRAM_WIDTH + x];
    return 1;
}

uint32_t PE_GPU_VSyncQuery(void)
{
    return g_gpu.state.vsync_count;
}

void PE_GPU_VBlankStep(void)
{
    g_gpu.state.vsync_count++;
}

void PE_GPU_GetState(PeGpuState *out)
{
    if (out) {
        *out = g_gpu.state;
        out->dicr = PE_GPU_ReadDICR();
    }
}
