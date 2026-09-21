# First sewer battle: model fades and ambient reload

The connected cold-boot route remains Day 1 prerequisite work. Full Day 2
completion and whole-route retail fidelity are not established. All traversal,
combat and healing in these probes use pad input; captured RAM is used only
for separate differential tests.

## Fade wait and original translation

`/tmp/pe-sewer-battle-connected.{log,bin}` reached m0027i at frame49176 and
started its first battle at50636. All three enemies waited at script80197FF8,
opcode92 with argument60. The handler waits for embedded model+9C bit4 to
clear. The port's 3AF14 omitted the retail call to3C638, leaving phaseFF and
flags0004 forever. This was a runtime omission, not a script or pad problem.

`game/boot/func_8003C638_port.c` translates the original fade-in3C638,
fade-out3C818, packet transparency3CCB0, texture-page blend mode3CEF8,
color restoration3B708 and height-dependent recoloring3C0B4. The caller3AF14
now performs its original restore flags, fade completion and color branches.
Existing geometry/lighting routines are reused. 3B97C now skips its second
NCCT and A6360 stores when all three triangle color tags are zero, matching
the branch at3BC4C. No fade timer or completion flag is bypassed.

Authority: original `asm/disc1/2A19C.s` and `2CE38.s`; EXE SHA1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

- `pe_model_fade_oracle.py`:125 scenarios /381 original-instruction frames,
  both GPU banks, all four polygon types, tagged/untagged triangles, signed
  phase and scale bytes, both fade directions, restoration and height cuts.
  Generated native comparison passes (`/tmp/pe-model-fade-native2.log`).
- `pe_live_model_compare.py`:195 ticks across actual enemy instances
  800BF490,800BF710,800BF990. All RAM below1FE000 matches retail after each
  tick, including both restoration frames; scratch stack/scratchpad, GTE
  register state and frame timing are excluded. Log `/tmp/pe-live-model-final.log`.
  Input capture SHA256
  da0c4c1f85cdd4f71b703c05be98b4e298436f778562f9c26322a44de963a1e8.
- Two old hand-authored tests needed correction: a constructed-model fixture
  omitted retail3D768's initial phaseFF and treated the subsequent fade-in as
  a second fade-out. It now checks initial visibility, disappearance, return
  to visibility and cleared fade flags through the field loop, stopping
  before animation stages that require additional pose data. Another test
  expected a second NCCT on an untagged triangle that the original skips.
  These corrections follow the original instructions and differential cases.

Reproduce the captured comparison (local capture required):

```sh
python3 pc_port/tools/pe_live_model_compare.py /tmp/pe-sewer-battle-connected.bin \
  --actor 0x800BF490 --actor 0x800BF710 --actor 0x800BF990 \
  --build-dir /tmp/pe-day2-release
```

## Connected fight and music restoration

`/tmp/pe-sewer-fade-connected.{log,bin}` proceeds through the enemy fade,
normal AT/menu/club combat and defeat of all three enemies. Aya has30HP.
At frame51158 it stops at an explicit deferred branch in6D60C state40:
ambient music reload states3E/33. Overlay state is3E, with flags40008004.
No unimplemented weapon or enemy callback stops the fight.

`func_80029810_port.c` now translates states3E/33, the state40 redispatch,
and state32's original elapsed-frame fade timer. It requests the ambient
bank, retries pending loading or a failed playback handle, starts the
60-frame volume ramp, registers a nonzero handle, then restores overlay
flags/state. Its unused loader output uses a saved/restored temporary guest
stack word; it does not persist a fabricated game value.

`pe_ambient_reload_oracle.py` executes the original6D60C with explicit
CD/SPU provider contracts:426 scenarios /788 provider calls. Native test
`ambient-reload-control-flow` passes. This proves the loader's transitions,
arguments, timing arithmetic and return values under those contracts; it
is not a proof of audible playback or provider internals.

The replay with both fixes, `/tmp/pe-sewer-ambient-connected.{log,bin}`,
completes58000frames without a boundary. Aya has30HP, position
(119.3301,773,-3067.3823), flags408, D1A04080, inactive battle mode9,
overlay40000040/F2=0. Story68/persist1=1A; field control is restored.
It uses the same758 prefix pad pairs and the same
input-only m27 controller in `/tmp/pe-sewer-battle-route.c`. The controller
prints `SEWER_FINAL_PAD` changes for later fixed-input regression capture.
The old battle telemetry's enemyhp field selects a type2 trigger in m27;
use the three type3 actor bodies to assess enemy HP instead.

## Fixed recording and next hallway

The first sewer fight is now recorded as961 pad pairs:751 rehearsal-prefix
pairs,5 sewer-travel pairs and205 raw changes during[50500,51200). The new
suffix is in`tests/route_sewer_pads.h`. Canonical endpoint:52000frames,
45milestones, m27PC80199734, liveHP30/status-copy36, three enemy actor slots
with released bodies/tasks and their death flags, restored field/music state.
Full fixed pad SHA256, excluding its trailing newline:
99149fb604b73e56f514f113393f742714111e2779610f6b99a4b4303895b37b.
The first replay reached that state; its assertion for the separate status
copy incorrectly expected30 and has been corrected to the observed36.
The corrected fixed CTest passes134.33s; log`/tmp/pe-sewer-fixed-final-ctest.log`,
capture`/tmp/pe-sewer-fixed-final.bin`.

