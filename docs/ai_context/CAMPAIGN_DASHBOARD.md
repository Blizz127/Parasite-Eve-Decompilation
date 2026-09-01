# SUPERSEDED_BY_RUNTIME_LANES

This historical dashboard is not current authority. Cross-lane status lives in
`~/dev/pe-continuous-decomp/RUNTIME_LANES.md`.

# Overnight campaign dashboard

```text
STATUS=SUPERSEDED_BY_RUNTIME_LANES
AUTHORITATIVE_CURRENT_STATUS=docs/ai_context/RUNTIME_LANES.md
DO_NOT_USE_THIS_FILE_FOR_CURRENT_COUNTS_OR_FRONTIERS=yes
```

This file is retained as historical campaign context only. For current
cross-lane runtime status, verified counters, execution frontiers, commands,
and policy contradictions, read `docs/ai_context/RUNTIME_LANES.md`.

Host writing this file: `Blizz-AW.localdomain`
Lane: matching decomp / native only.
UE5 tree on this machine is **not** authority. Dirty/uncommitted
state on the other VPS is never authority.

Exchange only: pushed SHA, branch/ref, retail slice hashes,
reports/contracts, captures, machine-readable state.

## Repos (durable)

| Lane | Repo | Writing host | Pushed ref | Pushed SHA | Local writing SHA |
|---|---|---|---|---|---|
| DECOMP/NATIVE | `parasite-eve-port-black` (`phase6e-b-provider-frontier`) | this workstation; origin `ovh-dev:/home/blizz/dev/parasite-eve` | `origin/phase6e-b-provider-frontier` | `c1efff529e1529893c2ae73f78637b3677c30f77` (B54B) | `a6edd081963cca3ba512c8b34d038b2b2b207f9a` (BTL98, unpushed, ahead 143) |
| DECOMP public | `Blizz127/Parasite-Eve-Decompilation` | other | `main` | `dfa25285dd820763434cd1086176c1753fcaf04c` | not this lane |
| UE5 | `Blizz127/parasite-eve-ue5` | other VPS / `ovh-pe-ue5` | `main` | `d3ae7db730a05a6cba7d7f0e42a652847c01d67e` | do not use |
| UE5 cloud branch | same | other | `cursor/parasite-eve-battle-runtime-46c4` | `c700b30bfe792b7fbc0ff13681ff4254a2d0745c` | do not use |

