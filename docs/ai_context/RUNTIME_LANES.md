# Runtime lanes — current-state reconciliation

Status date: 2026-08-30. This file is the maintained cross-lane status; the
native and matching sections below were revalidated from their authoritative
worktrees during the B54K-H rung.

## A. UE5 — `Blizz127/parasite-eve-ue5`

The UE5 checkout is not present on this host (`/home/blizz/dev/parasite-eve-ue5`
does not exist), so its executable behavior and current commit cannot be
independently run or inspected here.

The best available local evidence is the now-superseded campaign dashboard,
which historically reported the UE5
`main` tip as `d3ae7db730a05a6cba7d7f0e42a652847c01d67e` and describes a
partial PE-PLAY2 slice: a prefix through Carnegie/theater-related field work,
then an overlay wait drain and `0x89` mode-6 transition. The same dashboard
explicitly labels Day 1 field, text, and battle as `PARTIAL`, and lists
`m0377i→m0012i`, `m0005i` hop verification, and the HP path as pending. It
does not prove an end-to-end Day 1 route through the Eve battle.

Historical evidence: [CAMPAIGN_DASHBOARD.md](CAMPAIGN_DASHBOARD.md), especially
`CURRENT_UE_FRONTIER`, `DAY1_*`, and the UE task list. The parity policy
currently says `NO_UE_PLAYABLE_RUNTIME`, so no UE5 Eve-battle behavior can be
accepted from this host.

Current known gaps from those records:

- Disc verification for the theater-side `m0377i→m0012i` and `m0005i` hops.
- The `0x95 mode 0 → 0x299CC → 0x1D340 → 0x1F704` HP path is not parity-accepted.
- The UE overlay `anim_remain_0F` clear is a proxy; the retail `func_8006CC2C`
  relationship remains open.
- Camera/door/theater path and black-gap behavior are not established as a
  completed, row-equal retail slice.
- No evidence here proves Aya controllability through a complete Eve battle;
  battle work remains partial.

UE5 consumes native-lane findings through published evidence and parity
promotion, not through an executable/runtime dependency that can be verified
in this checkout. The dashboard's HP finding names native BTL98 evidence as a
future UE parity input, but records that promotion as not yet accepted.

## B. Native `pc_port`

Authoritative tree: `/home/blizz/dev/pe-continuous-decomp`, branch
`grind/continuous-decomp`.

The native test executable was run directly:

```text
pc_port/build/pe-native-tests
Results: 944 run, 944 passed, 0 failed, 0 skipped
```

The production executable is not a complete Day 1 field runtime. Its strict
real-disc execution frontier is:

```text
func_80030894_L11_cut (first excluded retail instruction 0x80031438)
func_80030894 is called from func_8006AD40 by jal @ 0x8006B0AC
```

The bootstrap-disc path has a separate earlier stop at
`func_8007F72C` / the `func_800698D4` mount family. The native executable can
exercise boot, graphics/CD, task-VM, field/battle components, and focused
retail-derived tests, but it cannot currently run through the full first-play
Day 1 field route, theater sequence, or Eve battle end to end.

Run commands from the native tree:

```bash
cd /home/blizz/dev/pe-continuous-decomp
./pc_port/build/pe-native-tests
./pc_port/build/parasite-eve-port --headless --disc-image "/path/to/Disc 1.bin" \
  --max-frames 1 --trace /tmp/pe-boot.trace
./pc_port/build/parasite-eve-port --headless --strict-stubs \
  --disc-image "/path/to/Disc 1.bin" --max-frames 2
```

Evidence: [ACTIVE_HANDOFF.md](ACTIVE_HANDOFF.md),
[B54K-B1 report](../evidence/pe-b54kb1-30894-l4/REPORT.md),
[B54K-B2 report](../evidence/pe-b54kb2-30894-l5/REPORT.md),
[B54K-C report](../evidence/pe-b54kc-30894-l6/REPORT.md),
[B54K-D report](../evidence/pe-b54kd-30894-l7/REPORT.md),
[B54K-E report](../evidence/pe-b54ke-30894-l8/REPORT.md),
[B54K-F report](../evidence/pe-b54kf-30894-l9/REPORT.md),
[B54K-G report](../evidence/pe-b54kg-30894-l10/REPORT.md),
[B54K-H report](../evidence/pe-b54kh-30894-l11/REPORT.md),
[pe-btl147 report](../evidence/pe-btl147-theater-eve-path/REPORT.md), and the
current native binary/test result above.

The next scheduler rung is the human-driven BTL151 PCSX capture of
`func_8006E3D4` inputs. Static provenance is exhausted and no forced
destination or `persist[0] |= 4` is permitted. Independently, the next
artifact-free production-reachability rung is the final 43-word
TPage/fixed-sprite/outer-loop epilogue after L11.

