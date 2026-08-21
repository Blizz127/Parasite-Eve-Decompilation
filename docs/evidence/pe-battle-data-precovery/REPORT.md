# PE battle-data precovery

Authority: Disc 1 EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
Disc 1 Form1 user data via `local/pe_disc1.path` / `PE_DISC1_BIN`.
PE.IMG LBA base `1013`. No matching `src/` C. No UE5 / gameplay edits.

This pack is a retail data contract for the live NYPD / m0005i encounter
and the immediate `0x31` dest `M0367I`. Identities are table/consumer
paths only. Do not name resources from appearance.

Verify:

```text
python3 pc_port/tools/pe_battle_data_precovery_oracle.py
```

`--write` regenerates the CSVs from EXE + PE.IMG.

## Live actor set

`125E0` desc at m0005i chunk2 `+0x25014` is `count=2`, type `1` then
type `6`. Type 6 is `D_8009D20C` head. Type 1 opcode `0x08` later
constructs types `3`, `0`, `5` in that order. Type 0 publishes
`D_8009D254`. Types `2` and `4` exist in the 12574 list of 7 and are
indexed here, but they are not on the 125E0 desc.

| type | first VM word | first op | spawn | +0x1AC source |
|---:|---|---|---|---|
| 0 | `0x0000609B` | `0x9B` | type1 `0x08` #2 | `6C118` CE2=14 `+0x8` |
| 1 | `0x0000C0EA` | `0xEA` | 125E0 desc[0] | empty on this path |
| 2 | `0x0000609B` | `0x9B` | listed only | `6B804` idB=2 |
| 3 | `0x00002002` | `0x02` | type1 `0x08` #1 | empty on this path |
| 4 | `0x00002002` | `0x02` | listed only | empty on this path |
| 5 | `0x00004014` | `0x14` | type1 `0x08` #3 | `6B804` idB=5 |
| 6 | `0x000080CE` | `0xCE` | 125E0 desc[1] | empty on this path |

Update vtable: type 0 `func_80035C84`; types 1–9 `func_80035E04`.

## Resource objects (`+0x1AC` / `+0x1B0` / `+0x1B4`)

Publication order on dest enter:

1. `func_8006B35C` (3F074 first jal, 103w, SHA-256
   `1106cb2a94fa877af5a067e36a3c24d08f186694b112c2139d2869f7cf341f24`)
   zeros `D_800B0E70[0..9]`, the ten `D_800B0E98` rows, and
   overlay `+0x944..+0x958`.
2. `func_8006B4F8` hdr+0x0C @ `0x8006B804` writes `D_800B0E70[idB]`.
   Live m0005i has **two** records: idB `2` and `5` only.
3. hdr+0x10 Writer A @ `0x8006B84C` writes
   `D_800B0E98[type*192+cmd*4]` from 21 room records.
4. hdr+0x14 / `12574` → `D_800B161C`; hdr+0x18 → `D_800B1620`
   (`0x8006B8E8`); hdr+0x1C → `D_800B1624` (`0x8006B90C`);
   hdr+0x20 → `D_800B1628`.
5. Later `func_8006C118` (inside `6BECC`, not `6B4F8`) writes
   `D_800B0E70[0]` from the CE2=14 package hdr+0x0C rel24.
   Writer B @ `0x8006C140` fills the type-0 command row, including
   command 4.

`func_80035038` copies `D_800B0E70[type]` to actor `+0x1AC` when
`type<10`. Empty `+0x1AC` ORs `+0x98` with `0xE0` and skips
`1A680` / `362B8` / `3D050`.

`func_8001A680` copies `D_800B0E98[type*192+cmd*4]` to actor
`+0x1B0` and `lbu resource+2-1` to `+0x0F`.

Actor `+0x1B4` is an **in-actor dest object**, not a table pointer.
`15240` uses `dest = actor+0x1B4`. `6CC68` publishes `D254+0x1B4`
to `D_800B0D10`. `362B8` / `3D050` initialize that dest only when
`+0x1AC != 0`. Do not invent a host-side equivalent.

## m0005i hdr+0x0C (`D_800B0E70`)

