# PE-VIS1 — read-only authoritative-VRAM visible diagnostic

Date: 2026-08-30

## Verdict

VIS1 provides the first reproducible, visibly non-black artifact from a normal
native Disc 1 execution. It is an observation of the existing authoritative
PSX VRAM after translated retail upload work. It is **not** a display renderer,
a gameplay frame, or a strict-frontier advance.

```text
SEMANTIC_IMPLEMENTATION=read-only PSX VRAM export
VISIBLE_ARTIFACT=PASS
RENDERED_DISPLAY_FRAME=NO
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L2L3_cut
PLANTED_STATE=NO
GUEST_OR_GPU_WRITES_FROM_DIAGNOSTIC=NO
```

The native route still stops at `func_80030894_L2L3_cut`, followed by the
pre-existing `func_8006AD40_post30894_cut`. Before that boundary, authentic
Disc 1 data has already traversed the translated loader/GPU path into the
single `pe_gpu` VRAM authority. VIS1 makes those words inspectable without
changing execution.

## Implementation and authority

`platform/host_vram.[ch]` adds four host-only operations:

- decode one PSX RGB555/STP word to RGB888;
- copy all 1024x512 authority words to a caller-owned RGB buffer;
- write all words as explicit little-endian raw bytes;
- write the same words as a P6 RGB888 PPM.

Every source word is obtained through `PE_GPU_ReadVRAM`. The module has no
guest-memory write, `PE_GPU_WriteGP0`, `PE_GPU_WriteGP1`, DMA, callback,
scheduler, or presentation call. STP remains metadata and does not alter the
diagnostic RGB value. CLI export happens only after the normal runtime returns.

The two opt-in surfaces are:

```text
--vram-raw PATH         1048576 bytes, 1024x512 little-endian RGB555/STP
--vram-screenshot PATH  P6 PPM, 1024x512 RGB888
```

The independent oracle `tools/visible_vram_oracle.py` imports no production
code. It parses every raw word, performs its own 5-to-8-bit expansion, compares
every output RGB byte, and reports occupancy and hashes.

## Focused contract

The four VIS1 tests establish:

| Test | Contract |
| --- | --- |
| `VIS1_rgb555_vectors` | red/green/blue/white vectors and STP behavior |
| `VIS1_copy_is_read_only` | decoded pixels are exact; GPU state and sampled authority words are unchanged |
| `VIS1_artifacts_match_authority` | raw byte order, PPM byte, exact sizes, and state inertia |
| `VIS1_zero_authority_stays_black` | zero authority produces no visible RGB byte and remains zero |

Focused raw tail:

```text
  TEST VIS1_rgb555_vectors... PASS
  TEST VIS1_copy_is_read_only... PASS
  TEST VIS1_artifacts_match_authority... PASS
  TEST VIS1_zero_authority_stays_black... PASS

Results: 932 run, 4 passed, 0 failed, 928 skipped
RC=0
```

The zero-authority test is the negative control. It proves that the exporter
does not manufacture visible content. The live nonzero words therefore come
from the pre-existing native execution path.

## Real Disc 1 run

Command (the executable was built from this uncommitted rung in an isolated
temporary CMake directory):

```sh
parasite-eve-port --headless \
  --disc-image "rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin" \
  --screenshot /tmp/pe-vis1-r1-host.ppm \
  --vram-raw /tmp/pe-vis1-r1.vram \
  --vram-screenshot /tmp/pe-vis1-r1.ppm \
  --trace /tmp/pe-vis1-r1.trace
```

Raw stderr:

```text
[DISC] opened '/home/blizz/dev/parasite-eve/rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin' (210685 user sectors)
[DISC] boot executable loaded into guest RAM
[TRACE 0001] native_executable_start
[TRACE 0002] call_func_8001220C
[STUB:BOOTSTRAP_RET] func_80030894_L2L3_cut (first invocation)
[STUB:BOOTSTRAP_RET] func_8006AD40_post30894_cut (first invocation)
[TRACE 0003] func_8001220C_returned
[TRACE 0004] shutdown_begin
[SCREENSHOT] /tmp/pe-vis1-r1-host.ppm (320x240)
[VRAM-RAW] /tmp/pe-vis1-r1.vram (1024x512 RGB555/STP little-endian)
[VRAM-SCREENSHOT] /tmp/pe-vis1-r1.ppm (1024x512 RGB555 diagnostic)
[FB] vsyncs=0 drawsyncs=0 presents=1 mask=0 main_iters=1
[HOST] stop_reason=unresolved-boundary
[TRACE 0005] shutdown_end
```

Exit status was zero. The independent oracle output was:

```text
raw_sha256=47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072
ppm_sha256=871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce
nonzero_words=2063
nonzero_bounds=256,64..735,456
decode=PASS all 524288 words / 1572864 RGB bytes
```

Visual inspection of the PPM shows uploaded texture content on black: a small
green concentric/ring-like image near the upper-right of the full VRAM atlas
and a small light region near the lower-left bound. No semantic asset name is
assigned because the pixels alone do not prove one.

The ordinary 320x240 host screenshot remains wholly black and retains its
pre-VIS1 hash:

```text
fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb  pe-vis1-r1-host.ppm
```

That contrast is intentional evidence that VIS1 exposes VRAM contents rather
than silently relabeling the old framebuffer as rendered output.

## Determinism

Three separate normal real-disc runs exited zero. Their artifact hashes were:

```text
47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072  r1.vram
47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072  r2.vram
47388fd370b957a60a85f295094e946aeba160fdc8bf1c89ab7038f2972fd072  r3.vram
871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce  r1.ppm
871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce  r2.ppm
871f3d14e187449d675e655a0f2bb51d6519eaaff8b4d1335b69ec0c17296cce  r3.ppm
```

Raw `cmp` passed for all three raw snapshots and all three PPMs. Host PPM and
trace hashes were also identical across all three runs. A fresh sanitizer
production run generated raw and PPM artifacts byte-identical to the normal
run, and the independent oracle passed again.

## Gates

Full normal suite with `PE_DISC1_BIN` set to the local retail image:

```text
Results: 932 run, 932 passed, 0 failed, 0 skipped
RC=0
```

Fresh `RelWithDebInfo` ASan/UBSan build and suite, with the same image:

```text
Results: 932 run, 932 passed, 0 failed, 0 skipped
SAN_RC=0
```

Sanitized live run and cross-build comparison:

```text
SAN_LIVE_RC=0
decode=PASS all 524288 words / 1572864 RGB bytes
RAW_NORMAL_SAN=IDENTICAL
PPM_NORMAL_SAN=IDENTICAL
```

The frozen retail structural oracle for the current function/frontier also
remains green:

```text
B54J audit oracle: 19 check groups passed.
B54J_RC=0
```

The matching executable and matching-C lane are untouched:

```text
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe
matching-C count: 351
```

## Honest boundary

VIS1 answers a narrower question than a renderer: whether normal native
execution has produced real, reproducible visible data before its current
strict cut. It has. Converting GPU display environment and drawing state into
the actual 320x240 presented frame remains future work. Completing the retail
suffix of `func_80030894` remains a separate semantic frontier. Neither claim
is implied by this diagnostic.
