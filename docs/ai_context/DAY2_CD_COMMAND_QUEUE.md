# SDK command queue and completion polling

Stage155 ports the original145-word7EE84 enqueue graph and124-word7F418
completion-poll graph, plus7FC64 status forwarding,80998 response copying and
the80DC4 blocking wrapper. With the CD device enabled,80D5C uses that same
original wrapper structure. The disabled-device Setloc accommodation remains
for existing callers; it is not evidence of complete command/controller fidelity.

The enqueue routine checks the per-command word table at9B4BC using the low
command byte. A nonzero entry and non-null parameter require a Setloc descriptor
before the requested descriptor. Each descriptor gets a separately incremented
sequence at9B53C, skipping zero on wrap. Capacity is checked separately: if only
one slot remains, Setloc can be queued while the requested command returns0.
The code preserves stale parameter bytes on the null-parameter branch, copies
four bytes on the supplied branch, records the original pointer, and only puts
the extra/callback words on the requested command. If the SDK is ready and this
sequence is the current head, the actual7E8F4 dispatcher is called.

Completion polling first runs7FC64(0)→7B010(1,0) for a nonzero sequence. That
existing SDK poll provides device-clock/IRQ service when enabled. It scans the
completion ring forward fromA3690; absent sequences use the original signed
comparison against the starting record. An unresolved pending sequence returns0.
Eligible sequences are searched backward for their latest matching record;
sequence0 selects the immediately preceding record. Missing history returns6.

A selected16-byte record is copied toA3500. The first three words are loaded
before stores; the fourth is loaded afterward. Eight bytes fromA3505 are then
copied forward into the caller's non-null response buffer, and the live byte
A3504 is returned. Aliasing the response buffer with the snapshot can therefore
change the returned status. 80998 also supports its original null-source branch,
which clears just one destination byte. No memset/memmove substitution is used.

The blocking wrapper masks the command to8 bits, enqueues with zero extra and
callback words, returns0 on failed enqueue, and polls until a nonzero low status
byte appears. It returns1 only for status2. The enabled path gets status and
response bytes from actual modeled device interrupts and the existing SDK
completion machinery. It does not fabricate responses or successful completion.
If pending work lacks an enabled device, or exceeds a host watchdog of100001hex
polls, it stops at explicit CD_command_wait. This is a host unsupported-wait
boundary, not a retail timeout or hardware cycle-fidelity claim.

`pe_cd_command_queue_oracle.py` authenticates the original executable, executes
96 enqueue graphs with original allocator/copy/getter calls and an explicit
7E8F4 dispatch provider, and324 poll graphs with an explicit7FC64 status provider.
It varies prefix requirement, parameter presence, zero/full/nearly-full queue,
sequence wrap, head index, SDK lane, ring direction, missing/duplicate history,
signed sequence edges, saved status and output aliasing. Native tests compare
original RAM/result hashes while executing real helpers; isolated enqueue
cases leave the hardware-init gate closed so dispatch has no device side effect.

A separate integration starts the mounted fixture through the actual public
SDK initializer and then executes queued Setloc with a non-null response and
queued Pause. It checks returned status, response bounds, queue retirement and
ready state. An invalid-BCD Setloc then returns0 with the device response
03,10hex, retires the queue, and does not hit an unresolved native boundary.
This closes the response-buffer rejection in the enabled Setloc
path needed by the movie updater. It does not prove the whole updater/player,
XA handling, or measured hardware timing. Remaining opening-through-Day2 scope
stays active, including121C04/122040/14E30 integration and pixel fidelity.

Validation: oracle regeneration, Python compilation and whitespace checks pass.
Normal and ASan/UBSan builds are warning-free. Both focused runs pass47 groups
with1297 skipped. After adding the device-error case, both rebuilt binaries pass
the updated group (1 pass/1343 skipped). Full normal CTest passes8/8 in143.02s,
including1344/1344 native groups with0 skipped. All stage155 jobs completed.
Logs: `local/live/*day2-155*.log`.

Follow-up game-loop evidence: original3F520 calls122040; a nonzero low byte
branches to the frame tail3F590. A zero low byte calls121A00 restoration and
then6E60C before jumping to3F678, bypassing the normal after-draw tail. That
6E60C dependency must also be translated before the loop can be wired fully.
