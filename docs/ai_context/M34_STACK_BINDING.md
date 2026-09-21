# M34 projectile retained-stack binding

Native F434 dispatch now obtains its six retained inputs from original
writer events. Positional sound, script construction/commands and subsequent
projectile/trail drawing contribute to the record. No captured tuple is used
as a gameplay constant. Unknown callback graphs, unverified script handlers,
nested VM calls and RAM-generation changes invalidate the record.

This is scoped native stack provenance. It is **not** a complete emulated
CPU stack or a proof of asynchronous BIOS/IRQ stack behavior. The script
handler whitelist comes from the observed original M34 call history; a
general static proof of all branches/call graphs remains outstanding.

## Native integration

`m34_boss_effect_port.c` owns six words and a validity mask. The existing
35558 field walk scopes the record and identifies its35E04 actor calls;
17018 scopes VM depth and opcode dispatch. Known words survive field returns.
Readers require the active field context, matching RAM generation and M34
overlay identity. F434 additionally checks its slot owner against the actor
whose command sound established the saved-register input.

`PE_SpatialSoundRequest` exposes the volume already computed by6DED4. The
2FAF8 command-frame crossing calls it once and records volume/body/target
after the sound request. There is no second sound request or substituted
volume. Missing-package/failed spatial computation cannot establish validity.
Other6DED4 requests and86608 sound requests invalidate the prior record.

The18774→6F39C→6914C constructor path records S1=80010690, established by
original17020/17024, at the observed idle6914C branch. The M34 command API
now forwards all six arguments; F0C8/F0D0's fifth/sixth argument stores remain
observable even though C2AF0 ignores those arguments. Whole F434 preserves
the six inputs: its two rotation outputs stop before the translation words.

Generic effect/weapon dispatches notify the provenance record. The existing
M0023I retained-position mechanism is separate. Optional `PE_M34_STACK_TRACE=1`
logs writer/initializer values without changing guest gameplay RAM.

## First projectile

The fresh original11-frame history described in
[M34_FRAME_STACK_PRESERVATION.md](M34_FRAME_STACK_PRESERVATION.md) reaches
F434 with these words:

| Offset from original F434 entrySP | Value | Writer |
|---|---|---|
| -76 | 87 | 6E184, positional-sound volume |
| -72 | 800A5D5C | 6DED8, saved body S0 |
| -68 | 800BF490 | 6DEE0, saved target S1 |
| -36 | 80010690 | 6916C, saved VM decode-table S1 |
| -32 | 0 | F0C8, fifth command argument |
| -28 | 0 | F0D0, sixth command argument |

The first cold boot with this binding repeats all three sewer victories,
initializes the projectile at60,271, and reaches the next projectile in the
burst at60,275. At that intermediate implementation the later draw writers
were still unbound and the second projectile stopped explicitly. Log:
`/tmp/pe-m34-bound-projectile-connected.log`, terminal exit1 with the
historical optional50/57 milestone result. This is not a passing Day2 route.

Captures `pc_port/build/day2-victory-evidence/m34-bound-projectile` include
60,270..60,275; the final capture is from the partially stopped frame.
The pre-initializer60,270 image is unchanged from the previous run. The
60,271 capture SHA-256 is
`5a4e42341ff47eff2293eb5505c7e81ad3fe022f90682f80fa3ef849178b1386`.
Its full72-byte first projectile record equals the original/native oracle.
Frame60,274 SHA-256 is
`5a6f278b6444d1d883fa8f50c0da9a6530f40144aab7a36f910eb3063b7f5dc8`.

## Partial writes before the second projectile

The continuous original device-input probe now supports `--initializers N`.
From the corrected60,260 capture, target2 reaches the second initializer
after15 frames /3,159,708 instructions /12,615 PCs:

```
python3 pc_port/tools/pe_m34_frame_device_probe.py \
  pc_port/build/day2-victory-evidence/m34-destination-window/frame-060260.bin \
  --mode continuous --initializers 2 --instruction-budget 5000000
```

Output `/tmp/pe-m34-second-projectile-stack.json`. Device, BIOS, initial CPU
and GTE limitations are unchanged from the earlier probe. The final words
mix old high halves with newly written low halves:

