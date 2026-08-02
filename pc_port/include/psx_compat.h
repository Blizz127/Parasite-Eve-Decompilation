/*
 * Phase 6A — PS1 SDK compatibility layer.
 *
 * Maps PS1 types, globals, and SDK calls to native host equivalents.
 * Every replacement is explicitly classified.
 *
 * NOT an emulator.  No PS1 CPU state, no GTE, no MDEC, no SPU.
 */

#ifndef PSX_COMPAT_H
#define PSX_COMPAT_H

#include "host_framebuffer.h"
#include "stub_registry.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* ── Retail PS1 types ────────────────────────────────────────────────── */

typedef struct { int x, y, w, h; } RECT;

/* DISP_ENV as used by PE boot (SetDefDispEnv / PutDispEnv) */
typedef struct {
    uint8_t disp[20];   /* opaque; PE boot uses a 20-byte blob */
} DISP_ENV;

/* ── Retail PE globals (allocated as host globals) ──────────────────── */

/* func_8006E9A0 references these */
extern DISP_ENV  D_800BCE80;        /* DISP_ENV storage for PutDispEnv */
extern uint8_t  *D_80011614;        /* arena pointer (typed by func_8006A8D4) */
extern int       D_8009CDDC;        /* typed by func_8003E680.c */
extern uint32_t  D_8009D280;        /* typed by func_8003E680.c */

/* func_8001220C (main) references */
extern uint32_t  D_800B0CD8;
extern uint32_t  D_8009D1C4;
extern uint32_t  D_800A7918;

/* ── SDK function replacements ──────────────────────────────────────── */

/* VSync — IMPLEMENTED: just count */
static inline void func_80073A44(int mode) {
    HostFB_VSync(mode);
}

/* SetDispMask — IMPLEMENTED */
static inline void func_80074D28(int mask) {
    HostFB_SetDispMask(mask);
}

/* PutDispEnv — HOST_ADAPTED: store the env, present framebuffer */
static inline void func_800755F0(void *env) {
    (void)env;
    HostFB_Present();
}

/* ClearImage — IMPLEMENTED: fills host framebuffer */
static inline void func_80074F44(RECT *rect, uint8_t r, uint8_t g, uint8_t b) {
    if (rect)
        HostFB_ClearImage(rect->x, rect->y, rect->w, rect->h, r, g, b);
}

/* DrawSync — IMPLEMENTED */
static inline void func_80074DC0(int mode) {
    HostFB_DrawSync(mode);
}

/* ClearOTagR — BOOTSTRAP_RET: ordering table is not relevant for headless */
static inline void func_800752AC(void *otag, int n) {
    Stub_Record("ClearOTagR", "BOOTSTRAP_RET");
    (void)otag; (void)n;
}

/* ── Bootstrap stubs for functions called by the boot slice ─────────── */

/* func_8005E588 — BOOTSTRAP_RET */
static inline void func_8005E588(void) {
    Stub_Record("func_8005E588", "BOOTSTRAP_RET");
}

/* func_80066B60 — BOOTSTRAP_RET */
static inline void func_80066B60(int a) {
    Stub_Record("func_80066B60", "BOOTSTRAP_RET");
    (void)a;
}

/* func_80068E24 — BOOTSTRAP_RET */
static inline void func_80068E24(void) {
    Stub_Record("func_80068E24", "BOOTSTRAP_RET");
}

/* func_80070E54 — BOOTSTRAP_RET */
static inline void func_80070E54(void) {
    Stub_Record("func_80070E54", "BOOTSTRAP_RET");
}

/* func_80038D1C — BOOTSTRAP_RET (matched C exists, stub for now) */
static inline int func_80038D1C(void) {
    Stub_Record("func_80038D1C", "BOOTSTRAP_RET");
    return 0;
}

/* ── Bootstrap stubs for main (func_8001220C) callees ───────────────── */

static inline void func_800725DC(void) {
    Stub_Record("func_800725DC", "BOOTSTRAP_RET");
}
static inline void func_8003E610(void) {
    Stub_Record("func_8003E610", "BOOTSTRAP_RET");
}
static inline void func_8003E680(void) {
    Stub_Record("func_8003E680", "BOOTSTRAP_RET");
}
static inline void func_8006A5BC(void) {
    Stub_Record("func_8006A5BC", "BOOTSTRAP_RET");
}
static inline void func_8006A64C(void) {
    Stub_Record("func_8006A64C", "BOOTSTRAP_RET");
}
static inline void func_8006A9E4(void) {
    Stub_Record("func_8006A9E4", "BOOTSTRAP_RET");
}
static inline void func_8006AD40(void) {
    Stub_Record("func_8006AD40", "BOOTSTRAP_RET");
}
static inline void func_8006ECEC(void) {
    Stub_Record("func_8006ECEC", "BOOTSTRAP_RET");
}
static inline void func_8006F044(void) {
    Stub_Record("func_8006F044", "BOOTSTRAP_RET");
}
static inline void func_80069B08(int a) {
    Stub_Record("func_80069B08", "BOOTSTRAP_RET");
    (void)a;
}
static inline void func_8003F3C4(void) {
    Stub_Record("func_8003F3C4", "BOOTSTRAP_RET");
}
static inline void func_801235DC(void) {
    Stub_Record("func_801235DC", "BOOTSTRAP_RET");
}
static inline void func_8019234C(void) {
    Stub_Record("func_8019234C", "BOOTSTRAP_RET");
}
static inline int  func_801909B4(void) {
    Stub_Record("func_801909B4", "BOOTSTRAP_RET");
    return 0;
}
static inline int  func_800698D4(void) {
    Stub_Record("func_800698D4", "BOOTSTRAP_RET");
    return 1;  /* return non-zero to exit main's while loop */
}

/* func_8006E834 — BOOTSTRAP_RET (called before func_8006E9A0) */
static inline void func_8006E834(void) {
    Stub_Record("func_8006E834", "BOOTSTRAP_RET");
}

#endif /* PSX_COMPAT_H */
