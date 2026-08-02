/*
 * Phase 6C — Native adaptation of func_8006E834 (post-mount image loader).
 *
 * PE_PORT from exact matched C (91 words).
 * MIPS register pins and asm barriers removed.
 * Image-load functions routed through bootstrap-disc adapter.
 * Display setup preserved exactly.
 */

#include "psx_compat.h"
#include "stub_registry.h"
#include <string.h>

/* ── Globals ───────────────────────────────────────────────────────── */
extern signed char D_800B0DB2, D_800B0DB3, D_800B0DB4, D_800B0DB5, D_800B0DB6, D_800B0DB7;
extern unsigned int D_800B0CD8;
extern int D_800B0DD8;
extern unsigned char *D_80011614;
unsigned short D_80093164[4];     /* offset/size pair table */
unsigned char D_800BCE80_arr[0x18]; /* DISP_ENV proxy */
#undef  D_800BCE80
#define D_800BCE80  D_800BCE80_arr

extern void func_80086FF8(void);
extern int  func_8006E6D4(int a0, int a1, unsigned char *a2, int a3);
extern int  func_800811E4(void *p);
extern void func_80072714(void);
extern void func_800726C4(void);
extern void func_80072724(void);
extern void func_80073A44(int a);
extern void func_80074D28(int a);
extern void func_800749D8(void *env, int x, int y, int w, int h);
extern void func_800755F0(void *env);

/* ── Bootstrap stubs for callees ──────────────────────────────────── */
void func_80086FF8(void) { Stub_Record("func_80086FF8", "BOOTSTRAP_RET"); }
int  func_8006E6D4(int a0, int a1, unsigned char *a2, int a3) {
    Stub_Record("func_8006E6D4", "BOOTSTRAP_RET");
    (void)a0; (void)a1; (void)a2;
    /* Return a3 (size) to indicate successful read */
    return a3;
}
int  func_800811E4(void *p) {
    Stub_Record("func_800811E4", "BOOTSTRAP_RET");
    (void)p;
    return 0;  /* Complete — no error, no retry */
}
void func_80072714(void) { Stub_Record("func_80072714", "BOOTSTRAP_RET"); }
void func_800726C4(void) { Stub_Record("func_800726C4", "BOOTSTRAP_RET"); }
void func_80072724(void) { Stub_Record("func_80072724", "BOOTSTRAP_RET"); }
void func_800749D8(void *env, int x, int y, int w, int h) {
    Stub_Record("func_800749D8(SetDefDispEnv)", "BOOTSTRAP_RET");
    (void)env; (void)x; (void)y; (void)w; (void)h;
    memset(env, 0, 0x18);
}

/* ── Adapted function ──────────────────────────────────────────────── */
int func_8006E834(void)
{
    char env[0x18];
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

    /* Completion poll — register pins removed (PE_PORT) */
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
    func_800749D8(env, 0, 0, 0x140, 0xF0);
    env[0x11] = 1;
    func_800755F0(env);
    return 0;
}