| idB | chunk2 ptr | size | SHA-256 | consumer |
|---:|---|---:|---|---|
| 2 | `0x90B4` | 23700 | `b68b7c6bf5ee7986…` (full in CSV) | type 2 `+0x1AC` |
| 5 | `0xED48` | 628 | `e6fdc055a9912e18…` (full in CSV) | type 5 `+0x1AC` |

Window `0x8006B7C8..0x8006B820` SHA-256
`f2f5b2382224b409acea599d5bdf1572a082fe30d76d19731ff712b4aef0e2d0`.

## CE2=14 type-0 command bank (`D_800B0E98` row 0)

`D_800930D8[22]=396`, `[23]=428`. Bank 65536 bytes, SHA-256
`db785a5eea1f78f945284e57955605326f5856adda1ebc83d0a95d7a0142b1b2`.

25 directory records, 18 unique payloads. Command 4 (first `29810`
bind): ptr `0x6C14`, 1700 bytes, enc=1, bones-1=30, frames=18,
SHA-256 `6207fbca2fe44a3549bf0b7fbcf1ce3e979a0a12e8b130e606ea4a985b1885e4`.

`6C118` type-0 resource object: package `+0x8`, 23864 bytes,
SHA-256 `bcbdf4f4117bda46…` (full in CSV). Window
`0x8006C0E4..0x8006C174` SHA-256
`59c4bd179f4f37cbc881c9d847d80e469f4311f69c5efa904b2c583169cf70f2`.

## Writer A (m0005i hdr+0x10)

21 records. Type 0 commands are `0x18,0x1D,0x1E,0x1F,0x20` only
(no command 4). Type 2 command `0x17` at `+0x1F3A8` has `byte+2=51`,
so `1A680` sets `+0x0F=50`. Type 5 commands `0..3` are 60/88-byte
payloads, not 31-bone clips.

## VM script corpus

`17018` fetch is `word@pc`, kinds-extension `word2@pc+4`, immediates
at `pc+8`, span `8+argc*4`. Opcode = `word & 0x1FFF`. First 24
well-formed words per type are in `BATTLE_SCRIPTS.csv` with window
SHA-256, branch rels (`0`/`5`/`0x12` as `imm<<1`), and yield ops
(`1`/`2`/`0x1F`/`0x20`/`0x30`/`0x64`/`0x9C`).

Upcoming / still-open on the live streams (do not implement here):

- type 0 after `0x9B`: `0xED` `0xA29`, then `0x14`×2, `0x0B`…
- type 1 prefix `0xEA` then `0x08` types 3, 0, 5 (later listed 2, 4)
- type 3 after first `0x02`: `0x5E` / `0x77` (pose copy + edge test)
- type 6 after `0xCE`: `0xEA` (AKAO-adjacent; not named here)
- M0367I spawned type 2/3 first visit: `0x9B`/`0xED`/`0x1E`/`0x79` then `0xC1` (`D_800910A0[0xC1]=0x80019AC0`)
- M0367I type 4: same head then `0x9D` (`0x80019450`)

## Type-0 `0x2E` / `0x2F` (m0005i)

`D_800910A0[0x2E]=0x80017AE8` → `1A680(actor, lhu *arg0)`.
`D_800910A0[0x2F]=0x80017B34` writes `actor+0x12 = min(+0x0F, imm)`.
`0x2F` is a frame **cap**, not a command ID.

First live type-0 bind after `29810` cmd 4: script `+0x31C`
`0xAA` → `0x40` → `0x65` → **`0x2E(0x15)`** → `0x82(1)`.

Unique type-0 `0x2E` command IDs: `0x15`, `0x1C`, `0x1E`, `0x1F`, `0x20`.

| cmd | bank | idB | ptr | size | frames | SHA-256 |
|---|---|---:|---|---:|---:|---|
| `0x15` | CE2=14 Writer B | 21 | `0xEF28` | 2776 | 34 | `194a37c66679857b8a4ca3e8df72b3812cb5fafb2e03dbeb45c9e79b3fda1537` |
| `0x1C` | CE2=14 Writer B | 28 | `0x5D40` | 3796 | 69 | `cabc0c490153091a3d83b5f79da71c108d1cbe5004276ccae70707397e25e85e` |
| `0x1E` | m0005i Writer A | 30 | `0x10F40` | 10796 | 105 | `85170bf01e1bd9419fd9a1d5ab91837a975cb2c3320b02792cb926cc70111832` |
| `0x1F` | m0005i Writer A | 31 | `0x1396C` | 4716 | 40 | `77343a19f342efda44e564306c077ce8cf53667ae3264f460fcdf6b91c64287f` |
| `0x20` | m0005i Writer A | 32 | `0x14BD8` | 1344 | 22 | `51e84dd712d7a9869459edd14080efec19316365fd8e15b9e15480143175660f` |

