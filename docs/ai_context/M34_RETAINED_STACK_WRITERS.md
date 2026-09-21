# M34 retained stack writers across the firing window

Update: [native stack binding](M34_STACK_BINDING.md) now connects the first
projectile and tracks the partial drawing writes needed by the next one.
The discovery state below is historical; full fight/route fidelity remains
unproved.

The original positional sound call made by the boss's command animation
writes all three previously unresolved F434 retained words. This is writer
discovery from original instructions on native captured state. A native
binding and a proof covering intervening calls remain necessary; connected
F434 dispatch is still unresolved.

Subsequent [continuous frame probes](M34_FRAME_STACK_PRESERVATION.md) now
preserve these writers through11 original frames to F434 using explicit
device inputs and BIOS contracts. They cover the previously omitted original
input/draw/presentation routines, but exclude BIOS/IRQ stack internals and
do not yet establish the native binding.

## Captures and independent calls

After the [guest frame-counter repair](FIELD_FRAME_COUNTER_RAM.md), an
ordinary-input cold boot captures71 complete2MB images at present-hook
frames60,200..60,270, then stops at the existing F434 boundary at60,271.
Three sewer victories repeat at52,111/53,823/57,791. No gameplay RAM injection
or capture restoration. Directory:
`pc_port/build/day2-victory-evidence/m34-pistol-window`.

```
python3 pc_port/tools/pe_m34_window_stack_probe.py \
  pc_port/build/day2-victory-evidence/m34-pistol-window
```

All **71 independent original counter/mailbox/field probes pass their source
checks**. Each executes original3F4D0..3F4F8 and watches six words relative to
prospective F434 entrySP801FEEE8. Every executed instruction and following
word is checked against the original EXE/M34 overlay. These are independent
calls with supplied initial CPU stack/GTE state, not a continuous original
replay. Input, outer drawing and presentation between captures are omitted.

Report `independent-field-writers.json` has SHA-256
`cd14f9e26c57c832fc1e8561e23f9d30ac7a4946d4613bd020fcae4bf5a971be`;
log `/tmp/pe-m34-window-writers.log`. The checked-in wrapper reproduces the
same field-probe calls and writes the report after all calls succeed.

Selected capture SHA-256 values:

| Present-hook frame | SHA-256 |
|---|---|
| 60,200 | 24bf2efd48fb3fd84bafdddad61b23240285612956bb98b128b3ba0981554225 |
| 60,237 | 9714630d75a0274c06bcf631ddf0f6fddd7eac95f374bec84e5d2b19e6cf747c |
| 60,260 | 202b8af8828eff8a508189dcf6f9c51c58409759f47f6fab18758d250c407942 |
| 60,270 | a4cc2395fc25bb7da4c52b54f38413fc7c5027266c28c1d51b7a8ca181b187aa |

## Latest observed writers

The independent call from capture60,260 executes the boss's script64 handler
184EC, then2FAF8, then6DCE4 and6DED4. Source checks cover6,690 PCs. Additional
register trace: `/tmp/pe-m34-spatial-stack-registers.json`.

| Retained offset from F434 entrySP | Address at this probe depth | Writer | Observed value and origin |
|---|---|---|---|
| -76 | 801FEE9C | 8006E184 | 87, calculated positional-sound volume |
| -72 | 801FEEA0 | 8006DED8 | 800A5D5C, saved caller S0, boss body |
| -68 | 801FEEA4 | 8006DEE0 | 800BF490, saved caller S1, boss actor |
| -36 | 801FEEC4 | 8006916C in the60,270 probe | 80010690, saved VM argument-table S1 |
| -32 | 801FEEC8 | 8018F0C8 in the60,270 probe | 0, fifth command argument |
| -28 | 801FEECC | 8018F0D0 in the60,270 probe | 0, sixth command argument |

At6DED4 entry, SP801FEEB0 and RA8006DD28. Incoming saved S0/S1 are the boss
body and actor; arguments are package8018EFE8, sound4D7, group0, X=-2004,
with Y505 and Z=-217 passed through the original wrapper. Its56-byte frame
puts volume output atSP+36, saved S0 atSP+40 and saved S1 atSP+44. The volume
calculation writes87 through6DFA8's output pointer at6E184. These three words
must be tracked as data; none is a generally valid constant.

The independent calls from60,261..60,269 do not write any watched words.
The60,270 call reaches F434 and overwrites only the last three, as shown.
This establishes candidate writer provenance. It does not prove that the
omitted outer/input/presentation calls preserve those words, nor that the
native runtime already tracks this original stack state.

Earlier in the window, sound dispatcher8CBA8 saves S1/S2/RA over the first
three words: observed0,128,80086684. Its latest writes come from8CBAC/8CBD8/
8CBD0. Casing rendering later changes only the final two watched words.
Separate synthetic original C9FD8 execution at the calculated weapon-draw
depth proves that muzzle scaling can overwrite offsets-72/-68 at78D0C/78D4C;
this is not the latest writer in the connected-window probes. Logs:
`/tmp/pe-m34-synthetic-muzzle-stack.json` (1,006 PCs),
`/tmp/pe-m34-synthetic-casing-stack.json` (704 PCs).

An independent original outer-draw fragment3F4F8..3F590 from the older60,270
capture checks588 PCs and writes none of the watched words; output
`/tmp/pe-m34-outer-draw-stack.json`. That fragment excludes presentation and
uses the older incomplete-counter capture, so it is only scoped preservation
evidence. Do not treat these probes as full boot/hardware stack validation.

## Next binding work

Track the original script64→2FAF8→6DCE4→6DED4 context and preserve the
computed volume plus saved body/actor values. The native spatial calculation
already lives in `func_8006DE80_port.c`; it must expose the observed result
without substituting87 or repeating the sound request. Preserve command
arguments five/six in M34 command dispatch, which currently forwards four
arguments. Verify intervening stack writes before connecting F434 and
invalidate unknown contexts rather than inventing values. Subsequent boss
projectiles may have different writers, so the first observed tuple cannot
stand in for the complete fight.
