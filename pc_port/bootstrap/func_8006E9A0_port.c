/*
 * Phase 6A — Native adaptation of func_8006E9A0 (clear-frame + arena init).
 *
 * PE_PORT adaptation of matched C at src/func_8006E9A0.c.
 * MIPS register pins and empty asm barriers removed.
 * All PS1 SDK calls routed through psx_compat.h stubs.
 * Arena globals allocated as host variables.
 *
 * Classification: HOST_ADAPTED
 */

#include "psx_compat.h"
#include <string.h>

/* ── Arena globals (PS1 memory → host static allocation) ──────────── */

static unsigned char D_800F34F8_buf[64 * 1024];  /* proxy for 0x800F34F8 region */
static unsigned char D_8010BD00_buf[64 * 1024];
static unsigned char D_80120D08_buf[64 * 1024];
static unsigned char D_801ED800_buf[32 * 1024];

#define D_800F34F8  (D_800F34F8_buf)
#define D_8010BD00  (D_8010BD00_buf)
#define D_80120D08  (D_80120D08_buf)
#define D_801ED800  (D_801ED800_buf)

static unsigned char *D_800B0E24;
static unsigned char *D_800B0E28;
static unsigned char *D_800B0E2C;
static unsigned char *D_800B0E30;
static unsigned char *D_800B0E34;
static unsigned char *D_800B0E38;
static unsigned char *D_800B0E3C;
static unsigned char *D_800B0E40;
static unsigned char *D_800B0E44;
static unsigned char *D_800B0E48;
static unsigned char *D_800B0E4C;
static unsigned char *D_800B0E50;
static unsigned char *D_800B0E54;
static unsigned char *D_800B0E58;
static unsigned char *D_800B0E5C;
static unsigned char *D_800B0E60;
static unsigned char *D_800B0E64;
static unsigned char *D_800B0E68;
static unsigned char *D_800B0E6C;

static unsigned char  D_800BCE80_buf[20];  /* DISP_ENV proxy */
#undef  D_800BCE80
#define D_800BCE80  D_800BCE80_buf

static unsigned char  D_800BCFEE;           /* poll loop condition byte */
// D_800B0DC6 now in pe_globals.c;           /* post-loop store */
static unsigned char *arena_lookup[2];      /* for ClearOTagR lookup */

int func_8006E9A0(int arg)
{
    int saved_arg  = arg;
    int one        = 1;
    RECT rect;

    /* Plain locals (were MIPS register-pinned) */
    unsigned char *cursor;
    unsigned char *next;
    unsigned int   step_8000;
    unsigned int   step_48000;
    unsigned char **lookup;

    /* Init arena proxy for D_80011614 */
    memset(D_800F34F8_buf, 0, sizeof(D_800F34F8_buf));
    D_80011614 = (uint8_t *)arena_lookup;

    /* 1. Display init */
    func_80073A44(0);                           /* VSync(0) */
    func_80074D28(0);                           /* SetDispMask(0) */
    func_800755F0(D_800BCE80);                  /* PutDispEnv */

    rect.x = 0;                                 /* x */
    rect.y = 0;                                 /* y */
    rect.w = 0x140;                             /* w */
    rect.h = 0x1C0;                             /* h */
    func_80074F44(&rect, 0, 0, 1);              /* ClearImage */

    func_80074DC0(0);                           /* DrawSync(0) */

    /* 2. Pointer arena — retail store/compute interleave */
    step_48000 = 0x48000;
    lookup = &D_800B0E38;

    cursor = D_800F34F8;
    next = cursor + 0x1800;
    D_800B0E24 = cursor;
    cursor += 0x6000;
    D_800B0E28 = next;
    next = (unsigned char *)0xE000;
    D_800B0E2C = cursor;
    cursor += (unsigned int)(uintptr_t)next;
    D_800B0E30 = cursor;

    cursor = D_8010BD00;
    next = D_80120D08;
    D_800B0E40 = cursor;
    cursor = next + 0x1C98;
    D_800B0E34 = next;
    next += 0x5C98;
    step_8000 = 0x8000;
    D_800B0E38 = cursor;
    cursor += step_8000;
    D_800B0E3C = next;
    next = cursor + 0x2400;
    D_800B0E44 = cursor;
    cursor += 0x4800;
    D_800B0E4C = cursor;
    cursor += step_48000;
    D_800B0E48 = next;
    next = cursor + 0x4000;
    D_800B0E50 = cursor;
    cursor += step_8000;
    D_800B0E54 = next;
    next = cursor + 0x3800;
    D_800B0E5C = next;
    next = D_80011614;
    D_800B0E58 = cursor;
    cursor += 0x7000;
    D_800B0E60 = cursor;

    cursor = D_801ED800;
    D_800B0E6C = cursor;
    cursor = next - 8;
    D_800B0E64 = cursor;
    D_800B0E68 = next;

    /* 3. Post-arena calls */
    func_8005E588();
    func_80066B60(2);

    /* 4. Poll loop — bootstrap: exit immediately (real mode polls CD) */
    {
        int poll_count = 0;
        do {
            func_800752AC(lookup[D_8009CDDC], 0x1000);  /* ClearOTagR */
            func_80068E24();
            func_80070E54();
        } while (++poll_count < 1);
    }

    /* 5. Post-loop */
    D_800B0DC6 = 0;
    func_80038D1C();

    /* 6. Dispatch exit */
    if (saved_arg == 1) {
        D_8009D280 = 0xA80830C8;
    } else if (saved_arg == 3) {
        D_8009D280 = 0xA80651C8;
    }
    return 0;
}
