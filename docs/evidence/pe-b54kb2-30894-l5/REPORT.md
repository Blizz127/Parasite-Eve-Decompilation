# PE-B54K-B2 — `func_80030894` complete L5 sprite loop

Date: 2026-08-30

## Verdict

B54K-B2 translates the next complete retail unit in `func_80030894`, moving
the production strict frontier from the first L5 instruction to the first
instruction of the following packet group.

```text
IMPLEMENTED=0x80030CA0..0x80030D20 exclusive
WORDS=32
BYTES=0x80
WINDOW_SHA256=b45f5a6c9a6d1565f3fcc6affce6edc91a679ebe2bd538734d75fd2c933575d0
NEW_STRICT_FRONTIER=func_80030894_L5_cut
FIRST_EXCLUDED=0x80030D20 lbu s5,24(sp)
PLANTED_STATE=NO
NEW_DEPENDENCIES=NONE
```

Together with B54K-A/B1, the native prefix is now:

```text
0x80030894..0x80030D20 = 0x48C bytes = 291 words translated
0x80030D20..0x800314E4 = 0x7C4 bytes = 497 words remaining
291 + 497 = 788 words (full function)
```

## Boundary and control-flow proof

The full `func_80030894` body, sole caller, ABI, epilogue, 42-call census,
and ten inner loops plus outer loop remain frozen by
`pc_port/tools/b54j_30894_audit_oracle.py`. The B54K-B2 boundaries are real
instructions inside that proven function:

```text
0x80030C98  32C200FF  andi v0,s6,0xff        # prior delay slot
0x80030C9C  0000B021  move s6,zero           # predecessor / L5 counter init
0x80030CA0  93A30018  lbu v1,24(sp)          # first included / bank load
...
0x80030D14  2C420005  sltiu v0,v0,5
0x80030D18  1440FFEB  bnez v0,0x80030CC8
0x80030D1C  32C200FF  andi v0,s6,0xff        # last included / delay slot
0x80030D20  93B50018  lbu s5,24(sp)          # first excluded / next group
0x80030D24  3C10800B  lui s0,0x800b
```

The window has one static `jal`, at `0x80030CDC`, to the already-native
`func_800370DC` sprite packet wrapper. It executes once per iteration. There
is no `jalr`, forward branch, alternate exit, hardware operation, upload,
callback, or scheduler action.

The only branch is the back-edge at `0x80030D18`. Its literal
`sltiu v0,v0,5` and byte-masked counter establish exactly five iterations.
The back-edge and delay slot both close before the new cut.

## Retail address geometry and semantics

The instruction chain computes the bank stride without a semantic guess:

```text
bank * 8
+ bank       = bank * 9
<< 2         = bank * 36
- bank       = bank * 35
<< 2         = bank * 140
```

The loop computes `(index*8-index)<<2 = index*28`. Therefore each packet
head is:

```text
0x8009E1D0 + bank*140 + index*28, index = 0..4
```

This prefix executes bank 0 because the enclosing two-bank increment remains
in the untranslated epilogue. For each head, retail calls
`func_800370DC(head, 0x34)`, stores CLUT `0x7E13` at `head+0x16`, and stores
dimensions `6 x 10` at tail offsets `0x10` and `0x12` (`tail = head+8`).

The resulting compound packet has draw-mode word `0xE1000234`, combined
length 6, sprite code `0x64`, and zeroed tail link word. No higher-level asset
name is assigned.

## Independent oracle

`pc_port/tools/b54kb2_30894_l5_oracle.py` imports no production source. It
reads the SHA-1-exact retail executable, verifies the complete 32-word
window, 24 instruction-exact boundary/stride/store words, and the exact
call/branch census. Its separate zero-backed byte model expands the packet
wrapper independently and builds all five packets.

Raw output:

```text
OK window: 32 words / 0x80 bytes, SHA-256 exact
OK boundaries/strides/stores: 24 instruction-exact words
OK control flow: one native jal site; one five-item back-edge
model_unique_written_bytes=80
model_write_map_sha256=e50f04b1034c31a1fc080584a39ab93843f624ad8adab1a33772571f8085c3f1
OK independent bank-0 model: five compound sprites; next group untouched
```

