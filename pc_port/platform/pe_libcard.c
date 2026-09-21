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

/*
 * ── libcard file API (BIOS B0 32h..43h) ─────────────────────────────────
 *
 * Host model over the present 128 KiB image, following the psx-spx Memory
 * Card Data Format:
 *   - block 0, frame 0      : header ("MC" + XOR)
 *   - block 0, frames 1..15 : the 15 directory entries
 *   - block b (1..15)       : frame b*64 is the block header (same 128-byte
 *                             directory-entry layout), frames +1..+63 are data
 *                             (63 * 128 = 8064 bytes per block)
 * A directory entry stores a 32-bit allocation state (+0x00), a 32-bit byte
 * size (+0x04), a 16-bit next-block link (+0x08) and a 20-byte name (+0x0A);
 * every frame's last byte (+0x7F) is the XOR checksum of +0x00..+0x7E.
 *
 * Nothing is faked: an empty or absent image returns the documented failure.
 */
#define PE_CARD_BLOCK_FRAMES   64u
#define PE_CARD_BLOCKS         16u
#define PE_CARD_DIR_ENTRIES    15u
#define PE_CARD_BLOCK_DATA     (63u * PE_CARD_FRAME_BYTES)  /* 8064 */
#define PE_CARD_ENTRY_USED     0x51u
#define PE_CARD_BLOCK_END      0xFFFFu

typedef struct {
    int used;
    int entry;          /* directory entry index 0..14 */
    uint32_t pos;
    int writable;
} PeCardFd;

static PeCardFd g_card_fd[8];
static int g_card_dir_cursor;
static pe_addr_t g_card_dir_addr;
static int g_card_dir_init;

static uint8_t *pe_card_entry_ptr(uint32_t i)
{
    return g_card_image + (1u + i) * PE_CARD_FRAME_BYTES;
}

static uint8_t *pe_card_block_ptr(uint32_t b)
{
    return g_card_image + b * PE_CARD_BLOCK_FRAMES * PE_CARD_FRAME_BYTES;
}

static uint32_t pe_card_rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void pe_card_wr32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
}

static uint16_t pe_card_rd16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void pe_card_wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8);
}

static int pe_card_entry_used(uint32_t i)
{
    const uint8_t *e = pe_card_entry_ptr(i);
    uint32_t st = pe_card_rd32(e);
    return st != 0u && st != PE_CARD_STATE_FREE;
}

static int pe_card_block_used(uint32_t b)
{
    uint32_t st = pe_card_rd32(pe_card_block_ptr(b));
    return st != 0u && st != PE_CARD_STATE_FREE;
}

static void pe_card_sync_frame(uint8_t *frame)
{
    frame[0x7F] = pe_card_frame_checksum(frame);
}

static uint32_t pe_card_entry_size(uint32_t i)
{
    return pe_card_rd32(pe_card_entry_ptr(i) + 0x04);
}

/* Locate the data block holding byte `pos` (0-based) in entry `i`, allocating
 * and linking fresh blocks as needed when `grow` is set.  Returns 0 on
 * exhaustion.  The chain lives in each block header +0x08; the directory
 * entry's +0x08 links the first block. */
