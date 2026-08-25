# Matching-C volume campaign run log — 2026-08-25

Base: a648f45, 287 accepted leaves; pool refresh and subsequent screens reduced the active pool to 1136 candidates before this attempt (TIER 1 90, TIER 2 214, TIER 3 99, SKIP 733).

| attempt | target | pool row | outcome | words | iterations | commit or stash |
|---:|---|---|---|---:|---:|---|
| 1 | func_80072714 @ 0x62F14 | TIER 1; jr-ra; 26 direct callers; real boundaries | SKIP-SDK-LIBRARY-SYSCALL after function-hood/static screen; adjacent func_80072724 screened with same proof | 4 | 0 | labeled stash; committed c28edcc |
| 2 | func_80062A20 @ 0x53220 | TIER 1; jr-ra; 39 direct callers; real boundaries; argument-indexed load | MATCHED@era -O2 -G0; 5/5 exact | 5 | 2 | committed 655628e; target leaf 288 |
| 3 | func_800824C8 @ 0x72CC8 | TIER 1; jr-ra; 12 direct callers; scalar global read/write; address-retention | PARKED-ADDRESS-RETENTION after two allowed phrasings; no integration | 5 | 2 | labeled stash; docs committed f8703c0 |
| 4 | func_80085084 @ 0x75884 | TIER 1; jr-ra; 6 direct callers; constant-add getter; real boundaries | MATCHED@era -O2 -G0; 5/5 exact | 5 | 0 | committed 37ab013; target leaf 289 |
| 5 | func_80077AA4 @ 0x682A4 | TIER 1; jr-ra; 50 direct callers; packed coordinate helper; real boundaries | MATCHED@era -O2 -G0; 6/6 exact; full SHA exact; verify green at 290 | 6 | 0 | committed 3894b85 |

The adjacent func_800824DC @ 0x72CDC has the same scalar-global exchange
shape over D_800B8AB8 and was screened into the same park family without a
separate attempt.

The next Tier 1 span, func_80078120 @ 0x68920, was screened without a C
attempt as SKIP-SDK-LIBRARY-GTE-TAIL; its five-word tail entry branches into
func_80078134’s shared handwritten-COP2 continuation. No attempt was consumed.

The next Tier 1 span, func_80081E5C @ 0x7265C, was screened without a C
attempt as the same PARKED-ADDRESS-RETENTION-FAMILY established by 824C8/824DC.
No attempt was consumed.

The pool family pass then found the unreferenced exact scalar-exchange twin
func_800824B4 @ 0x72CB4 over D_800B8AB0. It was moved to
SKIP-ADDRESS-RETENTION-FAMILY without an attempt; no matching-C count change.
Evidence: `ADDRESS_RETENTION_SCREEN.md`.

The next Tier-1 attempt, func_8005DBF8 @ 0x4E3F8, passed function hood but was
parked after two natural-C phrasings. Retail's address/load register coloring
is not reproduced by either expression; the pointer-local retry also exceeded
the exact 0x18-byte body and was rejected by the trim guard. Evidence:
`func-8005dbf8/PARK.md`; candidate/integration work remains in its labeled
stash; no count change.

The next eligible Tier-1 leaf, func_800877BC @ 0x77FBC, matched on the first
`-O2 -G0` phrasing as a volatile hardware halfword setter. Six words exact;
carve `0x394 + 0x18 + 0x3348 = 0x36F4`; full SHA exact and verify green at
291. Evidence: `func-800877bc/REPORT.md`; committed as the next leaf.

The following `func_8005DC10` @ `0x4E410` is the exact six-word
address-register-coloring twin of parked `func_8005DBF8`, over `D_800A8048`
with offset `-0x20` and two callers. It was screened without a duplicate
attempt; no count change. Evidence: `func-8005dc10/SKIP.md`.

The next independent attempt, `func_800534CC` @ `0x43CCC`, passed function
hood but was parked after two `-O2 -G8` phrasings. Both emitted an absolute
address instead of retail's `$gp+0x2D8` load; full mismatch was 22 bytes.
Evidence: `func-800534cc/PARK.md`; no count change.

The following `func_8005DE70` @ `0x4E670` is the same six-word
address-register-coloring family over `D_800A8044` with offset `-0x1C` and one
caller. It was screened without a duplicate attempt; no count change.
Evidence: `func-8005de70/SKIP.md`.

The following `func_80057D18` @ `0x48518` is a gp-indexed getter/clear twin of
`func_800534CC`, with an exact-start function-pointer construction and no jal
caller. It was screened under the same gp-absolute-form blocker without an
attempt; no count change. Evidence: `func-80057d18/SKIP.md`.

