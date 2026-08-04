/*
 * Phase 6D-S — PS1 SDK compatibility layer.
 *
 * All BOOTSTRAP_RET stubs now go through the centralized Bootstrap_ReturnInt /
 * Bootstrap_ReturnVoid policy, so strict mode is enforced uniformly.
 */
#ifndef PSX_COMPAT_H
#define PSX_COMPAT_H

#include "host_framebuffer.h"
#include "stub_registry.h"
#include "pe_guest_ram.h"
#include "pe_callback.h"
#include "pe_bootstrap.h"
#include "pe_port_compat.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Retail PS1 types ─────────────────────────────────────────────── */
typedef struct { int x, y, w, h; } RECT;
typedef struct { uint8_t disp[20]; } DISP_ENV;

/* ── Guest-RAM-resident named globals ─────────────────────────────────
 *
 * These symbols name bytes inside the contiguous 2 MiB guest RAM.
 * Each macro is a typed lvalue over PE_Translate, so reads AND writes
 * (e.g. `D_800B0CDC = 10;`) land in guest RAM with bounds checking.
 * Host endianness is little-endian on all supported build targets,
 * matching the PS1.  Address-of (`&D_800B0CD8`) yields a host pointer
 * into guest RAM; arithmetic from it stays inside the one allocation. */
#define PE_GUEST_U8(a)   (*(uint8_t  *)PE_Translate((a), 1))
#define PE_GUEST_S8(a)   (*(int8_t   *)PE_Translate((a), 1))
#define PE_GUEST_U16(a)  (*(uint16_t *)PE_Translate((a), 2))
#define PE_GUEST_S16(a)  (*(int16_t  *)PE_Translate((a), 2))
#define PE_GUEST_U32(a)  (*(uint32_t *)PE_Translate((a), 4))
#define PE_GUEST_S32(a)  (*(int32_t  *)PE_Translate((a), 4))

#define D_800BCE80     ((DISP_ENV *)PE_Translate(0x800BCE80u, sizeof(DISP_ENV)))

#define D_800B0CD8     PE_GUEST_U32(0x800B0CD8u)
#define D_800B0CDC     PE_GUEST_U16(0x800B0CDCu)
#define D_800B0CDE     PE_GUEST_S16(0x800B0CDEu)
#define D_800B0CE0     PE_GUEST_S8(0x800B0CE0u)
#define D_800B0CE1     PE_GUEST_S8(0x800B0CE1u)
#define D_800B0CE2     PE_GUEST_S8(0x800B0CE2u)
#define D_800B0CE3     PE_GUEST_S8(0x800B0CE3u)
#define D_800B0CE4     PE_GUEST_S8(0x800B0CE4u)
#define D_800B0CE5     PE_GUEST_S8(0x800B0CE5u)
#define D_800B0CE6     PE_GUEST_S8(0x800B0CE6u)
#define D_800B0CE7     PE_GUEST_S8(0x800B0CE7u)
#define D_800B0CE8     PE_GUEST_S8(0x800B0CE8u)
#define D_800B0CE9     PE_GUEST_S8(0x800B0CE9u)
#define D_800B0CEA     PE_GUEST_S8(0x800B0CEAu)
#define D_800B0CEB     PE_GUEST_S8(0x800B0CEBu)
#define D_800B0DB2     PE_GUEST_S8(0x800B0DB2u)
#define D_800B0DB3     PE_GUEST_S8(0x800B0DB3u)
#define D_800B0DB4     PE_GUEST_S8(0x800B0DB4u)
#define D_800B0DB5     PE_GUEST_S8(0x800B0DB5u)
#define D_800B0DB6     PE_GUEST_S8(0x800B0DB6u)
#define D_800B0DB7     PE_GUEST_S8(0x800B0DB7u)
#define D_800B0DC6     PE_GUEST_U8(0x800B0DC6u)
#define D_800B0DCD     PE_GUEST_U8(0x800B0DCDu)
#define D_800B0DD4     PE_GUEST_U16(0x800B0DD4u)
#define D_800B0DD8     PE_GUEST_S32(0x800B0DD8u)
#define D_80094488     PE_GUEST_U8(0x80094488u)
#define D_8009448C     ((uint8_t *)PE_Translate(0x8009448Cu, 64))

/* D_80011614 — retail guest pointer (global at 0x80011614 holding a guest
 * address).  Stored host-side as pe_addr_t; translated only at real access
 * sites.  No translated retail writer exists yet; the initial value is
 * bootstrap policy (see pe_globals.c). */