| Offset | Value | Last writes |
|---|---|---|
| -76 | 000002BB | C62A0 low half: collision vertex2 Z; high half retained from6E184 |
| -72 | 0000F821 | C6264 low half: vertex3 X; C625C high half: zero Y |
| -68 | 800B02DD | C62B8 low half: vertex3 Z; high half retained from6DEE0 |
| -36 | 8001023C | C4544 low half: trail world Z; high half retained from6916C |
| -32 | 015CFF1A | 78D0C: first packed word of the main projectile's second scaled matrix |
| -28 | 00000000 | 78D4C: second packed matrix word |

The native collision-quad helper now exposes those exact vertex writes to
the record. Main-projectile drawing records the packed scaled matrix words;
trail drawing records world Z before its billboard camera transformation.
Partial stores preserve high halves and cannot establish validity for an
unknown high half. These bindings allow the next projectile to consume the
rendering history instead of reusing the original sound tuple.

## Verification and remaining differences

```
python3 pc_port/tools/pe_m34_stack_binding_oracle.py \
  pc_port/build/day2-victory-evidence/m34-burst-window \
  --end-frame 60274 --build-dir /tmp/pe-day2-release \
  --report pc_port/build/day2-victory-evidence/m34-burst-window/native-stack-binding.json
```

The fixture directory contains links to the corresponding captured frames.
Each field invocation loads that frame's copied input. Original stack/GTE/
scratchpad and native provenance persist between invocations. This verifies
writer binding against original counter/mailbox/field calls, not a continuous
whole-native replay. Every executed instruction and captured following word
is checked against original EXE/M34 code.

Five histories pass with original and altered volume envelopes. First-call
volumes are87,0,49,127,127. Both six-word initializer inputs and both complete
72-byte projectile records match;10,508 unique source PCs are checked.
**136 other non-stack RAM bytes differ** in each final field comparison.
The report lists every differing address; these are unresolved frame-level
differences and are not masked into the initializer proof. The earlier
first-projectile-only histories had140 differing bytes and9,135 source PCs.
Logs `/tmp/pe-m34-{native-stack-binding,burst-stack}-oracle.log`.

The added native regression covers nonzero fifth/sixth arguments, partial
collision stores, cross-frame retention, unbound callbacks, nested VM depth
and reset isolation. All **1,398 native tests pass, zero failed/skipped**;
all **10 non-route CTest checks pass in62.57 seconds**. Full Release and Debug
builds pass. Logs `/tmp/pe-m34-burst-stack-{build,debug-build,ctest}.log`.
The rebuilt PSX EXE remains byte-identical to retail, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

Later bursts, alternate call paths, the full M34 fight and the complete Day2
route still require connected verification and any missing translations.

## Connected burst after partial-write binding

The subsequent cold boot executes five projectile initializers at frame
60,271 /60,275 /60,279 /60,283 /60,287 with no unresolved callback. All three
sewer victories repeat. It reaches the62,000 frame limit after a reset to the
opening: story/persistence clear at60,742, New Game at60,746, M0010I at60,753.
This is **not M34 completion**. Log
`/tmp/pe-m34-burst-bound-connected.log`; terminal exit1 is the historical
optional50/57 milestone result.

The17 captures60,274..60,290 are in `m34-burst-bound`. At60,275, both complete
projectile records match the corresponding original/native oracle records.
The continuous original target4 probe reaches F434 after23 frames,
4,973,703 instructions and12,631 PCs. Its six retained words exactly equal
the fourth connected initializer's logged inputs. The fifth is executed
natively but does not yet have a corresponding continuous target5 comparison.
Output `/tmp/pe-m34-fourth-projectile-stack.json`, SHA-256
`7ca39cef2e81d80c61174a439545e4677de7fdca466fcf3b901333e76a9402bd`.
No captured RAM was restored into either connected run.

The subsequent read-only battle diagnostic confirms defeat. Aya loses10 HP
at60,304 (35→25), another4 at60,499, and reaches0 HP/mode3 at60,533. The
controller remains nearX=-873,Z=11 until enemy contact displaces her. PE92
and36 reserve rounds remain; the pilot has no M34 movement/healing logic.
Two later shots reduce the first enemy's displayed HP1000120→1000118;
its initial million-point offset is original scene behavior. The route
then resets and reaches its60,800 frame limit in M0010I. Log
`/tmp/pe-m34-burst-outcome.log`, terminal exit1; complete final RAM capture
`pc_port/build/day2-victory-evidence/pe-m34-burst-outcome.bin`.
Next work is ordinary-input M34 survival/targeting, followed by the remaining
fight and Day2 progression. No projectile callback is unresolved in this run.
