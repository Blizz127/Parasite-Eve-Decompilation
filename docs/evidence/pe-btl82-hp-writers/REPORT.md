# PE-BTL82 — HP-field writer census; 0x85 is a door

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
m0005i chunk2 SHA-256 `01a64ba3…7e3b`.
No matching `src/` C.

Proven HP triple remains Aya record `D_8009D278`
`+0x0C` current / `+0x0E` snapshot / `+0x1C` cap.
Actor `+0x0C` is type. `2FF78` dest is `*D254`.

`hp_mutated` is not emitted. The BTL72 TRACE
endpoint is unchanged.

## Census (stores through D278)

| VA | Function | Width | Source | Class | Live from BTL72 |
|---|---|---|---|---|---|
| `80029418` | `293F4` | sh | clamp to `+0x1C` | COPY | yes (`a0=0`) |
| `80029444` | `293F4` | sh | `lhu +0x0C` | COPY | yes |
| `8002AE7C` | `2AE60` JT[7] | sh | sign-ext `+0x1C` | HEAL | no (`mode==3` and `gp+0x104==7`) |
| `8001E940` | `1D340` | sh | `HP - lbu(s1+0x92)` | DAMAGE | no (`4D4==0`) |
| `8001F704` | `1F4D4` | sh | `HP - s0` (halve if weap==10) | DAMAGE | no (jal from `1D340`) |
| `80020210` | `201DC` | sh | `HP - record+0x38` | STATUS | no (jal `1F9F4` inside `1F4D4`) |
| `8001F4B0` | `1D340` epilogue | sh | `$zero` after `1A680(19)` | DEATH/CLEAR | no |
| `8001FC98` | heal tick | sh | `+0x0C+1` if below cap | HEAL | no |
| `80023EF0`… | item JT | sh | `+45/+90/+180/+400` | HEAL | no |
| `80033B00` | `33A2C` family | sh | clamp to cap | COPY | no (`19D20` zero jal, zero pointer) |
| `80030098` | `2FF78` tag 4 | sh | script value | UNKNOWN | dest `*D254`, not D278 |
| `8002F658` | default copy | memcpy | `10928` HP 45 → `B8A20` | INIT | boot only |

`31F4C` / `320F0` `sb +0x0C` are GPU packet bytes, not HP.

Other subtractive `sh +0x0C` in the EXE (not D278 loads):

| VA | Function | Class | Live from BTL72 |
|---|---|---|---|
| `80027ED0` | `27D14` `*actor+0x0C` | UNKNOWN (clip/ATB-adjacent; callers `2A53C` after `1D340`, `2A994` mode 3, `2CEC8` mode-7 body) | no |
| `800E049C` | `E03A0` overlay | STATUS-like (`+0x0C -= +0x1`) | no (zero jal/pointer to `E022C`) |
| `800CDB4C` | overlay jalr table | not HP (scale/`-50`) | no |
| `800C2AB4` | overlay | timer decrement | no |

`1D340` sole TEXT jal is `2A4FC`. That site is the
`299CC` mode==0 / `4D4!=0` body. `293F4(0)` clears
`4D4`. The only `4D4=1` setter with a live shape is
`293F4(1)` at `2AE60`, reached from `2AA98` JT[7]
when mode==3. Mode 3 is stored only at `1F41C`
inside `1D340`. `33A2C` also writes `4D4=1` but has
no jal and no pointer word in the EXE. Do not invent
`4D4`.

Full-EXE store census of `gp+0x4D4` / `lui` `D244`:
`29464` set, `29488`/`2B99C`/`2F100`/`1F43C` clear,
`33A34` set (dead). No overlay extra.

`293F4` jal `a0=1` only at `2AE80`. Every other site
passes `a0=0` (delay-slot `move $a0,$zero` at `29910`).

## Live path after BTL72 `0x85`

Type-3 `+0xC8` `0x85(0x1E)` → `0x9C` → `0x0A`
persist[1]=5 → `0x31` `0xA8000248` (M0004I).
Second rect hops the same token. Later rects hop
`0xA80004C8` (M0009I). `0x85` is fade + door. It
is not ATB, Attack, or HP damage. The hop is not
playable-loop `field_return`.

`68E24` expires `CFEE=2` to `1` after 30 ticks
(bit 4 clear). `0x9C` then continues.

## Type-6 scratch wait (not a first-entry unlock)

`+0x190` `0x09` subop 3: `cond[0] = scratch[0] & 4`.
`+0x1A8` subop 7: `cond[1] = !cond[0]`.
`+0x1C0` `0x05` skip-if-false `cond[1]` → `+0x1E8`.
Clear bit falls into `0x02` + goto `+0x190` (wait).
Set bit skips to `0x12`. The only m0005i
`0x2A[4,0][0,2]` setter is type-6 `+0x1850`, after
`0x55(2)` after that wait. EXE `B6A80` sites are
zero / kind-4 decode only. Do not force the bit.

## 0xAE

`func_80019728` — 8 words `0x80019728..0x80019748`,
SHA-256 `a0eb25f9…7e2b`. `D2E8 |= 4`; v0=1.
Set twin of `0xAD`. Type-6 `+0x1100` after `0x55(2)`.
Not the scratch[0]&4 setter.

## Hard stop

No retail-derived HP mutation is reachable from the
accepted BTL72 endpoint without inventing `4D4` or
`scratch[0]&4`. Mode 7 (`2CEE0`/`2CF24`) has zero
jal and zero pointer; `encounter_complete` as
defined by the TRACE contract is also unreachable.

## Verify

```text
python3 pc_port/tools/pe_btl82_hp_writer_oracle.py
python3 pc_port/tools/pe_btl72_playable_loop_oracle.py
PE_TEST_FILTER=BTL82 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL72 ./pc_port/build/pe-native-tests
```
