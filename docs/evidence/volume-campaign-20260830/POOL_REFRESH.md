# Matching-C pool refresh — 2026-08-30

Status: authoritative live pool for the campaign resumed from `main` at
`634dd1b` (335-leaf base). After VOLUME-117, the matching-C count is **347**.

## Method and closure

The refresh starts from the current YAML rather than the historical pool. For
every active `[address, asm]` subsegment in `configs/USA/disc1.yaml`, it scans
the generated `asm/disc1/*.s` bodies and retains active `nonmatching func_*`
spans of at most 40 words. Stale generated files outside the selected YAML
geometry are excluded.

The current active universe contains **1,082** unique spans. The four-way
scheduling closure is:

```text
TIER 1                   0
TIER 2                 208
TIER 3                  99
SKIP / suppressed      775
                       ---
TOTAL                 1082
```

The primary tables in `volume-campaign-20260824/POOL.md` contain 1,032
historical rows: 0 Tier 1, 214 Tier 2, 99 Tier 3, and 719 SKIP. VOLUME-112
removed six Tier-2 rows by integrating `func_800C6ED8`,
`func_800C6EC0`, `func_80050020`, `func_8007C544`, `func_800812F4`, and
`func_80062F1C`, leaving 1,026 active primary rows. The 56 active spans below
were added after the primary tables through campaign overlays. Thus
`1026 + 56 = 1082`; the suppressed class
remains `719 + 56 = 775`.

The refresh commit itself changed no C source, YAML, build script, verifier,
or toolchain file. VOLUME-97 subsequently removed `func_8003C5D8` from Tier 1
after exact integration; VOLUME-98 moved `func_80070D6C` to the suppressed
residual set after two bounded phrasings; VOLUME-99 then removed exact
`func_8006346C` from Tier 1; VOLUME-100 screened handwritten COP2 helper
`func_8003F798` without a C attempt; VOLUME-101 removed exact
`func_80012700` from Tier 1; VOLUME-102 screened handwritten libGTE helper
`func_80079304` without a C attempt; VOLUME-103 removed exact
`func_8003DF50` from Tier 1; VOLUME-104 screened handwritten libGTE helper
`func_80078554` without a C attempt; VOLUME-105 removed exact
`func_800CE870` from Tier 1; VOLUME-106 moved `func_800339A0` to the
suppressed residual set after two bounded phrasings; VOLUME-107 did the same
for `func_80080C48`; VOLUME-108 did the same for `func_800762BC` and fired
hard stop H5. A new explicit close-out authorization then let VOLUME-109
screen byte-identical `func_8007AA34` into the `func_80080C48` residual
family without a duplicate compile. VOLUME-110 then removed exact
`func_80021850` from Tier 1 after a first-phrasing 34/34 match. VOLUME-111
screened handwritten `func_800783E4` into the established libGTE/COP2 family
and closed Tier 1. VOLUME-112 opened the separately authorized Tier-2 probe
and removed exact `func_800C6ED8` on its first natural phrasing. VOLUME-113
did the same for adjacent dual setter `func_800C6EC0`; VOLUME-114 removed
exact callback getter `func_80050020` using the established three-word
symbolic-load gate. VOLUME-115 then removed exact three-state setter
`func_8007C544` with the established store-delay gate. VOLUME-116 removed
exact range-guarded setter `func_800812F4` on its first natural phrasing.
VOLUME-117 removed exact forwarding wrapper `func_80062F1C` on its first
natural phrasing. A suppressed `func_` label is not assumed to be a retail
function.

## Reconciled post-table spans

