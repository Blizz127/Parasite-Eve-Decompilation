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
    uint32_t rectangle_command;
    uint32_t rectangle_position;
    uint32_t rectangle_uv_clut;
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
    g_gpu.rectangle_command = 0;
    g_gpu.rectangle_position = 0;
    g_gpu.rectangle_uv_clut = 0;
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

static int32_t SignExtend11(uint32_t value)
{
    value &= 0x7FFu;
    return (value & 0x400u) != 0u ?
        (int32_t)(value | 0xFFFFF800u) : (int32_t)value;
}

static uint32_t ApplyTextureWindow(uint32_t coordinate, int horizontal)
{
    uint32_t command = g_gpu.state.texture_window;
    uint32_t mask = horizontal ? command & 0x1Fu :
        (command >> 5) & 0x1Fu;
    uint32_t offset = horizontal ? (command >> 10) & 0x1Fu :
        (command >> 15) & 0x1Fu;

    mask <<= 3;
    return ((coordinate & ~mask) | ((offset << 3) & mask)) & 0xFFu;
}

static uint16_t ModulateTextureColor(uint16_t texture, uint32_t command)
{
    uint32_t red = ((uint32_t)(texture & 0x1Fu) *
                    (command & 0xFFu)) >> 7;
    uint32_t green = ((uint32_t)((texture >> 5) & 0x1Fu) *
                      ((command >> 8) & 0xFFu)) >> 7;
    uint32_t blue = ((uint32_t)((texture >> 10) & 0x1Fu) *
                     ((command >> 16) & 0xFFu)) >> 7;

    if (red > 0x1Fu) red = 0x1Fu;
    if (green > 0x1Fu) green = 0x1Fu;
    if (blue > 0x1Fu) blue = 0x1Fu;
    return (uint16_t)(red | (green << 5) | (blue << 10) |
                      (texture & 0x8000u));
}

/* Execution-proven GP0(64h) subset. Rectangles do not dither. The E2h..E6h
 * environment is applied only after the corresponding command has actually
 * been observed; this preserves the pre-PutDrawEnv physical-clip behavior. */
static void DrawTexturedRectangle4(uint32_t size)
{
    uint32_t draw_mode = g_gpu.state.draw_mode;
    uint32_t width = size & 0x3FFu;
    uint32_t height = (size >> 16) & 0x1FFu;
    int32_t origin_x = SignExtend11(g_gpu.rectangle_position);
    int32_t origin_y = SignExtend11(g_gpu.rectangle_position >> 16);
    uint32_t origin_u = g_gpu.rectangle_uv_clut & 0xFFu;
    uint32_t origin_v = (g_gpu.rectangle_uv_clut >> 8) & 0xFFu;
    uint32_t clut = g_gpu.rectangle_uv_clut >> 16;
    uint32_t clut_x = (clut & 0x3Fu) * 16u;
    uint32_t clut_y = (clut >> 6) & (PE_GPU_VRAM_HEIGHT - 1u);
    uint32_t texture_x = (draw_mode & 0xFu) * 64u;
    uint32_t texture_y = ((draw_mode >> 4) & 1u) * 256u;
    int flip_x = (draw_mode & 0x1000u) != 0u;
    int flip_y = (draw_mode & 0x2000u) != 0u;
    uint32_t row;

    if (g_gpu.state.drawing_offset_count != 0u) {
        origin_x += SignExtend11(g_gpu.state.drawing_offset);
        origin_y += SignExtend11(g_gpu.state.drawing_offset >> 11);
    }

    for (row = 0u; row < height; row++) {
        int32_t destination_y = origin_y + (int32_t)row;
        uint32_t texture_v = (origin_v +
            (flip_y ? (0u - row) : row)) & 0xFFu;
        uint32_t column;

        if (g_gpu.state.texture_window_count != 0u)
            texture_v = ApplyTextureWindow(texture_v, 0);
        if (destination_y < 0 || destination_y >=
            (int32_t)PE_GPU_VRAM_HEIGHT) {
            continue;
        }
        if (g_gpu.state.drawing_area_top_left_count != 0u &&
            g_gpu.state.drawing_area_bottom_right_count != 0u &&
            ((uint32_t)destination_y <
                 ((g_gpu.state.drawing_area_top_left >> 10) & 0x1FFu) ||
             (uint32_t)destination_y >
                 ((g_gpu.state.drawing_area_bottom_right >> 10) & 0x1FFu)))
            continue;
        for (column = 0u; column < width; column++) {
            int32_t destination_x = origin_x + (int32_t)column;
            uint32_t texture_u;
            uint16_t packed;
            uint32_t palette_index;
            uint16_t texture_color;

            if (destination_x < 0 || destination_x >=
                (int32_t)PE_GPU_VRAM_WIDTH) {
                continue;
            }
            if (g_gpu.state.drawing_area_top_left_count != 0u &&
                g_gpu.state.drawing_area_bottom_right_count != 0u &&
                ((uint32_t)destination_x <
                     (g_gpu.state.drawing_area_top_left & 0x3FFu) ||
                 (uint32_t)destination_x >
                     (g_gpu.state.drawing_area_bottom_right & 0x3FFu)))
                continue;
            texture_u = (origin_u +
                (flip_x ? (0u - column) : column)) & 0xFFu;
            if (g_gpu.state.texture_window_count != 0u)
                texture_u = ApplyTextureWindow(texture_u, 1);
            packed = g_gpu.vram[
                ((texture_y + texture_v) &
                 (PE_GPU_VRAM_HEIGHT - 1u)) * PE_GPU_VRAM_WIDTH +
                ((texture_x + texture_u / 4u) &
                 (PE_GPU_VRAM_WIDTH - 1u))];
            palette_index =
                (packed >> ((texture_u & 3u) * 4u)) & 0xFu;
            texture_color = g_gpu.vram[
                clut_y * PE_GPU_VRAM_WIDTH +
                ((clut_x + palette_index) &
                 (PE_GPU_VRAM_WIDTH - 1u))];
            if (texture_color == 0u)
                continue;
            {
                uint16_t *destination = &g_gpu.vram[
                    (uint32_t)destination_y * PE_GPU_VRAM_WIDTH +
                    (uint32_t)destination_x];
                uint16_t result;

                if (g_gpu.state.mask_setting_count != 0u &&
                    (g_gpu.state.mask_setting & 2u) != 0u &&
                    (*destination & 0x8000u) != 0u)
                    continue;
                result = ModulateTextureColor(texture_color,
                                              g_gpu.rectangle_command);
                if (g_gpu.state.mask_setting_count != 0u &&
                    (g_gpu.state.mask_setting & 1u) != 0u)
                    result |= 0x8000u;
                *destination = result;
            }
        }
    }
}