`0x1C` shares the CE2=14 3796B payload with idB `0/1/2/3/7/22/23`.
Do **not** treat every type-0 `0x2E` as CE2=14.

Type-2 listed stream `0x2E` cmds `0x02/0x06/0x13/0x14/0x15/0x16/0x17`
resolve to **m0005i Writer A type 2**, not CE2=14. Type-5
`0x08`-spawned stream uses Writer A cmds `0x00` and `0x02` (60B).

## M0367I Writer A

36 rows, 25 unique payloads, writer `0x8006B84C`.
No type-1 row. Type 0 is cmd `0x18` only (1272B, 30 frames,
SHA-256 `03f7338305ade7941ff5115bbacc5dd83481dc3cdc4058f5ea043a65617fd24c`,
same hash as m0005i type-0 cmd `0x18`). Types 2 and 3 share
cmds `0x00–0x0A` (22-bone clips). Type 4 has cmds `0x00–0x0C`.

`6B35C` zeros `D_800B0E98` on dest enter. Writer A then writes
the 36 room rows (type 0 cmd `0x18` only). `6BECC` is jal'd
every `3F074` at `0x8003F20C` and is busy-waited until
`v0!=1`, so state 6 **does** run in the same `3F074` after
the clear. It does **not** republish CE2=14. `6B4F8` sets
`CE2 = hdr+1 = 10`. `6BE4C` sets overlay bit `0x00200000`
when `CE2!=CE3` and `CE2` is in `[10,14]`. State 6 then
publishes the CE2=10 bank (`29753bd66426838dae50e1487a663004aa938b1ebf58008fff09705af9f59d93`).
Listed M0367I type-0 `0x2E(0x15)` therefore resolves to CE2=10
cmd `0x15` (396B, 1 frame,
`e1cb9dfd14eafe873e4768722b39f7fd51e763ea5147279d6a09ad401c1ba40e`),
not the CE2=14 2776B/34-frame clip. First-visit `0x08` does
not spawn type 0.

After `0xC1` (`19AC0`: `actor+0x98 |= 0x400`, return 1), spawned
type 2/3 issue `0x2E(0x09)` (300B, 1 frame, SHA-256
`6bd22d6ce60cab2b0d70ec11433cd51d321394b9f9d1533da14ec8b02e970eb3`).
Type 4 after `0x9D` issues `0x2E(0x07)` then `0x2E(0x01)`.

## M0367I type-1 first visit / `0x08`

Prefix: `0x40` → `0x84(0,0x450)` → `0x88` → `0x02(1)` → persist
`0x09` tag 74 / `0x05` → `0x08`. Type-1 itself has no `0x2E`.

First eight `0x08` descriptors (all idB=0, pose 0): types
`2,2,4,3,4,2,3,4`. Those actors take `B0E70[2/3/4]` from
hdr+0x0C. Later `0x08` repeats types 2/3/4. Persist-gated;
do not claim every spawn always fires.

## M0367I type-1 `0x31` dest tokens

Persist-gated hops (token → `D_8009D280`; table index = name
number − 1). Do not hop the runtime.

| name | table | chunk2 SHA-256 | hdr+0x0C idB | Writer A | 125E0 |
|---|---:|---|---|---:|---|
| M0005I | 4 | `01a64ba3…` (full in CSV) | 2,5 | 21 | types 1,6 |
| M0319I | 318 | `fafb08d77f8d9f0c79b44835c8910fdca6fb0a1fe37e82cc406bdc9362995d85` | 2,3,4 | 6 | type 1 |

