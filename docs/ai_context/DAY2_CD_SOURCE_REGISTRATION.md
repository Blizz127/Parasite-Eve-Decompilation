# CD CPU interrupt registration

Stage142 fixes a missing dependency discovered while tracing7BBFC startup:
`func_800740D0` in `pc_port/platform/pe_libetc.c` rejected source2 before
performing any registration. The existing73CC4 wrapper therefore could not
register the translated7C13C CD handler. Source2 is now accepted alongside
previously supported sources0 and3.

The original worker confirms source2 needs no BIOS side calls. It returns the
previous callback immediately when unchanged or when ResetCallback's guard is
zero. Otherwise it disables I_MASK, updates the callback slot and registered
mask, and restores I_MASK with bit2 added or removed. Source0's BIOS behavior
and unsupported-source boundaries retain their existing scope.

Original executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 73CC4..73CF4 | 12 | ff71c9ce2b4ad8e8d5afb17da0e186ee953dabddd306f06499c40fc17b3c879e |
| 740D0..74218 | 82 | a6991559f3a0292a07423fdaf10d16d6da15eba33543f02dd1e1041aa5715332 |

`python3 pc_port/tools/pe_cd_registration_oracle.py --check` executes270
complete original installed-wrapper/worker graphs. Cases cover guard0/1,
null/same/replacement callbacks, source-bit and unrelated mask bits, and full
16-bit masks. Original I_MASK is redirected to RAM; no registration result
or guest-state mutation is supplied by a provider. Native tests compare the
entire50-byte guard/callback/registered-mask region, previous-handler return
and I_MASK. The canonical installed SDK table is assumed, as in the existing
native73CC4 wrapper; dirty/pre-install indirect-table behavior is not covered.

The52 CD interrupt integration cases now call73CC4(2,7C13C) instead of planting
the callback slot and registered mask. They verify the resulting slot/masks
before asserting source2 and entering native CPU IRQ service. ResetCallback's
initialized guard and device response ingress are still test setup. This
establishes real registration-to-delivery integration, not automatic startup
registration or physical command/sector production.

This corrects the earlier handoff's implication that the registration helper
already supported CD. Initialization7BBFC must still restore diagnostics,
ResetCallback, registration, pending-tag clearing and Nop/Init/Demute commands
in original order. Physical disc responses/sector FIFO/DMA, MDEC output and
movie player/updater/loader remain unfinished, as does opening-through-Day2
acceptance. Runtime128 remains published.

Normal and ASan/UBSan focused runs each pass35 DAY2 groups with1297 skipped.
Both builds are warning-free; fixture regeneration/check, Python compilation
and scoped whitespace checks pass.
Full CTest passes8/8 in118.29s, including1332/1332 native groups with0 skipped;
log `local/live/ctest-day2-142.log`.
