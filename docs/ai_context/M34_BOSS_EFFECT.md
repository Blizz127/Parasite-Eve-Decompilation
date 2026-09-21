# M34 boss effect 8

The connected route's `8006F39C_constructor` boundary selected effect 8,
descriptor `8018FF7C`, constructor `8018F00C`, large-pool slot `801861A0`,
owner `800BF490`. The constructor and effect lifecycle are now translated;
the projectile initializer is translated with explicit retained stack inputs,
but its connected dispatch is still unresolved. See the subsequent
[initializer comparison and full-frame stack trace](M34_PROJECTILE_INITIALIZER.md).
Full Day 2 completion and
whole-route fidelity remain unproved.

## Source and translated scope

Original Disc 1 M34 C2: LBA 16597, 100 sectors, base `8018EFE8`, SHA-256
`0eb2efb10e4779672a00f6da46c2d54f915f1b3e433048513fd08de296eedd5a`.
Original and rebuilt candidate executable SHA-1 both remain
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

The initial lifecycle change translated 291 source words across these spans:

| Span | Words | SHA-256 |
|---|---:|---|
| 8018F00C..8018F24C | 144 | 07b3c9cbfe710ecfc38d209a9caac3e4dd3ff07d96ca0ef77a1d8caef309e353 |
| 8018F24C..8018F434 | 122 | fbc9f66003424f65ee69d83c0c4e46b30d70052b5f2025acac0b67c1981f0b13 |
| 8018FDD4..8018FDE4 | 4 | 1949ceca109e59390c19d20cf905418a47320b334641b2c6fbe1d0d75755d909 |
| 8018FEE0..8018FF34 | 21 | 3d3a85978a16406093e5c896c5978183e8d93d814575a9c4fa0dbce42cb204f1 |

The seven descriptor callbacks cover initialization, commands, drawing,
updating, destruction and two original empty leaves. Native dispatch is
wired into 6F39C, 6F6D4, the effect scheduler, child callback dispatcher and
6FC18 destruction. Overlay instruction/descriptor checks distinguish reused
addresses. Initialization calls the existing full C22F8 storage initializer,
sets program `8018FF98` and configures effect styles. Destruction clears the
owner's effect flags and sets its action state to 4. Update and draw retain
the original child dispatch and still stop for untranslated children.

Child translations preserve grouped matrix loads/stores, timer wrapping,
signed comparisons, original empty callbacks and child allocation. The
allocating F3C4 arm currently leads into unresolved F434 dispatch; it is translated
but not yet covered by a successful complete original/native execution.
F434 has since been translated separately with explicit stack inputs; F830,
FC54 and FDE4 are now also translated, with their comparisons recorded in
[M34_PROJECTILE_CHILDREN.md](M34_PROJECTILE_CHILDREN.md). Connected F434
dispatch still requires faithful stack inputs. This is not a completed fight.
No matching source, assembly, split metadata or index changes were made.

## Differential evidence

```
python3 pc_port/tools/pe_m34_boss_effect_oracle.py \
  --build-dir /tmp/pe-day2-release \
  --capture pc_port/build/day2-victory-evidence/pe-d1-connected.bin \
  --write-header
```

The first completed comparison passes **177 cases plus the captured
constructor, 534 unique instruction PCs**. It executes full original callees,
compares all RAM below `1FE000` plus defined return values, checks executed
and following instruction words against the original EXE/overlay, and
asserts every original write in compared RAM is covered by generated hash
ranges. Only original stack space is excluded. No callee contracts/mocks.

Cases cover six owner classes, two initial byte patterns, command modes,
indices and signed values, empty-child draw/update termination, pose copies
including Aya's alternate branch, and signed/wrapping timer/fade values.
They do not prove populated projectile update/draw or the full scheduler.
Captured constructor input SHA-256:
`c0917383343f485315fd135ea17797fecb0e0bd74ffced88c9bf4b387cc935e9`.
No input memory is patched for that isolated call.

Initial oracle failure was an incorrect fixture program pointer (slot+80
instead of original slot+78), corrected before the passing comparison.
The first generated native regression fixture omitted the read-only program
terminator at `80190020`; its update loop consequently waited indefinitely.
That test process was terminated, and the terminator was added to the fixture
ranges. This is a fixture repair, not a production behavior change. An intermediate regeneration also incorrectly filled the terminator with
fixture noise; that range was corrected before the final successful run.

Final oracle: `/tmp/pe-m34-core-verified2-oracle.log`, **177 cases plus capture
PASS, 534 PCs**. Release and Debug full builds pass. The native suite reports
**1,395 run, 1,395 passed, zero failed/skipped**; **10/10 non-route CTest PASS
in 61.36 seconds** (native 59.36 seconds). Logs:
`/tmp/pe-m34-core-verified-build.log`,
`/tmp/pe-m34-core-verified-debug-build.log`,
`/tmp/pe-m34-core-verified-ctest.log`, and stable
`/tmp/pe-m34-core-verified-lasttest.log`. All processes are terminal.
These checks cover the initial lifecycle change. Subsequent initializer
changes and their newer passing checks are recorded in the linked evidence.

## Connected replay

Fresh ordinary-input cold boot repeats all three sewer victories at
52,111 / 53,823 / 57,791, M33 entry 58,538 and M34 entry 59,996. At **60,271**
it passes effect construction and stops at child **8018F434**, slot `801861A0`,
record `80186226`, story `6C`, arrival `21`, room token `A8003248`.
The frame count is unchanged because the newly reached child runs in the
same frame. This is verified control-flow progress, not elapsed-frame progress.

Log `/tmp/pe-m34-core-connected.log`; complete 2 MB capture
`pc_port/build/day2-victory-evidence/pe-m34-core-connected.bin`, SHA-256
`fe8b6a9ba4b02dc3cbb61d35faf4287a06f950acd20d13e698dc653b691c106c`.
No connected checkpoint restoration or gameplay RAM injection. The optional
historical supply route still reports 50/57 milestones and is not a complete
Day 2 regression.

## Projectile stack dependency

```
python3 pc_port/tools/pe_m34_effect_stack_probe.py \
  pc_port/build/day2-victory-evidence/pe-d1-connected.bin
```

This original-only diagnostic executes the constructor and update until
F434, then executes F434 in seven isolated stack cases. Executed/following
instruction words are checked; 911 unique PCs. It does not claim that its
stack is an actual retail gameplay stack. Output:
`/tmp/pe-m34-effect-stack-probe.json`.

F434 builds rotation matrices at its SP+20 and SP+48 but 79754 writes only
their nine rotation halfwords. Later GTE code reads the unwritten translation
words. Relative to F434's entry SP these are **-76,-72,-68,-36,-32,-28**.
None is written by the traced constructor/update sequence. In the isolated
capture-derived sequence the entry SP is `801FEF70`, data `801863A0`, and
these six words initially contain zero.

They are observable: original output translation at data+38 is
`(-1280,993,-1868)`. Changing only entry-SP-76 to 1000 gives
`(-2373,993,-1762)`; changing only entry-SP-36 to 1000 gives
`(-280,993,-1868)`. Each of the six individual perturbations changes output.
Therefore zero-initializing native matrix translations is not proven faithful.
The remaining task is to supply the translated initializer's original stack
dependencies faithfully and verify the connected effect through the fight.
Do not substitute guessed zeros or claim this core comparison proves the
projectile or full boss fight.