static uint32_t pe_card_resolve_block(uint32_t i, uint32_t pos, int grow)
{
    uint8_t *dir = pe_card_entry_ptr(i);
    uint32_t block = pe_card_rd16(dir + 0x08);
    uint32_t index = pos / PE_CARD_BLOCK_DATA;
    uint32_t n;

    if (block == PE_CARD_BLOCK_END || block == 0u || !pe_card_block_used(block)) {
        if (!grow)
            return 0u;
        block = 0u;
    }
    if (block == 0u) {
        for (n = 1u; n < PE_CARD_BLOCKS; n++) {
            if (!pe_card_block_used(n)) {
                block = n;
                break;
            }
        }
        if (block == 0u)
            return 0u;
        pe_card_wr32(pe_card_block_ptr(block), PE_CARD_ENTRY_USED);
        pe_card_wr16(pe_card_block_ptr(block) + 0x08, PE_CARD_BLOCK_END);
        memcpy(pe_card_block_ptr(block) + 0x0A, dir + 0x0A, 20u);
        pe_card_sync_frame(pe_card_block_ptr(block));
        pe_card_wr16(dir + 0x08, (uint16_t)block);
        pe_card_sync_frame(dir);
    }
    for (n = 0; n < index; n++) {
        uint8_t *h = pe_card_block_ptr(block);
        uint32_t next = pe_card_rd16(h + 0x08);
        if (next == PE_CARD_BLOCK_END || next == 0u || !pe_card_block_used(next)) {
            if (!grow)
                return 0u;
            next = 0u;
            for (uint32_t cand = 1u; cand < PE_CARD_BLOCKS; cand++) {
                if (!pe_card_block_used(cand)) {
                    next = cand;
                    break;
                }
            }
            if (next == 0u)
                return 0u;
            pe_card_wr32(pe_card_block_ptr(next), PE_CARD_ENTRY_USED);
            pe_card_wr16(pe_card_block_ptr(next) + 0x08, PE_CARD_BLOCK_END);
            memcpy(pe_card_block_ptr(next) + 0x0A, dir + 0x0A, 20u);
            pe_card_sync_frame(pe_card_block_ptr(next));
            pe_card_wr16(h + 0x08, (uint16_t)next);
            pe_card_sync_frame(h);
        }
        block = next;
    }
    return block;
}

int func_80071A04(pe_addr_t a, pe_addr_t b, int n)
{
    int i;
    if (n < 0)
        return 1;
    for (i = 0; i < n; i++) {
        uint8_t x = PE_LoadU8(a + (uint32_t)i);
        uint8_t y = PE_LoadU8(b + (uint32_t)i);
        if (x != y)
            return (int)x - (int)y;
    }
    return 0;
}

static pe_addr_t pe_card_fill_dirent(pe_addr_t dirent, uint32_t i)
{
    const uint8_t *e = pe_card_entry_ptr(i);
    uint32_t k;
    for (k = 0; k < 0x28u; k++)
        PE_StoreU8(dirent + k, 0u);
    for (k = 0; k < 20u; k++)
        PE_StoreU8(dirent + k, e[0x0A + k]);
    PE_StoreU32(dirent + 0x14u, 0u);                       /* attr */
    PE_StoreU32(dirent + 0x18u, pe_card_rd32(e + 0x04));   /* size */
    PE_StoreU32(dirent + 0x1Cu, 0u);
    PE_StoreU32(dirent + 0x20u, 0u);
    PE_StoreU32(dirent + 0x24u, 0u);
    return dirent;
}

static pe_addr_t pe_card_dir_advance(pe_addr_t dirent)
{
    while (g_card_dir_cursor < (int)PE_CARD_DIR_ENTRIES) {
        uint32_t i = (uint32_t)g_card_dir_cursor++;
        if (pe_card_entry_used(i))
            return pe_card_fill_dirent(dirent, i);
    }
    return 0u;
}

pe_addr_t func_800727B4(pe_addr_t dirspec, pe_addr_t dirent)
{
    (void)dirspec;
    if (!PE_Card_IsPresent() || dirent == 0u)
        return 0u;
    g_card_dir_cursor = 0;
    g_card_dir_addr = dirent;
    g_card_dir_init = 1;
    return pe_card_dir_advance(dirent);
}

pe_addr_t func_80072794(pe_addr_t dirent)
{
    if (!PE_Card_IsPresent() || dirent == 0u)
        return 0u;
    if (!g_card_dir_init || dirent != g_card_dir_addr) {
        g_card_dir_cursor = 0;
        g_card_dir_addr = dirent;
        g_card_dir_init = 1;
    }
    return pe_card_dir_advance(dirent);
}

