/*
 * pe_exec_coverage.h — production-run reachability counter (executed-path
 * coverage) for the native port.
 *
 * This header is only meaningful in a coverage build (`-DPE_EXEC_COVERAGE=1`,
 * selected by the CMake option `PE_EXEC_COVERAGE`).  In a normal build nothing
 * here is compiled or linked, so the field runtime keeps its usual
 * performance.
 *
 * The counter is host-only.  It records the HOST address of every instrumented
 * function entered during a run (GCC `-finstrument-functions` calls
 * `__cyg_profile_func_enter`), plus, for each unresolved loud boundary, the
 * call chain that led to the stop request.  It never reads or writes guest
 * RAM/VRAM and never changes guest-visible behaviour.
 *
 * See tools/progress/exec_coverage.py for the offline step that resolves the
 * recorded host addresses against the coverage build's symbol table and maps
 * them onto the retail guest-function boundaries (asm/disc1 + disc1.yaml).
 */
#ifndef PE_EXEC_COVERAGE_H
#define PE_EXEC_COVERAGE_H

#ifdef PE_EXEC_COVERAGE

/* Record the guest *unresolved loud boundary* call chain at the moment a
 * stop is requested with PE_PORT_STOP_UNRESOLVED_BOUNDARY.  Called from
 * PE_Port_RequestStop; the shadow call stack it captures still has
 * PE_Port_RequestStop on top, so the offline tool walks down to the nearest
 * guest function that triggered the boundary. */
void PE_ExecCoverage_NoteUnresolvedBoundary(void);

/* Write the hit set and the boundary call chains to the configured dump
 * files.  Registered through atexit() by a constructor, so a normal exit
 * (including exit() from a failed assertion) dumps; the env vars
 * PE_EXEC_COVERAGE_OUT and PE_EXEC_COVERAGE_BOUNDARIES override the
 * defaults (build/exec_coverage.txt and build/exec_coverage_boundaries.txt). */
void PE_ExecCoverage_Dump(void);

#else

#define PE_ExecCoverage_NoteUnresolvedBoundary() ((void)0)
#define PE_ExecCoverage_Dump() ((void)0)

#endif /* PE_EXEC_COVERAGE */

#endif /* PE_EXEC_COVERAGE_H */
