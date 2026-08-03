/* Phase 6D-S — PE_PORT compatibility layer.
 * All BOOTSTRAP_RET stubs now use centralized Bootstrap_* policy.
 * No (int)(uintptr_t) casts remain on the first-clear path. */
#ifndef PE_PORT_COMPAT_H
#define PE_PORT_COMPAT_H
#include "psx_compat.h"

/* ── Boot Rung globals (guest-address backed) ─────────────────────── */
/* D_800B0CD8..D_800B0CEB, D_800B0DD4, D_80094488/D_8009448C are guest-RAM
 * lvalue macros defined in psx_compat.h — no externs here. */
extern unsigned int D_8009D1A0, D_8009D250;

/* ── Arena pointers — now pe_addr_t guest addresses ───────────────── */
extern pe_addr_t D_800B0E24,D_800B0E28,D_800B0E2C,D_800B0E30,D_800B0E34,D_800B0E38,D_800B0E3C,D_800B0E40,D_800B0E44,D_800B0E48,D_800B0E4C,D_800B0E50,D_800B0E54,D_800B0E58,D_800B0E5C,D_800B0E60,D_800B0E64,D_800B0E68,D_800B0E6C;

/* ── func_8003E610 callees ─────────────────────────────────────────── */
/* All ten callees are now REAL implementations (Phase 6E-A batch 1):
 * func_80073C94/func_80072714/func_80072724  — pe_libetc.c
 * func_80074924/749D8/74A44/74BB8            — pe_libgpu.c
 * func_8007D054                              — pe_libsnd.c
 * func_80077F7C/func_80079004/func_80079024  — pe_gte.c
 * func_800409B4                              — pe_libcard.c
 * func_800844E4/func_80082534                — pe_save.c
 * func_8007EC14/func_8007ED58/func_8007F72C/func_8007F778/
 * func_8007FBF0/func_80080CC8/func_8007F7A8  — pe_libcd.c
 * func_8003E754/func_8003E944                — game/boot/*.c          */
#include "pe_sdk.h"

/* ── func_8006A5BC callees ─────────────────────────────────────────── */
/* func_8007ED58/func_8007F7A8 are real (pe_libcd.c).  The streaming
 * providers remain bootstrap stubs until Phase 6E-A batch 2. */
static inline void func_80085644(void)    { Bootstrap_ReturnVoid("func_80085644", "func_8006A5BC"); }
static inline void func_80087024(void)    { Bootstrap_ReturnVoid("func_80087024", "func_8006A5BC"); }
static inline void func_8008682C(int a)   { Bootstrap_ReturnVoid("func_8008682C", "func_8006A5BC"); (void)a; }

/* ── func_8003E680 callees ─────────────────────────────────────────── */
static inline void func_80070D10(void)    { Bootstrap_ReturnVoid("func_80070D10", "func_8003E680"); }
static inline void func_80070D6C(void)    { Bootstrap_ReturnVoid("func_80070D6C", "func_8003E680"); }
static inline void func_8003E974(void)    { Bootstrap_ReturnVoid("func_8003E974", "func_8003E680"); }
static inline void func_80036DC8(void)    { Bootstrap_ReturnVoid("func_80036DC8", "func_8003E680"); }
static inline void func_8003E91C(void)    { Bootstrap_ReturnVoid("func_8003E91C", "func_8003E680"); }

/* func_80073D24 — callback registration.  Uses the host-safe callback
 * registry (PE_Callback_*) instead of a narrowed (int) cast. */
static inline void func_80073D24(int a)   {
    Bootstrap_ReturnVoid("func_80073D24", "func_8003E680");
    if (a == 0) {
        PE_Callback_Reset();
    }
    /* Non-zero: a host function pointer cannot travel through the narrowed
     * PS1 int argument, so the translated caller registers via
     * PE_Callback_Register directly (see func_8003E680_port.c). */
}
static inline void func_800371A4(int a)   { Bootstrap_ReturnVoid("func_800371A4", "func_8003E680"); (void)a; }
static inline void func_80029388(void)    { Bootstrap_ReturnVoid("func_80029388", "func_8003E680"); }
static inline void func_8005BCA8(void)    { Bootstrap_ReturnVoid("func_8005BCA8", "func_8003E680"); }
static inline void func_80068D28(void)    { Bootstrap_ReturnVoid("func_80068D28", "func_8003E680"); }
static inline void func_800124F8(void)    { Bootstrap_ReturnVoid("func_800124F8", "func_8003E680"); }
static inline void func_8001A890(void)    { Bootstrap_ReturnVoid("func_8001A890", "func_8003E680"); }
static inline void func_80034F10(void)    { Bootstrap_ReturnVoid("func_80034F10", "func_8003E680"); }
static inline void func_8006536C(void)    { Bootstrap_ReturnVoid("func_8006536C", "func_8003E680"); }

/* ── REAL translated Boot Rung functions ────────────────────────────── */
extern void func_8003E610(void);
extern void func_8003E680(void);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A674(void);
extern void func_8006A8D4(void);

#endif