static void pe_card_name_from_guest(pe_addr_t name, uint8_t out[20])
{
    uint8_t raw[40];
    uint32_t i, n = 0u, colon = sizeof(raw), src;

    /* Read the whole guest name first: it is "buXX:" (6) + up to 20 filename
     * chars, so capping at 20 before stripping the device prefix truncated the
     * last five filename chars — exactly the two the game's directory match
     * uses (name[0x12]=variant, name[0x13]='A'+slot). */
    for (i = 0; i < sizeof(raw); i++) {
        uint8_t c = PE_LoadU8(name + i);
        raw[i] = c;
        n = i + 1u;
        if (c == 0u)
            break;
    }
    /* Strip the leading device specifier ("bu00:", "bu10:", ...): the card
     * stores only the filename part.  The game's directory match compares the
     * card name against its own name template *after* the "buXX:" prefix
     * (state 2: dirent vs [0x80092224]+6). */
    for (i = 0; i < 6u && i < n; i++) {
        if (raw[i] == ':') { colon = i; break; }
    }
    src = (colon < n) ? colon + 1u : 0u;
    for (i = 0; i < 20u; i++)
        out[i] = (src + i < n) ? raw[src + i] : 0u;
}

int func_80072734(pe_addr_t name, int mode)
{
    uint8_t want[20];
    int entry = -1;
    int create;
    uint32_t i;
    int f;

    if (!PE_Card_IsPresent() || name == 0u || !PE_RangeIsRam(name, 1u))
        return -1;
    pe_card_name_from_guest(name, want);
    for (i = 0; i < PE_CARD_DIR_ENTRIES; i++) {
        if (!pe_card_entry_used(i))
            continue;
        if (memcmp(pe_card_entry_ptr(i) + 0x0A, want, 20u) == 0) {
            entry = (int)i;
            break;
        }
    }
    create = (mode & 0x200) != 0 || mode == 2;
    if (entry < 0) {
        if (!create)
            return -1;
        for (i = 0; i < PE_CARD_DIR_ENTRIES; i++) {
            if (!pe_card_entry_used(i)) {
                uint8_t *e = pe_card_entry_ptr(i);
                memset(e, 0, PE_CARD_FRAME_BYTES);
                memcpy(e + 0x0A, want, 20u);
                pe_card_wr32(e + 0x00u, PE_CARD_ENTRY_USED);
                pe_card_wr32(e + 0x04u, 0u);
                pe_card_wr16(e + 0x08u, PE_CARD_BLOCK_END);
                pe_card_sync_frame(e);
                entry = (int)i;
                break;
            }
        }
        if (entry < 0)
            return -1;
    }
    for (f = 0; f < 8; f++) {
        if (!g_card_fd[f].used) {
            g_card_fd[f].used = 1;
            g_card_fd[f].entry = entry;
            g_card_fd[f].pos = 0;
            g_card_fd[f].writable = create || mode != 1;
            return f;
        }
    }
    return -1;
}

int func_80072744(int fd, int offset, int whence)
{
    uint32_t size;
    int32_t base;

    if (fd < 0 || fd >= 8 || !g_card_fd[fd].used)
        return -1;
    size = pe_card_entry_size((uint32_t)g_card_fd[fd].entry);
    if (whence == 1)
        base = (int32_t)g_card_fd[fd].pos;
    else if (whence == 2)
        base = (int32_t)size;
    else
        base = 0;
    base += offset;
    if (base < 0)
        base = 0;
    if ((uint32_t)base > size)
        base = (int32_t)size;
    g_card_fd[fd].pos = (uint32_t)base;
    return (int)base;
}

int func_80072754(int fd, pe_addr_t buf, int len)
{
    PeCardFd *f;
    uint32_t size, pos, end, done = 0u;

    if (fd < 0 || fd >= 8 || !g_card_fd[fd].used || len < 0)
        return -1;
    if (len > 0 && (buf == 0u || !PE_RangeIsRam(buf, (size_t)len)))
        return -1;
    f = &g_card_fd[fd];
    size = pe_card_entry_size((uint32_t)f->entry);
    pos = f->pos;
    end = pos + (uint32_t)len;
    if (end > size)
        end = size;
    while (pos < end) {
        uint32_t block = pe_card_resolve_block((uint32_t)f->entry, pos, 0);
        uint32_t within = pos % PE_CARD_BLOCK_DATA;
        uint32_t chunk = PE_CARD_BLOCK_DATA - within;
        if (chunk > end - pos)
            chunk = end - pos;
        if (block == 0u)
            break;
        memcpy(PE_Translate(buf + done, chunk),
               pe_card_block_ptr(block) + PE_CARD_FRAME_BYTES + within, chunk);
        pos += chunk;
        done += chunk;
    }
    f->pos = pos;
    return (int)done;
}

