/*
 * Phase 6E-A — libcard: InitCARD + StartCARD.
 *
 * func_800409B4 (asm/disc1/307CC.s @ file 0x311B4).
 * Classification: 2 (SDK host implementation).
 *
 * Retail structure:
 *   - word guard D_800A1850: skip init when already done.
 *   - EnterCriticalSection; 8x OpenEvent (BIOS B(08h) via func_800726E4)
 *     with classes 0xF4000001/0xF0000011, specs {4,0x8000,0x100,0x2000},
 *     mode 0x1000, handlers func_80042BD8..func_80042C64; handles stored to
 *     D_800BCDA8..D_800BCDC4 (word stride 4).
 *   - func_8007DDD4(0) (_card_init), func_8007DE40, func_800726D4 (A(70h)),
 *     func_8007DD64(0) (A(ADh)): memory-card hardware/kernel bring-up —
 *     collapsed no-ops (no guest-RAM effects).
 *   - 8x EnableEvent (func_80072704) over the stored handles;
 *     ExitCriticalSection.
 *   - unconditionally: sb 0 -> D_800A0ED4+0x418 and D_800A0ED4+0.
 *
 * Kernel Event Control Blocks live outside the 2 MiB guest window; the host
 * event shim (pe_libetc.c) supplies deterministic handles.  The retail
 * handler addresses are preserved verbatim as the OpenEvent argument even
 * though the host never invokes them.
 */
#include "psx_compat.h"
#include "pe_sdk.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const struct {
    uint32_t cls;
    uint32_t spec;
    pe_addr_t handler;
} kCardEvents[8] = {
    { 0xF4000001u, 0x0004u, 0x80042BD8u },
    { 0xF4000001u, 0x8000u, 0x80042BECu },
    { 0xF4000001u, 0x0100u, 0x80042C00u },
    { 0xF4000001u, 0x2000u, 0x80042C14u },
    { 0xF0000011u, 0x0004u, 0x80042C28u },
    { 0xF0000011u, 0x8000u, 0x80042C3Cu },
    { 0xF0000011u, 0x0100u, 0x80042C50u },
    { 0xF0000011u, 0x2000u, 0x80042C64u },
};

void func_800409B4(void)
{
    int i;
    if (PE_LoadU32(0x800A1850u) == 0) {
        PE_StoreU32(0x800A1850u, 1);
        func_80072714();                    /* EnterCriticalSection */
        for (i = 0; i < 8; i++) {
            PE_StoreU32(0x800BCDA8u + (uint32_t)i * 4u,
                        (uint32_t)PE_Event_Open(kCardEvents[i].cls,
                                                kCardEvents[i].spec,
                                                0x1000,
                                                kCardEvents[i].handler));
        }
        /* func_8007DDD4(0), func_8007DE40, func_800726D4, func_8007DD64(0):
         * card hardware/kernel bring-up — collapsed no-ops */
        for (i = 0; i < 8; i++) {
            PE_Event_Enable((int)PE_LoadU32(0x800BCDA8u + (uint32_t)i * 4u));
        }
        func_80072724();                    /* ExitCriticalSection */
    }
    PE_StoreU8(0x800A0ED4u + 0x418u, 0);
    PE_StoreU8(0x800A0ED4u, 0);
}

/* Open + enable the eight card events exactly as func_800409B4 does, without
 * the guard or the guest-RAM writes.  Tests that drive func_800405A4 from a
 * fixture use this to register the same callback events the retail boot path
 * installs, so the empty-slot completion runs its real callback. */
void PE_Card_OpenEvents(void)
{
    int i;
    for (i = 0; i < 8; i++) {
        PE_StoreU32(0x800BCDA8u + (uint32_t)i * 4u,
                    (uint32_t)PE_Event_Open(kCardEvents[i].cls,
                                            kCardEvents[i].spec,
                                            0x1000,
                                            kCardEvents[i].handler));
    }
    for (i = 0; i < 8; i++) {
        PE_Event_Enable((int)PE_LoadU32(0x800BCDA8u + (uint32_t)i * 4u));
    }
}