M0319I Writer A (6 rows) is in `WRITER_A_CLIPS.csv` scene `m0319i`.
Type 0 cmds `0x18/0x1D/0x1E/0x1F/0x20`; type 2 cmd `0x03` only.
| M0239I | 238 | `1de117c799dab0d0d04803e89a922ff7bbb027958c7a47d37f6eb516a6d814f8` | 2,3,4 | 66 | type 1 |
| M0058I | 57 | `be04c5f835c72485c62191b080e02a629e635700d09666f333158293618505d2` | 2,3,4 | 32 | type 1 |
| M0035I | 34 | `f2f7e7049f1fcd1f99b1cdaf4b3b0fcb2de435baff7b49c4ab3b87b94d31109d` | 2,3,4 | 41 | type 1 |
| M0136I | 135 | `6fa9dee4763af16cbed129b8d1f799067ede252a44b84f2aed1b469e33d08a72` | none | 12 | type 1 |

## Destination / reload

| token | name | table | rel | packed | sectors | full SHA-256 |
|---|---|---:|---|---|---|---|
| `0xA80002C8` | M0005I | 4 | `0x266A` | `0x0600A921` | 33+169+96 | `fc48530a84811c31ebf4cde06bb12c9c7bcc815348db8dd8c2bb7dacd3410724` |
| `0xA80663C8` | M0367I | 366 | `0x15050` | `0x04E0AA21` | 33+170+78 | `671d8c53b0c12b13e1580b844e37c51dc6120b6cf7af78472037d7ed1a7da6de` |

m0005i chunk2 SHA-256
`01a64ba3769dae259e9c3151aa9eaacf977a44ac7db4ec8053f2d338ef017e3b`.
M0367I chunk2 SHA-256
`ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e`.

`6B4F8` is the sole TEXT caller from `3F074@0x8003F088`. `6B35C`
clears the publish slots before that load. M0367I Writer A has 36
rows. 12574 list at chunk2 `+0x1B314` has 5 type entries. Word0
`0x13C0` makes the 125E0 desc at `+0x1C6D4`: **count=1, type=1,
idB=0**. First type-1 word is `0x00000040` (`D2E8 |= 1`). Types
0/2/3/4 are listed (`0x14` / `0x9B` / `0x9B` / `0x9B`) but not
in that desc.

M0367I hdr+0x0C:

| idB | ptr | size | SHA-256 |
|---:|---|---:|---|
| 2 | `0x8` | 24248 | `c55bd4c8c80895452ae3c6d786e4cbfa403311cee384aa87e63360231bf86015` |
| 3 | `0x8` | 24248 | same as idB 2 |
| 4 | `0x5EC0` | 21524 | `acec62834711c862d1abf5e50e1efc39ce5268ec159106487c79c811fac80b24` |

The 125E0 type-1 actor therefore still takes empty `+0x1AC`.
Later `0x08` constructs types 2/3/4, which bind `B0E70[2/3/4]`.

## M0367I destination / resource lifecycle

Initiator: `0x31` (`0x80017BB4`, 19w SHA-256
`69306ee0f7e8b3dc37f2dfbcda9ea6c9073ce409672f83716aa7ebdbe76d1ecf`)
stores the token to `D_8009D280` and `D_8009D1A0 |= 0x2000`.
`1220C@0x800122D8` copies `D280 → D1C4` before `jal 3F3C4`.
`3F3C4@0x8003F3D4` unconditionally jals `3F074`. `3F074` first
jals `6B35C` then `6B4F8(a0=D280)`.

D280 is a 32-bit token. Other writers: `1220C` init
`0xA9400048`, `1239C` `0xA80830C8`, `3E680` zero, `3ED9C`,
`6A2C8`, `6EBB0`, plus the `15834` cluster. Loads: `122D0`,
`123E8`, `15918`, `3F084`, `3F3E0`, `3F67C`, `5D948`, `6A2B8`.

Change detection does **not** gate the `6B4F8` jal.
`3F3C4` compares `D1C4` vs `D280` at `0x8003F3E8` **after**
`3F074`; mismatch takes `0x8003F68C`. Same-tick `0x31` is
therefore loaded on the **next** `3F074`. `6B4F8` has no
token-compare early-out (540w SHA-256
`02320912544ccb61d5aae520c607ec62e5293f3ae31ccdfcbfb9d70980cbe827`).
Host change-gating is host policy, not a retail skip.

### 6B35C reset order (before disc)

103w. `$s0 = D_800B0CD8`.

