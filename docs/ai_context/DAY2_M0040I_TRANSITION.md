# M0040I choice, fade and music-start continuation

Stage125 extends actual M0040I message1, selection1 and confirmation through
its60-tick fade, EA314, story D8 assignment, persistent flag updates, EA200
music14 request, previous-room value999 and destination token A8000048.
The module ends this component at taskPC801C9FA4. Script and executable
pins are shared with DAY2_M0040I_CHOICE.md.

The room's actual music directory contains one record: music14/bank14,
1896 bytes at chunk offset440A0 (guest801D3088). Its AKAO header carries
song ID11 and SPU mode4; the directory music ID14 is a different field. The component explicitly
seeds that music as cached in slot0/1 and its requested SPU mode as already
configured. The original loader still reads the real package directory;
no invalid music header or empty package substitutes for the actual request.
There is no CD/SPU transfer or next-room load in this fixture.

The initial original probe with the real package and no cache entered the
blocking loader. A cached-bank probe without configured SPU state reached
BIOS B0 service0A through8CF70. Those probes do not establish completion;
they identify the loader and mode-change work still required. Their logs
remain under local/live. The checked fixture sets the explicit cached/mode
preconditions and executes full original calls without intercepted callees.

Four runs cover both render/cache slots and confirmation delays0/4,
producing264 VM/render/fade checkpoints. The first native comparison failed
on the final frame: native8CBA8 validated the real AKAO header but omitted
the playback command. Stage125 restores original8CC68..8CD04 behavior for
commands10/12/19 when the audio mode is already configured. If the song ID
already matches stream-state+54, return0 without queueing. Otherwise queue
payload+16 and the song ID, plus CD88 for command12, and return that ID.
A required SPU mode change now raises the explicit unported boundary;
8CB54/8CF70 and their hardware/event graph still require implementation.

pe_music_payload_oracle.py separately executes36 complete original command
graphs across commands10/12/19, song IDs0/65535, same/different current IDs
and queue indices0/7/15. Its native test also checks that an unported mode
change cannot silently queue playback. These are command-state checks,
not score sequencing or audible synthesis acceptance.

Validation results are recorded in ACTIVE_HANDOFF.md and local/live logs.
Full Day1/Day2 scene, battle, presentation and release acceptance remains open.

The first full regression exposed BTL94's isolated M0367I fixture: it bypassed
streaming bring-up and had no9D2C8 state pointer. The fixture now supplies
B6980 and configured mode4, matching all three original M0367I music headers
(directory IDs34/35/36). Its existing persistence and actor assertions remain.

Final validation is partial:264 transition checkpoints and36 command graphs
pass normal and ASan/UBSan; corrected BTL94 also passes both. Builds and
fixture checks pass. Full CTest rerun fails1/8: SKIP2 opening cannot allocate
Aya because its song2 requests SPU mode5 while configured mode is0. GDB
confirmed payload800F4CF8 and saved boundary RAM under local/live. This is a
real opening regression exposed by the explicit boundary, not an accepted
fixture exception. Implementing the SPU mode-change graph is the next priority;
stage125 is not release-ready. Full log:ctest-day2-125-final.log (118.28s).
