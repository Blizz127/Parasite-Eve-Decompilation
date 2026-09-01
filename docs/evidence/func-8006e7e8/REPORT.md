# func_8006E7E8 — morning corrections 1–3 (2026-08-23)

Baseline of record: `481a911`, **279** matching C leaves, EXACT SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. Post-baseline commits
`278006a` (6E7E8) and `9eec0f9` (17EC4) were reverted; candidates
preserved under `docs/evidence/unreviewed-17exx/`. Everything below is
presented UNCOMMITTED.

## Correction 1 — record fixed: PARKED-INTEGRATION-ERROR, NOT "link order disruption"

The initial park diagnosis ("link order disruption", with a recommendation
to adjust the linker script) was wrong and is retracted. No linker change
was made, none is needed, and the linker-adjustment recommendation is
DELETED. The ld script's ROM-order section lists are untouched.

Actual mechanism: **PARKED-INTEGRATION-ERROR → RESOLVED** — a botched carve
registration in the first attempt (preserved verbatim in `stash@{0}`,
"WIP on tooling/maspsx-expand-div"):

| Item | Failed attempt (`stash@{0}`) | Correct |
| --- | --- | --- |
| `SIZE_5B1E4` | `0x3e10` (16-byte-ALIGNED prefix length) | `0x3e04` = `0x5EFE8 − 0x5B1E4` (true prefix) |
| Consequence | trimmed prefix object keeps 12 extra bytes → C leaf lands at `0x5EFF4` (+0xC), downstream shift → link/SHA mismatch resembling a link-order problem | C leaf lands exactly at `0x5EFE8`; resume `0x5F034` |

Additional record defects noted: commit `278006a`'s message names the four
register pins with EMPTY parentheses ("return (), temp (), mask (), ptr ()"),
and its pin-necessity claim is falsified by Correction 2.

## Correction 2 — C4 audit of the four register pins

Method: six-way compile matrix through the exact `era_compile` pipeline
(cpp → cc1 `-O2 -G0` → maspsx `--aspsx-version=2.21 --dont-expand-li` → GNU as),
each object's `.text` compared byte-wise against the retail window
(`SLUS_006.62` @ file `0x5EFE8`, len `0x4C`, sha256
`968f5fb0f55b63c6b38d3a2cc155e06bd89095b6e621c665590d280501e54c89`).
Variants differing from retail ONLY in link-time reloc placeholders
(`jal` R_MIPS_26; HI16/LO16 for `D_800B0CD8`) plus the ELF tail pad are
mutually identical codegen.

| Variant | Change vs all-pins candidate | `.text` result |
| --- | --- | --- |
| `v_all` | — (all four pins, as committed in 278006a) | identical to `v_none` |
| `v_none` | NO pins at all | identical to `v_all` |
| `v_no_r5` | drop `r` ($5) pin | identical to `v_all` |
| `v_no_a0mask` | drop `mask` ($4) pin | identical to `v_all` |
| `v_no_v1ptr` | drop `ptr` ($3) pin | identical to `v_all` |
| `v_no_v0` | drop `t` ($2) pin | **DIFFERENT — collapses to an 18-word shape** |

Verdict:

| Binding | C4 verdict |
| --- | --- |
| `register int t asm("$2")` | **NECESSARY — kept** (sole structural lever) |
| `register int r asm("$5")` | INERT — removed |
| `register unsigned int mask asm("$4")` | INERT — removed (mask builds in `$a0` naturally: `lui $a0,0xFEFF` / `addiu $a0,$a0,-0x4001`) |
| `register unsigned int *ptr asm("$3")` | INERT — removed (`&D_800B0CD8` builds in `$v1` naturally) |

This FALSIFIES 278006a's claim that the pins "reproduce retail's
branch-delay mask load and branch-target pointer load" — those loads are
natural `-O2 -G0` output. The corrected candidate (`src/func_8006E7E8.c`)
carries exactly one pin. Harness preserved at `/tmp/opencode/audit6e7e8/`
(v_* sources + run_audit.sh); objects regenerated reproducibly.

