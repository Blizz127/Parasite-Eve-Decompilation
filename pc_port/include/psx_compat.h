#include "pe_port_compat.h"
/*
 * Phase 6C — PS1 SDK compatibility layer (expanded for main call graph).
 */
#ifndef PSX_COMPAT_H
#define PSX_COMPAT_H

#include "host_framebuffer.h"
#include "stub_registry.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Retail PS1 types ─────────────────────────────────────────────── */
typedef struct { int x, y, w, h; } RECT;
typedef struct { uint8_t disp[20]; } DISP_ENV;

/* ── PE globals ───────────────────────────────────────────────────── */
extern DISP_ENV  D_800BCE80;
extern uint8_t  *D_80011614;
extern int       D_8009CDDC;
extern uint32_t  D_8009D280;
extern uint32_t  D_8009D1C4;
extern uint32_t  D_800A7918;
extern uint32_t  D_800B0CD8;
extern signed char D_800B0DB2, D_800B0DB3, D_800B0DB4, D_800B0DB5, D_800B0DB6, D_800B0DB7;
extern int       D_800B0DD8;
extern unsigned char D_800B0DCD;
extern unsigned char D_800B0DC6;
extern unsigned short D_80093164[];

/* ── SDK IMPLEMENTED ──────────────────────────────────────────────── */
static inline void func_80073A44(int m)  { HostFB_VSync(m); }
static inline void func_80074D28(int m)  { HostFB_SetDispMask(m); }
static inline void func_80074DC0(int m)  { HostFB_DrawSync(m); }
static inline void func_80074F44(RECT *r, uint8_t rv, uint8_t g, uint8_t b) {
    if (r) HostFB_ClearImage(r->x, r->y, r->w, r->h, rv, g, b);
}
static inline void func_800755F0(void *e) { (void)e; HostFB_Present(); }
static inline void func_800752AC(void *o, int n) { Stub_Record("ClearOTagR","BOOTSTRAP_RET"); (void)o;(void)n; }

/* ── BOOTSTRAP_RET — func_8001220C callees ────────────────────────── */
static inline void func_800725DC(void)   { Stub_Record("func_800725DC","BOOTSTRAP_RET"); }
static inline void func_8006A9E4(void)   { Stub_Record("func_8006A9E4","BOOTSTRAP_RET"); }
static inline void func_8006AD40(void)   { Stub_Record("func_8006AD40","BOOTSTRAP_RET"); }
static inline void func_8006ECEC(void)   { Stub_Record("func_8006ECEC","BOOTSTRAP_RET"); }
static inline void func_8006F044(void)   { Stub_Record("func_8006F044","BOOTSTRAP_RET"); }
static inline void func_80069B08(int d)  { Stub_Record("func_80069B08","BOOTSTRAP_RET"); (void)d; }
static inline void func_8003F3C4(void)   { Stub_Record("func_8003F3C4","BOOTSTRAP_RET"); }
static inline void func_801235DC(void)   { Stub_Record("func_801235DC","BOOTSTRAP_RET"); }
static inline void func_8019234C(void)   { Stub_Record("func_8019234C","BOOTSTRAP_RET"); }
static inline int  func_801909B4(void)   { Stub_Record("func_801909B4","BOOTSTRAP_RET"); return 0; }
static inline void func_8005E588(void)   { Stub_Record("func_8005E588","BOOTSTRAP_RET"); }
static inline void func_80066B60(int a)  { Stub_Record("func_80066B60","BOOTSTRAP_RET"); (void)a; }
static inline void func_80068E24(void)   { Stub_Record("func_80068E24","BOOTSTRAP_RET"); }
static inline void func_80070E54(void)   { Stub_Record("func_80070E54","BOOTSTRAP_RET"); }
static inline int  func_80038D1C(void)   { Stub_Record("func_80038D1C","BOOTSTRAP_RET"); return 0; }

/* ── REAL translated functions (in bootstrap/*_port.c) ────────────── */
extern void func_8001220C(void);
extern int  func_800698D4(void);
extern int  func_8006E834(void);
extern int  func_8006E9A0(int);

#endif