The next independent attempt, `func_8003708C` @ `0x2788C`, passed function
hood with 65 direct callers but was parked after two `-O2 -G0` fixed-point
product phrasings. Both retained wrong HI/LO temporary coloring and nonzero
text beyond the exact 7-word body; no count change. Evidence:
`func-8003708c/PARK.md`.

## Stop / continuation

This is not a hard stop: the syscall family is a proven SDK/handwritten architecture class, so it is removed from scheduling. No C count increase, carve, build integration, or false progress is claimed. Continue with the next eligible Tier 1 candidate after the docs commit.

The next Tier-1 attempt, `func_800762A0` @ `0x66AA0`, passed function hood
with three direct callers and was parked after two `-O2 -G0` packed-command
phrasings. Both retain the first masked/shifted value in `$v0` instead of
retail `$a1`; the full candidate differed in 13 bytes. Evidence:
`func-800762a0/PARK.md`; no count change. Candidate/integration work remains
in the labeled stash.

The next Tier-1 leaf, `func_800631C0` @ `0x539C0`, passed function hood with
two direct callers and matched on the second `-O2 -G0` phrasing. An explicit
result accumulator reproduced retail's filled null-branch delay slot; carve
`0x1C + 0x186C = 0x1888`, packed span and full SHA exact, verify green at 292.
Evidence: `func-800631c0/REPORT.md`.

The next Tier-1 leaf, `func_80087728` @ `0x77F28`, passed function hood with
two direct callers and matched on the second `-O2 -G0` phrasing. Direct fixed
address stores overflowed; one volatile base with halfword indices reproduced
the `$at` MMIO form. Carve `0x300 + 0x1C + 0x78 = 0x394`, packed span/full SHA
exact, verify green at 293. Evidence: `func-80087728/REPORT.md`.

The next Tier-1 leaf, `func_8008770C` @ `0x77F0C`, matched on the first
`-O2 -G0` phrasing using the proven one-base volatile MMIO pointer. Carve
`0x2E4 + 0x1C = 0x300`, packed span/full SHA exact, verify green at 294.
Evidence: `func-8008770c/REPORT.md`.

The next Tier-1 leaf, `func_80087744` @ `0x77F44`, matched on the first
`-O2 -G0` phrasing using the proven one-base volatile MMIO pointer. Carve
`0x1C + 0x1C + 0x5C = 0x90`, packed span/full SHA exact, verify green at 295.
Evidence: `func-80087744/REPORT.md`.

The next Tier-1 leaf, `func_80087760` @ `0x77F60`, matched on the first
`-O2 -G0` phrasing using the proven one-base volatile MMIO pointer. Carve
`0x1C + 0x1C + 0x40 = 0x78`, packed span/full SHA exact, verify green at 296.
Evidence: `func-80087760/REPORT.md`.

The next Tier-1 leaf, `func_8008777C` @ `0x77F7C`, matched on the first
`-O2 -G0` phrasing using the proven one-base volatile MMIO pointer. Carve
`0x1C + 0x1C + 0x24 = 0x5C`, packed span/full SHA exact, verify green at 297.
Evidence: `func-8008777c/REPORT.md`.

The next Tier-1 leaf, `func_800877D4` @ `0x77FD4`, is a distinct per-voice
SPU halfword setter and matched on the first natural `-O2 -G0` indexed-store
phrasing. Carve `0x1C + 0x332C = 0x3348`, packed span/full SHA exact, verify
green at 298. Evidence: `func-800877d4/REPORT.md`.

The next Tier-1 leaf, `func_800877F0` @ `0x77FF0`, is the per-voice SPU
halfword-setter twin of `877D4` and matched on the first natural `-O2 -G0`
phrasing. Carve `0x1C + 0x3310 = 0x332C`, packed span/full SHA exact, verify
green at 299. Evidence: `func-800877f0/REPORT.md`.

The next Tier-1 leaf, `func_80083C20` @ `0x74420`, passed function hood via
the exact-start callback-address construction in `func_80083BB8`; the pool's
`0/2` is its HI/LO reference pair. Natural `-O2 -G0` C matched all seven words
on the first phrasing. Carve `0x660 + 0x1C + 0x234 = 0x8B0`, packed span/full
SHA exact, verify green at 300. Evidence: `func-80083c20/REPORT.md`.

