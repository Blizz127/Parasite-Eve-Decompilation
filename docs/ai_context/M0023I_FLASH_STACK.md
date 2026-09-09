# M0023I flash: borrowed stack position

The native `PE_M0023I_Flash` translates original `8018F710..8018FC14`
(321 words). Its retained-position parameter represents the six bytes at
the original callback's `sp+0x30`. It is both an input and an output.
The production69594 pump now binds this input by tracking the original
writers. Unknown contexts still stop explicitly on mode2/state0 when that
input is unavailable.

## Original instructions

The state0 call at `8018F8DC` passes `sp+0x30` to `800D0728` before any
instruction in F710 initializes the vector. Later, `8018FA50` calls
`800CE8F0` with that same output address. A second F710 call at the same
stack depth therefore sees the first call's joint position unless another
routine has overwritten it.

Original code SHA256:
`6bee6882d0a259e2d326e4bd70941ff28d6622266e24d4f098dcb6cf5cc228aa`.
The oracle pins the full executable SHA1 and extracts the M0023I overlay
from Disc1 LBA13904,172 sectors. No native callback formulas replace
original instructions in the oracle.

## Effect-pump trace

Offsets below are relative to SP on entry to original `80069594`:

| Path or write | Offset | Consequence |
| --- | --- | --- |
| 69594 → 6F8EC → D4704 → F710 | -0xD8 | F710 frame base |
| F710 borrowed vector | -0xA8 through -0xA3 | Three signed halfwords |
| D4704 → CE78C → F004, instruction8018F014 | -0xA8 | Saves RA800CE818, producing X=-6120,Y=-32756 |
| 69594 → 6F9F0 → D413C → F3C8 → CE8F0, instruction800CE934 | -0xA4 | Stores zero across Z and the following padding |
| First F710 → CE8F0, instructions800CE988/9A4/9C4 | -0xA8/-0xA6/-0xA4 | Writes all three joint-position halfwords before the second flash |

Original script80190670 creates two particle callbacks, two flashes and
two beams, changing descriptor kind between each pair. Tracing the original
effect pump on a copy of run24 RAM observed both flashes on fixture frames
13 through20. With initial stack fills00 andA5, the first flash read
(-6120,-32756,0); the second read (647,-483,1328), the first flash's joint
position in that copied actor pose. The six last-writer addresses were
recorded for every call. A direct D4704 call with no prior update instead
retains arbitrary Z, so these values must not be generalized into a constant.

Evidence in ignored `local/live/`:
`m0023i_stack_writers.py`, `m0023i-stack-writers.json`,
`m0023i_frame_stack_pairs.py`, `m0023i-frame-stack-pairs.json` and its log.
The copy probe reverses the stopped allocation's bookkeeping and executes
the complete original allocation before the pump. Projection controls and
per-invocation PRNG seeds are fixture inputs; actor poses do not advance.
No gameplay RAM is modified.

## Verified callback and remaining integration

`python3 pc_port/tools/pe_m0023i_flash_oracle.py --write-header` generates
173 complete original callback cases, including12 paired calls. Native
comparisons include the six retained bytes, all persistent effect state,
sound request state, and defined GPU packet fields. Only unused GPU padding
and fields not written in unlinked packets are masked. There is no mask for
the borrowed vector or its defined projected coordinates.

