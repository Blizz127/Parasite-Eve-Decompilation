/*
 * Phase 6E-B54K-AS — retail title-screen system of the boot overlay.
 *
 * All words come from PE.IMG (SHA-1 146c0ce7308bf9fdc2ba5a84230e198db0663f3b)
 * sectors [0x03D2,0x0457) loaded at 0x8018EFF0.  Spans and SHA-256:
 *   func_8018F2F4 [0x8018F2F4,0x8018F468)  93w 98797b65…886ed68d3  bg upload
 *   func_8018F468 [0x8018F468,0x8018F7F0) 226w 6f722cf5…0bc4e24a11b compositor
 *   func_8018F7F0 [0x8018F7F0,0x8018F958)  90w e4f0123a…fedea19d8ab85 blend blit
 *   func_8018F958 [0x8018F958,0x8018FBC0) 154w ac36c797…5383bf21304b3c8a cursor blit
 *   func_8018FBC0 [0x8018FBC0,0x8018FD04)  81w fe21ba8d…3c31b5208e9ab spawn(kind)
 *   func_8018FD04 [0x8018FD04,0x8018FE1C)  70w 4dab35eb…621306f486c9b5 spawn kind 4
 *   func_8018FE1C [0x8018FE1C,0x80190064) 146w e70cfcb9…85817e0181a5 menu spawn
 *   func_80190064 [0x80190064,0x80190660) 383w 9234e856…7114b68b8c217 input
 *   leaves 0x80192F98/2FE8/3084/30D8/316C/319C/31BC/3200 (see report)
 * Full hashes: docs/evidence/pe-b54kas-title-system/REPORT.md.
 *
 * Task nodes are 0x34-byte records in the overlay pool 0x801D11CC (7 nodes):
 *   +0x00 next  +0x04 x  +0x06 y  +0x08 w  +0x0A h  (int16)
 *   +0x0C update fn  +0x10 draw fn  +0x14 handler fn  +0x18 image header
 *   +0x1C level/alpha  +0x20 A  +0x24 B  +0x28 C  +0x2C kind  +0x30 retire
 * Function-pointer fields hold the retail guest addresses; the dispatchers
 * below map them to the translations and cut on anything unknown.
 *
 * Sprites are 24-bit images inside the overlay: header at
 * 0x80193254 + table[kind] (table 0x80193258), +0x10 width in 16-bit units,
 * +0x12 height, pixels at +0x14.  The compositor builds a packed 24-bit
 * region of the dirty rect in the arena at env + 0x8080; func_801909B4's
 * loop LoadImages it (PE_Overlay_UploadDirtyRect).
 */
#include "psx_compat.h"
#include "game_port.h"

#define GA_TASK_FREE_HEAD    0x801D136Cu
#define GA_TASK_ACTIVE_HEAD  0x801D1370u
#define GA_TASK_ACTIVE_TAIL  0x801D1374u
#define GA_TASK_PENDING_HEAD 0x801D1378u
#define GA_TASK_PENDING_TAIL 0x801D137Cu
#define GA_ENV_PAIR          0x801D11BCu
#define GA_ENV_ACTIVE        0x801D11C4u
#define GA_ENV_INDEX         0x801D11C8u
#define GA_PAD_PREVIOUS      0x801D11B8u
#define GA_ATTRACT_COUNTER   0x801D1380u
#define GA_IMAGE_BASE        0x80193254u
#define GA_IMAGE_TABLE       0x80193258u
#define GA_KIND_PARAMS       0x801D0D5Cu

#define T_NEXT    0x00u
#define T_X       0x04u
#define T_Y       0x06u
#define T_W       0x08u
#define T_H       0x0Au
#define T_UPDATE  0x0Cu
#define T_DRAW    0x10u
#define T_HANDLER 0x14u
#define T_IMAGE   0x18u
#define T_LEVEL   0x1Cu
#define T_A       0x20u
#define T_B       0x24u
#define T_C       0x28u
#define T_KIND    0x2Cu
#define T_RETIRE  0x30u

