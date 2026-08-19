# PE-BTL129 — first-visit event graph; 36448 recovered

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
m0005i chunk2 SHA-256 `01a64ba3…7e3b`. No matching `src/` C.
Do not poke `scratch[0]`. Do not inject `0x81` / `0x70` / `0x7F` /
`0x7D` / `0x84` / `0x7B` / `0x86` / `0x7C`. Do not force `+0x1A0`
onto type 6.

## A. Type-3 `0x77` is a volume poll, not a type-6 mailbox

Handler is still `14DA0` / `1CAB0` (BTL26). Type-3 has **no**
`0x14` (no mailbox, no `+0x1A0`). Main task:

```text
+0x000 0x02
+0x00C 0x5E pose copy
+0x030 0x77 rect (0x0994,0x04CD)-(0x080A,0x063D)
+0x064 local[4]==1
+0x07C skip-if-false → +0xF8
hit: persist[0x18]|=1 / 0x1C(0,0,0xFE) / 0x85(30) / 0x31 hop
miss: next 0x5E/0x77
+0x3F0 goto +0x0C   # loop
```

Four volumes. Every hit mails **type 0 payload `0xFE`**, then dest-hops.
No `0x1C` to type 6. First-visit type-0 pose is left of rect1
(BTL73); the loop keeps missing until the player walks in.
`0xFE` on type-0 mailbox parks. It does not reach `+0xCAC`.

## B. `36448` recovered; it cannot start type-6 mail

`func_80036448` is 608 words `0x80036448..0x80036DC8`,
SHA-256 `10e6b058…19c2`. Sole live parent `35558@35C1C`.
`D1A0&4` skips `36448` / `12774` / `360B4`.

Pair walk over `D_8009D20C`:

```text
if actor+0x1A0 != 0 and no live A4 task for the other:
    task = 12700(actor+0x1A0, 0)
    task+8 |= 1
    task+0xC  = other+0x24
    task+0x18 = other type
    task+0x1C = other idB
    prepend actor+0xA4
```

Four `jal 12700` sites (basic proximity and the `+0x1AC`-both
collision arm). `12774` then reaps parked slot tasks.

m0005i `0x14` code 1 (`+0x1A0`):

| Type | `+0x1A0` | Island |
|---|---|---|
| 0 | `+0x610` | `0x20` park |
| 2 | `+0xA0` | `0x20` park |
| 5 | `+0xF8` | `0x1C(0,0,0xFE)` / `0x1C(0,0,0xFF)` |
| 1,3,4,6 | none | |

No A4 island mails type 6. Live dest-ready after 16 ticks:
type-5 `+0x1A0=+0xF8`, type-6 `+0x1A0=0`, every `A4` empty
(`36448` still deferred in native `35558`). Do not plant one
on type 6.

## C/D. Type-0 `0x81` / `0x70` are not dest-enter

Mailbox `+0x618` `0x1F` → `local[4]`. New-game persist 0:

| Payload | Reaches |
|---|---|
| `0xFF` (type-1) | persist`!=39` park. No type-6 mail. |
| `0xFE` (type-3/5) | park. No type-6 mail. |
| `0x0D` (type-4) | not `+0xCAC` unless `persist[0]&4` |
| `0x65` | `+0x1128 0x7F` / `+0x113C 0x70` / `+0x1150 scratch[0]\|=0x10` |
| any other | not `+0xCAC` |

`+0xCAC 0x1C(6,0,0x81)` needs **`persist[0]&4` and payload `0x0D`**.
m0005i has no setter of `persist[0]` bit 2. Type-4 `0x0D` also
needs a `0x77` hit and `persist[0x4A] ∈ [17,40)`. Not first visit.

`0x70` / `0x7F` are the **`0x65` reply**, not a dest-enter send.

## E. Type-2 senders need type-6 (or type-0 `0x7A`) first

Mailbox `+0xA8`. Body is **not** required to send:

| Incoming | Send | Body |
|---|---|---|
| `0x7D` | `0x84` | no |
| `0x7A` | `0x7B` | no |
| `0x79` | `0x86` | no |
| `0x7E` | `0x7C` | no |
| `0x7F` | none | `+0x804 0x6F` |

`0x7D` is type-6 after the wait escape. `0x79`/`0x7E` are type-6
after it already received `0x84`. `0x7A` is type-0 `+0x1388`,
not dest-enter. First-visit type 2 parks on `0x20`.

## Type-6 mailbox arms (payload-sensitive)

`+0xD08 0x1F` → `local[15]`. New-game persist/scratch 0:

```text
0x81 → +0xFC8 0x55 → 0x89 → 0x1C(0,0,0x65) → park +0xFFC
       NOT +0x1850
0x70 → +0x1088 wait scratch[0]&0x10
       bit 4 set → 0xAE → +0x1850 scratch[0]|=4
0x84 → 0x1C(2,0,0x79)
```

BTL128's unconstrained CFG put `0x55` and `0xAE` on one island.
They are **different payload arms**. `0x81` does the 0x55 wait
and replies `0x65`; that reply is what makes type 0 send `0x70`
and set scratch bit 4. A later `0x70` delivery is what reaches
`+0x1850`.

## First-visit chronology that *does* run

```text
dest-ready 1266C zeros scratch
125E0 type 1 then type 6
type-6 +0x180 0x14 → +0x19C=+0xD08; wait scratch[0]&4
type-1 0x08 types 3,0,5,2,4 (persist[0x4A]<40)
type-1 0x1C(0,0,0xFF); park
type-0 mailbox 0xFF, persist!=39, park
type-2 0x20 park, +0x19C=+0xA8, no body
type-3 0x77 miss-loop
type-5 +0x1A0=+0xF8 (A4 still empty without 36448)
no 0x1C to type 6
```

The authentic first `0x81` is still missing. It is not dest-enter,
not `36448`, not type-3 `0x77`, and not new-game `0xFF`/`0xFE`.
Next is `persist[0]&4` provenance (not an m0005i script store)
or the type-4 `0x0D` persist`[0x4A]>=17` path — without poking
either.

PCSX watch: `pc_port/tools/pe_btl128_ordering.lua` now also
hooks `36448` / `12774` / `0x77`. Theater memcard is still not
m0005i.

## Verify

```text
python3 pc_port/tools/pe_btl129_first_visit_graph.py
PE_TEST_FILTER=BTL129 ./pc_port/build/pe-native-tests
```
