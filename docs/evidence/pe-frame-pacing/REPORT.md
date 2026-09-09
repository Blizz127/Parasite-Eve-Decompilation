# PACE1: normal speed and fast-forward

Normal visible presentation now uses monotonic deadlines at60000/1001 Hz.
F6 toggles uncapped fast-forward and updates the window title. Switching modes
resets the clock; a long pause cannot accumulate a burst of catch-up frames.
Input is polled during waits. Deterministic runtime tests never sleep.

Release Linux build, visible X11 windows, default scale 2, `--skip-movie` and
`--max-frames`; wall times include startup and shutdown:

| Frames | Mode | Wall time |
| --- | --- | --- |
|300|Normal|5.370s|
|600|Normal|10.137s|
|600|Fast-forward|5.968s|

The 600-frame normal target is 10.010s before startup/shutdown. Under CPU load,
rendering can still take longer than a deadline. Fast-forward speed is CPU
limited. See timing.json for exact measurements. Earlier scale 3 Debug runs
with two older game processes active were not comparable to the shipped build.
Those completed processes were paused before the final Release measurements.

Physical F6 (keycode 72) visibly switched Normal → Fast-forward → Normal. Holding the
second press for 2 seconds produced one toggle; title and log remained Normal.
See toggle.json. Local xdotool sent Alt forF6, so the verification used ordinary
XTest key events instead. This was a test-tool issue, not a game input failure.
The live updated runtime is left open at normal speed.

The host keyboard source also now publishes status 00 / ID 41 along with the button
word, following the [BIOS controller-buffer format](https://psx-spx.consoledev.net/kernelbios/).
A regression check starts with a stale analog header, presses/releases a
host direction, and verifies the digital header and idle button/edge state.
Window focus loss clears held keys. The user confirmed movement/drift is fine;
this does not claim the old header caused drift. The two inspected movement
branches treat both zero and 4100 as non-analog.

Full normal and ASan/UBSan CTest pass 2/2 each, 1,187 native groups:
/tmp/pe-pace1-tests.log (59.87s), /tmp/pe-pace1-san-tests.log (71.97s).
The clock regression verifies 300/600-frame durations with changing render cost
and recovery from a 5-second pause. Windows has the equivalent QPC/waitable-timer
backend but was not compiled or run on this Linux host.

The R2 package contains the exact tested Release binary and records
sourceDirty=true alongside its source commit. Published 2026-09-05T18:39Z as
build PE-PACE1-2c968d754c54 (package 290631257 B, sha256
c3ec4bb11dab4eb6e29afb460ebb90360e21ade14f6cd73eea321326caa86bfe). The publish
script verified the full remote archive SHA-256 by readback, a ranged signed
GET, and both the pinned and mutable `channels/dev.json` documents. A later
read-only `rclone` re-check confirmed `dev.json` names this build and the
package is listed at the recorded size. The previous head PE-INV17-7821e7c3
remains available for rollback. Broader retail fidelity work remains active;
Scan/notice updates and movie decoding were not part of this speed change.