/* Retail guest identities of the task callbacks. */
#define FN_FADE_UPDATE   0x80192FE8u
#define FN_RAMP_UPDATE   0x80193084u
#define FN_CURSOR_UPDATE 0x801930D8u
#define FN_FADEOUT_16C   0x8019316Cu
#define FN_SPAWN_KIND2   0x8019319Cu
#define FN_ARM_FADEOUT   0x801931BCu
#define FN_KIND2_FADEOUT 0x80193200u
#define FN_KIND5_HANDLER 0x80192F98u
#define FN_BLEND_BLIT    0x8018F7F0u
#define FN_CURSOR_BLIT   0x8018F958u

extern uint32_t func_8005E038(void);
extern int func_80042770(int slot);
extern int func_8003FFCC(void);
extern void func_800525EC(void);
extern void func_8005267C(void);
extern int func_8007506C(const RECT *rect, pe_addr_t data);
extern pe_addr_t func_80075424(pe_addr_t env);
void PE_Overlay_UploadDirtyRect(void);

pe_addr_t func_8018FBC0(int kind);
void func_8018FD04(void);
void func_8018FE1C(void);

static int16_t LoadS16(pe_addr_t a) { return (int16_t)PE_LoadU16(a); }
static int32_t LoadS32(pe_addr_t a) { return (int32_t)PE_LoadU32(a); }
static void StoreS16(pe_addr_t a, int32_t v) { PE_StoreU16(a, (uint16_t)v); }
static void StoreS32(pe_addr_t a, int32_t v) { PE_StoreU32(a, (uint32_t)v); }

pe_addr_t PE_Title_FindKind(int kind);
static pe_addr_t FindKind(int kind) { return PE_Title_FindKind(kind); }

pe_addr_t PE_Title_FindKind(int kind)
{
    pe_addr_t node = PE_LoadU32(GA_TASK_ACTIVE_HEAD);
    while (node != 0u && LoadS32(node + T_KIND) != kind)
        node = PE_LoadU32(node + T_NEXT);
    return node;
}

static void UnknownGuestFn(const char *cut, pe_addr_t fn)
{
    Bootstrap_ReturnVoid1(cut, "overlay_title", fn);
    PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY);
}

/* ── task pool ─────────────────────────────────────────────────────── */

/* Shared prologue of the three spawn shapes: pop the free head, append to
 * the pending list, zero every field. */
static pe_addr_t TaskAllocAppend(void)
{
    pe_addr_t node = PE_LoadU32(GA_TASK_FREE_HEAD);
    pe_addr_t tail = PE_LoadU32(GA_TASK_PENDING_TAIL);
    pe_addr_t next = PE_LoadU32(node + T_NEXT);

    PE_StoreU32(node + T_NEXT, 0u);
    PE_StoreU32(GA_TASK_FREE_HEAD, next);
    if (tail != 0u) {
        PE_StoreU32(tail + T_NEXT, node);
        PE_StoreU32(GA_TASK_PENDING_TAIL, node);
    } else {
        PE_StoreU32(GA_TASK_PENDING_TAIL, node);
        PE_StoreU32(GA_TASK_PENDING_HEAD, node);
    }
    PE_StoreU32(node + T_UPDATE, 0u);
    PE_StoreU32(node + T_DRAW, 0u);
    PE_StoreU32(node + T_HANDLER, 0u);
    PE_StoreU32(node + T_IMAGE, 0u);
    PE_StoreU16(node + T_X, 0u);
    PE_StoreU16(node + T_Y, 0u);
    PE_StoreU16(node + T_W, 0u);
    PE_StoreU16(node + T_H, 0u);
    PE_StoreU32(node + T_LEVEL, 0u);
    PE_StoreU32(node + T_A, 0u);
    PE_StoreU32(node + T_B, 0u);
    PE_StoreU32(node + T_C, 0u);
    PE_StoreU32(node + T_KIND, 0u);
    PE_StoreU32(node + T_RETIRE, 0u);
    return node;
}