## C. Matching-C decomp

Authoritative tree for the matching-C lane:
`/home/blizz/dev/parasite-eve`.

At accepted HEAD `634dd1b`, the worktree is clean and the authoritative exact
count is:

```text
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
335
```

There are also 19 policy-qualified accepted residuals. They remain ASM in the
YAML, are excluded from the 335 exact count, and are not byte-identity claims.
`func_8007FBF0` (`0x703F0`) is one of those documented assembler-temp
residuals; no unaccepted C integration remains in the tree.

The current ordinary-C campaign is stopped at its standing hard gate after
three consecutive bounded parks (`func_8007E6B0`, `func_8005BCBC`, and
`func_80083D9C`). Sixteen Tier-1 rows remain queued, but the queue requires
review/refresh before another attempt rather than silently bypassing the
stop.

Evidence: `/home/blizz/dev/parasite-eve/docs/ai_context/ACTIVE_HANDOFF.md`,
`/home/blizz/dev/parasite-eve/docs/ai_context/MATCHING_RESIDUAL_POLICY.md`,
`parked_blockers.json`, and the accepted-HEAD YAML count above.

## D. Other PE trees

| Tree | Current state | Tip |
|---|---|---|
| `/home/blizz/dev/parasite-eve-port-black` | Live historical native/decomp worktree, branch `phase6e-b-provider-frontier`; clean at inspection. It is not the current grind authority. | `c1efff529e1529893c2ae73f78637b3677c30f77`, 2026-08-14, “Phase 6E-B54B: complete func_8006AD40 counted loop” |
| `/home/blizz/dev/parasite-eve-scratch` | Directory exists but is not a Git worktree; no current lane authority found. Treat as abandoned/scratch. | No Git tip |
| `/home/blizz/dev/pe-decomp-main-verify` | Separate historical verification worktree, branch `pe-btl121-tid406`; not live authority for current matching/native state. | `ae8b5243afcc39fa630561c9dd60cbe1c25f9d98`, 2026-08-18, “PE-BTL121: 55610/556E8 pack C0E24 bits into the menu index table.” |
| `/home/blizz/dev/parasite-eve-ue5` | Missing on this host; external UE5 tree only. | Not locally verifiable |

## Answers

### 1. Best playable experience today

No complete playable Day 1→theater→Eve-battle runtime is verified on this
host. The external dashboard describes UE5 as the best *partial* interactive
slice, while the native runtime is currently a headless/testable bootstrap
and component runtime. The only directly runnable authoritative experience
here is the native executable's bounded headless behavior and its 944-test
suite; it is not a complete game route.

Native commands are listed in section B. A UE5 run command cannot be stated
authoritatively because the UE5 checkout and its project/run documentation are
not present on this host.

### 2. Single next change with greatest impact

For the best partial runtime, the highest-impact change is to close and
promote the retail-verified theater-to-first-Eve-battle transition contract:
for UE5 this means proving the `m0377i/m0012i` and `m0005i` hops and the
row-equal `0x95 → 0x299CC → 0x1D340 → 0x1F704` HP path. For the native lane,
the corresponding prerequisite is the missing generic scheduler provenance
for natural `m0360i` entry; it must be proven before implementation.

### 3. Next rung per lane

- UE5: obtain/verify the retail theater-side hops, then promote the HP path
  under the parity trace contract.
- Native `pc_port`: capture the generic event/scheduler decision that naturally
  enters `m0360i`; separately continue the artifact-free production prefix
  from `func_80030894_L11_cut` through the final 43-word epilogue.
- Matching-C: review/refresh the 16-row Tier-1 queue after the three-park hard
  stop; retain 335 exact plus 19 explicitly non-exact residuals.
- `parasite-eve-port-black`: no work here; it is a historical branch, not the
  current native authority.
- Scratch/main-verify trees: no next production rung; preserve as historical
  references unless explicitly reassigned.

## Known documentation boundaries

1. `CAMPAIGN_DASHBOARD.md` is explicitly superseded and retained only as
   historical UE5 context. It must not supply current counts or frontiers.
2. This host still cannot inspect the UE5 checkout, so no locally verified
   end-to-end UE5 claim is possible.
3. `UE_NATIVE_PARITY_POLICY.md` retains useful acceptance rules but its status
   table is historical; this file carries the measured 944-test native state.
4. Matching residuals are evidence dispositions, not exact leaves: 335 is the
   only YAML-derived matching-C count.

```text
REPORT_STATUS=CURRENT
LAST_REFRESH=2026-08-30_B54K-H
```
