/*
 * Phase 6E-A — PsyQ SDK provider layer.
 *
 * Real host implementations of the SDK providers on the native boot path,
 * replacing the Phase 6D-S BOOTSTRAP_RET stubs.  Every function is classified
 * per the Phase 6E-A provider-frontier audit:
 *
 *   class 1 — translated game logic (guest-RAM state transcribed verbatim)
 *   class 2 — PsyQ/SDK behavior requiring a host implementation
 *   class 3 — deterministic platform provider with documented ordering
 *
 * Convention: retail guest addresses stay pe_addr_t; all guest-state effects
 * go through the bounds-checked PE_Load / PE_Store / PE_Translate API.
 * Hardware-only effects (SPU/CD/GPU registers, kernel events, DMA) are
 * represented by narrow named providers or explicitly documented no-ops.
 * Nothing
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
    int16_t rt[3][3]; /* C2CTRL 0-4 rotation, 12.12 */
    int32_t tr[3];    /* C2CTRL 5-7 TRX/TRY/TRZ */
    int32_t ir[3];    /* C2DR 9-11 IR1-3 */
    int32_t mac[3];   /* C2DR 25-27 MAC1-3 */
    int16_t v0[3];    /* C2DR 0-1 VXY0/VZ0 */
} PeGteState;
extern PeGteState g_pe_gte;

void func_80077F7C(void);            /* InitGeom */
void func_80079004(int a, int b);    /* SetGeomOffset: OFX=a<<16, OFY=b<<16 */
void func_80079024(int a);           /* SetGeomScreen: H=a */

/* Exact 32-bit GTE LZCS/LZCR arithmetic (Phase 6E-B4): count of leading
 * bits equal to the sign bit.  Defined for every input (0 -> 32,
 * 0xFFFFFFFF -> 32); pure arithmetic, no g_pe_gte state. */
uint32_t PE_GTE_LZCR(uint32_t v);

/* Exact integer MVMVA (psx-spx): no host float.
 * cmd bits: sf@19, mx@17-18, v@15-16, cv@13-14, lm@10. */
void PE_GTE_LoadRT(pe_addr_t matrix);
void PE_GTE_SetIR(int16_t ir1, int16_t ir2, int16_t ir3);
void PE_GTE_SetV0(int16_t vx, int16_t vy, int16_t vz);
void PE_GTE_MVMVA(uint32_t cmd);

/* ── libetc (pc_port/platform/pe_libetc.c) ──────────────────────────── */
void func_80073C94(void);            /* ResetCallback */
pe_addr_t func_80073CC4(uint32_t source, pe_addr_t handler);
pe_addr_t func_800740D0(uint32_t source, pe_addr_t handler);
uint32_t func_80073D24(pe_addr_t handler); /* VBlank callback slot 4 setter */
uint16_t func_80073E10(uint16_t new_mask); /* I_MASK exchange (PE_IRQ authority) */
int  func_80072714(void);            /* EnterCriticalSection */
void func_80072724(void);            /* ExitCriticalSection */
int  PE_Irq_LockDepth(void);         /* diagnostic: current critical depth */

/* B53I-B1 bounded host equivalents of source 0's BIOS auto-ack controls.
 * This value-only snapshot includes live guest slot/mask observations at
 * each BIOS call; it is diagnostic state, never a callback authority. */
typedef struct {
    uint32_t pad_clear_mode;
    uint32_t vblank_clear_mode;
    uint32_t pad_calls;
    uint32_t vblank_calls;
    uint32_t pad_argument;
    uint32_t vblank_counter;
    uint32_t vblank_argument;
    uint16_t mask_at_pad_call;
    uint16_t mask_at_vblank_call;
    pe_addr_t source0_slot_at_pad_call;
    pe_addr_t source0_slot_at_vblank_call;
    uint16_t registered_mask_at_pad_call;
    uint16_t registered_mask_at_vblank_call;
    uint64_t pad_call_order;
    uint64_t vblank_call_order;
    uint64_t source0_mask_restore_order;
} PeIrqSource0BiosState;

