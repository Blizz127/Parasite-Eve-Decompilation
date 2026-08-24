# Runtime lanes — current-state reconciliation

Status date: 2026-08-24. This is a read-only reconciliation. No runtime,
YAML, build, or verifier files were changed for this report.

## A. UE5 — `Blizz127/parasite-eve-ue5`

The UE5 checkout is not present on this host (`/home/blizz/dev/parasite-eve-ue5`
does not exist), so its executable behavior and current commit cannot be
independently run or inspected here.

The best available evidence is the campaign dashboard, which reports the UE5
`main` tip as `d3ae7db730a05a6cba7d7f0e42a652847c01d67e` and describes a
partial PE-PLAY2 slice: a prefix through Carnegie/theater-related field work,
then an overlay wait drain and `0x89` mode-6 transition. The same dashboard
explicitly labels Day 1 field, text, and battle as `PARTIAL`, and lists
`m0377i→m0012i`, `m0005i` hop verification, and the HP path as pending. It
does not prove an end-to-end Day 1 route through the Eve battle.

Evidence: [CAMPAIGN_DASHBOARD.md](CAMPAIGN_DASHBOARD.md), especially
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

Authoritative tree for this report: `/home/blizz/dev/pe-continuous-decomp`,
branch `grind/continuous-decomp`, HEAD
`2746417bee3a5b42f7984e707a2fc82767d0e2b5`.

The native test executable was run directly:

```text
pc_port/build/pe-native-tests
Results: 928 run, 928 passed, 0 failed, 0 skipped
```

The production executable is not a complete Day 1 field runtime. Its strict
real-disc execution frontier is:

```text
func_80030894_L2L3_cut @ 0x8006B0AC
called from func_8006AD40
```

The bootstrap-disc path has a separate earlier stop at
`func_8007F72C` / the `func_800698D4` mount family. The native executable can
exercise boot, graphics/CD, task-VM, field/battle components, and focused
retail-derived tests, but it cannot currently run through the full first-play
Day 1 field route, theater sequence, or Eve battle end to end.

Run commands from the native tree:

```bash
cd /home/blizz/dev/pe-continuous-decomp/pc_port
./build/pe-native-tests
./build/parasite-eve-port --headless --disc-image "/path/to/Disc 1.bin" \
  --max-frames 1 --trace /tmp/pe-boot.trace
./build/parasite-eve-port --headless --strict-stubs \
  --disc-image "/path/to/Disc 1.bin" --max-frames 2
```

Evidence: [ACTIVE_HANDOFF.md](ACTIVE_HANDOFF.md),
[pe-btl147 report](../evidence/pe-btl147-theater-eve-path/REPORT.md), and
the current native binary/test result above.

The next native rung is scheduler provenance for natural `m0360i` entry. The
BTL148 census found no proven generic scheduler in the available executable;
it deliberately did not add a forced destination or `persist[0] |= 4`.

## C. Matching-C decomp

Authoritative tree for the matching-C lane:
`/home/blizz/dev/parasite-eve`.

At accepted HEAD `15ef1320a5df81fc8b04b82a4847710f1480cba2`,

```text
grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
281
```

That worktree is currently dirty. Its uncommitted experiment changes the
`0x703F0` span from `[0x703F0, asm]` to a six-word C leaf and makes the local
count 282. It is not accepted evidence and must not be reported as the
project count.

The boot ledger is the accepted handoff/parked-blocker record, not the dirty
0x703F0 proposal. The parked set includes unresolved compiler allocation or
scheduling families such as `func_800698D4`, `func_8006E834` (historical
entry now documented as resolved in the handoff but with stale historical
rows), `func_80055724`, `func_80062CE4`, `func_800725DC`/`func_8007264C`,
and `func_8001220C`; each has evidence in
`docs/ai_context/parked_blockers.json` or its named report. The abandoned
0x80077B78–0x80077C38 batch is classification evidence/padding, not C
progress.

Next matching-C rung: resolve the accepted baseline/worktree discrepancy,
then only pursue a separately authorized real leaf; do not count the dirty
0x703F0 proposal until it is reviewed, exact, integrated, and accepted.

Evidence: `/home/blizz/dev/parasite-eve/docs/ai_context/ACTIVE_HANDOFF.md`,
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
here is the native executable's bounded headless behavior and its 928-test
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
- Native `pc_port`: recover the generic event/scheduler decision that naturally
  enters `m0360i`; keep production reachability separately recorded at
  `func_80030894_L2L3_cut`.
- Matching-C: reconcile the dirty 282-count proposal against accepted 281,
  then continue only with an accepted real leaf.
- `parasite-eve-port-black`: no work here; it is a historical branch, not the
  current native authority.
- Scratch/main-verify trees: no next production rung; preserve as historical
  references unless explicitly reassigned.

## Policy/dashboard contradictions

The current sources disagree with measured state in several places:

1. `CAMPAIGN_DASHBOARD.md` is stale: it reports `MATCHED=227`, an old
   `phase6e-b-provider-frontier` native authority, and old UE/native tips.
   The current accepted matching-C HEAD is 281; the current grind native
   authority is HEAD `2746417` with 928/928 tests.
2. The dashboard labels a UE5 prefix “playable” and gives a UE frontier, but
   this host cannot inspect or run that tree. That is external evidence, not a
   current locally verified end-to-end claim.
3. `UE_NATIVE_PARITY_POLICY.md` says the current native suite is 907 tests and
   UE5 has `NO_UE_PLAYABLE_RUNTIME`; the current grind binary actually runs
   928/928, while the UE5 status remains unverified here. The policy's
   acceptance rules are still applicable, but its status table is stale.
4. The matching-C worktree's dirty 282 count must not overwrite the accepted
   281 count in either document until the leaf is reviewed and accepted.

```text
REPORT_STATUS=PRESENTED_UNCOMMITTED
NO_CODE_CHANGES=yes
```
