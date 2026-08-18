# FIRST_DAY1_ENCOUNTER

Walkthrough names are not used.

## Negative on the SYS0 prefix

Opcode census of decoded slot-4 scripts:

| Scene | `0x89` | `0x6F` | `0x70` | `0xB7` |
|---|---|---|---|---|
| m0002i | 0 | 0 | 0 | 0 |
| m0003i | 0 | 0 | 0 | 0 |
| m0372i | 0 | 0 | 0 | 0 |
| m0004i | 0 | 0 | 0 | 0 |
| m0378i | 0 | 0 | 0 | 0 |
| m0377i | 0 | 0 | 0 | 0 |
| m0005i | 3 | 1 | 1 | 1 |

`m0377i` (SYS0 next dest) only `0x31`s back to `m0378i`. It is not a
fight.

## How first-play reaches m0005i

After the RD5-C2 reel, m0004i type-0 parks at `+0x05D0` `0x1F`
(mailbox poll). Arms:

| local[4] | Action |
|---|---|
| 4 | cinematic + `persist[1]=4` + `0x31 0xA80002C8` **m0005i** |
| 3 | cinematic + `persist[1]=4` + `0x31 0xA80002C8` **m0005i** |
| 0xFE / 0xFF / 5 / 6 | other (talk / restore / skip) |

Senders on the same map, module 4, after first-play control:

| Volume PC | Quad (int XZ) | Send |
|---|---|---|
| `+0x0EB0` | X[-2474,-1299] Z[8534,9344] | **not** mailbox; `0x31 m0378i` (RD6-A north) |
| `+0x0F78` | X[1913,3560] Z[1860,2898] | `0x1C (0,0,3)` if `0x5E` sample local[5] `< 768` or `>= 3072` |
| `+0x10E4` | X[-2973,-2030] Z[1920,2896] | `0x1C (0,0,4)` same local[5] tests |

Post-reel spawn (RD5-C2) is `(-2049, 0xFEBA11F8, 7468)`. North (+Z)
is m0378i. The mailbox-3/4 quads sit at **lower Z** (south/side).

m0005i module 0 `+0x00B0` tests `persist[1]==4` — the value the hop
writes. First-play entrance is that arm.

Class:

```text
random_walk_table     no
scripted              yes
avoidable             only by never walking the mailbox volumes
mandatory_for_day1    yes if the SYS0 route is to continue past Carnegie
first_play_gate       persist[0x4A]==0x18 (post-reel) + persist[1]==4
```

## First request inside m0005i

Module 2 builds the slot table, then module 6 requests mode 6.

Module 2 preface (`+0x1DCC`):

```text
0x1A  local[24] = rng(0, 100)          ; func_800176FC -> 70DD0
0x09  slt  local[24] < 19
      true  -> local[24] = 49
      false -> local[24] = 50
0x6F  func_8002F7D8                    ; claim a D_800A5D58 slot, copy default body
0x5A  field 40=0, 41=1, 42=2, ...      ; tagged writes through Aya/slot
0xB7  func_8002FAA4(3,0,6,7,1)
0x70  func_8002FA10(0,0,8,9,1,5,-1,-1)
0x5A  50=1333, 51=1332, 52=1334        ; numeric resource fields
```

Module 6 first request:

```text
+0x34B4  0x94          poll D_8009D28C into local[3]
         wait until nonzero
+0x350C  0x89          D_8009D28C = 6
+0x3514  0x94 loop     wait until D_8009D28C == 7
+0x3560  0x40 0xAA     inhibit + load bit
+0x3570  0x1C          mailbox (2,0,125)
```

```text
first_day1_source_scene     m0004i
first_day1_trigger          module4 +0x0F78 mailbox 3  OR  +0x10E4 mailbox 4
first_day1_dest_scene       m0005i
first_day1_encounter_id     m0005i_mod2_setup + mod6_+0x350C
first_day1_formation_id     variant 49 or 50; slot fields 1332/1333/1334
```

Two later `0x89` exist on the same map (`+0x3728`, `+0x414C`). They
are not the earliest request.

## Enemy names

Not decoded. Keep `1332` / `1333` / `1334` and variant `49`/`50`.