/* kind, image, x/y from the 12-byte parameter record, w = units*2/3, h. */
static void TaskSetImage(pe_addr_t node, int kind, pe_addr_t params)
{
    pe_addr_t image = GA_IMAGE_BASE + PE_LoadU32(GA_IMAGE_TABLE + (uint32_t)kind * 4u);
    int32_t width;

    StoreS32(node + T_KIND, kind);
    PE_StoreU32(node + T_IMAGE, image);
    StoreS16(node + T_X, LoadS32(params));
    StoreS16(node + T_Y, LoadS32(params + 4u));
    width = LoadS16(image + 0x10u);
    StoreS16(node + T_W, (width * 2) / 3);       /* mult 0x55555556 idiom */
    PE_StoreU16(node + T_H, PE_LoadU16(image + 0x12u));
}

pe_addr_t func_8018FBC0(int kind)
{
    pe_addr_t params = GA_KIND_PARAMS + (uint32_t)kind * 12u;
    pe_addr_t node = TaskAllocAppend();

    TaskSetImage(node, kind, params);
    PE_StoreU32(node + T_UPDATE, FN_FADE_UPDATE);
    PE_StoreU32(node + T_DRAW, FN_BLEND_BLIT);
    StoreS32(node + T_A, LoadS16(node + T_Y));
    PE_StoreU32(node + T_C, PE_LoadU32(params + 8u));
    StoreS32(node + T_B, -0x10);
    return node;
}

void func_8018FD04(void)
{
    pe_addr_t params = GA_KIND_PARAMS + 4u * 12u;   /* 0x801D0D8C */
    pe_addr_t node = TaskAllocAppend();

    TaskSetImage(node, 4, params);
    PE_StoreU32(node + T_UPDATE, FN_RAMP_UPDATE);
    PE_StoreU32(node + T_DRAW, FN_BLEND_BLIT);
    PE_StoreU32(node + T_C, PE_LoadU32(params + 8u));
}

void func_8018FE1C(void)
{
    pe_addr_t params;
    pe_addr_t node;

    /* kind 7: draw only (update stays 0). */
    params = GA_KIND_PARAMS + 7u * 12u;             /* 0x801D0DB0 */
    node = TaskAllocAppend();
    TaskSetImage(node, 7, params);
    PE_StoreU32(node + T_DRAW, FN_BLEND_BLIT);
    PE_StoreU32(node + T_C, PE_LoadU32(params + 8u));

    (void)func_8018FBC0(3);
    func_8018FD04();
    node = func_8018FBC0(5);
    PE_StoreU32(node + T_HANDLER, FN_KIND5_HANDLER);

    /* kind 6: cursor. */
    params = GA_KIND_PARAMS + 6u * 12u;             /* 0x801D0DA4 */
    node = TaskAllocAppend();
    TaskSetImage(node, 6, params);
    PE_StoreU32(node + T_UPDATE, FN_CURSOR_UPDATE);
    PE_StoreU32(node + T_DRAW, FN_CURSOR_BLIT);
    PE_StoreU32(node + T_C, PE_LoadU32(params + 8u));
}

/* ── task leaves ───────────────────────────────────────────────────── */

static void Leaf_SpawnKind2(pe_addr_t node)            /* 0x8019319C */
{
    (void)node;
    (void)func_8018FBC0(2);
}

static void Leaf_ArmFadeout(pe_addr_t node)            /* 0x801931BC */
{
    pe_addr_t target = FindKind(2);
    (void)node;
    if (target == 0u) {
        UnknownGuestFn("overlay_title_801931BC_null_kind2_cut", 0u);
        return;
    }
    PE_StoreU32(target + T_UPDATE, FN_KIND2_FADEOUT);
}

