# PE-B54K-B1 — `func_80030894` complete L4 packet group

Date: 2026-08-30

## Verdict

B54K-B1 translates the next complete retail unit in `func_80030894`, moving
the production strict frontier from the end of L2/L3 to the first instruction
of L5.

```text
IMPLEMENTED=0x80030AC4..0x80030CA0 exclusive
WORDS=119
BYTES=0x1DC
WINDOW_SHA256=592dc73fa08202d91812b463aa71ef93cafd3af7713a0724d61e0815b17bc3ec
NEW_STRICT_FRONTIER=func_80030894_L4_cut
FIRST_EXCLUDED=0x80030CA0 lbu v1,24(sp)
PLANTED_STATE=NO
NEW_DEPENDENCIES=NONE
```

Together with B54K-A, the native prefix is now:

```text
0x80030894..0x80030CA0 = 0x40C bytes = 259 words translated
0x80030CA0..0x800314E4 = 0x844 bytes = 529 words remaining
259 + 529 = 788 words (full function)
```

## Boundary and control-flow proof

The predecessor and successor words are real instructions in the same proven
788-word function:

```text
0x80030ABC  1440FFDA  bnez v0,0x80030A28
0x80030AC0  00009821  move s3,zero             # prior delay slot
0x80030AC4  00002021  move a0,zero              # first included
0x80030AC8  00002821  move a1,zero
...
0x80030C98  32C200FF  andi v0,s6,0xff           # L4 delay slot
0x80030C9C  0000B021  move s6,zero              # last included
0x80030CA0  93A30018  lbu v1,24(sp)             # first excluded / L5
0x80030CA4  3C12800A  lui s2,0x800a
```

There is one branch in the window:

```text
0x80030C94  bnez v0,0x80030C44
```

Its bound is the literal `sltiu v0,v0,4` at `0x80030C90`; the delay slot
re-masks the byte counter. Thus the back-edge owns exactly four iterations
and closes before the new cut. There is no forward branch or alternate exit.

The seven `jal` sites, in retail order, are:

| Call PC | Target | Proven role |
| --- | --- | --- |
| `0x80030AD0` | `func_80077A64` | `GetTPage(0,0,0,0)` |
| `0x80030AF4` | `func_80037140` | compound tile wrapper |
| `0x80030B18` | `func_80077B04` | set tile semitrans bit |
| `0x80030B30` | `func_80077C44` | SetTile header |
| `0x80030B78` | `func_80077BC4` | SetPolyG4 header |
| `0x80030B94` | `func_800370DC` | standalone sprite wrapper |
| `0x80030C58` | `func_800370DC` | four-entry sprite-loop wrapper |

Every target was already native at B54I/GPU1. There is no `jalr`, hardware
operation, callback, scheduler action, upload, or new boundary in the window.

## Retail address geometry

All address scales come directly from the instruction sequence and use the
outer bank byte at `24(sp)`. This rung executes bank 0 because the enclosing
two-bank loop increment remains in the untranslated epilogue.

| Storage | Retail base | Bank stride | Construction |
| --- | ---: | ---: | --- |
| compound tile | `0x8009E068` | 24 | `(bank*2+bank)<<3` |
| standalone tile | `0x8009E098` | 16 | `bank<<4` |
| PolyG4 | `0x800B00E8` | 36 | `(bank*8+bank)<<2` |
| standalone sprite | `0x800B6920` | 28 | `(bank*8-bank)<<2` |
| L4 four-sprite array | `0x8009E0F0` | 112 | `(bank*8-bank)<<4` |
| each L4 sprite | array base | 28 | `(index*8-index)<<2` |

The L4 array bound and strides are checked instruction-exact by the oracle.

## Final bank-0 state

The translated order follows retail: wrapper/header calls first, then the
literal byte/halfword fields, then the four-item loop.

- `0x8009E068`: compound tile head, draw-mode word `0xE1000200`, tail RGB
  `30 30 30`, code `0x62`.
- `0x8009E098`: standalone tile, header `len=3/code=0x60`, RGB
  `1D 3E 32`, dimensions `0x38 x 3`.
- `0x800B00E8`: PolyG4, header `len=8/code=0x38`, with the two proven
  `00 46 82 / 9F FF F9` color pairs.
- `0x800B6920`: compound standalone sprite with draw mode `0xE1000234`,
  RGB `9F FF F9`, UV-like bytes `C8 E0`, CLUT `0x7E13`, dimensions `4 x 8`.