| file off | generated label | words | current scheduling disposition | evidence |
|---:|---|---:|---|---|
| `0x6B234` | `func_8007AA34` | 32 | `PARKED-INDEPENDENT-LOAD-AND-BCD-ACCUMULATOR-SCHEDULING-FAMILY` | `volume-campaign-20260830/func-8007aa34/PARK.md` |
| `0x66ABC` | `func_800762BC` | 32 | `PARKED-TEXTURE-WINDOW-CONTROL-FLOW-LOAD-SCHEDULE-AND-COLORING` | `volume-campaign-20260830/func-800762bc/PARK.md` |
| `0x71448` | `func_80080C48` | 32 | `PARKED-INDEPENDENT-LOAD-AND-BCD-ACCUMULATOR-SCHEDULING` | `volume-campaign-20260830/func-80080c48/PARK.md` |
| `0x241A0` | `func_800339A0` | 32 | `PARKED-GP-LOAD-STORE-SCHEDULING-AND-REGISTER-HOME` | `volume-campaign-20260830/func-800339a0/PARK.md` |
| `0x68D54` | `func_80078554` | 31 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260830/func-80078554/SKIP.md` |
| `0x69B04` | `func_80079304` | 30 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260830/func-80079304/SKIP.md` |
| `0x2FF98` | `func_8003F798` | 26 | `SKIP-HANDWRITTEN-COP2` | `volume-campaign-20260830/func-8003f798/SKIP.md` |
| `0x2788C` | `func_8003708C` | 7 | `PARKED-FIXED-POINT-REGISTER-COLORING` | `volume-campaign-20260825/func-8003708c/PARK.md` |
| `0x33C74` | `func_80043474` | 19 | `PARKED-THRESHOLD-LADDER-BLOCK-LAYOUT` | `volume-campaign-20260825/func-80043474/PARK.md` |
| `0x43CCC` | `func_800534CC` | 6 | `PARKED-GP-ABSOLUTE-FORM` | `volume-campaign-20260825/func-800534cc/PARK.md` |
| `0x467E0` | `func_80055FE0` | 11 | `PARKED-BIT-TEST-CANONICALIZATION` | `volume-campaign-20260825/func-80055fe0/PARK.md` |
| `0x48518` | `func_80057D18` | 6 | `SKIP-PARKED-GP-ABSOLUTE-FORM-FAMILY` | `volume-campaign-20260825/func-80057d18/SKIP.md` |
| `0x4C4BC` | `func_8005BCBC` | 21 | `PARKED-GP-STATUS-REGISTER-AND-DELAY-SCHEDULE` | `volume-campaign-20260825/func-8005bcbc/PARK.md` |
| `0x4E38C` | `func_8005DB8C` | 8 | `PARKED-ADDRESS-DAG-COLORING` | `volume-campaign-20260825/func-8005db8c/PARK.md` |
| `0x4E3AC` | `func_8005DBAC` | 19 | `PARKED-SYMBOLIC-ADDRESS-LIFETIME-COLORING` | `volume-campaign-20260825/func-8005dbac/PARK.md` |
| `0x4E3F8` | `func_8005DBF8` | 6 | `PARKED-ADDRESS-REGISTER-COLORING` | `volume-campaign-20260825/func-8005dbf8/PARK.md` |
| `0x4E410` | `func_8005DC10` | 6 | `SKIP-PARKED-ADDRESS-REGISTER-COLORING-FAMILY` | `volume-campaign-20260825/func-8005dc10/SKIP.md` |
| `0x4E670` | `func_8005DE70` | 6 | `SKIP-PARKED-ADDRESS-REGISTER-COLORING-FAMILY` | `volume-campaign-20260825/func-8005de70/SKIP.md` |
| `0x4F188` | `func_8005E988` | 16 | `PARKED-CONTROL-FLOW-CONSTANT-SCHEDULING` | `volume-campaign-20260825/func-8005e988/PARK.md` |
| `0x569CC` | `func_800661CC` | 8 | `SKIP-HANDWRITTEN-COP2` | `volume-campaign-20260825/func-800661cc/SKIP.md` |
| `0x5EC54` | `func_8006E454` | 17 | `PARKED-INDEPENDENT-LOAD-SCHEDULING` | `volume-campaign-20260825/func-8006e454/PARK.md` |
| `0x6156C` | `func_80070D6C` | 25 | `PARKED-REGISTER-HOME-AND-CROSS-BLOCK-SCHEDULING` | `volume-campaign-20260830/func-80070d6c/PARK.md` |
| `0x62F14` | `func_80072714` | 4 | `SKIP-SDK-LIBRARY-SYSCALL` | `volume-campaign-20260825/func-80072714/PARK.md` |
| `0x62F24` | `func_80072724` | 4 | `SKIP-SDK-LIBRARY-SYSCALL` | `volume-campaign-20260825/func-80072714/PARK.md` |
| `0x63A44` | `func_80073244` | 20 | `PARKED-LEXICOGRAPHIC-COMPARE-CANONICALIZATION` | `volume-campaign-20260825/func-80073244/PARK.md` |
| `0x66AA0` | `func_800762A0` | 7 | `PARKED-REGISTER-COLORING` | `volume-campaign-20260825/func-800762a0/PARK.md` |
| `0x682A0` | `func_80077AA0` | 1 | `SKIP-ALIGNMENT-PADDING` | function-hood audit below |
| `0x682BC` | `func_80077ABC` | 1 | `SKIP-ALIGNMENT-PADDING` | function-hood audit below |
| `0x682C4` | `func_80077AC4` | 15 | `PARKED-MASK-CONSTANT-COLORING` | `volume-campaign-20260825/func-80077ac4/PARK.md` |
| `0x6832C` | `func_80077B2C` | 1 | `SKIP-ALIGNMENT-PADDING` | function-hood audit below |
| `0x6835C` | `func_80077B5C` | 1 | `SKIP-ALIGNMENT-PADDING` | function-hood audit below |
| `0x684EC` | `func_80077CEC` | 1 | `SKIP-ALIGNMENT-PADDING` | function-hood audit below |
| `0x68920` | `func_80078120` | 5 | `SKIP-SDK-LIBRARY-GTE-TAIL` | `volume-campaign-20260825/func-80078120/SKIP.md` |
| `0x69434` | `func_80078C34` | 23 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260830/func-80078c34/SKIP.md` |
| `0x69494` | `func_80078C94` | 9 | `PARKED-AGGREGATE-RETURN-COLORING` | `volume-campaign-20260825/func-80078c94/PARK.md` |
| `0x69604` | `func_80078E04` | 12 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260824/COP2_SDK_SCREEN.md` |
| `0x69694` | `func_80078E94` | 8 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260824/COP2_SDK_SCREEN.md` |
| `0x697AC` | `func_80078FAC` | 3 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260824/COP2_SDK_SCREEN.md` |
| `0x697B8` | `func_80078FB8` | 3 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260824/COP2_SDK_SCREEN.md` |
| `0x69824` | `func_80079024` | 3 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260824/COP2_SDK_SCREEN.md` |
| `0x69978` | `func_80079178` | 22 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260825/func-80079178/SKIP.md` |
| `0x699D0` | `func_800791D0` | 22 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260829/func-800791d0/SKIP.md` |
| `0x69A44` | `func_80079244` | 11 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260825/func-80079244/SKIP.md` |
| `0x69AD4` | `func_800792D4` | 10 | `SKIP-SDK-LIBRARY-COP2` | `volume-campaign-20260825/func-800792d4/SKIP.md` |
| `0x6EEB0` | `func_8007E6B0` | 21 | `PARKED-SYMBOLIC-ADDRESS-REGISTER-COLORING` | `volume-campaign-20260825/func-8007e6b0/PARK.md` |
| `0x7265C` | `func_80081E5C` | 5 | `SKIP-PARKED-ADDRESS-RETENTION-FAMILY` | `volume-campaign-20260825/func-80081e5c/SKIP.md` |
| `0x72CB4` | `func_800824B4` | 5 | `SKIP-ADDRESS-RETENTION-FAMILY` | `volume-campaign-20260825/ADDRESS_RETENTION_SCREEN.md` |
| `0x72CC8` | `func_800824C8` | 5 | `PARKED-ADDRESS-RETENTION` | `volume-campaign-20260825/func-800824c8/PARK.md` |
| `0x72CDC` | `func_800824DC` | 5 | `SKIP-PARKED-ADDRESS-RETENTION-FAMILY` | `volume-campaign-20260825/ADDRESS_RETENTION_SCREEN.md` |
| `0x732DC` | `func_80082ADC` | 11 | `PARKED-SYMBOLIC-BASE-RETENTION` | `volume-campaign-20260825/func-80082adc/PARK.md` |
| `0x73F90` | `func_80083790` | 14 | `PARKED-ARITHMETIC-ASSOCIATION-SCHEDULING` | `volume-campaign-20260825/func-80083790/PARK.md` |
| `0x7459C` | `func_80083D9C` | 21 | `PARKED-SWITCH-TAIL-BLOCK-LAYOUT` | `volume-campaign-20260825/func-80083d9c/PARK.md` |
| `0x77F98` | `func_80087798` | 9 | `PARKED-MMIO-ADDRESS-RETENTION` | `volume-campaign-20260825/func-80087798/PARK.md` |
| `0x7800C` | `func_8008780C` | 12 | `PARKED-INDEPENDENT-OP-SCHEDULING` | `volume-campaign-20260825/func-8008780c/PARK.md` |
| `0x7803C` | `func_8008783C` | 10 | `PARKED-VOLATILE-STORE-SCHEDULING` | `volume-campaign-20260825/func-8008783c/PARK.md` |
| `0x78064` | `func_80087864` | 10 | `PARKED-VOLATILE-STORE-SCHEDULING-FAMILY` | `volume-campaign-20260825/func-80087864/SKIP.md` |

## One-word function-hood audit

All five one-word spans contain `00000000` and fail function hood. None ends
in a return or tail jump. A full executable scan found zero raw pointer words,
zero direct `j`/`jal` targets, and zero same-register `lui` plus
`addiu`/`ori` constructions for every exact start.

| generated label | boundary geometry | retail classification |
|---|---|---|
| `func_80077AA0` | follows the real return delay slot of `func_80077A64`; real `func_80077AA4` starts one word later | alignment padding |
| `func_80077ABC` | follows `func_80077AA4`'s live return delay slot; it and `0x80077AC0` precede real `func_80077AC4` | alignment padding |
| `func_80077B2C` | follows `func_80077B04`'s live return delay slot; it and `0x80077B30` precede real `func_80077B34` | alignment padding |
| `func_80077B5C` | follows `func_80077B34`'s live return delay slot; it and `0x80077B60` precede real `func_80077B64` | alignment padding |
| `func_80077CEC` | follows `func_80077CB4`'s `jr ra; nop`; it and `0x80077CF0` precede real `func_80077CF4` | alignment padding |

For each row, `RETAIL_CLASSIFICATION=alignment/padding`. The generated
`func_` name is only a split/disassembler label and supplies no function
provenance. All five remain asm and never enter the matching-C count.

## Base gate

The refresh was checked from the clean 335-leaf base:

```text
docker build: Compare: EXACT SHA-1 MATCH
candidate SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh: exit 0; C conversion: 335 leaves
yaml matching-C count: 335
```

The first refreshed row, `func_80078C34`, was screened as a handwritten
libGTE/COP2 routine without a C attempt. The Tier-1 queue therefore continues
at `func_8003C5D8` (`0x2CDD8`, 24 words). Function hood and all screens must
still be re-proven before its first C attempt.