## Correction 3 — diagnostic re-integration

Templates (same-file mid-segment carves): `func_8003E610` mid-`2E7D8`
(yaml line ~314; `SIZE_2E7D8=0x638` / `SIZE_C_3E610=0x70`) and
`func_8002F970` mid-`11718` (`SIZE_11718=0x8470` / `SIZE_C_2F970=0x5c`).

Geometry arithmetic (shown):

```
prefix : 0x5B1E4 → 0x5EFE8 = 0x5EFE8 − 0x5B1E4 = 0x3E04   (SIZE_5B1E4)
C leaf : 0x5EFE8 → 0x5F034 = 19 words       = 0x4C        (SIZE_C_6E7E8)
resume : 0x5F034 = existing C leaf func_8006E834 — no .s resume needed
```

Registrations applied (mirroring template set):
yaml comment + `[0x5EFE8, c, func_8006E7E8]` between `[0x5B1E4, asm]` and
`[0x5F034, c, ...]`; build_us.sh geometry comments, `SIZE_5B1E4 0x3e50→0x3e04`,
`SIZE_C_6E7E8=0x4c`, SOURCES + OBJECTS entries, `era_compile … -O2 -G0`,
trim line, four ld section refs. Re-split via splat (asm/ back in sync).

Diff vs failed attempt: exactly one functional delta — `SIZE_5B1E4=0x3e04`
(true prefix) instead of `0x3e10` (aligned prefix), per the table above;
plus the audited pin reduction in the candidate itself.

### Gates

| Gate | Result |
| --- | --- |
| `grep -c ',\s*c,' configs/USA/disc1.yaml` | **280** (279 + 1) |
| Trim lines | `5B1E4.s.o .text: 0x3E10→0x3E04`; `func_8006E7E8.c.o .text: 0x50→0x4C` |
| docker `scripts/build_us.sh` | Assemble/Compile/Trim/Link/Pack OK; **Compare: EXACT SHA-1 MATCH** `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |
| host `scripts/verify_us.sh` | exit 0; "candidate SHA-1 452fb033… compare: EXACT MATCH"; Phase 5HD-12850 header, 280 leaves |

### Pre-existing out-of-scope observations (no action taken)

1. `tools/analysis/at_absolute_store_counter.py` hard-fails on BOTH states
   of the zero-width yaml row `[0x96E0, asm]` (5GN-era leftover,
   pre-481a911): with the old orphan `96E0.s` present it flags stale
   glabels; absent it flags a missing unit. Authoritative gates are
   unaffected (build + verify green). Needs a maintainer ruling: drop the
   zero-width row or teach the counter to skip zero-width segments.
2. The reverted `278006a` also carried a corrupt commit message (empty pin
   parens) — moot after revert, recorded here for history.

## Bounded pass on the surviving `$2` pin (review verdict 2026-08-23)

**Collapse identified.** Natural pinless C (`t = ret+1` named temp) compiles
to **17 words vs retail 19**: cc1 coalesces `ret` and `t` into one web
rooted at `$v0` (return-in-place) and pushes the RMW scratch to `$a1`.
The two missing words are exactly retail's paired `$a1`-home moves:
w05 `move a1,v0` (home established after `jal`) and w17 `move v0,a1`
(home retired before `jr`). Everything else — frame, arg delay slot,
`addiu/sltiu/beqz` range test, mask build order, pointer reuse for
lw/sw — is already identical.

Two structural attempts (R6 bound), same pipeline, `.text` vs retail:

| Attempt | Phrasing | Result |
| --- | --- | --- |
| S1 | make `t` the live-out value: `return next - 1;` | **19 words, retail shape restored** (paired moves back) |
| S2 | temp eliminated: test written INLINE — `if ((unsigned int)(pollStatus + 1) < 2)` | **19 words, retail shape restored, PINLESS** |

S2 closes it. The named-temp form was itself the blocker: naming `t`
gives cc1 one merged web; the inline form keeps `pollStatus` an
accumulator and cc1 re-establishes the secondary home naturally.
Integrated source is S2 with semantic names (`ioArgs`, `pollStatus`,
`pollMask`, `flagsPtr`) and zero register bindings.

### Final gates (pinless)

| Gate | Result |
| --- | --- |
| `grep -c ',\s*c,' configs/USA/disc1.yaml` | **280** |
| Trim lines | `5B1E4.s.o .text: 0x3E10→0x3E04`; `func_8006E7E8.c.o .text: 0x50→0x4C` |
| docker `scripts/build_us.sh` | **Compare: EXACT SHA-1 MATCH** `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |
| host `scripts/verify_us.sh` | exit 0; EXACT MATCH; 280 leaves |

