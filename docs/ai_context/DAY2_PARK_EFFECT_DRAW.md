# Park effect update and packet drawing

Stage115 replaces the nonempty E01BC no-op in native func_8003F3C4_port.c
with the original record walk and both effect update paths. This completes
the update/draw graph used by the ED2400/2402 allocations restored in114.
The original executable SHA-1 is
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

E01BC..E026C walks backward through20-byte slots, skips inactive slots,
counts active unsupported modes, and reloads the signed active count after
each slot. Retirement can therefore shorten the current walk; the native
implementation preserves this behavior. Mode0 E026C..E03A0 decrements its
byte timer and advances UV frames by16, retiring after frame48. Mode1
E03A0..E051C computes colors from the previous signed intensity, then raises
or lowers intensity by the timing byte. Rising intensity clamps at255 and
switches state; falling intensity retires when its signed half becomes
negative. Both decrement the global count when retiring.

E051C..E0808 loads the camera rotation/translation into GTE, projects the
position using RTPS, allocates a40-byte semi-transparent textured quad and
links it into the selected depth ordering table. The size computation
preserves low32-bit multiplication and signed division. Original division
traps request an explicit native stop rather than executing undefined host
division. Valid test fixtures do not exercise those traps.

`pe_park_effect_tick_oracle.py` executes the complete original E01BC graph,
including both update leaves and packet drawing, without callee substitution.
Its192 configurations cover both packet banks and effect modes, four state
bytes, four intensity/frame values, and three speeds. Pools also contain
inactive holes and an active unsupported mode. Eight calls per configuration
give1536 frame checkpoints, comparing record and global state, packet arenas,
ordering tables, camera inputs and scratchpad bytes0..37. Camera, projection
and pools are explicit synthetic fixtures. Generated headers store numeric
seeds/fingerprints only. Normal and ASan/UBSan pass all checkpoints in one
focused group, with1306 other groups skipped. Fixture regeneration passes.

This proves the translated graph under those fixtures, not a rendered GPU
image, a full original/native M0059I scene, camera extremes, malformed pool
termination, or complete Day1/Day2 effects coverage. Those acceptance tasks
remain open. Stage114's statement that nonempty effect update/draw is unported
is superseded by this change.

Full CTest passes8/8 in83.19s. Native app and sanitizer tests rebuilt without
compiler warnings. Python and whitespace checks pass.