1. `D_800B0E70[0..9]` countdown `+0x1BC` → `+0x198`
2. `D_800B0E98` 10 rows × 48 words, stride 192
3. overlay `+0x940`, `+0x944`, `+0x948`, `+0x94C` (one word each)
4. `+0x954` then `+0x950` (`a0=1`, `v0=s0+4`)
5. `+0x958`
6. copy `+0x150` → `+0x128`, `+0x150+0x1400` → `+0x12C`; `jal 6E498`
7. overlay `&= ~0x00400000`; `+0xE0=0x27`; `+0xE1=0x0D`

Does **not** touch `+0x0A` (CE2), `+0x0B` (CE3), `+0xEC`
(6BECC state), or `+0x154` (CE2 package pointer). Cleared
once does not mean resources stay absent.

### 6B4F8 M0367I load

Sole TEXT caller: `3F074@0x8003F088`. `6E2D0` decode →
`6E454` atoi → index atoi−1 into `D_80093378`. Three
`6E6A8` reads:

| chunk | dest | sectors | LBA | size | SHA-256 |
|---:|---|---:|---:|---:|---|
| 0 | overlay+0x194 | 33 | 87109 | 67584 | `b8a23aa2c259d4d7c448dbafa74fb6a4737a035675a1030403a3d93d3d0bc2b5` |
| 1 | overlay+0x168 | 170 | 87142 | 348160 | `daf7e449777a52af14d0b557da962a0f1513d06fbdc9d4f87307ea2382939f6c` |
| 2 | overlay+0x18C | 78 | 87312 | 159744 | `ab9af4f446a6f9a1f8f79f1f80b862c4d516beddbede1229afab2dfc30b44b1e` |

Uncompressed Form1 user data. Publish after arrival:
`hdr+1 → CE2` @ `0x8006B7B8`; `hdr+3 → overlay+0x08`;
`hdr+0x0C` @ `0x8006B804` → `B0E70[idB]`; Writer A @
`0x8006B84C`; `hdr+0x14` → `+0x944`; `+0x18` → `+0x948`;
`+0x1C` → `+0x94C`; `hdr+0x20` → `+0x950+id*4`.
`3F074` does not inspect the return.

### Writer B after the clear (M0367I-specific)

JT `0x800113B0`: 0→`6BF34` … 6→`6C0E4`. State 0: if bit
`0x00200000` set → state 1 (reload `D_800930D8[CE2+3]` and
`[CE2+8]` into `+0x194` / `+0x154`); else → state 6 using
leftover `+0x154`. States 1–5 return 1 while CD is in
flight. `3F074` polls until `v0!=1`, so state 6 completes
in the same dest-enter `3F074`.

State 6: `6C118` writes package hdr+0x0C rel24 to
`B0E70[0]`; Writer B loop `6C140–6C174` writes type-0
`B0E98[idB]`; `+0xEC=0`; `CE3=CE2`; clear bit `0x00200000`;
return 0.

M0367I `hdr+1=10`. After a prior `6C1CC(a0=1)` battle
switch, `CE3=14`, so `6BE4C` sets the bit and state 6
publishes **CE2=10**, not CE2=14. If `CE2==CE3==10`
already, state 0→6 republishes the leftover CE2=10
`+0x154`. Either way after settle: Writer B is the CE2=10
bank. CE2=14 does not stay. Type 1 receives no Writer B
row. `6C1CC` is the only EXE `sb 14` to `D_800B0CE2`
(callers `24A3C` / `25388`; not `144FC` / `29810` /
`3F074`).

CE2=10 `D_800930D8[18]=288,[19]=316` (28 sectors, LBA
`[1301,1329)`). Type-0 cmds `0x00–0x03`, `0x0D–0x17`,
`0x1C`. Has `0x15`, no `0x04`, no `0x18` (Writer A keeps
`0x18`). Cmd `0x15` is not the CE2=14 clip.

### Type-1 first visit

`35038`: if `type<10`, `+0x1AC = B0E70[type]`; then
`+0x1B0 = 0` (`0x800351E0`). `B0E70[1]` is never written
(hdr+0x0C is 2/3/4; `6C118` always writes `[0]`). Initial
`+0x1AC=0`, `+0x1B0=0`, `+0x1B4` unfilled (`362B8`/`3D050`
gated on `+0x1AC!=0`). First VM `0x40`; first yield `0x02`
at script `+0x20`; no `0x2E`. No clip required. Later
`0x08` constructs types 2/3/4 onto Writer A + `B0E70[2/3/4]`.
Do not manufacture a type-1 Writer A row.

