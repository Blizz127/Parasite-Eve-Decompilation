# Field frame counter uses guest RAM

`D_8009D250` now names its original guest word at `8009D250` through
`PE_GUEST_U32`. Its separate host scalar has been removed. The existing
3E680 reset, 3F3C4 increment, battle RNG seeding and menu animation readers
therefore share the same storage as original code and RAM captures.

The mismatch was observed while investigating M34's retained stack inputs.
The connected captures at frames60,197 and60,270 both contained zero at
guest9D250, although the native field loop incremented the separate host
variable. This made those captures incomplete inputs for original routines
that read the counter. It does not establish that every earlier isolated
comparison was invalid: many tests explicitly supplied both values, while
callbacks that do not read the counter remain separately scoped evidence.

Original six-word increment `8003F4D0..8003F4E8`, SHA-256:
`f2ec3d82628422ef69623f94644a1d2b1037a5d8d6374f27655f45765a915b99`.
Direct original execution verifies inputs0,1,7FFFFFFF,80000000,FFFFFFFE,
FFFFFFFF, including unsigned wrapping to zero. The native regression drives
the actual field tick from a guest-RAM valueFFFFFFFF, checks successive RAM
values0 and1, then checks that the skipped-update branch preserves1. The
3E680 reset regression also checks the guest word explicitly.

Release and Debug full builds pass. All **1,397 native tests pass**, with
zero failures/skips, and **10/10 non-route CTest checks pass in62.76seconds**
(native60.78seconds). Logs `/tmp/pe-counter-ram-release-build.log`,
`/tmp/pe-counter-ram-debug-build.log`, `/tmp/pe-counter-ram-ctest.log`.

The M34 history probe now optionally executes the original counter/mailbox
prefix3F4D0..3F4F8. From the old60,197 capture it completes100 calls and
visits11,789 PCs without reaching F434, then fails its reachability assertion.
That run omits input, graphics and outer frame-tail history; it is not a
complete replay and does not prove the missing retained values. Log
`/tmp/pe-m34-counter-history-error.log`. `--watch-entry-sp` now supports
reporting writers at an explicit prospective stack depth without requiring
the initializer to be reached, checking executed source words before reporting.

The fresh ordinary-input replay stops deliberately at frame60,250 with
`stop=frame-limit`, story6C/arrival21/tokenA8003248. It reproduces the three
sewer victories at52,111/53,823/57,791. The complete 2 MB capture
`pc_port/build/day2-victory-evidence/pe-m34-pistol-counter-connected.bin`
has SHA-256 `0a089c25504a18693bda7abe8a66892a930c84d9b09ff4d6af6e2fdabaafaa98`.
Its guest counter is nowEB54. Log `/tmp/pe-m34-pistol-counter-connected.log`.
The pistol children have already retired at this capture; its isolated
original field call checks6,381 PCs and writes none of the six watched words.

A second cold boot captures71 complete present-hook RAM images at60,200
through60,270, then reaches the existing F434 boundary at60,271. The new
optional `PE_ROUTE_RAM_WINDOW`/`PE_ROUTE_RAM_WINDOW_BEGIN`/`PE_ROUTE_RAM_WINDOW_END`
logging only reads RAM. Capture phase is explicitly inside the present hook,
before the remaining frame-tail instructions. No capture is restored into
gameplay. Both optional historical route checks remain50/57.
Log `/tmp/pe-m34-pistol-window-connected.log`. Subsequent Release and Debug
full builds, including the logging addition, pass; logs
`/tmp/pe-counter-window-final-release-build.log` and
`/tmp/pe-counter-window-final-debug-build.log`.

The firing-window writer results are recorded in
[M34_RETAINED_STACK_WRITERS.md](M34_RETAINED_STACK_WRITERS.md).
All build, route and diagnostic handles are terminal. The full Day2 goal
remains active; F434 dispatch still requires faithful stack inputs.
