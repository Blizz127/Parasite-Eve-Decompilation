# Day 2 shared-script audio dispatch

The native `func_80015DAC_default_cut` previously returned success without
performing six real EA commands. It now executes their retail branches:

| EA key | Original branch | Restored behavior |
| --- | --- | --- |
| 301 | `800162F0` | Stop an effect through `800866A4`, preserving the 16-bit handle and 24-bit group masks. |
| 303 | `80016338` | Fade an effect through the matching `80086948`; double the duration with 32-bit wrap before its 8-bit mask, and retain the 7-bit volume mask. |
| 401 / 402 | `800166E0` / `80016720` | Load the first / second music bank through the existing `8006D2B8`, without starting playback. Publish the returned slot/status only after completion. |
| 405 | `80016780` | Set byte `800B0CEA` to 1, selecting the alternate animation-sound bank for this frame. |
| 409 | `80016870` | Store the low volume byte at `800B0DBE` and apply CD gains `[volume,0,volume,0]` through matching `80080AC4` and native `8007B964`. |

For 401/402, operand 2 equal to zero means blocking; every nonzero value
means nonblocking. A loader return of 1 rewinds `8009CE00` by `0x20`, stores
1 at the current task's `+0x10`, leaves the output operand untouched, and
returns 0 to the VM. The host continues to use the existing temporary slot
at `80120FC0`; 409 uses four adjacent temporary bytes at `80120FC4` in place
of the original stack vector. These scratch addresses are host adaptations.

`800866A4`, `80086948`, and `80080AC4` already have matching C sources. This
change reuses them and the existing loader/controller graph. No matching
source or span manifest was changed by this work.

## Retail script references

These are concrete uses in the previously audited station/park script set.
Rooms can contain later-day branches, so this table proves command use,
not reachability of an entire scene on Day 2.

| Room | Script PC | Key | Script SHA-256 |
| --- | --- | --- | --- |
| m0042i | `801C5B90` | 301 | `70aefef84bb2c9b4ba1e8ba50726aac06fc7b4d81e2e9214034dedfbd4e1e4fd` |
| m0046i | `801AA9F4` | 303 | `00891a602db22f3bc3a88081af2270e676dbf34e92b3c2eb382e4f5457ed1320` |
| m0044i | `801D8510` | 401 | `6e9adedc3d9a252228dc6a803fcbd809fc1bd0440cd383517c727d8f0cf7b9a3` |
| m0047i | `801D76A4` / `801D7138` | 402 / 405 | `6e619093b70643e92dbdcf956a2dc21c09fd306075629b170580a8fb5f7c961a` |
| m0071i | `801C25DC` | 409 | `256b13f6fe6958af987c3f06795d6d519f0287034d1ad875b5a262f3ce6a9e75` |

Reproduce by decoding the user-supplied disc:

```sh
python3 pc_port/tools/pe_m0004i_mod4_probe.py m0042i m0046i m0044i m0047i m0071i
```

## Verification

`python3 pc_port/tools/pe_day2_audio_dispatch_oracle.py --check` passes 118
original-instruction graphs against the SHA-1-pinned retail executable
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`:

- Effect stop/fade: zero, width limits, overflow and signed-bit inputs;
  three FIFO positions, including wrap.
- Bank load: absent, cached, flagged, either bank, completed data copies,
  and busy nonblocking DMA polls. The busy path executes the original
  `6CDA4` and `870E0` rather than replacing their return values.
- Bank selector and CD gains: byte truncation, output preservation, and
  neighboring state bytes.

The oracle runs the original dispatcher and callees. Its sole substituted
callee is BIOS memcpy, implemented as the byte-copy contract. CD register
pointers target synthetic RAM so the oracle does not pretend to emulate
the device. Native tests compare the resulting relevant guest byte ranges
and return value, then separately repeat EA409 against the actual CD
register addresses and assert all four applied volume latches. Original
stack bytes and native temporary scratch are excluded from comparison.
This proves these tested control/data effects, not sample mixing or audible
playback.

Commands and results from 2026-09-12:

```text
python3 pc_port/tools/pe_day2_audio_dispatch_oracle.py --check
PASS 118 original audio-dispatch graphs

PE_TEST_FILTER=DAY2_audio_dispatch ./pc_port/build/pe-native-tests
1375 run, 1 passed, 0 failed, 1374 skipped (intentional filter)

ctest --test-dir pc_port/build --output-on-failure -j 4
10/10 passed; native-tests: 1375 run, 1375 passed, 0 failed, 0 skipped

python3 tools/analysis/verify_matched_leaves.py \
  --only func_80086948 --only func_80080AC4 --only func_800866A4 --quiet
VERIFY_SWEEP=PASS leaves=3

scripts/build_us.sh
795 registered C leaves, 1145 spans; RESULT: EXACT MATCH
orig/candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

The packed rebuild covers the complete executable, including its remaining
assembly. It proves the matching build is unchanged byte for byte; it does
not certify native gameplay, and the three-leaf sweep is not a full-image
leaf sweep. The rebuild log is `/tmp/pe-day2-audio-retail-build.log`.

The legacy compiler required execution outside the sandbox (the sandbox
terminated its 32-bit `cpp` with SIGSYS). Native build/test tools in this
environment are available through `source /tmp/pe-tools/env.sh`. Logs are
`/tmp/pe-day2-audio-{build,test,ctest,leaf-verify}.log` and the CTest
`pc_port/build/Testing/Temporary/LastTest.log`.

## Connected-route work remains

The boot harness still reaches its 14 milestones and stops in m0004i module
4 at `801B6CC8`. Additional input-only probes did not advance it:

```sh
PE_ROUTE_SWITCH4=9000 PE_ROUTE_PAD5=FFCF PE_ROUTE_FRAMES=11000 \
  PE_ROUTE_AYA_DUMP=1 ./pc_port/build/pe-route-boot-day2-tests
PE_ROUTE_SWITCH4=8900 PE_ROUTE_PAD5=FFCF PE_ROUTE_FRAMES=11000 \
  PE_ROUTE_AYA_DUMP=1 ./pc_port/build/pe-route-boot-day2-tests
```

The probes were run against a separately compiled Release build at
`/tmp/pe-day2-release`; its default trace matched the ordinary build's
milestones and frontier. Switching at 9000 leaves Aya near
`x=F81F9F79,z=087E3354`, outside the left doorway polygon. Switching at 8900
produces repeated collision corrections near `x=F8700000,z=0D300000`.
These observations do not determine whether another input sequence or a
collision-port correction is needed. Logs: `/tmp/pe-route-door-left.log`
and `/tmp/pe-route-door-left-8900.log`. No default pad or story state was
changed. Also, the older pad documentation's equation
`FFAF = FF9F & FFEF` is incorrect: that AND is `FF8F`; raw FFAF maps to
held `0x28`, as these live traces show.

Next: compare the retail collision/movement handling at this doorway and
continue the connected route. The full EA dispatcher, unskipped movie/audio
playback, and start-to-end Day 2 retail acceptance remain unfinished.