static void Leaf_Kind5Handler(pe_addr_t node)          /* 0x80192F98 */
{
    pe_addr_t target = FindKind(7);
    StoreS16(node + T_Y, 0xB4);
    PE_StoreU32(node + T_HANDLER, 0u);
    PE_StoreU32(node + T_UPDATE, 0u);
    PE_StoreU32(node + T_A, 0u);
    if (target == 0u) {
        UnknownGuestFn("overlay_title_80192F98_null_kind7_cut", 0u);
        return;
    }
    StoreS32(target + T_LEVEL, 0x100);
}

static void CallHandler(pe_addr_t fn, pe_addr_t node)
{
    switch (fn) {
    case FN_SPAWN_KIND2:   Leaf_SpawnKind2(node); break;
    case FN_ARM_FADEOUT:   Leaf_ArmFadeout(node); break;
    case FN_KIND5_HANDLER: Leaf_Kind5Handler(node); break;
    default: UnknownGuestFn("overlay_title_unknown_handler_cut", fn); break;
    }
}

static void Update_Fade(pe_addr_t node)                /* 0x80192FE8 */
{
    int32_t a = LoadS32(node + T_A);
    int32_t b = LoadS32(node + T_B);
    int32_t shifted = b << 4;
    pe_addr_t handler;

    StoreS16(node + T_Y, a + b);
    StoreS32(node + T_LEVEL, shifted < 0 ? shifted + 0x100 : 0x100 - shifted);
    b = LoadS32(node + T_B);
    handler = PE_LoadU32(node + T_HANDLER);
    b += (b != 0) ? 1 : 0;
    StoreS32(node + T_B, b);
    if (handler != 0u && b == LoadS32(node + T_C))
        CallHandler(handler, node);
    if (LoadS32(node + T_B) >= 0x10)
        StoreS32(node + T_RETIRE, 1);
}

static void Update_Ramp(pe_addr_t node)                /* 0x80193084 */
{
    int32_t level = LoadS32(node + T_LEVEL);
    int32_t delta = LoadS32(node + T_A);

    if (level <= 0) {
        if (delta < 0) { delta = 0; StoreS32(node + T_A, 0); }
    } else if (level >= 0x100) {
        if (delta > 0) { delta = 0; StoreS32(node + T_A, 0); }
    }
    StoreS32(node + T_LEVEL, level + delta);
}

static void Update_Cursor(pe_addr_t node)              /* 0x801930D8 */
{
    int32_t a = LoadS32(node + T_A);
    int32_t step = (a == 0) ? 0 : (a < 0x54 ? 1 : 0);
    int32_t level = LoadS32(node + T_LEVEL);
    int32_t v;

    a += step;
    StoreS32(node + T_A, a);
    if (level < 0x100)
        level += LoadS32(node + T_B) << 3;
    StoreS32(node + T_LEVEL, level);
    if (level == 0 || level == 0x100)
        StoreS32(node + T_B, 0);
    v = LoadS32(node + T_A);
    StoreS16(node + T_Y, (0x3C - v) >= 0 ? v + 0x50 : 0x8C);
    v = 0x54 - LoadS32(node + T_A);
    StoreS16(node + T_H, v < 0x18 ? 0x18 : v);
}

static void Update_Fadeout16C(pe_addr_t node)          /* 0x8019316C */
{
    int32_t level = LoadS32(node + T_LEVEL) - 0x10;
    if (level < 0)
        level = 0;
    StoreS32(node + T_LEVEL, level);
    if (level == 0) {
        PE_StoreU32(node + T_B, 0u);
        PE_StoreU32(node + T_A, 0u);
    }
}

static void Update_Kind2Fadeout(pe_addr_t node)        /* 0x80193200 */
{
    int32_t level = LoadS32(node + T_LEVEL) - 0x10;
    StoreS32(node + T_LEVEL, level);
    if (level == 0x80)
        func_8018FE1C();
    if (LoadS32(node + T_LEVEL) == 0)
        StoreS32(node + T_RETIRE, 1);
}