### First eight `0x08` resource needs

All idB=0, pose 0. Writer A rows are live before `125E0`.
First unresolved resource = none.

| order | type | script | first `0x2E` | yield | clip |
|---:|---:|---|---|---|---|
| 1,2,6 | 2 | `+0x1C230` | `0x09` @ `+0x94` | `0x02` | 300B / 1f / `6bd22d6c…` |
| 4,7 | 3 | `+0x1C32C` | `0x09` @ `+0x84` | `0x02` | same shared payload |
| 3,5,8 | 4 | `+0x1C3D4` | `0x07` @ `+0xF0` then `0x01` @ `+0x1E8` | `0x30` | `685f3ea8…` / `b4331c8a…` |

Type 2/3 first-visit consumed command is **`0x09` only**.
Dormant shared `0x00–0x0A` are indexed in `WRITER_A.csv`
but are not first-visit consumers. Type 4 later also
issues `0x2E(0x00)`.

### Type 4 `0x9D`

`D_800910A0[0x9D]=0x80019450` (13w SHA-256
`bcdc1c39f6225f7d21b2e1d36ab30e5b87f7845c1c0fec4265a37049d4fcb219`).
`lh D_8009D2F0+0x224`; `*imm` (`0x9999`); `mult`; `mflo`;
`sra 16`; `sh` to `*(D2F0+0x1B4)+0x14`; return 1. Does
**not** publish clips. Uses `D_8009D2F0` dest, not the
type-4 actor's `+0x1B4`.

### CE2=14 `+0x8` (fail-closed)

Package bounds: CE2=14 bank `+0x8`, 23864 bytes, SHA-256
`bcbdf4f4117bda4662847db35b1eca4ca61f1a099f17a809e3e3bf87bf6f1418`.
Head words `0x0F1F1D71`, `0x01B302CB`, `0x024A0081`.
hdr+0x0C count=1. Consumer: `6C118` → `B0E70[0]` →
`35038` → actor `+0x1AC` on the **CE2=14** path only.
`+0x1B4` init only if `+0x1AC!=0`. Structure still
partial (TMD / 3D050 tail gated). Neutral evidence only.
Do **not** wire into runtime. M0367I dest-enter replaces
this pointer with CE2=10 `+0x8` (24000B,
`1e9e9282452a6a29f9517557d308ad955128d1be15196cd7aa46bcc86adbb01a`).

### Mode 7 / 9 / 10 producer contract

`D_8009D28C` (`sw gp+0x51C`). Each value has exactly one
store. None is a dest-load or dest-ready write. M0367I
first visit uses **none** of them. Dest-ready =
`6B4F8` publish + `6BECC` returns 0 + `6C5BC` returns 0 +
`125E0` type-1 spawn. Type-6 wait on 7/9/10 is the
m0005i battle path. Do not write these modes.

| value | li / sw | window SHA-256 | precondition |
|---:|---|---|---|
| 7 | `0x8002CF24` / `0x8002CF28` | `bda2d942d0bd2fa5dbc753e66fab7b79cbd34c5eb6c3c5c0e1216511af3b7892` | `6914C(0)==0` and `s1` |
| 9 | `0x8002B278` / `0x8002B27C` | `8ffeb6f863c6066d878c9636e211feaeab0126e15040d8d443cf35dd6fa50794` | `2B0EC` a2==3; `6D60C(0)!=1`; `295E4` |
| 10 | `0x8002BC74` / `0x8002BC78` | `19d29a684296b879cacd37ba2508b793dce35b97a38ac5fa4ec60ab8b78fc2ae` | `6D60C(0)`; `2F9CC`; `1A680(D254,21)`; `295E4` |

### Persist `0x31` contrast

Same `6B4F8` walk. Different `hdr+1` / Writer A / hdr+0x0C.

| dest | hdr+1 CE2 | hdr+0x0C | Writer A |
|---|---:|---|---:|
| M0005I | 10 | 2,5 | 21 |
| M0367I | 10 | 2,3,4 | 36 |
| M0319I | 10 | 2,3,4 | 6 |
| M0239I | 12 | 2,3,4 | 66 |
| M0058I | 12 | 2,3,4 | 32 |
| M0035I | 12 | 2,3,4 | 41 |
| M0136I | 12 | none | 12 type0 only |

