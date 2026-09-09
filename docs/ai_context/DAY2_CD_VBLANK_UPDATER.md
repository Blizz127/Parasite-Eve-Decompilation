# CD VBlank startup and retry updater

Stage144 translates7FE24,800F4 and7F7E8 in `pc_port/game/boot/cd_stream_port.c`.
The updater decrements positive pending/delay counters, immediately retries
when the pending counter reaches zero, advances SDK startup/read/play states,
issues internal commands, invokes the enabled queue callback and polls idle
or error lanes. It retains signed gates, wrapped counters, command-kind stores
and the original ordering between command issue and later state changes.

800F4 resets the controller, increments the retry counter, reloads the pending
deadline to30 or960 from the command table, and reissues the saved command and
parameter pointer asynchronously. Existing7FCFC now propagates callee stops
before its later state publication.

Original executable SHA1:452fb033f2eaa4b18aa20a5bca60b8125af3a37b.

| Original span | Words | SHA256 |
| --- | ---: | --- |
| 7FE24..800F4 | 180 | 7df21a00b56e81e4d4a811bd38215db6ff65f4246173802e2e0beb262734fabd |
| 800F4..80164 | 28 | 682f6d2727c72b324641b5c066ddccd21071ca4a538717f8fc3415bdec737841 |
| 7F7E8..7F88C | 41 | 9f8735562027f76b4254a9497af42993ce833eb29bd517dea77b0d0d5868c3b0 |

The known7F7E8 queue callback checks readiness twice, requires a positive
queue count/nonzero current record, and invokes the original7FB44 issue path.
Unknown no-argument callback identities stop before the updater suffix.

The SetMode recovery arm writes only byte0 of an original four-byte stack
local. Native scratch801FFE90..93 retains this local; tests seed the matching
original1FEFF0..F3 word12345678 so the copied trailing bytes remain observable.
This is an explicit stack-address adaptation, not a claim about arbitrary
stack residue or nested/reentrant updater calls.

`pe_cd_vblank_oracle.py` executes original command reset/issue/retry paths,
with onlyVSync(-1) supplied as0. CD MMIO is redirected to RAM. Native tests
register a7FE24 binding in an actual VBlank slot and call the checked dispatcher;
the counter wraps to0, matching the original provider query. They compare SDK
state, pending/status bytes, issuer poll state and actual native CD-register
contents. Device response delivery is not supplied or inferred: commands issue
asynchronously, and successful physical completion remains unfinished.

Final oracle coverage is3360 original graphs/prefixes:2856 complete and504
unknown callback prefixes. Cases vary lanes1/2/3, states11..17, phases21..24,
negative/zero/expiring/pending counters, status/delay flags, callback enable
gates, null/unknown/known callbacks and empty/nonempty queues. Real asynchronous
command writes and both deadline-table choices execute in the original graph.

Automatic installation by outer7F994 and startup/playthrough integration remain
open. Physical sector FIFO/DMA, MDEC pixels/output/IRQ and movie player/updater/
loader integration are unfinished. Runtime128 remains published.

Normal and ASan/UBSan focused runs each pass37 DAY2 groups with1297 skipped.
Final builds are warning-free; original fixture regeneration/check, Python
compilation and scoped whitespace pass.
Full CTest passes8/8 in169.82s, including1334/1334 native groups with0 skipped;
log `local/live/ctest-day2-144.log`.
