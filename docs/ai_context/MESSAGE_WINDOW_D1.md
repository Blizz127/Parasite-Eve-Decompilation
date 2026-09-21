# D1 message window at M34

The connected ordinary-input route first reached M34 at frame 59,996 and
stopped at frame 60,096 on opcode D1 (`80019D84`, script `801B5E0C`, actor
`800BF490`). The message ID is 8, and preceding D0 configures window geometry
`(8,180,296,24)`. Full Day 2 completion and whole-route fidelity remain unproved.

## Translation

`pc_port/game/boot/func_80017018_port.c` now implements and dispatches the
complete 13-word wrapper `80019D84..80019DB8`. It reads a signed halfword ID,
passes mode 1 and a `{-1}` list to existing `800375E0`, and returns 1. Mode 1
copies configured geometry into the first free message record. All four
records occupied leaves message records untouched.

Original executable and unchanged candidate SHA-1:
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Original wrapper SHA-256:
`ccc7772c1178a03639fcbb9b6f3958fd16f58c0d7775664c52ededbab50b45b4`.

Existing full 161-word callee `800375E0..80037864` SHA-256:
`d7f8b96cf54e7f4e98ec49c081059b5e47916ce9e25a5f9e97294fd1eb58d58a`.
Matching source, assembly and split metadata are untouched by this change.
The native wrapper uses the existing guest temporary `80120F20` for the
original stack list; this is an explicit host-port representation difference.

## Differential evidence

```
python3 pc_port/tools/pe_message_window_oracle.py \
  --build-dir /tmp/pe-day2-release \
  --capture pc_port/build/day2-victory-evidence/pe-m33-forward-connected.bin \
  --write-header
```

**160 original/native cases plus captured D1 PASS; 91 unique instruction PCs.**
Log: `/tmp/pe-d1-oracle.log`. No callee contracts or mocked calls are used.
Cases cover five IDs (0,8,32767,-32768,-1), all 16 record occupancy masks,
and two record flag/window geometry patterns. Each original execution checks
executed instructions and following words against the retail executable.
All RAM below `1FE000` and return values are compared. Original stack space
is excluded. The two bytes at native temporary `120F20` are preseeded to
`FFFF` in both inputs and remain included in comparison; no game state is
modified in connected runs. Generated hash ranges cover all original writes
in compared RAM for synthetic cases, checked by the oracle.

Captured input is exactly 2 MB, SHA-256
`2d654c4dd8f000bbdc9b3cbc51f9fa02245229c86f968f25e736e0e4dbc33e70`.
The isolated call uses the captured VM argument bank at `80120F80`, with
its pointer `801B5E14` and ID 8 asserted. Only the temporary-list preseed is
applied. This capture is never restored into a connected route.

`retail_message_window_cases.h` contains generated fixtures consumed by
`test_message_window.h` in the native regression suite. Release and Debug full builds pass. The native suite reports
**1,394 run, 1,394 passed, zero failed/skipped**; all **10 non-route CTest
checks pass in 68.99 seconds** (native 66.60 seconds). Logs:
`/tmp/pe-d1-tests-build.log`, `/tmp/pe-d1-debug-build.log`,
`/tmp/pe-d1-ctest.log`, and stable `/tmp/pe-d1-lasttest.log`.


## Connected result and next missing routine

Fresh ordinary-input cold boot repeats the three sewer victories at
52,111 / 53,823 / 57,791, M33 entry 58,538 and M34 entry 59,996. D1's message
finishes; battle mode returns to 0 at 60,111, Aya queues a normal attack at
60,188/60,196, and loaded ammunition falls from 4 to 2. At frame **60,271**
the run stops on `func_8006F39C_constructor`, story `6C`, arrival `21`, token
`A8003248`. This is an unresolved effect callback, not Day 2 completion.
The historical optional supply route still reports only 50/57 milestones.

Log: `/tmp/pe-d1-connected.log`. Complete 2 MB capture:
`pc_port/build/day2-victory-evidence/pe-d1-connected.bin`, SHA-256
`c0917383343f485315fd135ea17797fecb0e0bd74ffced88c9bf4b387cc935e9`.
No gameplay RAM injection or checkpoint restoration was used.

The newly allocated effect is code **8**, large-pool slot 0 at `801861A0`,
user data/boss actor `800BF490`. Table `80094188` selects descriptor
`8018FF7C`: callbacks `8018F244,8018F00C,8018F0B8,8018F0E4,8018F12C,
8018F1B8,8018F23C`. Missing constructor **8018F00C..8018F0B8 (43 words)**
calls existing `800C22F8`, assigns table `8018FF98`, and configures overlay
data at `80190054..8019006B`. These routines have not been ported in this change.

Authoritative M34 C2 overlay: Disc 1 LBA **16597**, **100 sectors**, 204800 bytes,
SHA-256 `0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a`.
Constructor SHA-256:
`3fb78435b95216a969a5cbfbaff4155cce08c8fe67a5126b8b556fb44cd89b8c`.
All 43 captured constructor words match this disc source. Original overlay
extracted for inspection to `/tmp/pe-m34-c2-original.bin`, base `8018EFE8`.
Next work is the complete effect callback family and original/native
comparisons before resuming the connected route.