void PE_Irq_GetSource0BiosState(PeIrqSource0BiosState *out);

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
void func_8007D15C(void);            /* SPU IRQ event install */

/* ── streaming (pc_port/platform/pe_stream.c) ───────────────────────── */
void func_80085644(void);            /* streaming bring-up */
void func_80086FF8(void);            /* stream command 0xF0 */
void func_80087024(void);            /* stream command 0xF1 */
void func_8008682C(int a);           /* stream command select */
int  func_8008CBA8(void);            /* streaming command dispatcher */

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
int  func_80080C48(pe_addr_t fp);    /* CdPosToInt: BCD mm/ss/ff @fp → LBA */
int  func_80082314(void);            /* PVD verify; result word D_800B28F8 */
int  func_80081414(pe_addr_t fp, const char *name); /* DsSearchFile */
int  func_8006E6D4(int lba_base, int lba_off, pe_addr_t dest, int size);
int  func_800811E4(pe_addr_t fp);    /* read poll: 0 done, -1 timeout */

/* ── streaming wrappers (pc_port/game/boot/, Phase 6E-B16) ────────── */
int       func_8006E6A8(int lba, pe_addr_t dest, int sectors); /* issue */
int       func_8006E7E8(void);              /* poll + D_800B0CD8 RMW */
pe_addr_t func_8006E498(pe_addr_t base, uint32_t key); /* archive lookup */

/* ── dispatcher leaves (pc_port/game/boot/, Phase 6E-B17/18) ──────── */
void func_8005B890(int a0);                 /* D_8009D028 = a0 */
void func_8005BC98(int a0_ignored);         /* D_8009D218 = 1 */
void func_8004F808(void);                   /* ten-word clear */
void func_80042B38(void);                   /* D_800A1870/1874 = 0 */
void func_80051084(void);                   /* D_8009D014 = 0x800A1AA0 */
pe_addr_t func_8005332C(int32_t resource_id); /* resource-record lookup */
pe_addr_t func_80053968(int32_t resource_id); /* materialize archive record */
void func_80051CC4(void);                   /* resource command-state init */
void func_800528F0(void);                   /* PRNG table generator, 521 bytes */
void func_8005E588(void);                   /* display environment setup */
void func_80062568(void);                   /* free-list pool init, 24 0x90-byte slots */
void func_80064964(void);                   /* A(28h) clear + 8 0xFF flag bytes */
void func_8005DE88(void);                   /* resource-list/state initializer */
pe_addr_t func_80071A24(pe_addr_t dst, uint32_t len);  /* BIOS A(28h) bzero trampoline */
void func_8005E968(uint32_t packed);         /* pack-color halver */
void func_8005F844(int a0);                 /* conditional constant stores */
void func_80052E30(uint32_t a0);            /* resource-buffer init/reuse */
uint32_t func_80052F0C(void);                /* buffer-identity comparison */

/* ── save manager (pc_port/platform/pe_save.c) ──────────────────────── */
void func_800844E4(pe_addr_t base, pe_addr_t base2);
void func_80082534(void);

/* ── game boot (pc_port/game/boot/) ─────────────────────────────────── */
void func_8003E754(int w, int h);    /* video init (DISPENV/DRAWENV setup) */
void func_8003E944(void);            /* save-manager bring-up */

/* ── test determinism ───────────────────────────────────────────────── */
/* Reset every host-owned SDK state block (GTE state, IRQ lock depth,
 * event handles, I_STAT/I_MASK authority, SPU RAM and pending DMA).  It also
 * invalidates the guest DMA busy/callback/IRQ-handle state owned by those
 * host resources and coherently clears the guest-backed CPU IRQ registration
 * block.  Other guest RAM is reset via PE_RamReset. */
void PE_Sdk_ResetState(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_SDK_H */