/*
 * ── Memory-card kernel operations: present + empty models ────────────────
 *
 * func_8007DD44 / func_8007DD54 / func_8007DDC4 / func_8007DDB4 are the
 * BIOS veneers at asm/disc1/6E538.s (A0 ABh _card_info, A0 ACh _card_load,
 * B0 50h _new_card, B0 4Eh _card_write); func_8007DD74 is the retail
 * _new_card + _card_write(port, 3Fh, NULL) wrapper.
 *
 * The completion is reported to the game through the same events the retail
 * kernel delivers (psx-spx BIOS Event Summary), and func_800409B4 opens all
 * eight with mode 1000h, so delivery runs the matching verified callback:
 *
 *   F4000001h,0004h  card done okay        -> func_80042BD8 -> A1820
 *   F4000001h,2000h  card err eject/unfmt  -> func_80042C14 -> A1828
 *   F0000011h,0004h  finished okay         -> func_80042C28 -> A182C
 *   F0000011h,2000h  lower-level I/O err   -> func_80042C64 -> A1834
 *
 * Card presence is host-modelled.  `PE_CARD=empty` forces the documented
 * empty-slot timeout (psx-spx B(5Ch) _card_status 11h).  Otherwise a raw
 * 128 KiB memory-card image (16 blocks x 8 KiB; 1024 frames x 128 bytes) is
 * loaded from `PE_CARD_IMAGE`, default `build/pe_card1.mcr` (git-ignored);
 * when absent it is created and formatted to the psx-spx `Memory Card Data
 * Format`:
 *   - header frame 0: "MC" (4Dh 43h), 02h..7Eh zero, 7Fh = XOR of 00h..7Eh
 *   - directory frames 1..15: 32-bit allocation state A0h (free, freshly
 *     formatted), 04h..07h filesize, 08h..09h next block, 0Ah..1Eh filename,
 *     7Fh XOR checksum.
 * _card_write validates per psx-spx (sectors 0..3FFh valid, 400h accepted by
 * the documented retail quirk) and copies the 128-byte frame into the image.
 */
#define PE_CARD_IMAGE_BYTES 0x20000u   /* 128 KiB: 16 blocks x 8 KiB */
#define PE_CARD_FRAME_BYTES 0x80u      /* 128 bytes per frame/sector */
#define PE_CARD_FRAME_COUNT 0x400u     /* 1024 frames */
#define PE_CARD_DIR_FRAMES  16u        /* block 0 frames: 1 header + 15 dir */
#define PE_CARD_STATE_FREE  0x000000A0u

static uint8_t g_card_image[PE_CARD_IMAGE_BYTES];
static int g_card_present = -1;        /* -1 unresolved, 0 empty, 1 present */
static int g_card_dirty;
static char g_card_path[256];

static uint8_t pe_card_frame_checksum(const uint8_t *frame)
{
    uint8_t sum = 0;
    unsigned i;
    for (i = 0; i < 0x7Fu; i++)
        sum ^= frame[i];
    return sum;
}

static const char *pe_card_image_path(void)
{
    const char *path = getenv("PE_CARD_IMAGE");
    if (!path || !*path)
        path = "build/pe_card1.mcr";
    return path;
}

static int pe_card_save(void)
{
    FILE *f;
    snprintf(g_card_path, sizeof g_card_path, "%s", pe_card_image_path());
    f = fopen(g_card_path, "wb");
    if (!f)
        return -1;
    if (fwrite(g_card_image, 1, sizeof g_card_image, f) != sizeof g_card_image) {
        fclose(f);
        return -1;
    }
    fclose(f);
    g_card_dirty = 0;
    return 0;
}

static void pe_card_format(void)
{
    unsigned frame;
    memset(g_card_image, 0, sizeof g_card_image);
    g_card_image[0] = 'M';
    g_card_image[1] = 'C';
    g_card_image[0x7F] = pe_card_frame_checksum(g_card_image);
    for (frame = 1; frame < PE_CARD_DIR_FRAMES; frame++) {
        uint8_t *dir = g_card_image + frame * PE_CARD_FRAME_BYTES;
        dir[0] = (uint8_t)PE_CARD_STATE_FREE;   /* little-endian 000000A0h */
        dir[0x7F] = pe_card_frame_checksum(dir);
    }
    g_card_dirty = 1;
}

