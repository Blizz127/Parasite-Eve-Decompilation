/* Phase 6D — PE_PORT compatibility layer for Boot Rung 1 functions.
 * No register pins, no asm barriers. All callees explicitly registered. */
#ifndef PE_PORT_COMPAT_H
#define PE_PORT_COMPAT_H
#include "psx_compat.h"

/* ── Boot Rung globals ─────────────────────────────────────────────── */
extern unsigned short D_800B0DD4, D_800B0CDC;
extern signed short D_800B0CDE;
extern signed char D_800B0CE0,D_800B0CE1,D_800B0CE2,D_800B0CE3,D_800B0CE4,D_800B0CE5,D_800B0CE6,D_800B0CE7,D_800B0CE8,D_800B0CE9,D_800B0CEA,D_800B0CEB;
extern unsigned int D_8009D1A0, D_8009D250;

/* ── Arena buffers (func_8006A8D4, func_8006E9A0) ──────────────────── */
extern unsigned char D_800F34F8[0x8000];
extern unsigned char D_8010BD00[0x8000];
extern unsigned char D_80120D08[0x8000];
extern unsigned char D_801ED800[0x8000];
extern unsigned char *D_800B0E24,*D_800B0E28,*D_800B0E2C,*D_800B0E30,*D_800B0E34,*D_800B0E38,*D_800B0E3C,*D_800B0E40,*D_800B0E44,*D_800B0E48,*D_800B0E4C,*D_800B0E50,*D_800B0E54,*D_800B0E58,*D_800B0E5C,*D_800B0E60,*D_800B0E64,*D_800B0E68,*D_800B0E6C;
extern unsigned char D_80094488;
extern unsigned char D_8009448C[64];

/* ── func_8003E610 callees ─────────────────────────────────────────── */
static inline void func_80073C94(void) { Stub_Record("func_80073C94","BOOTSTRAP_RET"); }
static inline void func_8003E754(int w,int h) { Stub_Record("func_8003E754","BOOTSTRAP_RET"); (void)w;(void)h; }
static inline void func_8007D054(void) { Stub_Record("func_8007D054","BOOTSTRAP_RET"); }
static inline void func_80077F7C(void) { Stub_Record("func_80077F7C","BOOTSTRAP_RET"); }
static inline void func_80079004(int a,int b) { Stub_Record("func_80079004","BOOTSTRAP_RET"); (void)a;(void)b; }
static inline void func_80079024(int a) { Stub_Record("func_80079024","BOOTSTRAP_RET"); (void)a; }
static inline void func_800409B4(void) { Stub_Record("func_800409B4","BOOTSTRAP_RET"); }
static inline void func_8003E944(void) { Stub_Record("func_8003E944","BOOTSTRAP_RET"); }
static inline void func_8007EC14(void) { Stub_Record("func_8007EC14","BOOTSTRAP_RET"); }
static inline void func_80080CC8(int a) { Stub_Record("func_80080CC8","BOOTSTRAP_RET"); (void)a; }

/* ── func_8006A5BC callees ─────────────────────────────────────────── */
static inline void func_80085644(void) { Stub_Record("func_80085644","BOOTSTRAP_RET"); }
static inline void func_80087024(void) { Stub_Record("func_80087024","BOOTSTRAP_RET"); }
static inline void func_8008682C(int a) { Stub_Record("func_8008682C","BOOTSTRAP_RET"); (void)a; }
static inline int  func_8007ED58(void) { Stub_Record("func_8007ED58","BOOTSTRAP_RET"); return 1; }
/* func_8007F72C is already in func_800698D4_port.c */
static inline int  func_8007F7A8(void) { Stub_Record("func_8007F7A8","BOOTSTRAP_RET"); return 0; }

/* ── func_8003E680 callees ─────────────────────────────────────────── */
static inline void func_80070D10(void) { Stub_Record("func_80070D10","BOOTSTRAP_RET"); }
static inline void func_80070D6C(void) { Stub_Record("func_80070D6C","BOOTSTRAP_RET"); }
static inline void func_8003E974(void) { Stub_Record("func_8003E974","BOOTSTRAP_RET"); }
static inline void func_80036DC8(void) { Stub_Record("func_80036DC8","BOOTSTRAP_RET"); }
static inline void func_8003E91C(void) { Stub_Record("func_8003E91C","BOOTSTRAP_RET"); }
static inline void func_80073D24(int a) { Stub_Record("func_80073D24","BOOTSTRAP_RET"); (void)a; }
static inline void func_800371A4(int a) { Stub_Record("func_800371A4","BOOTSTRAP_RET"); (void)a; }
static inline void func_80029388(void) { Stub_Record("func_80029388","BOOTSTRAP_RET"); }
static inline void func_8005BCA8(void) { Stub_Record("func_8005BCA8","BOOTSTRAP_RET"); }
static inline void func_80068D28(void) { Stub_Record("func_80068D28","BOOTSTRAP_RET"); }
static inline void func_800124F8(void) { Stub_Record("func_800124F8","BOOTSTRAP_RET"); }
static inline void func_8001A890(void) { Stub_Record("func_8001A890","BOOTSTRAP_RET"); }
static inline void func_80034F10(void) { Stub_Record("func_80034F10","BOOTSTRAP_RET"); }
static inline void func_8006536C(void) { Stub_Record("func_8006536C","BOOTSTRAP_RET"); }

/* ── REAL translated Boot Rung functions ────────────────────────────── */
extern void func_8003E610(void);
extern void func_8003E680(void);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A674(void);
extern void func_8006A8D4(void);

#endif
