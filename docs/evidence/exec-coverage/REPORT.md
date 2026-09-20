# Production-run reachability counter (executed-path coverage)

Status: **MEASURED** — `tools/progress/native_metrics.py --json` no longer
reports `reachable_semantic_functions: UNMEASURED`.

This evidence records how the native port measures its **C-only executed-path
coverage**: which guest functions a real headless run actually reached, how
many of those are backed by a decompiled C leaf (`src/<name>.c`, i.e. a `c`
span in `configs/USA/disc1.yaml`), how many are backed by a pc_port
transcription only, and how many hit an unresolved loud boundary.

Worktree: `/tmp/pe-agent-coverage`, branch `agent/exec-coverage`, base commit
`6e27fbdd`.

---

## 1. What was built

### Coverage build option (`pc_port/CMakeLists.txt`)

```sh
cmake -S pc_port -B pc_port/build-coverage -DPE_EXEC_COVERAGE=ON
```

`PE_EXEC_COVERAGE=ON` adds `platform/pe_exec_coverage.c` and compiles the whole
`pe_field_runtime` static library with:

```
-finstrument-functions -fno-inline -fno-optimize-sibling-calls -g
```

`-finstrument-functions` makes GCC call `__cyg_profile_func_enter` on entry to
every instrumented function, so no generated or hand-written translation unit
needs an edit. `-fno-inline` / `-fno-optimize-sibling-calls` keep one real
symbol per function and keep the enter/exit shadow stack balanced. The option
is **OFF by default**: the normal build neither compiles the counter nor emits
any coverage files, and its `game_port.c.o` is byte-for-byte identical (see
§5).

### Counter (`pc_port/platform/pe_exec_coverage.{c,h}`)

* `__cyg_profile_func_enter` records each entered host function entry address in
  an open-addressing set (~1300 entries observed; 2^16 slots) and pushes it on
  a shadow call stack; `__cyg_profile_func_exit` pops it.
* `PE_ExecCoverage_NoteUnresolvedBoundary()` snapshots the shadow stack when
  `PE_Port_RequestStop(PE_PORT_STOP_UNRESOLVED_BOUNDARY)` first fires (wired in
  `pc_port/bootstrap/game_port.c` behind `#ifdef PE_EXEC_COVERAGE`).
* An `atexit` dump (registered by a constructor) writes:
  * `$PE_EXEC_COVERAGE_OUT` (default `build/exec_coverage.txt`): one host
    function entry address per line;
  * `$PE_EXEC_COVERAGE_BOUNDARIES` (default
    `build/exec_coverage_boundaries.txt`): one unresolved-boundary event per
    line, its shadow stack bottom-to-top, space separated.
* The module is compiled `-fno-instrument-functions` and its hook functions
  carry `__attribute__((no_instrument_function))`, so the counter never
  instruments itself. It touches host data only: it never reads or writes guest
  RAM/VRAM and never changes guest-visible behaviour.

### Mapper (`tools/progress/exec_coverage.py`)

Standard library only. It:

1. builds the guest-function boundary map from `configs/USA/disc1.yaml`
   (image-offset → VRAM geometry; `c` spans name decompiled leaves, sized to
   the next subsegment) plus `asm/disc1/*.s` (`nonmatching <name>, <size>`
   lines, with a `glabel`/`endlabel` fallback, for every remaining function);
2. resolves each recorded host address with `nm` on the exact coverage binary,
   recovers the guest VMA encoded in the `func_XXXXXXXX` port symbol name
   (tolerating `.cold` / `.constprop.N` suffixes), and lands it in the map;
3. classifies the executed guest functions into decompiled-C leaves (`c` span)
   vs pc_port-only transcriptions (`asm` function with no C leaf), reports the
   guest functions not in the static map (dynamically loaded overlays), and
   attributes each unresolved-boundary event to the innermost guest function on
   its recorded call chain;
4. emits `docs/evidence/exec-coverage/coverage.json`, which
   `tools/progress/native_metrics.py` now reads to report the MEASURED metric.

### Boundary map derived at this commit

`python3 tools/build/disc1_plan.py --check`:
`disc1 plan: 1177 spans (810 c, 365 asm, 2 rodata)`.

| source | functions |
| --- | --- |
| `configs/USA/disc1.yaml` `c` spans (decompiled C leaves) | 810 |
| `asm/disc1/*.s` `nonmatching` functions (no C leaf yet) | 1583 |
| **total distinct guest functions** | **2393** |

