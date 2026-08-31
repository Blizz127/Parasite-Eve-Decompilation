# PE-B54K-AC — restore the retail CD sector contract

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

While extending `func_801924F8` past `0x80192584`, the first read of its
overlay-resident filename table exposed an older unit mismatch in the host CD
provider. `func_8006E834` supplied `0x0457 - 0x03D2 = 0x85`, but the provider
treated that value as bytes. Retail treats it as sectors. The prior path had
therefore loaded only 133 bytes of a 133-sector overlay and happened to remain
viable only because the translated prefix had not yet read beyond those bytes.

## Retail proof

The SHA-1-exact executable proves the unit without relying on names:

```text
func_8006E6A8  a2 -> a3; jal func_8006E6D4
func_8006E6D4  incoming a3 -> s3
func_8006E6D4  s3 -> a1; jal func_80080E34
func_80080E34  incoming a1 -> s4
func_80080E34  sw s4,D_8009B6B4
```

`func_8006E834` loads two halfwords, computes `end - start` in the `jal`
delay slot, and passes that result unchanged as `a3`:

```text
D_80093164[0] = 0x03D2
D_80093164[1] = 0x0457
count          = 0x0085 sectors / 133 sectors
bytes          = 133 * 0x800 = 0x42800
destination    = [0x8018EFF0,0x801D17F0)
```

The independent PE.IMG identities close the geometry:

```text
PE.IMG SHA-1    146c0ce7308bf9fdc2ba5a84230e198db0663f3b
overlay SHA-1   a0b604ead810592dd9a8d74f7c74fe94151f4d32
overlay FNV-1a  55ec1574df7d6a3d
```

The already-documented `func_8006A9E4` consumers independently agree: a
count of 34 precedes a `0x10A50`-byte copy and a count of 3 precedes a
`0x1400`-byte copy. Those sizes fit 34 and 3 sectors respectively and cannot
fit 34 and 3 bytes.

## Native correction

`func_8006E6D4` now accepts a sector count and performs the host adaptation in
one place: checked `sectors * 0x800` bytes are copied synchronously from the
disc. Negative, overflowing, source-truncated, and guest-range-invalid reads
still fail before data mutation and clear the same command-state bits.

The two callers that previously compensated for the wrong provider contract
now forward retail values unchanged:

- `func_8006E6A8(..., sectors)` no longer shifts by 11.
- `func_8006CDA4_state7_cut(..., chunk)` no longer shifts by 11.

The explicit CLI load smoke converts its byte cap to a sector count before
calling the provider. Historical B16 and BTL6 reports remain unchanged as
records of the then-current adaptation; their statements that the host
provider takes bytes are superseded by this retail proof.

## Verification

The real-disc B54K-Y contract now executes `func_8006E834`, hashes all
`0x42800` loaded bytes, checks the exclusive-end canary, and verifies the
record-1 pointer at `0x801D0E14`. The general read tests cover exact-end,
one-byte overflow, truncated source, repeated sectors, zero sectors, negative
counts, and oversized counts.

```text
B54K-AC independent oracle: PASS
normal CTest:                2/2 PASS
native suite:                985/985
fresh ASan/UBSan CTest:      2/2 PASS
strict real-disc exit:       1
strict frontier:             func_801924F8_80192584_cut
disc-load smoke:             PASS, 32768 bytes
retail EXE SHA-1:            452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

No scheduler state, destination token, persistence bit, or overlay table was
planted. Every overlay byte in the positive contract comes from the registered
retail Disc 1 image through the generic CD path.

```text
CD_READ_UNIT=RETAIL_SECTORS
OVERLAY_LOAD=[03D2,0457)_TO_[8018EFF0,801D17F0)_EXACT
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut
NEXT_ARTIFACT_FREE_RUNG=continue_func_801924F8_filename_search
```