Appending52000:FFEF,52400:FFFF reaches m0028i at52344. The57000-frame
`/tmp/pe-sewer-next-connected.{log,bin}` completes without an unresolved
boundary. Aya(0,1153,-3455), liveHP30/status-copy36,flags408/task0,
D1A04000; story68/persist1=1B, cameraidentity. The status copy is not
synchronized by this room transfer; no claim is made that it is live HP.
Originalm28script`/tmp/pe-m0028i-probe.txt`: base801A4968, SHA256
c151687a3b08229226eb255dc576bf8fe44b5d1eb22da5b997b8212bbee65df3.

Final validation:1382/1382native tests pass (CTest56.55s), the fixed52,000-frame
route passes134.33s, and the other9CTestchecks pass. All11checks are green
across the final targeted reruns. Logs`/tmp/pe-sewer-native-verified-ctest.log`,
`/tmp/pe-sewer-native-final-lasttest.log`, `/tmp/pe-sewer-fixed-final-ctest.log`
and`/tmp/pe-fade-music-ctest.log`. Original/candidate EXEs retain SHA1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b; the matching plan remains1145spans,
795C/348asm/2rodata, geometry1EE000, plan3a0f11a72cde.

## m0028i actor flag handler

The next encounter starts53073 and stops53078 at opcode13, PC801A8C8C,
actor800C0390(type8). Its operand is04000000. The existing matching
`src/func_800176B8.c` supplies the exact10-word handler: OR the operand into
actor+98 and return1. Native VM dispatch now adapts that existing logic to
guest addresses. No matching source or executable was changed.
`pe_actor_flag_vm_oracle.py` verifies75 complete original/native VM cases,
all five operand modes and the following wait; logs
`/tmp/pe-actor-flag-{oracle,native}.log`.
The input-only replay with this handler is
`/tmp/pe-m28-flags-connected.{log,bin}`. It stops at frame53140, opcode DD
at801A91D8, after completing both enemy fade-in sequences.

After connecting opcode13, all1383native tests pass (CTest57.51s,
`/tmp/pe-m28-flags-native-ctest.log`, native results copied to
`/tmp/pe-m28-flags-native-lasttest.log`). The second-room captured model
comparison also passes195ticks across actors800BFE90,800C0110,800C0390;
all RAM below1FE000 matches after each tick. Log`/tmp/pe-live-m28-model.log`;
capture SHA25618c928feda84df5ac091f4990c6d9825d08b831960a453118b4efdc0fccf964f.
This brings the captured model comparisons to390ticks across both rooms.


## m0028i polar-coordinate handler and movement-effect boundary

Opcode DD is recovered in `src/func_8001A214.c`: all 55 words match the
original with era `-O2 -G0`. The YAML build selects it, and all 796 C spans
pass the strong full sweep. The rebuilt complete EXE remains byte-identical
to retail, SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. All 561
original/native full-VM DD cases pass, covering operand modes, signed angle
wrap, radii, aliased operands and the following wait. The native adaptation
is in `func_8001A15C_port.c`, with VM dispatch in `func_80017018_port.c`.
All 1,384 native tests and all 11 CTest checks pass; the fixed first-sewer
route completes in 143.74 seconds. Both build configurations succeed.
[Detailed proof and workflow limits](../evidence/func-8001A214/REPORT.md).

The next connected replay passes DD and stops at frame 53,141. It reaches
effect 49's untranslated constructor at script 801A938C, then missing
opcode 6C at 801A9450 in the same VM invocation. Aya still has 30 HP.
`/tmp/pe-m28-polar-connected.{log,bin}` retains the same 965 initial pad
pairs and input-only battle controller. No new victory is claimed.

Effect 49 resides in the original m28 C2 overlay, loaded at 8018EFE8 from
Disc 1 LBA15016, 69 sectors. Overlay SHA-256:
`c15d03313a578f7c7ba07f255345df8e17734e38dd807162426d0bc3a7fd7e94`.
The code at 80191514..801931DC matches the captured bytes. Its movement graph
uses constructor8019151C, command801915AC, update801917FC and cleanup801920A0;
it depends on unported DFF80/DFFB8/DFB20/DFB78 helpers. Opcode6C calls the
existing event dispatcher in query mode, passing output addresses.

Another existing defect was identified before translating this graph:
`func_8006F39C_port.c` returns the local index for the small effect pool;
matching retail C returns index+11 for codes46..54. The capture has event
index0 for a slot allocated from the small pool, so this needs correction
and original/native allocation tests along with the next effect work.
Neither this allocation fix nor effect49/opcode6C is implemented yet.