static void CallUpdate(pe_addr_t fn, pe_addr_t node)
{
    switch (fn) {
    case FN_FADE_UPDATE:    Update_Fade(node); break;
    case FN_RAMP_UPDATE:    Update_Ramp(node); break;
    case FN_CURSOR_UPDATE:  Update_Cursor(node); break;
    case FN_FADEOUT_16C:    Update_Fadeout16C(node); break;
    case FN_KIND2_FADEOUT:  Update_Kind2Fadeout(node); break;
    default: UnknownGuestFn("overlay_title_unknown_update_cut", fn); break;
    }
}

/* ── blits ─────────────────────────────────────────────────────────── */

/* One byte of the retail lighten blend: dest = max(dest, (src*alpha)>>8). */
static void BlendByte(pe_addr_t src, pe_addr_t dst, int32_t alpha)
{
    int32_t v = ((int32_t)PE_LoadU8(src) * alpha) >> 8;
    int32_t d = PE_LoadU8(dst);
    PE_StoreU8(dst, (uint8_t)(v < d ? d : v));
}

/* 0x8018F7F0: alpha blit of node->image into the packed region. */
static void Draw_Blend(pe_addr_t node, pe_addr_t dest, int32_t skip_words,
                       int32_t row_words)
{
    int32_t words = (LoadS16(node + T_W) * 3) / 4;
    int32_t alpha = LoadS32(node + T_LEVEL);
    pe_addr_t src = PE_LoadU32(node + T_IMAGE) + 0x14u;
    int32_t height = LoadS16(node + T_H);
    int32_t row, word;

    (void)row_words;
    for (row = 0; row < height; row++) {
        for (word = 0; word < words; word++) {
            if (PE_LoadU32(src) == 0u) {
                src += 4u;
                dest += 4u;
                continue;
            }
            BlendByte(src, dest, alpha);         src += 1u; dest += 1u;
            BlendByte(src, dest, alpha);         src += 1u; dest += 1u;
            BlendByte(src, dest, alpha);         src += 1u; dest += 1u;
            BlendByte(src, dest, alpha);         src += 1u; dest += 1u;
        }
        dest += (uint32_t)(skip_words << 2);
    }
}

/* 0x8018F958: the cursor image (0x54 px wide, 24 rows) drawn from row
 * (0x3C - A) downward, then its transposed tail from the last pixel. */
static void Draw_Cursor(pe_addr_t node, pe_addr_t dest, int32_t skip_words,
                        int32_t row_words)
{
    int32_t t9 = LoadS32(node + T_A);
    pe_addr_t image = PE_LoadU32(node + T_IMAGE);
    int32_t alpha = LoadS32(node + T_LEVEL);
    pe_addr_t src = image + 0x14u;
    pe_addr_t dst = dest;
    int32_t t7 = t9 + 0x18;
    int32_t row, px, t5, remaining, v;

    v = 0x3C - t9;
    if (v >= 0)
        dst += (uint32_t)((row_words * v) << 2);
    row = 0;
    do {
        t5 = t7 < 0x55 ? t7 : 0x54;
        for (px = 0; px < t5; px++) {
            BlendByte(src, dst, alpha);          src += 1u; dst += 1u;
            BlendByte(src, dst, alpha);          src += 1u; dst += 1u;
            BlendByte(src, dst, alpha);          src += 1u; dst += 1u;
        }
        remaining = 0x54 - t5;
        src += (uint32_t)(remaining * 3);
        dst += (uint32_t)((skip_words << 2) + remaining * 3);
        row++;
        t7--;
    } while (row < 0x18);

    src = image + 0x17B1u;
    dst = dest + (uint32_t)(t9 * 3);
    v = 0x54 - t9;
    if (v > 0) {
        int32_t t2 = 0x54;
        int32_t rows = v;
        for (row = 0; row < rows; row++) {
            pe_addr_t s = src;
            pe_addr_t d = dst;
            int32_t limit = t2 - t9;
            int32_t count = limit < 0x19 ? limit : 0x18;
            for (px = 0; px < count; px++) {
                BlendByte(s,      d, alpha);     d += 1u;
                BlendByte(s + 1u, d, alpha);     d += 1u;
                BlendByte(s + 2u, d, alpha);     d += 1u;
                s -= 0xFCu;
            }
            src -= 3u;
            dst += (uint32_t)(row_words << 2);
            t2--;
        }
    }
}

