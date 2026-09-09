# Music-start command consumers

Stage128 follows the restored SPU mode switch into its next opening boundary:
command10 dispatched to unported8008AE94. Native now translates consumers
8AE94 (fresh or cached primary music),8B040 (the same plus count-1 at9D22C),
and8AFB8 (preserve an active primary song in the secondary slot when empty,
then start the new primary song). The existing instrument setup code is shared
with fresh music initialization without changing effect-voice initialization.

The implementation includes original8A068's24-voice initialization, active
and paused masks, stream offsets, instrument defaults and pending updates;
8AC40 restores cached state/voices, relocates stream pointers, adjusts voice
counters and preserves the original pause behavior. 89FE0's physical-voice
mask query and8D820's forward byte copy are transcribed for these graphs.

pe_music_consumer_oracle.py pins the original executable and executes full
8CA84 consumer calls, including their CPU callees, without intercepted
providers. Its72 cases cover three commands, fresh/cached selection, pause
state, and six channel masks including zero/all/alternating/highest voice.
Guest state, both voice banks, instrument data, command data and globals are
compared. This does not implement score interpretation or audible synthesis.

The first consumer build had a numeric-fixture prefix collision with the older
message-choice fixture; the music-consumer fixture now uses MUSICCON_. No
runtime behavior was changed to resolve that build error. Validation and the
opening-test result are recorded in ACTIVE_HANDOFF.md. The Banshee update
remains pending until rebuilt packages pass the required startup checks.

Validation for release 128: focused normal/ASan comparisons and opening pass;
full normal CTest 8/8 passes. Linux and Windows Release startup reach the
120-frame limit (Windows under Wine). Both archive inventories, binary/disc
hashes and packaged Linux startup pass. Publication evidence is recorded in
ACTIVE_HANDOFF.md and local/live/publish-day2-128.