Handoff banking: carve-size boundary-arithmetic rule added to the
fingerprint table (per verdict), plus the inline-range-test lever so no
future agent re-introduces the unnecessary pin.

Harness/artifacts preserved at `/tmp/opencode/audit6e7e8/`
(six-way matrix sources, s1/s2 shape sources, run_audit.sh,
run_shapes.sh, dis assemblies).

## State

Tree restored to `481a911` first (candidates preserved in
`docs/evidence/unreviewed-17exx/`), then corrections applied on top.
All of the above is UNCOMMITTED pending review. No new targets started.

**Final presented state (post-verdict bounded pass):** `src/func_8006E7E8.c`
is PINLESS (S2 inline-compare shape); yaml + build-script comments updated
to match; gates re-run green (280 leaves, EXACT SHA-1 both directions).
Ready to commit at 280 on accept.

## Pin-question record — complete presentation

### 1. Natural-C collapse: 18 emitted words versus the 19-word ROM leaf

The natural named-temporary form was:

```c
pollStatus = func_800811E4(&ioArgs);
unsigned int t = (unsigned int)(pollStatus + 1);
if (t < 2) { ... }
return pollStatus;
```

With no `$2` result-home pin, cc1 merged the call result and the named test
temporary into one `$v0` web. The resulting 18-word instruction shape was:

```asm
8006e7e8: 27bdffe0  addiu sp,sp,-32
8006e7ec: afbf0018  sw    ra,24(sp)
8006e7f0: 0c000000  jal   0x80000000
8006e7f4: 27a40010  addiu a0,sp,16
8006e7f8: 24430001  addiu v1,v0,1
8006e7fc: 2c630002  sltiu v1,v1,2
8006e800: 10600007  beqz  v1,0x8006e820
8006e804: 3c04feff  lui   a0,0xfeff
8006e808: 3c030000  lui   v1,0
8006e80c: 24630000  addiu v1,v1,0
8006e810: 8c650000  lw    a1,0(v1)
8006e814: 3484bfff  ori   a0,a0,0xbfff
8006e818: 00a42824  and   a1,a1,a0
8006e81c: ac650000  sw    a1,0(v1)
8006e820: 8fbf0018  lw    ra,24(sp)
8006e824: 27bd0020  addiu sp,sp,32
8006e828: 03e00008  jr    ra
8006e82c: 00000000  nop
```

The collapse is the loss of the accumulator-register home shape: ROM has
`move a1,v0` immediately after the call and `move v0,a1` immediately before
the return. The merged `$v0` web makes the first home move unnecessary to
cc1 and leaves no live `$a1` home from which to restore the result; the
second home move therefore disappears as well. The epilogue is consequently
scheduled one word earlier in the collapsed shape. The two visible ROM home
instructions are the exact structural evidence of the merged web; the
packed emitted shape is 18 words rather than ROM's 19.