static void CallDraw(pe_addr_t fn, pe_addr_t node, pe_addr_t dest,
                     int32_t skip_words, int32_t row_words)
{
    switch (fn) {
    case FN_BLEND_BLIT:  Draw_Blend(node, dest, skip_words, row_words); break;
    case FN_CURSOR_BLIT: Draw_Cursor(node, dest, skip_words, row_words); break;
    default: UnknownGuestFn("overlay_title_unknown_draw_cut", fn); break;
    }
}

/* ── compositor ────────────────────────────────────────────────────── */

void func_8018F468(void)
{
    pe_addr_t node = PE_LoadU32(GA_TASK_ACTIVE_HEAD);
    int32_t min_x = 0x7FFF, min_y = 0x7FFF, max_x = 0, max_y = 0;
    int32_t ux, uy, uw, uh;
    pe_addr_t env;

    while (node != 0u) {
        pe_addr_t update = PE_LoadU32(node + T_UPDATE);
        int32_t x, y;
        if (update != 0u) {
            CallUpdate(update, node);
            if (PE_Port_ShouldStop())
                return;
        }
        x = LoadS16(node + T_X);
        y = LoadS16(node + T_Y);
        if (x < min_x) min_x = x;
        if (y < min_y) min_y = y;
        if (max_x < x + LoadS16(node + T_W)) max_x = x + LoadS16(node + T_W);
        if (max_y < y + LoadS16(node + T_H)) max_y = y + LoadS16(node + T_H);
        node = PE_LoadU32(node + T_NEXT);
    }

    env = PE_LoadU32(GA_ENV_ACTIVE);
    if (LoadS16(env + 0x7Cu) > 0) {
        int32_t px = LoadS16(env + 0x78u), py = LoadS16(env + 0x7Au);
        int32_t pw = LoadS16(env + 0x7Cu), ph = LoadS16(env + 0x7Eu);
        int32_t right, bottom;
        ux = px < min_x ? px : min_x;
        uy = py < min_y ? py : min_y;
        right = (max_x < px + pw) ? px + pw : max_x;
        uw = right - ux;
        bottom = (max_y < py + ph) ? py + ph : max_y;
        uh = bottom - uy;
    } else {
        uw = max_x - min_x;
        uh = max_y - min_y;
        ux = min_x;
        uy = min_y;
    }

    if (uw > 0 && uh > 0) {
        int32_t row_words = (uw * 3) / 4;
        pe_addr_t bg = GA_IMAGE_BASE + PE_LoadU32(GA_IMAGE_TABLE);
        int32_t src_bytes = ((uy - 0x14) * 320 + ux) * 3;
        pe_addr_t src = bg + 0x14u + (uint32_t)((src_bytes / 4) * 4);
        pe_addr_t dst = env + 0x8080u;
        uint32_t src_skip = (uint32_t)((0xF0 - row_words) << 2);
        int32_t row, word;

        for (row = 0; row < uh; row++) {
            for (word = 0; word < row_words; word++) {
                PE_StoreU32(dst, PE_LoadU32(src));
                src += 4u;
                dst += 4u;
            }
            src += src_skip;
        }

        for (node = PE_LoadU32(GA_TASK_ACTIVE_HEAD); node != 0u;
             node = PE_LoadU32(node + T_NEXT)) {
            pe_addr_t draw = PE_LoadU32(node + T_DRAW);
            int32_t pixel, bytes, skip;
            if (draw == 0u)
                continue;
            pixel = (LoadS16(node + T_Y) - uy) * uw + (LoadS16(node + T_X) - ux);
            bytes = pixel * 3;
            skip = ((uw - LoadS16(node + T_W)) * 3) / 4;
            CallDraw(draw, node, env + 0x8080u + (uint32_t)((bytes / 4) * 4),
                     skip, (uw * 3) / 4);
            if (PE_Port_ShouldStop())
                return;
        }
    }

    StoreS16(env + 0x7Cu, max_x - min_x);
    StoreS16(env + 0x78u, min_x);
    StoreS16(env + 0x7Au, min_y);
    StoreS16(env + 0x7Eu, max_y - min_y);
    StoreS16(env + 0x70u, ux);
    StoreS16(env + 0x72u, uy);
    StoreS16(env + 0x74u, uw);
    StoreS16(env + 0x76u, uh);
}

