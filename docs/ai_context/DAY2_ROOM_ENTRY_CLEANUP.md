# Retail room-entry cleanup and connected hallway traversal

The native destination-ready path now calls the existing
`func_8003F074_after_poll_cut` continuation. Its previous manual tail omitted
the effect cleanup, room counter resets, display synchronization and flag
resets present in original `8003F23C..8003F2F8`.

The continuation calls `1A918`, `371B0`, `125E0`, and `E0060`, clears
`8009CDA4` and the halfword at `800942EC`, synchronizes and enables display,
then clears D1A0 bit `40`, input bits `C`, and overlay bits `402`. The effect
cleanup walks occupied slots backward with stride `14`, preserving other
bytes and the neighboring halfword of its signed count.

`pc_port/tools/pe_room_entry_cleanup_oracle.py --check` executes 30 original
region pairs: `8003F284..8003F298` including the complete `E0060` call, then
`8003F2A8..8003F2E8`. Six flag patterns and counts -1, 0, 1, 2 and 5 exercise
empty slots, signed counts, preserved bits and neighboring halfwords.
`RCLEAN_retail_room_cleanup` compares native guest-byte fingerprints with
these original outputs. The existing BTL90 destination-ready integration
test also asserts the omitted resets through the live caller. Both pass.
Full CTest passes **10/10**, including **1380/1380 native checks**, zero skips
(`/tmp/pe-room-cleanup-ctest.log`). That full run used the prior 17500-frame
route; the final **22500-frame Release route CTest passes** in 76.85s
(`/tmp/pe-m0020-route-ctest.log`). Both route binaries were rebuilt.
This oracle covers these persistent cleanup effects; it does not execute
the whole loader, actor spawning or display-sync calls. Other documented
loader cuts remain, including `6BD68` and `3F758`.

Connected cold-boot input traversal now observes:

| Frame | Original script result |
|---:|---|
| 17809 | m0011i → m0012i, token `A8001148` |
| 17810 | story `39` |
| 18882 | m0012i → m0013i, token `A80011C8`, story `40` |
| 19366 | return to m0012i |
| 19457 | story `48` |
| 20434 | enter m0020i, token `A8002048` |

After the previous m0011i input prefix, FF7F at 17500 centers Aya before
FFEF at 17580 approaches the exit. In m0012i, FF7F at 20000, FFEF at 20110,
FF7F at 20350 and FFFF at 20400 enter the open m0020i door. Periodic Cross
continues throughout. No positions, persistence words, inventory, commands
or save state are injected. Exploratory logs are
`/tmp/pe-room-cleanup-route.log` and `/tmp/pe-m0020-approach.log`; their old
m0011i frontier assertion reports the intentional advance as an exit-1
failure, independently of the successful room transitions.

The default harness now requires **29 milestones**, token `A8002048`,
module 4 PC `801A1E98`, story 48, persist[1]=D, Aya present and cleared battle
state at 22500 frames. The first key has not yet been collected.

The full retail candidate remains byte-identical to the extracted original:
SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. No matching source or manifest
was changed for this native fix. This is still Day 1. Four documented
HOST_ADAPTED movie/menu skip functions remain; full start-to-end Day 2,
hardware rendering, audio and timing fidelity are not established.

Reproduction (local retail assets and configured build directories required):

```sh
python3 pc_port/tools/pe_room_entry_cleanup_oracle.py --check
source /tmp/pe-tools/env.sh
ctest --test-dir pc_port/build --output-on-failure
ctest --test-dir /tmp/pe-day2-release -R '^route-boot-day2-control-flow$' --output-on-failure
cmp build/extracted/disc1/SLUS_006.62 build/disc1.candidate.exe
sha1sum build/extracted/disc1/SLUS_006.62 build/disc1.candidate.exe
```

The full-suite log above predates the route-only extension; its native and
production code is current. The separate final Release check covers the
extended route. Further key-approach probes finish without a runtime error
but have not set the first-key flag; this is the next traversal frontier.