DAY1-7 extends observation through original35558, with moving actor poses
and paused frames. Test-interpreter SQR, shifted OP and supported-command
flags follow the [PSX-SPX GTE reference](https://psx-spx.consoledev.net/geometrytransformationenginegte/).
`python3 pc_port/tools/test_gte_oracle.py` passes17 ISA checks; strict flag
reads reject unimplemented results. Existing173 flash and187 beam original
return/hash pairs remain unchanged. This interpreter is test tooling only.

The wider trace confirms the same writer identities. With update paused on
fixture frames14..16, the first flash's Z on frames15..17 is1755, written
by the previous second flash's CE9C4. Its next normal update restores zero.
The second flash reads the first flash's moving joint position. Evidence:
`local/live/m0023i-field-stack-pairs-flags.log`,
`m0023i-field-stack-pairs.json`, `m0023i-field-stack-paused.log` and
`m0023i-field-stack-paused.json`. Values are specific to the copied scene;
the native implementation tracks writes instead of using those tuples.

The scheduler keeps six host bytes and a validity mask, scoped to the69594
pump. F3C8 updates establish Z; particle draws establish X/Y; flashes replace
the whole vector. Unknown callback graphs and RAM generation changes
invalidate the context. A fresh pump without the prior update remains an
explicit boundary. This is scoped stack behavior, not a guest CPU emulator.

`pe_m0023i_pump_oracle.py --write-header` validates84 original/native frames
across six histories, both GPU banks, two particle pools, paired flashes,
pause and resume. The fixture retains effect state and CPU stack between
frames and provides fresh GPU output arenas/OT. This prevents unused packet
padding from a longer prior frame being mistaken for persistent state.
All current defined geometry and preallocated bytes remain compared.

A separate64-frame copy comparison uses the actual run24 script, actor pose
and meshes, through the beam phase and with pause/resume. Every defined
non-stack byte matches after every pump. Projection, PRNG seeds and fresh
GPU output arenas are explicit fixture inputs; no gameplay RAM is changed.
`local/live/m0023i-pump-live-comparison.json` records64 zero-difference rows.
Normal CTest4/4 passes52.84s; sanitizer CTest4/4 passes58.78s
(1215 native groups). Live encounter validation remains necessary.

The earlier arbitrary-position probes changed defined vertex/ordering bytes;
visible pixel differences have not been established. This limitation does
not justify dropping the original ring or masking its coordinates.

## DAY1-11: concurrent pistol draws

Run27 reached the rehearsal battle and stopped at frame48900: the first
flash's borrowed Z had been invalidated by the preceding weapon slots.
Full original69594 execution on copied stop RAM, with initial stack fills
00 andA5, gives (-6120,-32756,640). Its last writers are F014 for X/Y and
C9EF8 for Z. C9EF8 stores the casing's signed-byte age times64; this reached
casing has age10. The value is not a constant or an actor position.

At the69594→6F8EC→C9B68→C2414 draw depth, C9EA8's sp+30/+32/+34
coincide with the borrowed vector. C9ED8/DC write zero to X/Y and C9EF8
writes the rotation to Z. C9FD8 instead copies C21BC's word and C21C0's
word into sp+78/+7C at CA444/450, overlapping the same six bytes.
Empty C9EA0/CDD04 callbacks and empty C2414 tables preserve those bytes.
Other descriptor callbacks still invalidate the context. This tracking only
applies within the corresponding69594 weapon-draw wrapper.

Original instruction traces with a deliberately nonzero C21C0 fixture word
prove muzzle Z=-1234; swapping casing/muzzle record order changes the final
writer and Z. Both stack fills agree. Evidence:
`local/live/m0023i-weapon-stack-writers.{json,log}` and
`m0023i_weapon_stack_writers.py`; actual run27 writer proof is
`m0023i-run27-stack-writers.json` and `m0023i_run27_stack.py`.

The pump oracle now covers644 frames in46 histories: both GPU banks,
signed casing ages0/1/10/127/128/255, casing/muzzle order, repeated casings,
empty and active empty hit callbacks, weapons before/after the room slot,
and pause/resume. Mixed fixtures assign the room slot code55 so the original
100 flag can hold weapon updates while the room VM advances. Real original
callbacks and pump instructions run throughout; no callee is substituted.
Eight histories remove all particles, verifying that all six weapon-written
bytes reach the flash. Every history must reach both flashes by frame2. Native normal and sanitizer
comparisons pass (`native{-asan,}-day1-11-644.log`); the fresh-RAM unknown-context stop remains tested.

The actual run27 copy additionally runs64 complete pumps with real weapon
updates, through casing expiry, flashes, beams and pause/resume. All defined
non-stack bytes match after every frame. GPU packet padding/unwritten fields
use the same existing masks; no defined coordinates are masked. Inputs are
captured RAM, explicit projection/PRNG state, and fresh GPU arenas/OT per
frame. No live gameplay RAM is changed. Evidence:
`local/live/m0023i-pump-run27-comparison.{json,log}` and its source probes.
Fresh live run28 passed the flash boundary and stopped at VM36/13E84
onframe49014, HP4 alive. Battle victory remains unverified. Screenshots and
provenance: `docs/evidence/pe-day1-flash-weapon/`. FullDay1 remains unfinished.

Full suites used532frames: normal5/5 PASS67.46s, sanitizer5/5 PASS87.15s.
Final644frames additionally pass focused normal/sanitizer checks. Windows
runtime crossbuild passes; execution remains untested. Production code did
not change during the final no-particle fixture expansion.

An additional run27 copied-state attempt through the wider35558 field call
was rejected by the strict test interpreter on unsupported NCCT flag command
118043F before reaching the flash. This is not accepted comparison evidence;
strict checking was retained. Log `m0023i-run27-field-stack-writers.log`.
The64-frame pump comparisons and prior run24 field traces remain the scope
of the completed original-execution evidence.