/* ── background upload ─────────────────────────────────────────────── */

void func_8018F2F4(void)
{
    RECT rect;
    int i;

    for (i = 0; i < 2; i++) {
        uint32_t next = PE_LoadU32(GA_ENV_INDEX) == 0u ? 1u : 0u;
        pe_addr_t env = PE_LoadU32(GA_ENV_PAIR + next * 4u);
        PE_StoreU32(GA_ENV_INDEX, next);
        PE_StoreU32(GA_ENV_ACTIVE, env);
        rect.x = 0;
        rect.y = next != 0u ? 0x14 : 0x104;
        rect.w = 0x1E0;
        rect.h = 0xCC;
        (void)func_8007506C(&rect, GA_IMAGE_BASE + PE_LoadU32(GA_IMAGE_TABLE) + 0x14u);
        if (PE_Port_ShouldStop())
            return;
        func_80074DC0(0);
        PE_Overlay_UploadDirtyRect();
        func_80073A44(0);
        (void)func_80074A44(1);
        (void)func_80075424(env);
        func_800755F0((void *)(uintptr_t)(env + 0x5Cu));
    }
}

/* ── input ─────────────────────────────────────────────────────────── */

void func_80190064(void)
{
    uint32_t pad = func_8005E038();
    uint32_t previous = PE_LoadU32(GA_PAD_PREVIOUS);
    pe_addr_t node, cursor, other;
    int32_t y;

    /* Start: arm the kind-1 task once the kind-2 sprite has faded in. */
    if ((pad & 0x800u) != 0u && (previous & 0x800u) == 0u) {
        node = FindKind(2);
        if (node != 0u && LoadS32(node + T_LEVEL) >= 0x81) {
            node = FindKind(1);
            /* retail dereferences the (possibly null) result; kind 1 lives
             * for the whole title loop, so a miss is an unresolved state. */
            if (node == 0u) {
                UnknownGuestFn("overlay_title_80190128_null_kind1_cut", 0u);
                return;
            }
            if (LoadS32(node + T_B) == 0) {
                StoreS32(node + T_B, 1);
                StoreS32(node + T_C, 8);
                PE_StoreU32(node + T_HANDLER, FN_ARM_FADEOUT);
                PE_StoreU32(GA_ATTRACT_COUNTER, 0u);
                func_800525EC();
            }
        }
    }

    cursor = FindKind(5);
    if (cursor != 0u && PE_LoadU32(cursor + T_UPDATE) == 0u) {
        if (func_80042770(0) != 0 || func_80042770(1) != 0) {
            /* a card slot is present: the cursor may reach row 0xC8 */
            y = LoadS16(cursor + T_Y);
            if (y < 0xC8) {
                StoreS16(cursor + T_Y, y + 1);
                other = FindKind(4);
                if (other == 0u) {
                    UnknownGuestFn("overlay_title_80190220_null_kind4_cut", 0u);
                    return;
                }
                StoreS32(other + T_A, 0x10);
                PE_StoreU32(GA_ATTRACT_COUNTER, 0u);
            }
            if (LoadS16(cursor + T_Y) == 0xC8 && func_8003FFCC() != 0) {
                other = FindKind(6);
                if (other != 0u && LoadS32(other + T_A) == 0) {
                    PE_StoreU32(other + T_UPDATE, FN_CURSOR_UPDATE);
                    PE_StoreU32(other + T_DRAW, FN_CURSOR_BLIT);
                    StoreS32(other + T_A, 0x14);
                    StoreS32(other + T_B, 1);
                }
                if (other == 0u) {
                    UnknownGuestFn("overlay_title_80190288_null_kind6_cut", 0u);
                    return;
                }
            }
        } else {
            y = LoadS16(cursor + T_Y);
            if (y >= 0xB5) {
                pe_addr_t item = FindKind(7);
                StoreS16(cursor + T_Y, y - 1);
                if (item == 0u) {
                    UnknownGuestFn("overlay_title_80190310_null_kind7_cut", 0u);
                    return;
                }
                if (LoadS16(cursor + T_Y) < LoadS16(item + T_Y))
                    StoreS16(item + T_Y, LoadS16(cursor + T_Y));
                other = FindKind(4);
                if (other == 0u) {
                    UnknownGuestFn("overlay_title_8019039C_null_kind4_cut", 0u);
                    return;
                }
                StoreS32(other + T_A, -0x10);
                PE_StoreU32(GA_ATTRACT_COUNTER, 0u);
            }
        }

        /* 0x801903AC: no valid save -> the kind-6 task fades/retires and
         * the row-0xC8 item collapses to 0xA0 when the item sits at 0x8C. */
        if (func_8003FFCC() == 0) {
            other = FindKind(6);
            if (other == 0u) {
                UnknownGuestFn("overlay_title_801903F0_null_kind6_cut", 0u);
                return;
            }
            if (LoadS32(other + T_A) == 0x54)
                PE_StoreU32(other + T_UPDATE, FN_FADEOUT_16C);
            if (LoadS32(other + T_LEVEL) != 0) {
                pe_addr_t item = FindKind(7);
                StoreS32(other + T_B, -1);
                if (item == 0u) {
                    UnknownGuestFn("overlay_title_80190454_null_kind7_cut", 0u);
                    return;
                }
                if (LoadS16(item + T_Y) == 0x8C)
                    StoreS16(item + T_Y, 0xA0);
            }
        }

        /* 0x801904A0: confirm / up / down on the kind-7 item row. */
        node = FindKind(7);
        if (node == 0u) {
            UnknownGuestFn("overlay_title_801904D4_null_kind7_cut", 0u);
            return;
        }
        if ((pad & 0x20u) != 0u) {
            y = LoadS16(node + T_Y);
            if (y == 0xA0 || y == 0xB4 || y == 0xC8 || y == 0x8C) {
                PE_StoreU32(GA_ATTRACT_COUNTER, 0x3E9u);
                func_800525EC();
            }
        }
        if ((pad & 0x1000u) != 0u && (previous & 0x1000u) == 0u) {
            other = FindKind(6);
            if (other == 0u) {
                UnknownGuestFn("overlay_title_80190574_null_kind6_cut", 0u);
                return;
            }
            y = LoadS16(node + T_Y);
            if (LoadS32(other + T_LEVEL) == 0x100 ? (y >= 0x8D) : (y >= 0xA1)) {
                PE_StoreU32(GA_ATTRACT_COUNTER, 0u);
                StoreS16(node + T_Y, y - 0x14);
                func_8005267C();
            }
        }
        if ((pad & 0x4000u) != 0u && (previous & 0x4000u) == 0u) {
            other = FindKind(5);
            if (other == 0u) {
                UnknownGuestFn("overlay_title_80190610_null_kind5_cut", 0u);
                return;
            }
            y = LoadS16(node + T_Y);
            if (LoadS16(other + T_Y) - 0x14 >= y) {
                PE_StoreU32(GA_ATTRACT_COUNTER, 0u);
                StoreS16(node + T_Y, y + 0x14);
                func_8005267C();
            }
        }
    }
    PE_StoreU32(GA_PAD_PREVIOUS, pad);
}
