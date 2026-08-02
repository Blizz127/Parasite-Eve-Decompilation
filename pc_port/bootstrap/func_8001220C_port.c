/*
 * Phase 6C — Native adaptation of func_8001220C (main).
 *
 * PE_PORT from PARKED candidate (semantically complete, 187 words).
 * MIPS scratchpad stack atom replaced with safe host adapter.
 * All register pins, empty asm barriers, and scheduling contortions removed.
 *
 * This is the REAL Parasite Eve main control flow, not a host shortcut.
 */

#include "psx_compat.h"
#include "game_port.h"
#include "stub_registry.h"

/* ── Declarations ──────────────────────────────────────────────────── */

extern void func_800725DC(void);
extern void func_8003E610(void);
extern void func_8003E680(void);
extern void func_8006A5BC(void);
extern void func_8006A64C(void);
extern void func_8006A9E4(void);
extern void func_8006AD40(void);
extern void func_8006ECEC(void);
extern void func_8006F044(void);
extern int  func_8006E834(void);
extern void func_80069B08(int);
extern void func_8003F3C4(void);
extern void func_801235DC(void);
extern void func_8019234C(void);
extern int  func_801909B4(void);
extern int  func_8006E9A0(int);
extern int  func_800698D4(void);
extern void func_80073A44(int mode);
extern void func_80074D28(int mask);

/* D_800B0CD8 is a guest-RAM lvalue macro (psx_compat.h) */
extern unsigned int D_8009D280;
extern unsigned int D_8009D1C4;
extern unsigned int D_800A7918;

/* Host-owned stop flag — set by platform when first clear is reached */
int g_port_stop_requested = 0;
int g_port_main_iterations = 0;

/* ── Scratchpad host adapter ───────────────────────────────────────── */

/*
 * Retail atom (ROM 0x2B20..0x2B44, 10 words):
 *   lui $a1, 0x1F80; ori $a1, 0x3FC   → address 0x1F8003FC (scratchpad top)
 *   sw $sp, 0($t0)                     → save host SP to scratchpad
 *   addiu $t0, -4; addu $sp, $t0, $zero → SP = scratchpad - 4
 *   jal func_8019234C; nop             → call overlay on PS1 stack
 *   addiu $sp, 4; lw $sp, 0($sp)       → restore host SP
 *
 * PE_PORT adapter: trace, call natively, no stack manipulation.
 */
static void PE_Port_ScratchpadCall(void (*overlay)(void))
{
    Trace_Direct("scratchpad_atom_begin");
    Stub_Record("scratchpad_stack_handoff", "HOST_ADAPTED");
    if (overlay) overlay();
    Trace_Direct("scratchpad_atom_end");
}

/* ── Main — adapted from PARKED candidate ──────────────────────────── */

void func_8001220C(void)
{
    unsigned int *data      = &D_800B0CD8;
    int            dispatch = 0;
    unsigned char *flagbyte = (unsigned char *)data + 0xF5;
    unsigned int   state_val = 0xA9400048u;
    unsigned int   bitmask;
    unsigned int   v;

    func_800725DC();
    func_8003E610();
    bitmask = 0x00100000u;

    for (;;) {
        g_port_main_iterations++;
        func_8006A5BC();

        while (func_800698D4() == 0) {
            func_80073A44(0);
            if (g_port_stop_requested) return;
        }

        func_8006A64C();
        func_8003E680();
        func_8006A9E4();
        D_8009D280 = state_val;

        while (1) {
            if (g_port_stop_requested) return;

            v = *data;
            if (v & bitmask) {
                func_80069B08(dispatch);
                *data &= 0xFFEFFFFFu;
            }

            func_8006AD40();
            v = D_8009D280;
            D_8009D1C4 = v;

            /* A8-code three-way dispatch */
            if (v == state_val) {
                func_8006E834();
                v = func_801909B4();
                func_8006E9A0(v);
                *data |= 0x3;
            } else if (state_val < v) {
                if (v == 0xAA108448u) {
                    if (*flagbyte & 0x2) {
                        func_8006F044();
                        func_801235DC();
                        D_8009D280 = 0xA80830C8u;
                        *data |= 0x1;
                    } else {
                        dispatch = 2;
                        *data |= bitmask;
                    }
                } else {
                    func_8003F3C4();
                }
            } else {
                if (v == 0xA8000048u) {
                    func_8006ECEC();
                    PE_Port_ScratchpadCall(func_8019234C);
                    *data |= 0x1;
                } else {
                    func_8003F3C4();
                }
            }

            /* 7-constant skip chain */
            v = D_8009D280;
            if (v == 0xA80651C8u) continue;
            if (v == 0xA8065248u) continue;
            if (v == 0xA80652C8u) continue;
            if (v == 0xA80660C8u) continue;
            if (v == 0xA8066148u) continue;
            if (v == 0xA80661C8u) continue;
            if (v == 0xA8066348u) continue;

            /* volume gate */
            v = D_800A7918;
            if (v < 0x258u) {
                if (!(*flagbyte & 0x1)) {
                    dispatch = 1;
                    *data |= bitmask;
                }
            } else {
                if (!(*flagbyte & 0x2)) {
                    dispatch = 2;
                    *data |= bitmask;
                }
            }

            if (*data & 0x100) {
                func_80073A44(0);
                func_80074D28(0);
                *data &= ~0x101u;
                break;
            }
        }
    }
}