int func_80072764(int fd, pe_addr_t buf, int len)
{
    PeCardFd *f;
    uint32_t size, pos, end, done = 0u;

    if (fd < 0 || fd >= 8 || !g_card_fd[fd].used || !g_card_fd[fd].writable || len < 0)
        return -1;
    if (len > 0 && (buf == 0u || !PE_RangeIsRam(buf, (size_t)len)))
        return -1;
    f = &g_card_fd[fd];
    size = pe_card_entry_size((uint32_t)f->entry);
    pos = f->pos;
    end = pos + (uint32_t)len;
    while (pos < end) {
        uint32_t block = pe_card_resolve_block((uint32_t)f->entry, pos, 1);
        uint32_t within = pos % PE_CARD_BLOCK_DATA;
        uint32_t chunk = PE_CARD_BLOCK_DATA - within;
        if (chunk > end - pos)
            chunk = end - pos;
        if (block == 0u)
            break;
        memcpy(pe_card_block_ptr(block) + PE_CARD_FRAME_BYTES + within,
               PE_Translate(buf + done, chunk), chunk);
        pe_card_sync_frame(pe_card_block_ptr(block));
        pos += chunk;
        done += chunk;
    }
    if (pos > size) {
        uint8_t *e = pe_card_entry_ptr((uint32_t)f->entry);
        pe_card_wr32(e + 0x04u, pos);
        pe_card_sync_frame(e);
    }
    f->pos = pos;
    return (int)done;
}

int func_80072774(int fd)
{
    if (fd < 0 || fd >= 8 || !g_card_fd[fd].used)
        return -1;
    pe_card_sync_frame(pe_card_entry_ptr((uint32_t)g_card_fd[fd].entry));
    g_card_fd[fd].used = 0;
    g_card_dirty = 1;
    (void)pe_card_save();
    return 0;
}

int func_80072784(pe_addr_t dev)
{
    (void)dev;
    if (!PE_Card_IsPresent())
        return 0;
    for (int f = 0; f < 8; f++)
        g_card_fd[f].used = 0;
    pe_card_format();
    (void)pe_card_save();
    g_card_dir_init = 0;
    return 1;
}

/* BIOS B(45h) erase(filename) — delete a file on the device.  Releases the
 * entry's data-block chain, clears the directory entry, and persists the image.
 * Returns 1=okay, 0=failed (psx-spx: B(45h)). */
int func_800727A4(pe_addr_t name)
{
    uint8_t want[20];
    uint32_t i;

    if (!PE_Card_IsPresent() || name == 0u || !PE_RangeIsRam(name, 1u))
        return 0;
    pe_card_name_from_guest(name, want);
    for (i = 0; i < PE_CARD_DIR_ENTRIES; i++) {
        uint8_t *e;
        uint32_t block;
        if (!pe_card_entry_used(i))
            continue;
        e = pe_card_entry_ptr(i);
        if (memcmp(e + 0x0A, want, 20u) != 0)
            continue;
        block = pe_card_rd16(e + 0x08);
        while (block != 0u && block != PE_CARD_BLOCK_END &&
               block < PE_CARD_BLOCKS && pe_card_block_used(block)) {
            uint8_t *h = pe_card_block_ptr(block);
            uint32_t next = pe_card_rd16(h + 0x08);
            pe_card_wr32(h, PE_CARD_STATE_FREE);
            pe_card_sync_frame(h);
            block = next;
        }
        memset(e, 0, PE_CARD_FRAME_BYTES);
        pe_card_sync_frame(e);
        g_card_dirty = 1;
        (void)pe_card_save();
        return 1;
    }
    return 0;
}