The next Tier-1 leaf, `func_8005E8A4` @ `0x4F0A4`, has 247 direct calls and
updates the gp-relative pair `D_8009D124/D_8009D128`, which has 32 other
Stage-0 readers. Natural two-statement accumulator C matched all eight words
on the first era `-O2 -G8` phrasing. Carve `0x20 + 0x294 = 0x2B4`, packed
span/full SHA exact, verify green at 301. Evidence: `func-8005e8a4/REPORT.md`.

The next Tier-1 leaf, `func_8005E968` @ `0x4F168`, has 16 direct calls and
stores a packed value plus its signed-shifted, `0x7F7F7F`-masked companion in
the gp-relative pair `D_8009D110/D_8009D114`. Natural C matched all eight
words on the first era `-O2 -G8` phrasing. Carve
`0xA4 + 0x20 + 0x1D0 = 0x294`, packed span/full SHA exact, verify green at
302. Evidence: `func-8005e968/REPORT.md`.

The next Tier-1 row, `func_800661CC` @ `0x569CC`, is a real callable
handwritten helper with seven direct callers, but its semantic body is two GTE
OFX/OFY `ctc2` writes. Existing projection evidence proves it restores
`(160,112)`; the established COP2 screen proves ordinary-C expressibility is
absent. It was moved to `SKIP-HANDWRITTEN-COP2` without an attempt or count
change. Evidence: `func-800661cc/SKIP.md`.

The next Tier-1 leaf, `func_8005BEE8` @ `0x4C6E8`, has six direct callers and
returns one of two 0x10-separated addresses inside `D_800C0DE0` according to
gp-relative `D_8009D218`. An explicit result pointer retained the address in
`$v1` and matched all eight words on the first era `-O2 -G8` phrasing. Carve
`0x20 + 0x580 = 0x5A0`, packed span/full SHA exact, verify green at 303.
Evidence: `func-8005bee8/REPORT.md`.

The next Tier-1 leaf, `func_8005DADC` @ `0x4E2DC`, has five direct callers
and returns an eight-byte-indexed address using the word and adjusted base at
`D_800A8030`. Attempt 1 duplicated the symbol materialization; an explicit
pointer local made cc1 retain the shared address in `$v1` and matched all eight
words on attempt 2 under era `-O2 -G0`. Carve
`0x1644 + 0x20 + 0x618 = 0x1C7C`, packed span/full SHA exact, verify green at
304. Evidence: `func-8005dadc/REPORT.md`.

The next Tier-1 candidate, `func_8005DB8C` @ `0x4E38C`, has four direct
callers and a retained-address index shape. Two era `-O2 -G0` phrasings were
bounded: the explicit pointer shared the address but selected the `5DADC`
allocation/order; the adjusted-base phrasing duplicated symbol
materialization. Retail's copy-before-load DAG remains unreproduced, so it is
`PARKED-ADDRESS-DAG-COLORING` with no integration/count change. Evidence:
`func-8005db8c/PARK.md`; source is in the labeled stash.

The next Tier-1 row, `func_80078E94`, was pool drift: it and
`func_80078E04` were already members of the authoritative 23-function
handwritten libGTE/COP2 screen. Both are function-hood proven by three direct
callers and real boundaries, but both require `ctc2` semantic bodies. They
moved to `SKIP-SDK-LIBRARY-COP2` without attempts or count change; Tier 1
66→64 and SKIP 743→745. Evidence:
`../volume-campaign-20260824/COP2_SDK_SCREEN.md`.

The next Tier-1 leaf, `func_80083E50` @ `0x74650`, has two direct callers and
initializes object bytes plus an interior pointer. Natural argument-relative C
matched all eight words on the first era `-O2 -G0` phrasing, including the
final state-byte store in the return delay slot. Carve
`0x214 + 0x20 = 0x234`, packed span/full SHA exact, verify green at 305.
Evidence: `func-80083e50/REPORT.md`.

The next Tier-1 leaf, `func_80083E84` @ `0x74684`, has two direct callers and
is the tag-`0x4C` structural twin of `83E50`. Proven initializer C matched all
eight words on the first era `-O2 -G0` phrasing. Carve
`0x20 + 0x40 = 0x60`, packed span/full SHA exact. The first verifier run
caught a stale `[0x74684, asm]` expected marker; after correcting the cluster's
C/asm manifest, verify passed at 306. Evidence: `func-80083e84/REPORT.md`.

The next Tier-1 leaf, `func_80083EC4` @ `0x746C4`, has two direct callers and
is the tag-`0x47` structural twin of `83E50`/`83E84`. Proven initializer C
matched all eight words on the first era `-O2 -G0` phrasing. Carve
`0x20 + 0x20 = 0x40`, packed span/full SHA exact, verify green at 307.
Evidence: `func-80083ec4/REPORT.md`.