The project docs quote *2390 functions*; that is a slightly older snapshot.
This tool derives the number from the yaml + split rather than hard-coding it,
so it reports the current 2393.

---

## 2. Exact commands

Prerequisites (git-ignored, local-only; the retail image is registered in
`local/pe_disc1.path`):

```sh
export PE_IMAGE_DIR="/home/blizz/.attic-decomp/rom/image/Parasite Eve (USA) (Disc 1)"
bash scripts/extract_us.sh 1
bash scripts/split_us.sh            # splat -> asm/disc1 (needs splat on the host)

cmake -S pc_port -B pc_port/build-coverage -DPE_EXEC_COVERAGE=ON
cmake --build pc_port/build-coverage -j 12
```

Route run (Day-1/Day-2 autopilot, skip-movie, headless, 80000-frame budget):

```sh
PE_EXEC_COVERAGE_OUT=/tmp/route_cov.txt \
PE_EXEC_COVERAGE_BOUNDARIES=/tmp/route_bnd.txt \
./pc_port/build-coverage/parasite-eve-port \
  --disc-image "$(cat local/pe_disc1.path)" \
  --headless --route-pad --max-frames 80000 \
  --trace /tmp/route_cov_trace.log
```

Opening-movie run (real Disc 1, headless, no `--skip-movie`):

```sh
PE_EXEC_COVERAGE_OUT=/tmp/movie_cov.txt \
PE_EXEC_COVERAGE_BOUNDARIES=/tmp/movie_bnd.txt \
./pc_port/build-coverage/parasite-eve-port \
  --disc-image "$(cat local/pe_disc1.path)" \
  --headless --max-frames 2000 \
  --trace /tmp/movie_cov_trace.log
```

Classify and write the committed artifact:

```sh
python3 tools/progress/exec_coverage.py \
  --binary pc_port/build-coverage/parasite-eve-port \
  --config configs/USA/disc1.yaml --asm-dir asm/disc1 \
  --primary route \
  --run route:docs/evidence/exec-coverage/route_exec_coverage.txt:docs/evidence/exec-coverage/route_exec_coverage_boundaries.txt \
  --run movie:docs/evidence/exec-coverage/movie_exec_coverage.txt:docs/evidence/exec-coverage/movie_exec_coverage_boundaries.txt \
  --json-out docs/evidence/exec-coverage/coverage.json
```

The raw hit sets and boundary logs are committed next to this report; the
commands above regenerate them. `coverage.json` records the coverage-binary
SHA-256 so a re-run can be checked against the same instrumentation.

---

## 3. Measured numbers

Route run ended exactly as the project documents: `stop_reason=frame-limit`,
token `A8001148`, story `0x48`, 0 bootstrap stubs, 0 unported opcodes.

| run | host fns entered | executed guest fns | decompiled C leaf | C share | pc_port-only | not in static map | unresolved-boundary fns |
| --- | --- | --- | --- | --- | --- | --- | --- |
| route (`--route-pad --max-frames 80000`) | 1350 | **710** | **237** | **33.38%** | 473 | 6 | **0** |
| movie (`--max-frames 2000`, opening FMV) | 568 | **229** | **91** | **39.74%** | 138 | 18 | **0** |

* `executed guest fns` counts distinct guest functions from the boundary map
  whose `func_XXXXXXXX` port symbol was entered at least once.
* `decompiled C leaf` counts those whose VMA is a `c` span; every one of the
  route's 237 was cross-checked to have `src/<name>.c` present (0 missing).
* `not in static map` are dynamically loaded room/overlay functions transcribed
  in `pc_port` (e.g. `func_8018F018`, `func_8018F330`, `func_8018F614`,
  `func_8018FB84`, `func_8018FDC4`, `func_801909B4` on the route) — the static
  retail-image boundary map cannot see them.
* Both runs' executed paths contained **no unresolved loud boundary**
  (`PE_PORT_STOP_UNRESOLVED_BOUNDARY`), so the third requested number is 0 for
  both. That is the honest executed-path result, not a missing measurement.

The movie run genuinely drove the movie pipeline: `func_801924F8` (production
movie frame), `func_8010C89C` (VLC decoder), `func_8007C564` / `func_8007C214`
(stream assembly) and `func_80192934` / `func_80192CE8` are all in its hit set.

### Unresolved-boundary counter validation (non-zero case)

The two CLI runs above stop at the frame budget before any loud boundary, so to
prove the boundary half of the counter is live it was exercised directly:

```sh
PE_TEST_FILTER=DAY2_movie \
PE_EXEC_COVERAGE_OUT=/tmp/mt.txt \
PE_EXEC_COVERAGE_BOUNDARIES=/tmp/mt_bnd.txt \
./pc_port/build-coverage/pe-native-tests
```

This recorded **187 unresolved-boundary events** and attributed them to **4
guest functions** — `func_801214D4`, `func_80121C04`, `func_80122040`
(dynamic movie overlays, not in the static map) and `func_80080DC4` (static
`asm` span, 0x68 bytes). The recorded stacks are exact
(`PE_Port_RequestStop <- func_801214D4`), confirming the attribution.

`native_metrics.py --json` now reports the route run as the primary measured
value:

```json
"reachable_semantic_functions": {
  "status": "MEASURED",
  "primary_run": "route",
  "guest_boundary_map_total": 2393,
  "executed_guest_functions": 710,
  "decompiled_c": 237,
  "decompiled_c_share_percent": 33.38,
  "pc_port_only": 473,
  "unresolved_boundary_functions": 0
}
```

---

## 4. Honest limits

* **Host addresses, not guest addresses, are dumped.** `-finstrument-functions`
  hands the hook the host function pointer; the guest identity is recovered
  offline from the coverage binary's symbol table (`nm`) because the port's
  function symbols embed their guest VMA. `exec_coverage.py` therefore needs
  the exact coverage binary. A mismatched binary would mis-resolve hits; the
  recorded `binary_sha256` in `coverage.json` pins it.
* **Inlined/outlined port internals are not guest functions.** Only symbols
  named `func_XXXXXXXX` are counted; host-only helpers (`PE_*`, `HostFB_*`,
  `mul32`, …) are reported separately as `executed_host_helpers`. Guest
  functions ported under a non-`func_` name would be missed; at this commit the
  transcribed guest graph is named `func_XXXXXXXX`.
* **The static map cannot see dynamically loaded room/overlay code.** Those
  hits are reported in the `not in static map` bucket rather than being folded
  into the denominator.
* **Executed-path coverage is run-specific.** It measures what one recorded
  input sequence reached; it is not a claim that the 710 functions are the only
  reachable set. The route is the project's canonical Day-1/Day-2 autopilot and
  matches the documented frontier.
* Three of the boundary-map functions that the tool counts (2393) versus the
  historical 2390 quote are a snapshot difference, not a measurement claim.

---

## 5. Verification

Normal, uninstrumented configuration (golden rule):

```
cmake -S pc_port -B pc_port/build -DCMAKE_BUILD_TYPE=Release
cmake --build pc_port/build -j 12          # 21 warnings (unchanged from baseline)
cd pc_port/build && ctest                  # 11/11 PASS
```

* `build/CMakeFiles/pe_field_runtime.dir/bootstrap/game_port.c.o` is
  **byte-for-byte identical** before and after this change (SHA-256
  `f2d2fa134807df123e5af15920fdeb36c72c76eb10b17bb14368d69d614ad55f`);
  the boundary hook is entirely behind `#ifdef PE_EXEC_COVERAGE`.
* `nm pc_port/build/parasite-eve-port | grep __cyg_profile_func` → no matches
  (no counter in the normal build).
* `python3 tools/progress/test_progress.py` → 4/4 OK.
* `python3 tools/progress/native_metrics.py --check-status` → PASS.

Coverage configuration:

```
cd pc_port/build-coverage && ctest           # 11/11 PASS (280.63 s)
```

Instrumentation adds no build warnings beyond the normal build's 21.

---

## 6. Files

| file | change |
| --- | --- |
| `pc_port/CMakeLists.txt` | `PE_EXEC_COVERAGE` option, counter source, instrumentation flags |
| `pc_port/platform/pe_exec_coverage.c` | new counter / shadow stack / atexit dump |
| `pc_port/platform/pe_exec_coverage.h` | new host-only hook interface |
| `pc_port/bootstrap/game_port.c` | `#ifdef PE_EXEC_COVERAGE` unresolved-boundary note |
| `tools/progress/exec_coverage.py` | new address→guest-boundary mapper/classifier |
| `tools/progress/native_metrics.py` | reads `coverage.json`; reports MEASURED instead of UNMEASURED |
| `docs/generated/NATIVE_PORT_STATUS.md` | regenerated |
| `docs/evidence/exec-coverage/*` | this report, raw hit sets, boundary logs, `coverage.json` |