extern pe_addr_t D_80011614;

/* ── Host-owned scalar globals (plain data, no pointer arithmetic) ──── */
extern int       D_8009CDDC;
extern uint32_t  D_8009D280;
extern uint32_t  D_8009D1C4;
extern uint32_t  D_800A7918;
extern unsigned short D_80093164[];

/* ── SDK IMPLEMENTED ──────────────────────────────────────────────── */
static inline void func_80073A44(int m)  { HostFB_VSync(m); }
static inline void func_80074D28(int m)  { HostFB_SetDispMask(m); }
static inline void func_80074DC0(int m)  { HostFB_DrawSync(m); }
static inline void func_80074F44(RECT *r, uint8_t rv, uint8_t g, uint8_t b) {
    if (r) HostFB_ClearImage(r->x, r->y, r->w, r->h, rv, g, b);
}
static inline void func_800755F0(void *e) { (void)e; HostFB_Present(); }
static inline void func_800752AC(void *o, int n) {
    Bootstrap_ReturnVoid("ClearOTagR", "func_8006E9A0");
    (void)o; (void)n;
}

/* ── BOOTSTRAP_RET — func_8001220C callees ────────────────────────── */
static inline void func_8006AD40(void)   { Bootstrap_ReturnVoid("func_8006AD40", "func_8001220C"); }
static inline void func_8006ECEC(void)   { Bootstrap_ReturnVoid("func_8006ECEC", "func_8001220C"); }
static inline void func_8006F044(void)   { Bootstrap_ReturnVoid("func_8006F044", "func_8001220C"); }
static inline void func_80069B08(int d)  { Bootstrap_ReturnVoid("func_80069B08", "func_8001220C"); (void)d; }
static inline void func_8003F3C4(void)   { Bootstrap_ReturnVoid("func_8003F3C4", "func_8001220C"); }
static inline void func_801235DC(void)   { Bootstrap_ReturnVoid("func_801235DC", "func_8001220C"); }
static inline void func_8019234C(void)   { Bootstrap_ReturnVoid("func_8019234C", "func_8001220C"); }
static inline int  func_801909B4(void)   { return Bootstrap_ReturnInt("func_801909B4", "func_8001220C", 0); }
static inline void func_80066B60(int a)  { Bootstrap_ReturnVoid("func_80066B60", "func_8006E9A0"); (void)a; }
static inline void func_80068E24(void)   { Bootstrap_ReturnVoid("func_80068E24", "func_8006E9A0"); }
static inline void func_80070E54(void)   { Bootstrap_ReturnVoid("func_80070E54", "func_8006E9A0"); }

/* func_80038D1C is now a REAL translation too (Phase 6E-B15):
 * game/boot/func_80038D1C_port.c — D_80091A20 byte test-and-clear
 * status leaf (11 retail words; also a matched decomp C leaf).
 * func_8006A9E4 is REAL too (Phase 6E-B16):
 * game/boot/func_8006A9E4_port.c — PE.IMG streaming resource load
 * (215 retail words); unresolved callee func_80087090 routes through
 * the centralized bootstrap boundary.
 * func_800527C8 is REAL too (Phase 6E-B17):
 * game/boot/func_800527C8_port.c — multi-subsystem bootstrap
 * dispatcher (49 retail words, 17 calls); unresolved callees route
 * through the centralized bootstrap boundary in retail order.
 * func_800528F0 is REAL too (Phase 6E-B18):
 * game/boot/func_800528F0_port.c — PRNG table generator
 * (143 retail words, 521-byte output table at D_800A1B90).
 * func_8005BCBC is REAL too (Phase 6E-B24):
 * game/boot/func_8005BCBC_port.c — resource-state pointer/count
 * selector (21 retail words; guest-resident D_8009D0C0/C4/C8). */

/* ── REAL translated functions ────────────────────────────────────── */
extern void func_800725DC(void);
extern void func_8001220C(void);
extern int  func_80038D1C(void);
extern int  func_800698D4(void);
extern void func_8006A9E4(void);
extern void func_800527C8(void);
extern void func_800528F0(void);
extern void func_8005E588(void);
extern void func_80062568(void);
extern void func_80064964(void);
extern void func_8005DE88(void);
extern void func_80052C6C(void);
extern int  func_8005BCBC(pe_addr_t a0);
extern pe_addr_t func_80071A24(pe_addr_t dst, uint32_t len);
extern void func_8005E968(uint32_t);
extern void func_8005F844(int);
extern int  func_8006E834(void);
extern int  func_8006E9A0(int);

#endif
