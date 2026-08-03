/*
 * Phase 6E-B3 — func_8003EAC8: UNRESOLVED GTE-indexed registration provider.
 *
 * Complete audit (asm/disc1/2E7D0.s:804-821, exe 0x8003EAC8-0x8003EB03,
 * 15 words / 0x3C, "Handwritten function"):
 *   sw   $a0, 0($sp)
 *   mtc2 $a0, $30              ; GTE data reg 30 = LZCS (leading-zero-count
 *                              ;  source)
 *   beq  $a0, 0x80000000, done ; special case: idx = 31 ...
 *   addiu $v1, $zero, 0x1F     ; ... via the branch delay slot
 *   swc2 $31, 0($sp)           ; GTE data reg 31 = LZCR (result)
 *   lw   $v0, 0($sp)
 *   subu $v1, $v1, $v0         ; idx = 31 - LZCR = highest-set-bit index
 * done:
 *   D_800A76F0[idx] = $a1
 *
 * So: func_8003EAC8(mask, value) writes `value` into the 32-entry table
 * D_800A76F0 at the position of mask's highest set bit (0x80000000 forced
 * to slot 31 — LZCR counts leading ONES for negative inputs, which would
 * otherwise give slot 30).  No return value consumed by any caller.
 * 63 call sites exe-wide (20 from func_8003E974): a SHARED registration
 * provider, deliberately NOT folded into the func_8003E974 translation.
 *
 * Phase scope: kept unresolved — calls route through the centralized
 * bootstrap boundary, so strict mode stops at the first invocation.
 * The bounded argument log is deterministic instrumentation read only by
 * tests; it does not alter production strict behavior (identical record
 * + abort semantics as every other provider).
 */
#include "psx_compat.h"
#include "pe_port_compat.h"

#define PE_3EAC8_LOG_MAX  64

static struct { int a0, a1; } g_3eac8_log[PE_3EAC8_LOG_MAX];
static int g_3eac8_count;

void func_8003EAC8(int a0, int a1)
{
    Bootstrap_ReturnVoid("func_8003EAC8", "func_8003E974");
    if (g_3eac8_count < PE_3EAC8_LOG_MAX) {
        g_3eac8_log[g_3eac8_count].a0 = a0;
        g_3eac8_log[g_3eac8_count].a1 = a1;
    }
    g_3eac8_count++;
}

/* Test instrumentation (read-only accessors; ResetTestState resets). */
void PE_3EAC8_RecordReset(void) { g_3eac8_count = 0; }
int  PE_3EAC8_RecordCount(void) { return g_3eac8_count; }
int  PE_3EAC8_RecordAt(int index, int *a0, int *a1)
{
    if (index < 0 || index >= g_3eac8_count || index >= PE_3EAC8_LOG_MAX)
        return 0;
    *a0 = g_3eac8_log[index].a0;
    *a1 = g_3eac8_log[index].a1;
    return 1;
}
