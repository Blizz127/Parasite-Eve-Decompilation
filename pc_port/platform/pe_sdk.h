/*
 * Phase 6E-A — PsyQ SDK provider layer.
 *
 * Real host implementations of the SDK providers on the native boot path,
 * replacing the Phase 6D-S BOOTSTRAP_RET stubs.  Every function is classified
 * per the Phase 6E-A provider-frontier audit:
 *
 *   class 1 — translated game logic (guest-RAM state transcribed verbatim)
 *   class 2 — PsyQ/SDK behavior requiring a host implementation
 *   class 3 — deterministic platform provider (hardware modeled synchronously)
 *
 * Convention: retail guest addresses stay pe_addr_t; all guest-state effects
 * go through the bounds-checked PE_Load / PE_Store / PE_Translate API.
 * Hardware-only effects (SPU/CD/
 * GPU registers, kernel events, DMA) are collapsed to named no-ops and each
 * collapse is enumerated in the implementing file's header comment.  Nothing
 * here returns a fabricated value merely to advance strict mode: state
 * transitions reproduce the retail-observable guest-RAM effects.
 */
#ifndef PE_SDK_H
#define PE_SDK_H

#include <stdint.h>
#include "pe_guest_ram.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── libgte (pc_port/platform/pe_gte.c) ─────────────────────────────── */
/* cop2 control-register state has no guest-RAM backing; it is host-owned. */
typedef struct {
    int32_t ofx;   /* $24 screen offset X (16.16) */
    int32_t ofy;   /* $25 screen offset Y (16.16) */
    int32_t h;     /* $26 projection plane distance */
    int32_t dqa;   /* $27 depth cueing coefficient */
    int32_t dqb;   /* $28 depth cueing offset */
    int32_t zsf3;  /* $29 average-z scale (3 terms) */
    int32_t zsf4;  /* $30 average-z scale (4 terms) */
} PeGteState;
extern PeGteState g_pe_gte;

void func_80077F7C(void);            /* InitGeom */
void func_80079004(int a, int b);    /* SetGeomOffset: OFX=a<<16, OFY=b<<16 */
void func_80079024(int a);           /* SetGeomScreen: H=a */

/* ── libetc (pc_port/platform/pe_libetc.c) ──────────────────────────── */
void func_80073C94(void);            /* ResetCallback */
int  func_80072714(void);            /* EnterCriticalSection */
void func_80072724(void);            /* ExitCriticalSection */
int  PE_Irq_LockDepth(void);         /* diagnostic: current critical depth */

/* OpenEvent / EnableEvent host shims (BIOS B(08h)/B(0Ch)).
 * Retail allocates kernel Event Control Blocks outside the 2 MiB guest
 * window; the host models handles as an opaque deterministic counter.
 * Never returns -1 at boot (event classes used never exhaust). */
int  PE_Event_Open(uint32_t cls, uint32_t spec, uint32_t mode, pe_addr_t handler);
int  PE_Event_Enable(int handle);

/* ── libgpu (pc_port/platform/pe_libgpu.c) ──────────────────────────── */
pe_addr_t func_80074924(pe_addr_t env, int x, int y, int w, int h); /* SetDefDrawEnv */
pe_addr_t func_800749D8(pe_addr_t env, int x, int y, int w, int h); /* SetDefDispEnv */
int       func_80074A44(int mode);   /* ResetGraph */
int       func_80074BB8(int level);  /* SetGraphDebug */

/* ── libsnd (pc_port/platform/pe_libsnd.c) ──────────────────────────── */
void func_8007D054(void);            /* SsInit wrapper (tail-call 7D074(0)) */

/* ── libcard (pc_port/platform/pe_libcard.c) ────────────────────────── */
void func_800409B4(void);            /* InitCARD + StartCARD */

/* ── libcd (pc_port/platform/pe_libcd.c) ────────────────────────────── */
int  func_8007EC14(void);            /* CdInit */
int  func_8007ED58(void);            /* Cd reset + state clear (returns 1) */
int  func_8007FBF0(int idx);         /* CdStatus lane getter D_8009B574[idx] */
int  func_8007F72C(void);            /* CdReady */
int  func_8007F778(void);            /* CdReady queue-depth getter D_800A3608 */
int  func_80080CC8(int v);           /* exchange D_8009AFC0 */
int  func_8007F7A8(void);            /* getter D_8009B590 */

/* ── save manager (pc_port/platform/pe_save.c) ──────────────────────── */
void func_800844E4(pe_addr_t base, pe_addr_t base2);
void func_80082534(void);

/* ── game boot (pc_port/game/boot/) ─────────────────────────────────── */
void func_8003E754(int w, int h);    /* video init (DISPENV/DRAWENV setup) */
void func_8003E944(void);            /* save-manager bring-up */

/* ── test determinism ───────────────────────────────────────────────── */
/* Reset every host-owned SDK state block (GTE state, IRQ lock depth,
 * event handle counter).  Guest-RAM state is reset via PE_RamReset. */
void PE_Sdk_ResetState(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_SDK_H */