OVH origin default `HEAD` is `9273109ea8f355f4828176af0ac38235e057eaad`,
not the frontier branch. Do not treat it as this checkout.

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`
(verified `build/disc1.candidate.exe`).

DECOMP_HEAD=`79f25888e05a74ba56bc5e3c5eee0df28e064ae2` (local writing BTL99; not origin)
UE_HEAD=`d3ae7db730a05a6cba7d7f0e42a652847c01d67e` (GitHub `main` only)

## Match counters (this checkout)

MATCHED=227
NONMATCHING_C=0
ASM_REMAINING=UNKNOWN (152 yaml `asm` segments; not a function count)
STUBS=UNKNOWN (native shim inventory; not matching classification)
MATCHED_BYTES=UNKNOWN
BYTE_MATCH_PERCENT=UNKNOWN
NATIVE_TRANSLATED=`matching_native=828/828` claimed at BTL98 (functional ports, **not** MATCHED C)

Yaml `grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml` = 227.
Do not subtract yaml asm segments from 227.

## Frontiers

CURRENT_DECOMP_TARGET=`6DE80` after `1A680` (parked)
CURRENT_DECOMP_FRONTIER=`1F814` jtbl+305C8+1A680 live; `6DE80`/`1F078` death parked
CURRENT_UE_FRONTIER=`PE-PLAY2` 0x55 overlay wait drain → 0x89 mode 6; disc verify m0377i→m0012i and m0005i hop
CURRENT_DAY1_BLOCKER=UE5 disc verification of hops; decomp death/teardown after HP

## Cross-lane

UE_AHEAD_FINDINGS=UE5 PLAY2 overlay_d1a0 bit1 clear on type-0 `anim_remain_0F==0`
DECOMP_AHEAD_FINDINGS=BTL98 `1D340`/`1F704` HP 40→39
CROSS_LANE_CONTRADICTIONS=none proven; UE5 overlay proxy vs retail `6CC2C` is OPEN

### Finding 1 — HP mutation

```
SOURCE_HOST=Blizz-AW.localdomain
SOURCE_REPO=parasite-eve-port-black
SOURCE_SHA=a6edd081963cca3ba512c8b34d038b2b2b207f9a
EVIDENCE_PATH=docs/evidence/pe-btl83-retail-battle-transition/
RETAIL_AUTHORITY=SLUS_006.62 SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b; 1F704 sh +0x0C
FINDING=Native BTL98 reaches 1D340 after 0x95 mode 0; 1F704 subtracts HP 40→39. UE5 main d3ae7db has not published this path.
TARGET_LANE=UE5
VERIFICATION_STATUS=UE_PARITY_REQUIRED
```

### Finding 2 — overlay_d1a0 clear proxy

```
SOURCE_HOST=github (pushed)
SOURCE_REPO=Blizz127/parasite-eve-ue5
SOURCE_SHA=d3ae7db730a05a6cba7d7f0e42a652847c01d67e
EVIDENCE_PATH=docs/ai_context/ACTIVE_HANDOFF.md (PE-PLAY2)
RETAIL_AUTHORITY=func_8006CC2C andi 0xFC overlay epilogue (decomp BTL5/BTL81)
FINDING=UE5 clears overlay_d1a0 bit 1 when type-0 anim_remain_0F reaches 0, then 0x55 advances to 0x89. Retail writer is 6CC2C, not that host heuristic.
TARGET_LANE=DECOMP
VERIFICATION_STATUS=DECOMP_VERIFICATION_REQUIRED
```

## Next tasks

NEXT_5_MATCH_TASKS=
1. New small unmatched leaf (parked 698D4/6E834/374E8/62CE4/55724/main are BLOCKED cc1 skew)
2. Twin/search-and-clear family after 5FH 37548
3. Width-only setters still in parked_blockers TYPING-POLICY
4. Do not reopen 698D4 dbr_sched without a cc1 lever
5. Census TOTAL_FUNCTIONS from splat glabels (yaml, not asm/ disk)

NEXT_5_RUNTIME_TASKS=
1. BTL99: after 1F704, if HP!=0 jal 1F814 (audit in pe-btl99-1f814-post-hp)
2. 1F814 jump table on Aya+0x0E in [6,15]; park at 305C8
3. HP==0 path 1F7D8 / 20288; do not force HP to 0
4. 1D340 epilogue death 1F41C mode 3 / 1F4B0 HP clear — only if retail reaches it
5. Encounter complete → teardown → field return

NEXT_5_UE_TASKS= (for the other host; this host does not write UE5)
1. Disc-verify m0377i→m0012i 0x31 hop
2. Disc-verify m0005i 0x55 complete → 0x89
3. UE_PARITY_REQUIRED: 0x95 mode 0 → 299CC → 1D340 → 1F704
4. Do not promote anim_remain_0F overlay clear as retail-identical until 6CC2C is verified
5. Continue into m0367i only after those hops

## Day 1 status

DAY1_FIELD=PARTIAL (prefix playable on UE5 main; decomp native Carnegie/M0036I live)
DAY1_TEXT=PARTIAL
DAY1_BATTLE=PARTIAL (decomp HP first hit; UE5 command bind + 0x55 wait)
DAY1_AUDIO=RESEARCH
DAY1_FMV=RESEARCH
DAY1_PERSISTENCE=PARTIAL (do not invent persist[])
DAY1_SAVE_LOAD=RESEARCH
DAY1_UE_PARITY=UE_PARITY_REQUIRED on HP path
DAY1_OVERALL=IN_PROGRESS
