# PE-B54K-O — recover the `func_801909B4` overlay bytes

Status: **RETAIL BYTES RECOVERED; NOT IMPLEMENTED**.

The next canonical boundary predicted after a future completed
`func_8006AD40` is not an opaque runtime-only function. Retail tables map it
exactly into PE.IMG, and the local private retail artifact permits a complete
static extraction.

## Proven mapping

Retail rodata—not BSS—contains:

```text
D_80011614 = 0x8018EFF0
D_80093164 = 0x03D2
D_80093166 = 0x0457
```

`func_8006E834` reads sectors `[0x03D2,0x0457)` from PE.IMG into
`D_80011614`. Therefore:

```text
overlay byte start = 0x03D2 * 0x800               = 0x001E9000
function VA delta  = 0x801909B4 - 0x8018EFF0      = 0x000019C4
function PE.IMG off = 0x001E9000 + 0x000019C4      = 0x001EA9C4
```

The identities are:

```text
PE.IMG SHA-1       146c0ce7308bf9fdc2ba5a84230e198db0663f3b
overlay range      PE.IMG [0x1E9000,0x22B800)
overlay load range [0x8018EFF0,0x801D17F0)
overlay size       0x42800 bytes / 133 sectors
overlay SHA-1      a0b604ead810592dd9a8d74f7c74fe94151f4d32
function range     [0x801909B4,0x801918F8)
function size      0xF44 bytes / 977 words
function SHA-256   9072713338b26c335c1a31964282105dcd5f14951950c2d24585fa5948554d30
next function      0x801918F8 (addiu sp,sp,-0x38)
```

The function has a normal `jr ra; nop` return at `0x801918F0`, 70 direct
call sites to 32 unique targets, and returns the retained `$s2` value. Its
first instructions copy fixed main-executable records into overlay storage;
later blocks initialize display resources and call both executable and
overlay-local helpers. This is a substantial translation target, not a safe
stub or a tiny provider.

## Native prerequisite correction

This extraction also falsifies two comments/initializers in the current
native bootstrap model:

- `pc_port/src/pe_globals.c` initializes `D_80093164` to zeros and calls it
  BSS, but retail rodata supplies `{0x03D2,0x0457,...}`.
- native `D_80011614=0x8010BD00` is explicitly a bootstrap-policy address;
  retail's loaded pointer is `0x8018EFF0`.

Consequently current native `func_8006E834` performs a zero-length read and
does **not** populate the retail overlay. B54K-N correctly removes the generic
FlushCache boundary, but production cannot claim natural entry into
`func_801909B4` until a separate rung restores these table/pointer authorities,
tests the exact 0x42800-byte bounded load, and reconciles the existing arena
layout tests. No initializer was changed in this audit.

## Reproduction

The private artifact contains `runtime/data/disc1.bin`. Extraction used only
tracked tooling and `/tmp` scratch:

```bash
python3 tools/extract/psxiso.py extract DISC1.bin PE.IMG /tmp/PE.IMG
python3 pc_port/tools/b54ko_1909b4_overlay_oracle.py /tmp/PE.IMG
```

```text
  OK PE.IMG and [03D2,0457) overlay identities
  OK func_801909B4: 0xF44 bytes / 977 words / normal return
  OK call census: 70 sites / 32 unique targets
  OK next function starts at 0x801918F8

B54K-O overlay oracle: PASS.
```

The next honest artifact-free work is the retail overlay-load authority in
6E834, followed by decomposition of this 977-word function into coherent
subsystems. This finding does not change the current production frontier or
the 954-test count.

```text
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_D_80093126_archive_cut
FUNC_801909B4_BYTES=STATICALLY_RECOVERED_NOT_IMPLEMENTED
OVERLAY_LOAD_AUTHORITY=RETAIL_RODATA_VALUES_MISSING_IN_NATIVE
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```
