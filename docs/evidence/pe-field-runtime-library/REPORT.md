# Native field-runtime library boundary

Status: **VERIFIED ON THE NATIVE GRIND LANE**.

This rung creates the reusable CMake target `pe_field_runtime`, producing
`libpe_field_runtime.a`. It is a build/ownership change only: no translated
retail behavior, guest state, scheduler decision, destination token, or
persistence value was added or changed.

## Boundary

The archive owns the current translated retail graph and the host-safe
providers it already uses:

- `src/pe_globals.c`
- `platform/pe_callback.c` and `PLATFORM_SRCS`
- `BOOTSTRAP_SRCS`
- `GAME_SRCS`

The command-line entry point and X11 adapter remain outside the archive:

- `src/port_main.c`
- `platform/host_window.c`

Both production and tests consume the archive rather than compiling duplicate
copies of the runtime sources:

```text
parasite-eve-port          -> pe_field_runtime + host_window + dl
pe-native-tests            -> pe_field_runtime
pe-field-runtime-link-test -> pe_field_runtime
```

The archive contains 197 object members. Its public target interface carries
the existing include roots and the established 320x240 headless compile-time
contract. The X11 adapter and its `dl` dependency are not transitively exposed
to library consumers.

The translated graph still expects the host consumer to provide the existing
`Trace_Direct(const char *)` hook. The CLI and full test executable already do;
the standalone link test supplies a no-op sink explicitly.

## Focused consumer proof

`pc_port/tests/test_field_runtime_link.c` is compiled as a source outside the
archive. It initializes the established guest-RAM/callback/bootstrap/frame/run
control components, proves a guest word round trip, and checks the retained
retail-proven `func_80077AA4(0x130, 0x1F8) == 0x7E13` example. This is not a
new gameplay oracle; it proves that an external C consumer can link and use
both the host substrate and translated code through the target interface.

## Verification

Normal build:

```text
[ 97%] Built target pe_field_runtime
[ 98%] Built target pe-native-tests
[ 99%] Built target parasite-eve-port
[100%] Built target pe-field-runtime-link-test

Test #1: native-tests .............. Passed
Test #2: field-runtime-link ........ Passed
100% tests passed, 0 tests failed out of 2

native suite: 985 run, 985 passed, 0 failed, 0 skipped
archive: pc_port/build/libpe_field_runtime.a
archive members: 197
```

Fresh ASan/UBSan build instruments the archive itself and both consumers:

```text
Test #1: native-tests .............. Passed
Test #2: field-runtime-link ........ Passed
100% tests passed, 0 tests failed out of 2
```

A fresh split and containerized matching rebuild also remains byte exact:

```text
original SHA-1:  452fb033f2eaa4b18aa20a5bca60b8125af3a37b
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
RESULT: EXACT MATCH
```

The grind branch's legacy `scripts/verify_us.sh` still exits 1 after that
exact comparison because its split-artifact checks expect splat 0.41.0 while
this host has 0.33.2 and name four pre-carve asm files that the current YAML
no longer generates (`2E7D0.s`, `42658.s`, `4C4B0.s`, `4F094.s`). This is a
pre-existing verifier-manifest drift, not a green verifier claim and not a
`pc_port` regression.

Real Disc 1 strict execution through the linked archive still reaches the
pre-existing exact frontier and stops there:

```text
STRICT_RC=1
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
       func_801924F8_80192584_cut
       called from: func_801924F8
```

The expected nonzero strict result proves the boundary was preserved; it is
not a playable-field claim. The configured retail executable remains
SLUS-006.62 SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` as established by
the retained native oracle suite.

```text
PE_FIELD_RUNTIME_TARGET=PRESENT
PE_FIELD_RUNTIME_ARCHIVE=libpe_field_runtime.a
PRODUCTION_CONSUMER_LINKS_ARCHIVE=yes
NATIVE_TEST_CONSUMER_LINKS_ARCHIVE=yes
FOCUSED_EXTERNAL_CONSUMER=PASS
NORMAL_NATIVE_SUITE=985/985
SANITIZED_NATIVE_SUITE=985/985
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut
DAY1_FIELD_RUNTIME_COMPLETE=no
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```