static int TexturedRectangle4ModeSupported(void)
{
    /* Bits 7-8 select 4/8/15bpp; bit 11 selects a second-MiB Y base. */
    return (g_gpu.state.draw_mode & 0x180u) == 0u &&
           (g_gpu.state.draw_mode & 0x800u) == 0u;
}

int PE_GPU_WriteGP0(uint32_t value)
{
    uint64_t pixels;

    if ((g_gpu.state.status & PE_GPU_STATUS_READY_GP0) == 0) return 0;

    switch (g_gpu.state.gp0_state) {
    case PE_GPU_GP0_IDLE:
        if (value == 0u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.nop_count++;
            return 1;
        }
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
        if ((value & 0xFF000000u) == 0xE1000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.draw_mode = value;
            g_gpu.state.draw_mode_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE2000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.texture_window = value;
            g_gpu.state.texture_window_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE3000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.drawing_area_top_left = value;
            g_gpu.state.drawing_area_top_left_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE4000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.drawing_area_bottom_right = value;
            g_gpu.state.drawing_area_bottom_right_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE5000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.drawing_offset = value;
            g_gpu.state.drawing_offset_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0xE6000000u) {
            if (g_gpu.state.dma2_active) return 0;
            g_gpu.state.mask_setting = value;
            g_gpu.state.mask_setting_count++;
            return 1;
        }
        if ((value & 0xFF000000u) == 0x64000000u) {
            if (g_gpu.state.dma2_active ||
                !TexturedRectangle4ModeSupported()) {
                return 0;
            }
            g_gpu.rectangle_command = value;
            g_gpu.state.gp0_state = PE_GPU_GP0_RECT_EXPECT_POSITION;
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

    case PE_GPU_GP0_RECT_EXPECT_POSITION:
        if (g_gpu.state.dma2_active) return 0;
        g_gpu.rectangle_position = value;
        g_gpu.state.gp0_state = PE_GPU_GP0_RECT_EXPECT_UV_CLUT;
        return 1;

    case PE_GPU_GP0_RECT_EXPECT_UV_CLUT:
        if (g_gpu.state.dma2_active) return 0;
        g_gpu.rectangle_uv_clut = value;
        g_gpu.state.gp0_state = PE_GPU_GP0_RECT_EXPECT_SIZE;
        return 1;

    case PE_GPU_GP0_RECT_EXPECT_SIZE:
        if (g_gpu.state.dma2_active) return 0;
        DrawTexturedRectangle4(value);
        g_gpu.state.rectangle_command = g_gpu.rectangle_command;
        g_gpu.state.rectangle_position = g_gpu.rectangle_position;
        g_gpu.state.rectangle_uv_clut = g_gpu.rectangle_uv_clut;
        g_gpu.state.rectangle_size = value;
        g_gpu.state.rectangle_count++;
        ResetParser();
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

/* GP0(80h) copies each row left-to-right unless the destination begins to
 * the right of the source, in which case the row is copied right-to-left.
 * That horizontal overlap direction is hardware-tested by DuckStation's
 * software backend.  Rows advance top-to-bottom.  Split both axes at VRAM
 * wrap boundaries so every inner span has ordinary array bounds. */
static void CopyVramSpan(uint32_t source_x, uint32_t source_y,
                         uint32_t destination_x, uint32_t destination_y,
                         uint32_t width, uint32_t height)
{
    uint32_t row;

    if (source_x < destination_x) {
        for (row = 0u; row < height; row++) {
            uint32_t column = width;
            const uint16_t *source =
                &g_gpu.vram[(source_y + row) * PE_GPU_VRAM_WIDTH + source_x];
            uint16_t *destination =
                &g_gpu.vram[(destination_y + row) * PE_GPU_VRAM_WIDTH +
                            destination_x];

            while (column != 0u) {
                column--;
                destination[column] = source[column];
            }
        }
    } else {
        for (row = 0u; row < height; row++) {
            uint32_t column;
            const uint16_t *source =
                &g_gpu.vram[(source_y + row) * PE_GPU_VRAM_WIDTH + source_x];
            uint16_t *destination =
                &g_gpu.vram[(destination_y + row) * PE_GPU_VRAM_WIDTH +
                            destination_x];

            for (column = 0u; column < width; column++) {
                destination[column] = source[column];
            }
        }
    }
}

int PE_GPU_MoveImage(uint32_t source, uint32_t destination, uint32_t size)
{
    uint32_t source_x;
    uint32_t source_y;
    uint32_t destination_x;
    uint32_t destination_y;
    uint32_t width;
    uint32_t height;
    uint32_t rows_remaining;
    uint32_t current_source_y;
    uint32_t current_destination_y;

    if ((g_gpu.state.status & PE_GPU_STATUS_READY_GP0) == 0u ||
        g_gpu.state.gp0_state != PE_GPU_GP0_IDLE ||
        g_gpu.state.dma2_active ||
        g_gpu.state.gp1_dma_direction != 2u) {
        return 0;
    }

    source_x = source & (PE_GPU_VRAM_WIDTH - 1u);
    source_y = (source >> 16) & (PE_GPU_VRAM_HEIGHT - 1u);
    destination_x = destination & (PE_GPU_VRAM_WIDTH - 1u);
    destination_y = (destination >> 16) & (PE_GPU_VRAM_HEIGHT - 1u);
    width = (((size & 0xFFFFu) - 1u) &
             (PE_GPU_VRAM_WIDTH - 1u)) + 1u;
    height = ((((size >> 16) & 0xFFFFu) - 1u) &
              (PE_GPU_VRAM_HEIGHT - 1u)) + 1u;

    rows_remaining = height;
    current_source_y = source_y;
    current_destination_y = destination_y;
    while (rows_remaining != 0u) {
        uint32_t source_rows = PE_GPU_VRAM_HEIGHT - current_source_y;
        uint32_t destination_rows =
            PE_GPU_VRAM_HEIGHT - current_destination_y;
        uint32_t rows = rows_remaining;
        uint32_t columns_remaining = width;
        uint32_t current_source_x = source_x;
        uint32_t current_destination_x = destination_x;

        if (rows > source_rows) rows = source_rows;
        if (rows > destination_rows) rows = destination_rows;

        while (columns_remaining != 0u) {
            uint32_t source_columns =
                PE_GPU_VRAM_WIDTH - current_source_x;
            uint32_t destination_columns =
                PE_GPU_VRAM_WIDTH - current_destination_x;
            uint32_t columns = columns_remaining;

            if (columns > source_columns) columns = source_columns;
            if (columns > destination_columns) columns = destination_columns;
            CopyVramSpan(current_source_x, current_source_y,
                         current_destination_x, current_destination_y,
                         columns, rows);
            current_source_x =
                (current_source_x + columns) & (PE_GPU_VRAM_WIDTH - 1u);
            current_destination_x =
                (current_destination_x + columns) &
                (PE_GPU_VRAM_WIDTH - 1u);
            columns_remaining -= columns;
        }

        current_source_y =
            (current_source_y + rows) & (PE_GPU_VRAM_HEIGHT - 1u);
        current_destination_y =
            (current_destination_y + rows) &
            (PE_GPU_VRAM_HEIGHT - 1u);
        rows_remaining -= rows;
    }

    g_gpu.state.move_count++;
    g_gpu.state.move_source = source;
    g_gpu.state.move_destination = destination;
    g_gpu.state.move_size = size;
    return 1;
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
