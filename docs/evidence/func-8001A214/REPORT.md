# Opcode DD: actor-relative polar coordinates

The connected cold-boot continuation reaches `m0028i` at frame 52,344.
After connecting the existing matching opcode 13, actor `800C0390` reaches
the untranslated DD handler at script PC `801A91D8`, frame 53,140.

`src/func_8001A214.c` recovers the complete function at
`8001A214..8001A2F0`: 55 words, 220 bytes. It reduces `0x1400 - angle` to a
signed halfword, computes two signed fixed-point products using the original
trigonometric routines, and adds the actor's X and Z coordinates. Destination
stores and subsequent loads retain their original order when operands alias.

The era `-O2 -G0` build has zero differing words over the entire span and
only four zero alignment bytes after it. The strong size check passes. YAML
now selects this C source instead of generated `AA14.s`; the obsolete,
ignored assembly was removed through `disc1_plan.py --cleanup-stale-asm`.
The default compiler profile applies.

Authority: original Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Function SHA-256:
`e106f5b23ae9293b5fd44df82a0fcbcd3c2ede26ff9dff3938e4426a8ae55146`.
Local verification logs: `/tmp/pe-polar-match.log`,
`/tmp/pe-polar-strong-check.log`, `/tmp/pe-polar-matching-sweep.log`.
The full sweep passes all 796 C spans under plan
`b50a30290d4df4e0b10a2798d38c7fbc10b1da3d20f9d8f97e77b134634be131`.

The native adaptation in `pc_port/game/boot/func_8001A15C_port.c` uses guest
memory accessors and unsigned wrapping operations. VM dispatch calls it for
table entry `8001A214`. `pe_polar_vm_oracle.py` executes the original full VM,
DD, its trigonometric and multiplication callees, and a following wait.
All 561 original/native cases pass: all five operand modes, quadrant edges,
signed-halfword wrap, extreme radii, shared destinations and input/output
aliasing. The oracle checks that every original RAM write below `1FE000`
is covered by the compared ranges. Scratch stack, native VM frame adaptation,
timing and hardware state outside RAM are outside this comparison.
Generated fixtures: `retail_polar_vm_cases.h`; native test: `test_polar_vm.h`.
Logs: `/tmp/pe-polar-vm-oracle.log`, `/tmp/pe-polar-vm-native.log`.

The connected replay passes DD and stops one frame later, at frame 53,141:
effect 49's constructor is untranslated, followed by missing opcode 6C at
`801A9450`. Its capture is `/tmp/pe-m28-polar-connected.bin`, SHA-256
`0e5496adcdd0a514d113e1cf204673bfe0020f0809907e2dda820c512344ada3`.
This uses the same 965 initial pad pairs and an input-only battle controller;
no gameplay state is supplied from a capture. The historical m27 endpoint
assertion fails because this probe has continued into m28. No m28 victory,
complete Day 2, or whole-route retail fidelity is claimed.

The complete rebuilt candidate has the retail SHA-1, and a separate read-only
`disc1_verify.exact_checks` invocation verifies all 796 packed C spans.
Logs: `/tmp/pe-polar-retail-build.log`, `/tmp/pe-polar-packed-check.log`.
The complete `verify_us.sh` workflow stops at its tracked-source metadata gate
because this new source and the preexisting `src/func_800374E8.c` are still
untracked. The index was preserved; this workflow failure is distinct from
the successful binary comparison and strong matching sweep.

All 11 CTest checks pass after the native DD change (201.87 seconds total),
including 1,384 native tests and the 52,000-frame fixed route. Both Release
and Debug builds succeed. Logs: `/tmp/pe-polar-all-ctest.log`,
`/tmp/pe-polar-all-lasttest.log`, `/tmp/pe-polar-debug-build.log`.
