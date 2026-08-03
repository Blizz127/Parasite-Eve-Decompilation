/*
 * Phase 6D-S — Bootstrap func_800698D4 (disc mount / file search).
 *
 * All BOOTSTRAP_RET calls now use the centralized Bootstrap_ReturnInt policy.
 * No special-cased strict-mode exit — strict enforcement is centralized.
 */
#include "psx_compat.h"
#include "stub_registry.h"
#include "pe_bootstrap.h"
#include <string.h>

/* ── Globals ───────────────────────────────────────────────────────── */
/* D_800B0DCD / D_800B0DD8 are guest-RAM lvalue macros (psx_compat.h). */

extern int func_8007F72C(void);
extern int func_8007F778(void);
extern int func_80082314(void);
extern int func_80081414(void *fp, char *name);
extern int func_80080C48(void *fp);

int func_800698D4(void)
{
    if (g_bootstrap_disc) {
        D_800B0DCD = 0;
        Bootstrap_ReturnInt("func_8007F72C", "func_800698D4", 1);
        Bootstrap_ReturnInt("func_8007F778", "func_800698D4", 0);
        Bootstrap_ReturnInt("func_80082314", "func_800698D4", 4);
        Bootstrap_ReturnInt("DsSearchFile(PEDISC01.IDF)", "func_800698D4", 1);
        Bootstrap_ReturnInt("DsSearchFile(PE.IMG)", "func_800698D4", 1);
        Bootstrap_ReturnInt("func_80080C48", "func_800698D4", 0);
        D_800B0DD8 = 0;
        D_800B0DCD |= 1;
        D_800B0DCD |= 2;
        return 1;
    }

    /* Real disc mount not implemented.  Strict mode aborts centrally at
     * this first BOOTSTRAP_RET provider; normal mode records and returns
     * "ready" per bootstrap policy. */
    Bootstrap_ReturnInt("func_800698D4_real", "func_800698D4", 1);
    return 1;
}

/* ── Bootstrap stubs ───────────────────────────────────────────────── */
/* func_8007F72C / func_8007F778 are real (pe_libcd.c, Phase 6E-A). */

int func_80082314(void) {
    return Bootstrap_ReturnInt("func_80082314", "func_800698D4",
                               g_bootstrap_disc ? 4 : 0);
}
int func_80081414(void *fp, char *name) {
    (void)fp; (void)name;
    return Bootstrap_ReturnInt("DsSearchFile", "func_800698D4", 1);
}
int func_80080C48(void *fp) {
    (void)fp;
    return Bootstrap_ReturnInt("func_80080C48", "func_800698D4", 0);
}
