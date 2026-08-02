/*
 * Phase 6D-S — Full-width host-safe callback registry.
 *
 * No (int)(uintptr_t) casts.  Function pointers are stored and invoked at
 * their full native width.  Provides explicit Reset / Register / Get / Invoke.
 */
#ifndef PE_CALLBACK_H
#define PE_CALLBACK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*PECallback)(void);

/* ── Lifecycle ──────────────────────────────────────────────────────── */

void PE_Callback_Init(void);        /* one-time init, part of engine startup */

/* ── Operations ─────────────────────────────────────────────────────── */

void       PE_Callback_Reset(void);                       /* store NULL   */
void       PE_Callback_Register(PECallback callback);     /* store pointer */
PECallback PE_Callback_Get(void);                         /* read pointer  */
void       PE_Callback_Invoke(void);                      /* call if set   */

/* ── Diagnostic ─────────────────────────────────────────────────────── */

int        PE_Callback_RegistrationCount(void);  /* how many times registered */

#ifdef __cplusplus
}
#endif

#endif /* PE_CALLBACK_H */