/* Resolve presence once: env override, then an existing valid image, else a
 * freshly formatted image written to the git-ignored path. */
static void pe_card_resolve(void)
{
    const char *mode;
    const char *path;
    FILE *f;
    size_t got;

    if (g_card_present >= 0)
        return;
    mode = getenv("PE_CARD");
    if (mode && strcmp(mode, "empty") == 0) {
        g_card_present = 0;
        return;
    }
    path = pe_card_image_path();
    snprintf(g_card_path, sizeof g_card_path, "%s", path);
    f = fopen(path, "rb");
    if (f) {
        got = fread(g_card_image, 1, sizeof g_card_image, f);
        fclose(f);
        if (got == sizeof g_card_image && g_card_image[0] == 'M' &&
            g_card_image[1] == 'C' &&
            g_card_image[0x7F] == pe_card_frame_checksum(g_card_image)) {
            g_card_present = 1;
            g_card_dirty = 0;
            return;
        }
        fprintf(stderr, "[CARD] '%s' is not a valid 128 KiB image; reformatting\n",
                path);
    }
    pe_card_format();
    if (pe_card_save() != 0) {
        if (getenv("PE_CARD_DEBUG"))
            fprintf(stderr, "[CARD] warning: could not write '%s'; RAM only\n",
                    g_card_path);
    } else if (getenv("PE_CARD_DEBUG"))
        fprintf(stderr, "[CARD] formatted a fresh 128 KiB image at '%s'\n",
                g_card_path);
    g_card_present = 1;
}

void PE_Card_SetPresent(int present)
{
    g_card_present = present ? 1 : 0;
}

/* Return presence to unresolved so the next query re-resolves from the
 * environment/image (test hook; keeps the host runtime's default present). */
void PE_Card_Reset(void)
{
    g_card_present = -1;
    g_card_dirty = 0;
}

int PE_Card_IsPresent(void)
{
    pe_card_resolve();
    return g_card_present == 1;
}

int func_8007DD44(int port)                 /* A0(ABh) _card_info */
{
    (void)port;
    if (PE_Card_IsPresent()) {
        (void)PE_Event_Deliver(0xF4000001u, 0x0004u);   /* card done okay */
        return 1;
    }
    (void)PE_Event_Deliver(0xF4000001u, 0x2000u);       /* card err eject */
    return 0;
}

int func_8007DD54(int port)                 /* A0(ACh) _card_load */
{
    (void)port;
    if (PE_Card_IsPresent()) {
        (void)PE_Event_Deliver(0xF4000001u, 0x0004u);   /* card done okay */
        return 1;
    }
    (void)PE_Event_Deliver(0xF4000001u, 0x2000u);       /* card err eject */
    return 0;
}

int func_8007DDC4(pe_addr_t port)           /* B0(50h) _new_card */
{
    /* _new_card() only clears the BIOS card-change latch for the next
     * read/write; it has no guest-RAM effect and no completion event. */
    (void)port;
    return 0;
}

int func_8007DDB4(pe_addr_t port, int sector, pe_addr_t src)  /* B0(4Eh) _card_write */
{
    (void)port;
    if ((uint32_t)sector > 0x400u)
        return 0;                                   /* rejected, no I/O */
    if (PE_Card_IsPresent()) {
        if (src != 0u && (uint32_t)sector < PE_CARD_FRAME_COUNT &&
            PE_RangeIsRam(src, PE_CARD_FRAME_BYTES)) {
            memcpy(g_card_image + (uint32_t)sector * PE_CARD_FRAME_BYTES,
                   PE_Translate(src, PE_CARD_FRAME_BYTES), PE_CARD_FRAME_BYTES);
            g_card_dirty = 1;
            (void)pe_card_save();
        }
        (void)PE_Event_Deliver(0xF0000011u, 0x0004u);   /* finished okay */
        return 1;
    }
    (void)PE_Event_Deliver(0xF0000011u, 0x2000u);       /* lower-level I/O err */
    return 1;                                           /* accepted, async fail */
}
