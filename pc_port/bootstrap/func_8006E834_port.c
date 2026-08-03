/*
 * Phase 6D-S — Native adaptation of func_8006E834 (post-mount image loader).
 *
 * All bootstrap stubs now use centralized Bootstrap_ReturnInt/Void.
 * D_800BCE80 is guest-RAM-backed (defined in psx_compat.h), no local override.
 */
#include "psx_compat.h"
#include "pe_bootstrap.h"
#include <string.h>

/* ── Globals ───────────────────────────────────────────────────────── */
/* D_800B0DB2-B7, D_800B0CD8, D_800B0DD8 are guest-RAM lvalue macros
 * (psx_compat.h).  D_80011614 is a pe_addr_t guest address. */
extern unsigned short D_80093164[4];

extern void func_80086FF8(void);
extern int  func_8006E6D4(int a0, int a1, pe_addr_t a2, int a3);
extern int  func_800811E4(void *p);
extern void func_800726C4(void);
extern void func_80073A44(int a);
extern void func_80074D28(int a);
extern void func_800755F0(void *env);
/* func_80072714/func_80072724 (pe_libetc.c) and func_800749D8
 * (pe_libgpu.c) are real implementations (Phase 6E-A) via pe_sdk.h. */

/* Retail builds the DISPENV on the guest stack (sp+0x18).  The host does
 * not track a guest stack pointer, so the env lives at a fixed scratch
 * address inside guest RAM; it is local to this call and dead after
 * PutDispEnv.  All accesses stay bounds-checked pe_addr_t. */
#define PE_6E834_ENV_ADDR 0x801FFF00u

/* ── Bootstrap stubs ───────────────────────────────────────────────── */
void func_80086FF8(void) {
    Bootstrap_ReturnVoid("func_80086FF8", "func_8006E834");
}
int func_8006E6D4(int a0, int a1, pe_addr_t a2, int a3) {
    /* a2 is the guest destination address (D_80011614); a real
     * implementation would PE_Translate(a2, a3) and read into guest RAM.
     * Bootstrap policy does not model the read content yet. */
    (void)a0; (void)a1; (void)a2;
    return Bootstrap_ReturnInt("func_8006E6D4", "func_8006E834", a3);
}
int func_800811E4(void *p) {
    (void)p;
    return Bootstrap_ReturnInt("func_800811E4", "func_8006E834", 0);
}
void func_800726C4(void) { Bootstrap_ReturnVoid("func_800726C4", "func_8006E834"); }

/* ── Adapted function ──────────────────────────────────────────────── */
int func_8006E834(void)
{
    char local30[8];
    unsigned short *tbl;
    int r;

    D_800B0DB5 = -1;
    D_800B0DB4 = -1;
    D_800B0DB7 = -1;
    D_800B0DB6 = -1;
    D_800B0DB3 = -1;
    D_800B0DB2 = -1;
    D_800B0CD8 &= ~0xF0;
    func_80086FF8();

retry:
    tbl = D_80093164;
    do {
        r = func_8006E6D4(D_800B0DD8 + tbl[0], 0, D_80011614, tbl[1] - tbl[0]);
    } while (r == -1);

    for (;;) {
        r = func_800811E4(local30);
        if ((unsigned)(r + 1) < 2) {
            D_800B0CD8 &= 0xFEFFBFFF;
        }
        if (r == 0) break;
        if (r == -1) goto retry;
    }

    func_80072714();
    func_800726C4();
    func_80072724();
    func_80073A44(0);
    func_80074D28(0);
    func_800749D8(PE_6E834_ENV_ADDR, 0, 0, 0x140, 0xF0);
    PE_StoreU8(PE_6E834_ENV_ADDR + 0x11u, 1);   /* isrgb24 = 1 */
    func_800755F0(PE_Translate(PE_6E834_ENV_ADDR, 0x14));
    return 0;
}