## Files

| file | contents |
|---|---|
| `ACTOR_RESOURCES.csv` | listed types: script, model, writers, vtable |
| `BATTLE_SCRIPTS.csv` | 17018-decoded prefix per type |
| `RESOURCE_PUBLICATION.csv` | 6B35C → 6B4F8 → 6C118 order |
| `PEIMG_PACKAGES.csv` | dest packages + unique clip payloads |
| `UNKNOWN_DEPENDENCIES.csv` | 3D050-gated body, +0x1B4 init, dest-enter CE2 clear |
| `COMMAND_BINDS.csv` | every walked `0x2E`/`0x2F` site |
| `WRITER_A_CLIPS.csv` | M0367I 36 rows + m0005i `0x2E`-used rows |
| `SPAWN_DESCRIPTORS.csv` | type-1 `0x08` argc-5 descriptors |
| `DEST_HOPS.csv` | M0367I type-1 `0x31` tokens |
| `DEST_PACKAGE_HEADS.csv` | chunk2 / 125E0 / Writer A counts for those dests |
| `DESTINATION_LIFECYCLE.csv` | M0367I enter order + persist dest contrast |
| `M0367I_RESOURCE_TIMELINE.csv` | clear / load / publish / spawn events |
| `WRITER_A.csv` | dest-level Writer A slots + first-visit consume |
| `WRITER_B.csv` | CE2=14 battle bank + CE2=10 dest-enter bank |
| `ACTOR_RESOURCE_REQUIREMENTS.csv` | type1 + first eight `0x08` |
| `MODE_TRANSITION_WRITERS.csv` | unique mode 7/9/10 stores |

## RUNTIME_HANDOFF

```
type0_1AC_source=CE2_14_plus_8
writer=func_8006C118
actor_1B4=in_actor_dest
do_not_use_room_hdr_plus_0C_for_type0=YES
type0_2E_ce214_cmds=0x15,0x1C
type0_2E_writera_cmds=0x1E,0x1F,0x20
type0_2F_is_command_id=NO
type0_2F_dest=actor+0x12
first_live_type0_2E=script+0x334_cmd_0x15
cmd_0x15_ptr=0xEF28_size=2776_frames=34
cmd_0x15_sha256=194a37c66679857b8a4ca3e8df72b3812cb5fafb2e03dbeb45c9e79b3fda1537
m0367i_6B35C_clears_CE214=YES
m0367i_writera_type0_cmd=0x18_only
m0367i_type1_2E=NONE
m0367i_type1_08_first8=2,2,4,3,4,2,3,4
m0005i_type1_08=3,0,5,2,4
m0367i_type2_2E=0x09_writera
m0367i_type3_2E=0x09_writera
m0367i_type4_2E=0x07,0x01_writera
op_0xC1=actor_plus_98_OR_0x400
m0367i_reset_order=6B35C:B0E70,B0E98,+940,+944,+948,+94C,+954then+950,+958; no CE2/CE3/+EC/+154
m0367i_writera_publish_pc=0x8006B84C
m0367i_6B4F8_sets_CE2=hdr_plus_1=10
m0367i_writerb_present=YES
m0367i_writerb_bank=CE2_10_not_CE2_14
m0367i_writerb_precondition=3F074_polls_6BECC_to_v0_0; 6BE4C_bit200000_if_CE2!=CE3
m0367i_initial_type1_requires_clip=NO
m0367i_type1_1AC=0
m0367i_type1_1B0=0
m0367i_mode7_writer=0x8002CF24
m0367i_mode7_precondition=NOT_DEST_READY; battle_6914C0_and_s1
m0367i_first_mode_on_dest=NONE_OF_7_9_10
d280_change_triggers_load=NO_GATE_IN_3F074
d280_width=u32_token
d1c4_is_copy_of_d280_before_3F3C4=YES
m0367i_type0_2e_15_bank=CE2_10
cmd_0x15_ce210_sha256=e1cb9dfd14eafe873e4768722b39f7fd51e763ea5147279d6a09ad401c1ba40e
op_0x9D_is_clip_publisher=NO
```