The next Tier-1 leaf, `func_80083EA4` @ `0x746A4`, has one direct caller and
is the tag-`0x46` structural twin completing this initializer cluster. Proven
initializer C matched all eight words on the first era `-O2 -G0` phrasing.
The old asm span was exactly the leaf (`0x20 = 0x20`), so it closes with no
prefix or resume; packed span/full SHA exact, verify green at 308. Evidence:
`func-80083ea4/REPORT.md`.

The next Tier-1 candidate, `func_80087798` @ `0x77F98`, is a proven callable
nine-word per-voice SPU register-pair writer. Two era `-O2 -G0` phrasings
compiled identically: cc1 retains the scaled index in `$a0`, computes the
first address in `$v0`, and rematerializes the second through `$at`; retail
retains one computed address in `$a0` for both stores. It is
`PARKED-MMIO-ADDRESS-RETENTION`, with no integration or count change. This is
consecutive park 1. Evidence: `func-80087798/PARK.md`; source is in the
labeled stash.

The next Tier-1 leaf, `func_8005D970` @ `0x4E170`, has one direct caller and
selects signed value `4` or `-4` from a gp-relative input threshold of 300.
An explicit default accumulator matched all nine words on the first era
`-O2 -G8` phrasing, including the load-delay nop and branch-delay default.
Carve `0x14D8 + 0x24 + 0x148 = 0x1644`, packed span/full SHA exact, verify
green at 309. This match resets the consecutive-park count. Evidence:
`func-8005d970/REPORT.md`.

The next Tier-1 candidate, `func_80078C94` @ `0x69494`, is a proven callable
nine-word three-word-copy helper. Aggregate C preserved the size but homed the
destination/result in `$v0` at entry; scalar C retained that home and moved the
last store into the return delay slot. Retail retains `$a0` for all stores and
moves it to `$v0` afterward. It is `PARKED-AGGREGATE-RETURN-COLORING`, with no
integration/count change. This is consecutive park 1. Evidence:
`func-80078c94/PARK.md`; source is in the labeled stash.

The next Tier-1 row, `func_800792D4` @ `0x69AD4`, is a function-hood-proven
handwritten libGTE operation with six direct callers. Its ten words perform
`lwc2`, `mvmva`, `swc2`, and `cfc2` side effects that ordinary sanctioned C
cannot express. It moved to `SKIP-SDK-LIBRARY-COP2` without an attempt or
count change; the consecutive-park count remains 1. Evidence:
`func-800792d4/SKIP.md`.

The next Tier-1 leaf, `func_80077B04` @ `0x68304`, is a ten-word byte flag
set/clear helper with five direct callers. Canonical return and exact-start
calls prove function hood; the neighboring alignment nops remain outside the
leaf. Natural argument-relative C matched all ten words on the first era
`-O2 -G0` phrasing, including the local jump relocation and return-delay-slot
store. Carve `0x48 + 0x28 + 0x38 = 0xA8`, packed span/full SHA exact, verify
green at 310. This match resets the consecutive-park count. Evidence:
`func-80077b04/REPORT.md`.

Adjacent `func_80077B34` @ `0x68334` independently passes function hood via
four exact-start calls and canonical return. It is the bit-0 set/clear twin;
existing B54I evidence identifies its Psy-Q role as `SetShadeTex`. Minimal
argument-relative C matched all ten words on the first era `-O2 -G0`
phrasing. Both two-nop alignment fragments remain asm; carve
`0x08 + 0x28 + 0x08 = 0x38`, packed span/full SHA exact, verify green at
311. Evidence: `func-80077b34/REPORT.md`.

The next Tier-1 candidate, `func_8008783C` @ `0x7803C`, is a proven callable
ten-word per-voice SPU register RMW. Two era `-O2 -G0` phrasings reproduce
address formation, load, mask, shift, and merge exactly, but both emit
`sh; jr; nop` instead of retail `jr; sh`. An explicit returned value does not
move the volatile store. `PARKED-VOLATILE-STORE-SCHEDULING`; no integration
or count change, consecutive park 1. Evidence: `func-8008783c/PARK.md`;
source is in the labeled stash.

Adjacent `func_80087864` @ `0x78064` is function-hood proven by its exact
caller at `0x80087A80` and canonical return. Its complete body has the same
per-voice SPU volatile `sh` return-slot mechanism as parked `8783C`, changing
only the selected nibble. It is screened without a duplicate attempt as
`PARKED-VOLATILE-STORE-SCHEDULING-FAMILY`; count and consecutive-park state
remain 311 and 1. Evidence: `func-80087864/SKIP.md`.
