/*
 * Phase 6D-S — Centralized bootstrap and strict-mode policy.
 *
 * Every BOOTSTRAP_RET stub on the first-clear path must use these functions
 * instead of calling Stub_Record directly with a made-up return value.
 * Strict mode is enforced centrally: the first bootstrap provider invoked
 * triggers a controlled exit with the symbol and caller.
 */
#ifndef PE_BOOTSTRAP_H
#define PE_BOOTSTRAP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Lifecycle ──────────────────────────────────────────────────────── */

void Bootstrap_Init(void);       /* reset state, call once at startup */
void Bootstrap_Shutdown(void);   /* print summary                      */

/* ── Provider API ───────────────────────────────────────────────────── */

/* Register that `symbol` was invoked from `caller`, and return `value`.
 * In strict mode, the FIRST call to this function aborts with a diagnostic.
 * In normal mode, this just records the invocation and returns `value`. */
int  Bootstrap_ReturnInt(const char *symbol, const char *caller, int value);

/* One-argument variant for unresolved retail calls whose argument is part of
 * the boundary contract.  The call log is diagnostic/test evidence only; it
 * does not influence the scripted return value. */
typedef struct {
    const char *symbol;
    const char *caller;
    uint32_t arg0;
} BootstrapArgCall;

#define BOOTSTRAP_MAX_ARG_CALLS 256
extern BootstrapArgCall g_bootstrap_arg_calls[BOOTSTRAP_MAX_ARG_CALLS];
extern int g_bootstrap_arg_call_count;
int  Bootstrap_ReturnInt1(const char *symbol, const char *caller, int value,
                          uint32_t arg0);
void Bootstrap_ReturnVoid1(const char *symbol, const char *caller,
                           uint32_t arg0);
void Bootstrap_ResetArgCallLog(void);

/* Four-register variant for unresolved retail calls with a complete MIPS
 * argument contract.  uintptr_t preserves native transient-pointer width;
 * these diagnostic records are host-only and are never written to guest
 * RAM. */
typedef struct {
    const char *symbol;
    const char *caller;
    uintptr_t arg0;
    uintptr_t arg1;
    uintptr_t arg2;
    uintptr_t arg3;
} BootstrapArgCall4;

#define BOOTSTRAP_MAX_ARG4_CALLS 256
extern BootstrapArgCall4 g_bootstrap_arg4_calls[BOOTSTRAP_MAX_ARG4_CALLS];
extern int g_bootstrap_arg4_call_count;
void Bootstrap_ReturnVoid4(const char *symbol, const char *caller,
                           uintptr_t arg0, uintptr_t arg1,
                           uintptr_t arg2, uintptr_t arg3);
void Bootstrap_ResetArg4CallLog(void);

/* Same, for void functions.  In strict mode aborts on first call. */
void Bootstrap_ReturnVoid(const char *symbol, const char *caller);

/* ── Strict mode ────────────────────────────────────────────────────── */

/* Called once at startup when --strict-stubs is active. */
void Bootstrap_EnableStrict(void);

/* ── Deterministic provider sequences ─────────────────────────────────
 *
 * Test/runtime hook: script the values a named BOOTSTRAP_RET provider
 * returns on successive invocations.  When the sequence is exhausted the
 * provider falls back to its compiled-in default value.  This lets tests
 * exercise wait-loop bodies deterministically. */

#define BOOTSTRAP_MAX_SEQ 16

void Bootstrap_SetIntSequence(const char *symbol, const int *values, int count);
void Bootstrap_ClearSequences(void);

/* ── Diagnostic ─────────────────────────────────────────────────────── */

int  Bootstrap_InvocationCount(void);   /* unique bootstrap symbols seen   */
void Bootstrap_PrintSummary(void);

#ifdef __cplusplus
}
#endif

#endif /* PE_BOOTSTRAP_H */
