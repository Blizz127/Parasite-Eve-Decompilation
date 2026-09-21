# Main-loop destination snapshot in guest RAM

`D_8009D1C4` now names the original guest word at `8009D1C4` instead of a
separate host scalar. The existing main-loop assignment and boot reset now
publish their results to the same RAM used by original instructions and
captures. No route state is injected or restored.

## Original authority and failure

Original `122CC..122DC` loads `8009D280` and stores it to `8009D1C4` before
dispatching the field. These four words have SHA-256
`6922f1d5ecd983d866c65a6fa40ea8c7e920f52cf5c1aeed17f54423450c9177`.
Executing them with inputs `0,1,7FFFFFFF,80000000,A8003248,FFFFFFFF` copies
each value correctly. Original and candidate EXE remain byte-identical,
SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

The old native capture at M34 present-hook frame60,260 holds `D1C4=0` and
`D280=A8003248`. Original `3F678..3F688` therefore takes its destination-change
exit after one isolated frame. The native field adapter compares its local
entry snapshot, so this missing RAM publication did not change that adapter's
branch. It did prevent faithful observation of the original inner loop.

Changes are in `psx_compat.h`, `pe_globals.c` and obsolete declarations in
`func_8001220C_port.c`, `func_8003E680_port.c`, and the native/route tests.
The existing field adapter's one-pass scheduling is unchanged.

## Verification

The boot reset regression seeds guest RAM directly, checks named readers see
it, then verifies the original guest word is cleared. The existing120-frame
real-disc opening test checks that the actual `1220C` loop publishes the
M0010I destination in guest RAM.

Full Release and Debug builds pass. **1,397 native tests pass, zero failed or
skipped; all10 non-route CTest checks pass in63.52 seconds.** Logs:
`/tmp/pe-destination-ram-{release-build,debug-build,ctest}.log`.

The isolated [M34 frame stack probe](M34_FRAME_STACK_PRESERVATION.md) can
explicitly execute the original four-word snapshot before using older
captures. This is disclosed source execution in a diagnostic; it is not a
substitute for a fresh connected capture or full Day2 verification.

## Fresh connected evidence

An ordinary-input cold boot repeats the three sewer victories at
52,111 /53,823 /57,791 and reaches the existing unresolved F434 boundary at
60,271. The historical optional milestone checker reports50/57 and exits1;
this is not a passing full-route test. Log `/tmp/pe-m34-destination-connected.log`.

The11 present-hook captures60,260..60,270 in
`pc_port/build/day2-victory-evidence/m34-destination-window` all contain
`9D1C4=9D280=A8003248`. Comparing each complete2MB image to the corresponding
pre-fix capture finds changes **only at9D1C4,9D1C5,9D1C7**, the nonzero bytes
of that destination word. No other captured byte changed.

Frame60,260 SHA-256:
`9db65a73f20670c91ff8ac3477c5859aea9d060cabff0e4ef66ce4651865b36f`.
Frame60,270 SHA-256:
`c025a97acf805a40a59840d2bed061328b61d63c4aa00ed683429579e0ef23ab`.
The fresh frame60,260 capture supports11 continuous original frames to F434
without `--caller-snapshot`; see the linked frame-stack evidence.
