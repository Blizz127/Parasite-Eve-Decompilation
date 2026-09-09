# M0374I real model and animation continuation

Stage108 extends the stage107 message path using the original type2 model,
clipE and room mesh. Original script SHA-256 remains
`01b08019e76adebf20aeee364cf042b503fa2e4472885824bde7b8098a2cf33d`.
The field chunk occupies125 sectors from disc1 LBA88392.

`pc_port/tools/pe_m0374i_animation_oracle.py` executes the original resource
publication window6B7B0..6B898, complete model initializer3D050 with texture
adjustment disabled, mesh relocation1A918, actual sender54B8, queue drain,
receiver6354 and scene child5D54. Native comparison supplies command pointers
from that same original package directory, assembles the existing native
model initializer, then executes the corresponding native routines. It does
not validate the full room loader or actor constructor.

The published type2 model is8019690C; clipE is801B88C0 with123 frames.
The script's2F request123 clamps to the last valid frame122. Original1A4AC
animation ticks and the actual VM30 wait at801C5D94 run until frame122;
the task then advances to5D9C. Both packet modes and four explicit speeds
(0.5,1,1.5,2 frames/tick) are covered, with244,122,82,61 ticks respectively.
These are injected tick rates, not claims about wall-clock playback timing.
The full original3D834 produces frame0 and frame122 poses, including lighting
and packet generation. Intermediate ticks are executed but intermediate poses
are not produced in this comparison.

On the next VM pass, original opcode0B at5D9C requests position
(0x13A0000,0,0x32D0000). Original1AA78 applies the real mesh's floor height,
producing y=0x530000. The script selects clip0, then yields at dialogue62 poll
at801C5DC0. Dialogue remains open: completion is neither simulated nor claimed.

Eight original/native graphs compare five state digests each, covering actors,
tasks, selected VM/animation/dialogue globals, model packet arena and the whole
original field chunk. Actor/task publication, model allocation, initial storyE4,
speed and lighting are explicit inputs. No GPU submission, texture upload,
full loader/constructor, pre-sender scene, dialogue closure or subsequent E8
progression is proven.

Numeric fixtures: `pc_port/tests/retail_m0374i_animation_cases.h`.
Native check: `pc_port/tests/test_m0374i_animation.h`.
Original evidence: `local/live/m0374i-animation-108.json`.
Normal and ASan/UBSan focused tests each pass all8 cases in one test group
(1298 skipped). Separate sound-focused runs pass the existing512 consumer
cases and80 registration cases in two groups (1297 skipped). Final full CTest passes8/8 in77.87s; the native app is rebuilt. Both original
fixture regeneration checks, Python and whitespace checks pass. Results are
also recorded in ACTIVE_HANDOFF.md.


## Restored animation sound registration

Scene-entry inspection found that EA406 (`0x196`) and EA407 (`0x197`) both
select original16794..16870, while the native15DAC default handler silently
returned without their effects. The original block appends one8-byte record
at944A8+count*8 when byteB0CE9 is below16: actor type/subtype, command byte,
frame byte and two16-bit sound IDs. EA406 repeats arg3 for both sound banks;
EA407 takes arg4 for the second bank. At capacity it changes nothing. The
native handler now reproduces these stores and the bounded count update.

M0374I module2 has two real EA406 records at5840 and5860: command1, frames0
and18, soundIDs3F8 and3F9 respectively (same in both banks). The real-model
comparison now executes those original registrations and compares the native
results too. ClipE does not cross those command1 events, so the scene fixture
does not establish audible playback of those sounds.

`pe_animation_sound_register_oracle.py` adds80 original registration-to-6A318
sound-scan/projection/FIFO graphs. Cases cover both keys/banks, counts0/1/15/
16/255, truncated command/frame/sound arguments, and zero sound IDs. The sound
bank and projection state in these80 cases are synthetic. Each compares
registration state and post-playback state separately. Numeric expectations
are inretail_animation_sound_register_cases.h; native coverage is
`test_animation_sound_register.h`. This restores missing shared behavior;
no full-Day1 or full-Day2 audio acceptance claim follows from these fixtures.
