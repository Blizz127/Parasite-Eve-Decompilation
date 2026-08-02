/*
 * Phase 6C — Bootstrap func_800698D4 (disc mount / file search).
 *
 * Under --bootstrap-disc: simulates successful Disc 1 mount with
 * synthesized file search.  Under --strict-stubs: returns -2 (failure).
 *
 * PE_PORT from PARKED candidate (140/141 words, scheduling residual only).
 */

#include "psx_compat.h"
#include "stub_registry.h"
#include <string.h>

/* ── Globals updated by this function ─────────────────────────────── */

/* CD drive / file search calls */
extern int func_8007F72C(void);
extern int func_8007F778(void);
extern int func_80082314(void);
extern int func_80081414(void *fp, char *name);
extern int func_80080C48(void *fp);

int func_800698D4(void)
{
    if (g_bootstrap_disc) {
        /* Bootstrap Disc 1 mount path:
         *   - Drive ready → mount Disc 1 identity → find PE.IMG → success
         *   Traced and explicit. */
        D_800B0DCD = 0;
        Stub_Record("func_8007F72C", "BOOTSTRAP_RET");
        /* Simulate: func_8007F72C() returns 1 (drive ready) */
        Stub_Record("func_8007F778", "BOOTSTRAP_RET");
        /* Simulate: func_8007F778() returns 0 (ready) */
        Stub_Record("func_80082314", "BOOTSTRAP_RET");
        /* Simulate: returns 4 (Disc 1 detected) */

        /* Search PEDISC01.IDF → found */
        Stub_Record("DsSearchFile(PEDISC01.IDF)", "BOOTSTRAP_RET");
        /* Search PE.IMG → found */
        Stub_Record("DsSearchFile(PE.IMG)", "BOOTSTRAP_RET");

        /* Mount PE.IMG */
        Stub_Record("func_80080C48", "BOOTSTRAP_RET");
        D_800B0DD8 = 0;         /* opaque word — will be set by real implementation */
        D_800B0DCD |= 1;        /* Disc 1 mounted */

        /* Second pass (Disc 2 check) — not needed for boot */
        D_800B0DCD |= 2;

        /* Return non-zero to tell main the mount is complete */
        return 1;
    }

    if (g_strict_stubs) {
        fprintf(stderr, "FATAL: strict-stubs mode — func_800698D4 disc mount not implemented\n");
        exit(1);
    }

    /* Real implementation deferred — return "ready" for now */
    Stub_Record("func_800698D4_real", "UNSUPPORTED");
    return 1;
}

/* ── Bootstrap stubs for callees ──────────────────────────────────── */

int func_8007F72C(void) { Stub_Record("func_8007F72C", "BOOTSTRAP_RET"); return g_bootstrap_disc ? 1 : 0; }
int func_8007F778(void) { Stub_Record("func_8007F778", "BOOTSTRAP_RET"); return 0; }
int func_80082314(void) { Stub_Record("func_80082314", "BOOTSTRAP_RET"); return g_bootstrap_disc ? 4 : 0; }
int func_80081414(void *fp, char *name) { Stub_Record("DsSearchFile", "BOOTSTRAP_RET"); (void)fp; (void)name; return 1; }
int func_80080C48(void *fp) { Stub_Record("func_80080C48", "BOOTSTRAP_RET"); (void)fp; return 0; }