The retained full-RAM `6AD40_prefix_boundary_args` canary admits those same
80 bytes through a sparse predicate: 16 proven bytes in each 28-byte packet.
It does not whitelist the gaps, bank 1, or the next group.

```text
Results: 934 run, 1 passed, 0 failed, 933 skipped
CANARY_RC=0
```

## Focused and full gates

The B54K family retains the three A/B1 tests and adds one B2 test covering
all five packets, bank-1 negative storage, next-group negative storage, and
the named stop:

```text
TEST B54KA_30894_prologue_record_l2l3... PASS
TEST B54KA_6AD40_live_path_reaches_l2l3... PASS
TEST B54KB1_30894_l4_group... PASS
TEST B54KB2_30894_l5_group... PASS

Results: 934 run, 4 passed, 0 failed, 930 skipped
B54K_RC=0
```

The full suite was run with the retail Disc 1 fixture supplied explicitly:

```text
Results: 934 run, 934 passed, 0 failed, 0 skipped
FULL_DISC_RC=0
```

A fresh `RelWithDebInfo` ASan/UBSan rebuild passed the same full suite:

```text
Results: 934 run, 934 passed, 0 failed, 0 skipped
SAN_RC=0
```

The frozen full-function structural oracle remains green:

```text
OK window: 788 words @ file 0x21094, sha256 exact
OK branch census: 11 bnez + 1 jr $ra, no other control flow
OK loop map: 10 group loops + outer, bounds literal-exact
OK outer counter: init/inc/test instruction-exact
OK stride chains: 1400/140/28 instruction-exact
OK frame: 88-byte, 10 saves, font triple lb from 0x8009CD90
OK call census: 42 jal in exact order, all targets native
OK boundary: epilogue exact; func_800314E4 starts 0x800314E4
OK retail vectors: arg loads exact at both head call sites

B54J audit oracle: 19 check groups passed.
```

## Production route and visible artifact

Strict real-disc execution fails at exactly the new first excluded unit:

```text
STRICT_RC=1
[DISC] boot executable loaded into guest RAM
[TRACE 0000] native_executable_start
[TRACE 0000] call_func_8001220C
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80030894_L5_cut
       called from: func_80030894
```

Normal real-disc execution reaches that inner cut, then the existing outer
cut, emits the read-only diagnostics, and exits zero:

```text
[TRACE 0000] native_executable_start
[TRACE 0000] call_func_8001220C
[STUB:BOOTSTRAP_RET] func_80030894_L5_cut (first invocation)
[STUB:BOOTSTRAP_RET] func_8006AD40_post30894_cut (first invocation)
[TRACE 0000] func_8001220C_returned
[TRACE 0000] shutdown_begin
[VRAM-RAW] /tmp/pe-b54kb2.vram (1024x512 RGB555/STP little-endian)
[VRAM-SCREENSHOT] /tmp/pe-b54kb2.ppm (1024x512 RGB555 diagnostic)
[FB] vsyncs=0 drawsyncs=0 presents=1 mask=0 main_iters=1
[HOST] stop_reason=unresolved-boundary
[TRACE 0000] shutdown_end
LIVE_RC=0
```

L5 constructs guest packet state and does not issue a GPU upload. Its VIS1
artifacts are therefore byte-identical to B54K-B1:

```text
raw_sha256=47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072
ppm_sha256=871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce
host_ppm_sha256=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
nonzero_words=2063
nonzero_bounds=256,64..735,456
decode=PASS all 524288 words / 1572864 RGB bytes
VRAM_VS_B1=IDENTICAL
PPM_VS_B1=IDENTICAL
```

The sanitizer production run also exited zero, generated raw/PPM artifacts
identical to normal, and passed the all-word decoder:

```text
SAN_LIVE_RC=0
RAW_NORMAL_SAN=IDENTICAL
PPM_NORMAL_SAN=IDENTICAL
decode=PASS all 524288 words / 1572864 RGB bytes
```

The legacy 320x240 host framebuffer remains black. B54K-B2 makes no renderer,
field, gameplay, event, or battle claim. No guest state, VRAM, destination,
event, or battle state is planted. The next semantic unit starts at retail
`0x80030D20`.