- `0x8009E0F0 + index*28`, index 0..3: compound sprites with draw mode
  `0xE1000234`, CLUT `0x7E13`, dimensions `6 x 10`.

No semantic asset names are assigned; the packet layout and values are proven,
but their higher-level UI role is not.

## Independent oracle

`pc_port/tools/b54kb1_30894_l4_oracle.py` imports no production source. It
reads the exact retail executable, verifies the full-window hash, 27 critical
words, boundaries, branch and call census, then applies the decoded operations
to its own zero-backed byte model. The model stores explicit little-endian
bytes and implements the packet wrappers independently.

Raw output:

```text
OK window: 119 words / 0x1dc bytes, SHA-256 exact
OK boundaries/scales/constants: 27 instruction-exact words
OK control flow: seven native jal sites; one four-item back-edge
model_unique_written_bytes=121
model_write_map_sha256=812440af2f0f6c0778421e177ce747d616420bd91678e2a4d6ede3b49b2a1874
OK independent bank-0 model: tile/PolyG4/sprites; L5 untouched
```

The older full-RAM `6AD40_prefix_boundary_args` canary was extended with the
same **sparse** 121-byte predicate. It does not whitelist whole tables or the
next group. Its focused result is:

```text
Results: 933 run, 1 passed, 0 failed, 932 skipped
CANARY_RC=0
```

## Focused and full gates

The B54K family retains both B54K-A tests and adds one B1 test covering every
packet class, all four loop entries, bank-1 negative storage, L5 negative
storage, and the named stop:

```text
TEST B54KA_30894_prologue_record_l2l3... PASS
TEST B54KA_6AD40_live_path_reaches_l2l3... PASS
TEST B54KB1_30894_l4_group... PASS

Results: 933 run, 3 passed, 0 failed, 930 skipped
B54K_RC=0
```

Full normal suite with the real Disc 1 path configured:

```text
Results: 933 run, 933 passed, 0 failed, 0 skipped
FULL_RC=0
```

Fresh `RelWithDebInfo` ASan/UBSan build and full suite:

```text
Results: 933 run, 933 passed, 0 failed, 0 skipped
SAN_RC=0
```

## Production route and visible artifact

Strict real-disc execution now fails at exactly the new first excluded unit:

```text
STRICT_RC=1
[DISC] boot executable loaded into guest RAM
[TRACE 0000] native_executable_start
[TRACE 0000] call_func_8001220C
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider: func_80030894_L4_cut
       called from: func_80030894
```

Normal real-disc execution reaches the same named cut, then the existing outer
cut, and exits zero:

```text
[TRACE 0001] native_executable_start
[TRACE 0002] call_func_8001220C
[STUB:BOOTSTRAP_RET] func_80030894_L4_cut (first invocation)
[STUB:BOOTSTRAP_RET] func_8006AD40_post30894_cut (first invocation)
[TRACE 0003] func_8001220C_returned
[TRACE 0004] shutdown_begin
[VRAM-RAW] /tmp/pe-b54kb1.vram (1024x512 RGB555/STP little-endian)
[VRAM-SCREENSHOT] /tmp/pe-b54kb1.ppm (1024x512 RGB555 diagnostic)
[FB] vsyncs=0 drawsyncs=0 presents=1 mask=0 main_iters=1
[HOST] stop_reason=unresolved-boundary
[TRACE 0005] shutdown_end
LIVE_RC=0
```

L4 constructs guest packet state and does not issue a GPU upload. The VIS1
artifacts therefore remain byte-identical, which is the expected negative
effect:

```text
raw_sha256=47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072
ppm_sha256=871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce
nonzero_words=2063
nonzero_bounds=256,64..735,456
decode=PASS all 524288 words / 1572864 RGB bytes
VRAM_VS_VIS1=IDENTICAL
PPM_VS_VIS1=IDENTICAL
```

A sanitizer production run exited zero, generated raw/PPM artifacts identical
to the normal build, and passed the independent all-word VIS1 decoder:

```text
SAN_LIVE_RC=0
RAW_NORMAL_SAN=IDENTICAL
PPM_NORMAL_SAN=IDENTICAL
decode=PASS all 524288 words / 1572864 RGB bytes
```

The legacy 320x240 host framebuffer remains black; B54K-B1 does not claim a
display renderer. No guest state, VRAM, destination, event, or battle state is
planted. The next semantic unit starts at retail `0x80030CA0` (L5 setup).
