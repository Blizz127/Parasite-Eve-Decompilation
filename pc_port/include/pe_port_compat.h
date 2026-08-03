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

/* ── Arena pointers — guest-RAM lvalue macros (Phase 6E-B16) ────────
 * The 19-slot retail arena pointer table lives INSIDE the D_800B0CD8
 * struct region in guest RAM (0x800B0E24..0x800B0E6C); retail func_8006A8D4
 * stores to it with sw and retail readers (func_8006A9E4: lw 0x130(s3),
 * lw 0x194(s3)) load from guest RAM.  6D-S modeled these as host scalar
 * globals, splitting them from the guest struct — func_8006A9E4 then read
 * zeros.  They are now guest-RAM lvalue macros like D_800B0CD8. */
#define D_800B0E24  PE_GUEST_U32(0x800B0E24u)
#define D_800B0E28  PE_GUEST_U32(0x800B0E28u)
#define D_800B0E2C  PE_GUEST_U32(0x800B0E2Cu)
#define D_800B0E30  PE_GUEST_U32(0x800B0E30u)
#define D_800B0E34  PE_GUEST_U32(0x800B0E34u)
#define D_800B0E38  PE_GUEST_U32(0x800B0E38u)
#define D_800B0E3C  PE_GUEST_U32(0x800B0E3Cu)
#define D_800B0E40  PE_GUEST_U32(0x800B0E40u)
#define D_800B0E44  PE_GUEST_U32(0x800B0E44u)
#define D_800B0E48  PE_GUEST_U32(0x800B0E48u)
#define D_800B0E4C  PE_GUEST_U32(0x800B0E4Cu)
#define D_800B0E50  PE_GUEST_U32(0x800B0E50u)
#define D_800B0E54  PE_GUEST_U32(0x800B0E54u)
#define D_800B0E58  PE_GUEST_U32(0x800B0E58u)
#define D_800B0E5C  PE_GUEST_U32(0x800B0E5Cu)
#define D_800B0E60  PE_GUEST_U32(0x800B0E60u)
#define D_800B0E64  PE_GUEST_U32(0x800B0E64u)
#define D_800B0E68  PE_GUEST_U32(0x800B0E68u)
#define D_800B0E6C  PE_GUEST_U32(0x800B0E6Cu)

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
/* All eight callees are now real (Phase 6E-A batches 1-2):
 * func_80085644/func_80086FF8/func_80087024/func_8008682C — pe_stream.c
 * func_8007ED58/func_8007F72C/func_8007F7A8                     — pe_libcd.c */

/* ── func_8003E680 callees ─────────────────────────────────────────── */
/* func_80070D10/func_80070D6C are now REAL translations (Phase 6E-B1/B2):
 * game/boot/func_80070D10_port.c — lagged-Fibonacci RNG table init
 * game/boot/func_80070D6C_port.c — RNG advance (verbatim |= wrap)
 * game/boot/func_80070DD0_port.c — handwritten ranged-random wrapper
 * func_8003E974 is now a REAL translation (Phase 6E-B3):
 * game/boot/func_8003E974_port.c — bit-table init + 20 registrations;
 * func_8003EAC8 is now a REAL translation too (Phase 6E-B4):
 * game/boot/func_8003EAC8_port.c — GTE LZCS/LZCR-indexed table writer.
 * func_80036DC8 is now a REAL translation too (Phase 6E-B5):
 * game/boot/func_80036DC8_port.c — timer-record init dispatcher. */
static inline void func_8003E91C(void)    { Bootstrap_ReturnVoid("func_8003E91C", "func_8003E680"); }

/* func_80073D24 is now a REAL SDK implementation (Phase 6E-B6):
 * pe_libetc.c — VBlank callback slot-4 setter with previous-handler
 * return, guest-table backed (pe_callback.h).
 * func_800371A4 is now a REAL translation (Phase 6E-B7):
 * game/boot/func_800371A4_port.c — 3-word $gp-relative byte setter.
 * func_80029388 is now a REAL translation too (Phase 6E-B8):
 * game/boot/func_80029388_port.c — slot-table clear + default-record
 * init (with leaves func_8002F658 and func_80020EFC).
 * func_8005BCA8 is now a REAL translation too (Phase 6E-B9):
 * game/boot/func_8005BCA8_port.c — empty jr/nop stub (2 retail words).
 * func_80068D28 is now a REAL translation too (Phase 6E-B10):
 * game/boot/func_80068D28_port.c — double-buffered display-record
 * data initializer (63 retail words).
 * func_800124F8 is now a REAL translation too (Phase 6E-B11):
 * game/boot/func_800124F8_port.c — subsystem table/array/scalar clear
 * (31 retail words).
 * func_8001A890 is now a REAL translation too (Phase 6E-B12):
 * game/boot/func_8001A890_port.c — subsystem scalar/array clear
 * (34 retail words).
 * func_80034F10 is now a REAL translation too (Phase 6E-B13):
 * game/boot/func_80034F10_port.c — subsystem table clear +
 * D_800B0CD8 flag-bit clear (45 retail words).
 * func_8006536C is now a REAL translation too (Phase 6E-B14):
 * game/boot/func_8006536C_port.c — 28x3-word record-table clear +
 * index byte clear (19 retail words). */

/* ── REAL translated Boot Rung functions ────────────────────────────── */
extern void func_8003E610(void);
extern void func_8003E680(void);
extern void func_8003E974(void);
extern void func_8003EAC8(int, int);
extern void func_80036DC8(void);
extern void func_80036DF8(void);
extern void func_80036E34(void);
extern void func_80036E58(void);
extern void func_800371A4(int);
extern void func_80029388(void);
extern void func_8002F658(void);
extern void func_80020EFC(void);
extern void func_8005BCA8(void);
extern void func_80068D28(void);
extern void func_800124F8(void);
extern void func_8001A890(void);
extern void func_80034F10(void);
extern void func_8006536C(void);
extern void func_80070D10(void);
extern unsigned int func_80070D6C(void);
extern int func_80070DD0(int, int);

/* func_8003EAC8 deterministic argument recording (test instrumentation) */
void PE_3EAC8_RecordReset(void);
void PE_3EAC8_RecordSetEnabled(int enabled);
int  PE_3EAC8_RecordCount(void);
int  PE_3EAC8_RecordAt(int index, int *a0, int *a1);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A674(void);
extern void func_8006A8D4(void);

#endif
