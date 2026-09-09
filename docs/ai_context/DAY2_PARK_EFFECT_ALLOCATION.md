# Park effect color and allocation

Stage114 restores ED2401, ED2400 and ED2402 in native16910. M0059I uses
ED2401 at8019FDC4 with RGB192,8,8, then ED2402 at8019FDEC with position
(-1488,-350,-3588), parameter30 and timing byte10. These calls previously
returned success without setting colors or creating an effect record.

Original executable SHA-1:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
ED2401 stores the low byte of each color argument at8009CDF8..FA.
ED2400/2402 use the signed high halves of the three position arguments and
call EXE-resident E00CC..E01BC with modes0/1 respectively. Mode0 supplies
zero colors; mode1 reads the stored colors. The native allocation helper
preserves the original backward20-byte slot search, signed16-bit scan/count
comparisons, fallback to the head, field truncations and count increment
only while the signed count is below20. It does not impose a new pool limit.
A guest scratch vector at80122380 substitutes for the original stack local;
that temporary storage is outside the compared persistent memory ranges.

`pe_script_effect_spawn_oracle.py` executes full original16910 color and
allocation calls, with no callee boundary substitution. Its336 cases cover
seven signed count values, six hole/full-pool layouts, both allocation modes,
and four argument variants including truncation and sign boundaries. The
native test compares color state and then allocation state independently,
including the pool, count, head pointer and argument memory. Generated
fixtures contain numeric seeds and fingerprints, not executable assets.
Normal and ASan/UBSan each pass the focused group (1305 other groups skipped).
Original fixture regeneration also passes.

This restores allocation only. Native E01BC in func_8003F3C4_port.c still
skips the nonempty record walk. Original E01BC dispatches active mode0 records
to E026C and mode1 to E03A0; both use E051C..E0808 for projected textured-quad
packet emission. E03A0 updates a rising/falling halfword intensity, retires
records and decrements E21A4. E051C performs RTPS, builds a40-byte packet,
and links it into the depth ordering table. These bodies need translation
and original-graph verification next; visible effects and full M0059I
scene playback are not established by allocation tests.

Full CTest passes8/8 in82.97s. Normal app and sanitizer tests rebuilt without
compiler warnings; Python and whitespace checks pass.

Update: Stage115 restores the previously missing nonempty walk and both
update/draw paths. See [DAY2_PARK_EFFECT_DRAW.md](DAY2_PARK_EFFECT_DRAW.md)
for the complete-original comparison scope and remaining acceptance gaps.
