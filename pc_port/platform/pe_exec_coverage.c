/*
 * pe_exec_coverage.c — executed-path (reachability) counter for the native
 * field runtime.
 *
 * Built only when CMake's PE_EXEC_COVERAGE option is ON.  The option compiles
 * the field runtime with GCC's -finstrument-functions, so every instrumented
 * function entry reaches __cyg_profile_func_enter() below.  We keep an open
 * addressing set of host function entry addresses and a shadow call stack.
 *
 * Host-only by construction: the module touches no guest RAM/VRAM and no
 * guest-visible state, and it is compiled with -fno-instrument-functions so
 * its own frames never feed back into the counter.
 *
 * Output (defaults, overridable by env):
 *   PE_EXEC_COVERAGE_OUT         -> build/exec_coverage.txt
 *                                   one host function entry address per line
 *   PE_EXEC_COVERAGE_BOUNDARIES  -> build/exec_coverage_boundaries.txt
 *                                   one unresolved-boundary event per line,
 *                                   its shadow call stack bottom-to-top, space
 *                                   separated, hex addresses
 */
#include "pe_exec_coverage.h"

#ifdef PE_EXEC_COVERAGE

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Open-addressing hit set. ~1500 funcs observed; 2^16 slots is ample and the
 * table stays at a low load factor so probing is O(1). */
#define PE_EC_TABLE_BITS 16u
#define PE_EC_TABLE_SIZE (1u << PE_EC_TABLE_BITS)
#define PE_EC_TABLE_MASK (PE_EC_TABLE_SIZE - 1u)

/* Shadow call stack.  Deeper than any observed guest call chain; overflow is
 * clamped rather than fatal (coverage stays honest: a missing frame only
 * affects which caller an unresolved boundary is attributed to). */
#define PE_EC_STACK_MAX 8192u

/* Boundary events: enough to characterise a run without unbounded memory. */
#define PE_EC_BOUNDARY_EVENTS 512u
#define PE_EC_BOUNDARY_FRAMES 96u

static const void *g_hits[PE_EC_TABLE_SIZE];
static unsigned g_hit_count;

static const void *g_stack[PE_EC_STACK_MAX];
static unsigned g_depth;

typedef struct {
    const void *frames[PE_EC_BOUNDARY_FRAMES];
    unsigned depth;
} PEExecBoundaryEvent;

static PEExecBoundaryEvent g_boundary_events[PE_EC_BOUNDARY_EVENTS];
static unsigned g_boundary_event_count;
static unsigned g_boundary_event_overflow;

static int g_dumped;

__attribute__((no_instrument_function))
static unsigned pe_ec_hash(const void *fn)
{
    uintptr_t x = (uintptr_t)fn;
    x ^= x >> 33;
    x *= (uintptr_t)0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    return (unsigned)(x & PE_EC_TABLE_MASK);
}

__attribute__((no_instrument_function))
static void pe_ec_record(const void *fn)
{
    unsigned slot = pe_ec_hash(fn);
    unsigned probes = 0;

    /* Table never fills in practice; guard against probing forever. */
    if (g_hit_count * 2u >= PE_EC_TABLE_SIZE) return;
    while (g_hits[slot] != NULL) {
        if (g_hits[slot] == fn) return;
        slot = (slot + 1u) & PE_EC_TABLE_MASK;
        if (++probes > PE_EC_TABLE_MASK) return;
    }
    g_hits[slot] = fn;
    g_hit_count++;
}

__attribute__((no_instrument_function))
void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
    (void)call_site;
    if (g_depth < PE_EC_STACK_MAX) g_stack[g_depth++] = this_fn;
    pe_ec_record(this_fn);
}

__attribute__((no_instrument_function))
void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
    (void)call_site;
    if (g_depth > 0u && g_stack[g_depth - 1u] == this_fn) {
        g_depth--;
        return;
    }
    /* Defensive: an unbalanced frame (should not happen with
     * -fno-optimize-sibling-calls) resynchronises on the nearest match. */
    for (unsigned i = g_depth; i > 0u; i--) {
        if (g_stack[i - 1u] == this_fn) {
            g_depth = i - 1u;
            return;
        }
    }
}

__attribute__((no_instrument_function))
void PE_ExecCoverage_NoteUnresolvedBoundary(void)
{
    PEExecBoundaryEvent *event;
    unsigned keep;
    unsigned i;

    if (g_boundary_event_count >= PE_EC_BOUNDARY_EVENTS) {
        g_boundary_event_overflow++;
        return;
    }
    event = &g_boundary_events[g_boundary_event_count++];
    keep = g_depth < PE_EC_BOUNDARY_FRAMES ? g_depth : PE_EC_BOUNDARY_FRAMES;
    /* Keep the innermost `keep` frames in bottom-to-top order. */
    for (i = 0; i < keep; i++)
        event->frames[i] = g_stack[g_depth - keep + i];
    event->depth = keep;
}

static FILE *pe_ec_open(const char *env_name, const char *primary,
                        const char *fallback)
{
    const char *path = getenv(env_name);
    FILE *fp;

    if (path && path[0]) return fopen(path, "w");
    fp = fopen(primary, "w");
    if (fp) return fp;
    return fopen(fallback, "w");
}

__attribute__((no_instrument_function))
void PE_ExecCoverage_Dump(void)
{
    FILE *fp;
    unsigned i;

    if (g_dumped) return;
    g_dumped = 1;

    fp = pe_ec_open("PE_EXEC_COVERAGE_OUT",
                    "build/exec_coverage.txt", "exec_coverage.txt");
    if (fp) {
        for (i = 0; i < PE_EC_TABLE_SIZE; i++) {
            if (g_hits[i])
                fprintf(fp, "%lx\n", (unsigned long)(uintptr_t)g_hits[i]);
        }
        fclose(fp);
    }

    fp = pe_ec_open("PE_EXEC_COVERAGE_BOUNDARIES",
                    "build/exec_coverage_boundaries.txt",
                    "exec_coverage_boundaries.txt");
    if (fp) {
        for (i = 0; i < g_boundary_event_count; i++) {
            const PEExecBoundaryEvent *event = &g_boundary_events[i];
            unsigned f;
            for (f = 0; f < event->depth; f++) {
                if (f) fputc(' ', fp);
                fprintf(fp, "%lx", (unsigned long)(uintptr_t)event->frames[f]);
            }
            fputc('\n', fp);
        }
        fclose(fp);
    }

    fprintf(stderr,
            "[EXEC-COVERAGE] %u distinct host function(s) entered, "
            "%u unresolved-boundary event(s)%s\n",
            g_hit_count, g_boundary_event_count,
            g_boundary_event_overflow ? " (event log full)" : "");
}

__attribute__((constructor))
static void pe_ec_register_dump(void)
{
    atexit(PE_ExecCoverage_Dump);
}

#endif /* PE_EXEC_COVERAGE */