`MISSING_OR_COLLAPSED_WORD=` `0x00402821 move a1,v0` (ROM word 5) and
`0x00a01021 move v0,a1` (ROM word 17) are the paired accumulator-home
instructions absent from the 18-word object; the intervening result/test web
was register-folded into `$v0` and the RMW scratch was carried in `$a1`.

### 2. Closing phrasing and final C

The closing structural change was to remove the named temporary and write the
range test inline, keeping `pollStatus` as the accumulator while cc1 creates
the secondary `$a1` result home naturally:

```c
extern int func_800811E4(void *);
extern unsigned int D_800B0CD8;

int func_8006E7E8(void)
{
    int ioArgs;
    int pollStatus;

    pollStatus = func_800811E4(&ioArgs);
    if ((unsigned int)(pollStatus + 1) < 2) {
        unsigned int pollMask = 0xFEFFBFFF;
        unsigned int *flagsPtr = &D_800B0CD8;
        *flagsPtr &= pollMask;
    }
    return pollStatus;
}
```

This pinless inline-compare phrasing restores the two home moves:

```asm
move a1,v0       # home the call result before the range-test accumulator
...
move v0,a1       # restore the original result before jr
```

It produces 19 instruction words, matching ROM, with no hard-register
bindings.

### 3. Complete 19-word packed-span comparison

The following is the complete leaf span `0x8006E7E8..0x8006E833` (19 words).
The candidate column is the linked/packed candidate; its words equal ROM at
every offset. The unlinked per-object disassembly uses relocation placeholders
for the `jal` and absolute global address, but those relocations resolve to
the candidate words shown here.

| # | Address | ROM word / instruction | Candidate word / instruction |
|---:|---:|---|---|
| 1 | `0x8006E7E8` | `27bdffe0` — `addiu sp,sp,-32` | `27bdffe0` — exact |
| 2 | `0x8006E7EC` | `afbf0018` — `sw ra,24(sp)` | `afbf0018` — exact |
| 3 | `0x8006E7F0` | `0c020479` — `jal 0x800811e4` | `0c020479` — exact |
| 4 | `0x8006E7F4` | `27a40010` — `addiu a0,sp,16` | `27a40010` — exact |
| 5 | `0x8006E7F8` | `00402821` — `move a1,v0` | `00402821` — exact |
| 6 | `0x8006E7FC` | `24a20001` — `addiu v0,a1,1` | `24a20001` — exact |
| 7 | `0x8006E800` | `2c420002` — `sltiu v0,v0,2` | `2c420002` — exact |
| 8 | `0x8006E804` | `10400007` — `beqz v0,0x8006e824` | `10400007` — exact |
| 9 | `0x8006E808` | `3c04feff` — `lui a0,0xfeff` | `3c04feff` — exact |
| 10 | `0x8006E80C` | `3c03800b` — `lui v1,0x800b` | `3c03800b` — exact |
| 11 | `0x8006E810` | `24630cd8` — `addiu v1,v1,0xcd8` | `24630cd8` — exact |
| 12 | `0x8006E814` | `8c620000` — `lw v0,0(v1)` | `8c620000` — exact |
| 13 | `0x8006E818` | `3484bfff` — `ori a0,a0,0xbfff` | `3484bfff` — exact |
| 14 | `0x8006E81C` | `00441024` — `and v0,v0,a0` | `00441024` — exact |
| 15 | `0x8006E820` | `ac620000` — `sw v0,0(v1)` | `ac620000` — exact |
| 16 | `0x8006E824` | `8fbf0018` — `lw ra,24(sp)` | `8fbf0018` — exact |
| 17 | `0x8006E828` | `00a01021` — `move v0,a1` | `00a01021` — exact |
| 18 | `0x8006E82C` | `03e00008` — `jr ra` | `03e00008` — exact |
| 19 | `0x8006E830` | `27bd0020` — `addiu sp,sp,32` | `27bd0020` — exact |

Thus the complete packed span is byte-identical, not merely instructionally
similar: all 19 words match, and the full executable SHA-1 remains
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
