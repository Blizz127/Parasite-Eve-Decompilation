# Original display environment and GPU programming

DAY1-61 restores318 original words800755F0..80075AE8 (PutDispEnv), SHA256
c8dc390773e463aadf9158ed73f3cb4467a6c8b2af8f853c3950512ef0577ca1.
Native owner: platform/pe_dispenv.c. Public ABI now returns the environment
pointer, as the original does. Host framebuffer presentation follows successful
original state publication. It previously bypassed all GPU commands and writes.

The routine always emits GP1(05) display start. A changed first rectangle or
last environment word recomputes GP1(08), using signed width thresholds281,
353,401,561, video-standard getter956EC, interlace/RGB24 and reverse flags.
It sets environment byte18 to8 after that command, forcing range recomputation.
Changed screen rectangle or byte18==8 recomputes GP1(06)/(07), using original
95820..9584F tables and ordered NTSC/PAL clamps. It writes the standard back to
byte18 and copies20 bytes into cached957B8 only after all commands return.
Each command reloads the live jump table and requires installed76B20 identity.
The real writer's low-byte command cache atA3348+opcode is retained. Debug level
>=2 invokes the canonical diagnostic; unknown print/writer identities stop before
later writes or presentation. The table pointer is checked before host access.

pe_dispenv_oracle.py executes original PutDispEnv, standard getter and GP1 writer
instructions. A relocated GP1 bus pointer permits observing original stores;
BIOS memcpy/printf are explicit contracts.2048 cases compare exact command order,
input environment mutation, last-environment cache and command-byte cache.
Cases include signed extremes, every resolution threshold, NTSC/PAL, both
boolean display flags, reverse flag, debug logging and cache-hit paths.
These comparisons establish software behavior under the stated BIOS/bus inputs.
They do not establish GPU scanout or wall-clock equivalence.

GP1(05..08) now updates the platform's display registers. GPUSTAT receives mode
bits14,16..22. Hardware/GP1 reset restores display-register defaults. The four
recorded command words/count are diagnostics, not device inputs. See the
[GPU register reference](https://psx-spx.consoledev.net/graphicsprocessingunitgpu/)
for payload fields and status mapping. The existing hardware provider remains
partial outside this command subset.

The host presentation path still reads the DISPENV window. Real scanline/field
scheduling, source0 production, BIOS clear policy and a public VSync adapter are
unfinished. This stage supplies the original mode/range inputs needed by that
scheduler; it does not yet replace HostFB_VSync or resolve the outer menu tail.

Final validation: normal original-comparison group and ASan/UBSan group pass;
full CTest8/8 in66.67s,1263 native groups. Two caller nonreturn checks prevent
70E54 drawing/buffer flip and80190660 loop/cleanup after a stopped presentation.
Old frame fixtures now seed the original display dispatch/table data explicitly;
canary allowances add only the restored environment and command caches.
