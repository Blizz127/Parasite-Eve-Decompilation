# M34 frame stack preservation with explicit device inputs

Update: [native writer binding](M34_STACK_BINDING.md) now uses this evidence
and extends it through the second projectile. The initial binding status
below records the earlier preservation investigation.

The original field loop reaches `8018F434` after11 continuous isolated frames
from capture60,260. All six final retained words have the same last writers
as the earlier independent field probes. This closes the omitted frame-call
gap under explicit device/BIOS contracts. **It does not establish BIOS-stack,
interrupt, timing, raster, native-binding or whole-route fidelity.**

## Reproducible diagnostic

```
python3 pc_port/tools/pe_m34_frame_device_probe.py \
  pc_port/build/day2-victory-evidence/m34-pistol-window/frame-060260.bin \
  --mode continuous --caller-snapshot

python3 pc_port/tools/pe_m34_window_stack_probe.py \
  pc_port/build/day2-victory-evidence/m34-pistol-window --device-fragments
```

`--caller-snapshot` executes original `122CC..122DC` before the loop. The old
capture predates the [destination snapshot RAM repair](DESTINATION_SNAPSHOT_RAM.md)
and holds a stale zero at `9D1C4`. Without this explicit original prefix the
probe takes the original destination-change exit after one frame. The prefix
does not change the six watched stack words. The continuous loop then runs
`3F404` through its original `3F684` back edge, carrying CPU registers, GTE,
scratchpad and RAM until F434. It supplies the existing M34 pilot's digital
pad replies at each input prefix. No subsequent native captures are loaded.

The starting2MB image is a native present-hook capture, SHA-256
`202b8af8828eff8a508189dcf6f9c51c58409759f47f6fab18758d250c407942`.
Initial CPU stack base and GTE/scratchpad contents are supplied fixture state.
The capture occurs inside presentation; the continuous experiment starts at
the next input prefix, so it does not reconstruct the unexecuted tail of the
capture's own frame.

Every executed instruction, including delay slots, is checked against the
original Disc1 EXE/M34 overlay at execution time. Its captured instruction and
following word are also checked. Instrumentation observes stores without
replacing game/SDK instructions. Byte masks for SWL/SWR are explicit; RAM DMA
and BIOS-contract writes are included in the watched-address accounting.

## Device and BIOS boundaries

MMIO `1F801000..1F802000` is mapped separately from RAM. Fixture DPCR starts
at `07654321`, DMA6 CHCR at2, GPUSTAT at `14802000`, and other modeled port
storage starts zero. GPU command-port writes receive a ready status input.
GPU DMA must be RAM-to-GPU; completion clears its start bits, without packet
validation, VRAM writes or rasterization. DMA6 performs the reverse ordering
table RAM stores. These contracts follow the documented
[DMA register behavior](https://psx-spx.consoledev.net/dmachannels/) and
[GPU status bits](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/),
but implement only the explicitly described subset.

Every configurable number of executed instructions the fixture increments
`956AC` and timer counter port `1F801110`. This supplies VBlank/time progress;
it does not execute an interrupt handler or reproduce cycles. The SDK routines
themselves run, including their waits and original stack stores.

The reached BIOS calls are A(2Ah) memcpy for20-byte display and92-byte draw
environments. The probe models the [BIOS memory-copy contract](https://psx-spx.consoledev.net/kernelbios/),
including refusal of a zero destination or excessive length, with explicit
in-RAM/non-overlapping bounds for accepted copies. Existing interpreter
bzero/memset/random/diagnostic contracts remain available and are logged.
**BIOS instructions and their own possible stack writes are not modeled.**
Guest sound scheduling does execute; asynchronous BIOS/IRQ sound processing
does not. These are material limits on the preservation conclusion.

## Results

The142 independent prefix/tail probes cover71 captures60,200..60,270.
Prefixes `3F404..3F4D0` execute325 or343 unique PCs with minimumSP801FEF80.
Tails `3F4F8..3F678`, including original presentation, execute1,548..1,585 PCs
with minimumSP801FEF50. None writes the watched six words. Report:
`m34-pistol-window/independent-device-frame-writers.json`, SHA-256
`66d2cc6ef1be1802a75612085b4f725c4c00249a3724ba8bb9b74ed388e1c83b`.
Log `/tmp/pe-m34-frame-device-window.log`.

Continuous runs at counter periods256,1024 and2048 all reach F434 after11
frames and check10,895 unique PCs. Instruction counts are2,270,109 /2,271,901 /
2,278,047 respectively. All reach entrySP801FEEE8 with these final writers:

| Entry-SP offset | Value | Last writer | Isolated frame index |
|---|---|---|---|
| -76 | 87 | 8006E184, computed sound volume | 0 |
| -72 | 800A5D5C | 8006DED8, saved boss body S0 | 0 |
| -68 | 800BF490 | 8006DEE0, saved boss actor S1 | 0 |
| -36 | 80010690 | 8006916C, saved VM argument-table S1 | 10 |
| -32 | 0 | 8018F0C8, fifth command argument | 10 |
| -28 | 0 | 8018F0D0, sixth command argument | 10 |

Default output `/tmp/pe-m34-continuous-snapshot-device-probe.json`, SHA-256
`053735080ad2dc7bbf4c097af52739039d01cddf568ee06399cac386a6915edf`.
Fast/slow outputs `/tmp/pe-m34-continuous-snapshot-{fast,slow}-device-probe.json`.
This extends [writer discovery](M34_RETAINED_STACK_WRITERS.md) with continuous
original-call preservation. It does not authorize substituting this one
observed tuple for dynamic caller history. Native F434 dispatch remains
unresolved pending that binding and its verification.

## Fresh capture after the destination repair

The fresh cold boot captures11 frames in `m34-destination-window`. They differ
from their older counterparts only in the corrected destination snapshot.
The same diagnostic runs directly from the fresh frame60,260, **without**
`--caller-snapshot`, and reaches F434 after11 frames /2,271,911 instructions /
10,891 unique PCs. All six final values and last writers match the table.

```
python3 pc_port/tools/pe_m34_frame_device_probe.py \
  pc_port/build/day2-victory-evidence/m34-destination-window/frame-060260.bin \
  --mode continuous
```

Output `/tmp/pe-m34-fresh-continuous-device-probe.json`, SHA-256
`93170da0c506a9574c056f5138fad0e37c9a26819c43d7b15ea377f8daab1bde`.
The four-PC reduction is the omitted main-loop snapshot prefix. Fixture
counter scheduling explains the small instruction-count difference.

All22 independent prefix/tail probes on the fresh window also pass with no
watched writes. Checked-in wrapper command uses `--device-fragments` and the
fresh directory. Report `independent-device-frame-writers.json` SHA-256
`38f82ac7f795fb2ed5e9516748ab83473d91b19017a49e5e61e1a9c010e7e676`;
log `/tmp/pe-m34-fresh-device-window.log`. Device, BIOS, initial-state and
native-binding limitations above still apply.
