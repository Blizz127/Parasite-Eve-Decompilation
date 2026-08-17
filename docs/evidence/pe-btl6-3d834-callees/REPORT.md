# PE-BTL6 — 3D834 a1==0 callees 3A088 / 3B97C / 3BCE0

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. `andi 0xFC`, mode 7, and `0x55` completion
were not invented. GTE MVMVA / NCLIP walks were not invented.

## Bounds

| Leaf | VA | Words | SHA-256 | jals |
|---|---|---|---|---|
| `3D834` | `0x8003D834..0x8003D94C` | 70 | `520529b0…f7ad` | a1==0 skips first 3 |
| `3A088` | `0x8003A088..0x8003A6A8` | 392 | `9b66ed6c…c3fe` | `3DBE4`, `3DD08` |
| `3B97C` | `0x8003B97C..0x8003BCE0` | 217 | `bce3a039…c500` | none |
| `3BCE0` | `0x8003BCE0..0x8003C0B4` | 245 | `207d5d5f…e6e6` | none |

Live EE=13 `jal 3D834(overlay+0x14, 0, …)` always hits `3A088`,
`3DFD8` (already ported), `3B97C`, `3BCE0`×2.

## `func_8003A088` — mode-0 empty cut

`lh dest+0x28`. Live epilogue left `+0x28=0`, so:

- `+0x28==1` jal `3DBE4` skipped
- `+0x28==3` jal `3DD08` skipped
- `+0x28==4` GTE multiply skipped

Join `0x8003A348` zeros scratchpad `0x1F800000` / `+4` (not
`PE_RAM`) and `ctc2`s dest+0x34 into C2CTRL 0–7. Then
`lhu(obj+0x18); blez` at `0x8003A3AC` returns at `0x8003A684`.

Named cut `func_8003A088_mode0_empty_cut` refuses modes 1/3/4 and
returns on `obj+0x18<=0`. The non-empty walk at `0x8003A3B4` is
GTE `cop2 0x049E012` / `0x0480012` (MVMVA) with guest-RAM `sh` /
`swc2` into dest+0x84 / dest+0x80. Not this cut.

## `func_8003B97C` — empty early-out cut

Live `jal 3B97C(dest, D_800BEA40)`. Returns when dest+0==0,
`lh dest+0xBA==0`, or `lbu(obj+2); blez` (zero-extend, so only
byte 0). Non-empty lighting (NCLIP, scratchpad `0x1F800004`,
tables `0x800B1638` / `0x800A6360`) is not this cut.

## `func_8003BCE0` — four directory walks (full leaf)

Integer packet fills. Live: `3BCE0(dest, 1, D_8009CDDC)` then xor
CDDC and jal again. Stream cursor accumulates across four loops
from dest+0x10 using `lhu obj+8/+A/+C/+E`:

| Loop | count | pkt stride | rec | table | stores |
|---|---|---|---|---|---|
| 1 | obj+8 | 104 | dir+10 | `0x800B1638` | +4/+10/+1C/+28 |
| 2 | obj+A | 80 | dir+8 | `0x800B1638` | +4/+10/+1C |
| 3 | obj+C | 72 | dir+10 | `0x800A6360` | +4/+C/+14/+1C |
| 4 | obj+E | 56 | dir+8 | `0x800A6360` | +4/+C/+14 |

`sel=(int16)a2`, `force=(int16)a1`. Skip write if
`(lw(pkt)&0x00FFFFFF)==0 && force==0`. ROM `sw +4` then `sb` keep
at +7: the LE word at +4 includes the keep byte.

## What is not proven

`+0x158` is a separate arena from Writer B `+0x154` (init
`0x8006A904` / `0x8006A910`, 0xE000 apart). EE=13 a1 is
`package+(lw(section+0xC)+4 & 0x00FFFFFF)` when D1A0 bit1.
The CE2=14 command bank is **not** that package.

The only `6C5BC` CD issue into `+0x158` is EE=4
(`0x8006C77C` `jal 6E6A8`). Live bit1 EE is `0→11→12→13`
and never visits EE=4/5/6. `6BECC` fills `+0x154` / `+0x194`
only. So the live `+0x158` payload is still unproven: a prior
EE=4 bank, another `6E6A8` dest, or an unfilled arena (zeros
would make `obj+0x18`/`obj+2` empty and the named cuts live).

If CE4 is boot `-1` or test `0`, the EE=4 PE.IMG windows
`[526,532)` / `[532,537)` both have section+0xC objects with
`+2=2` and `+0x18=2`. Do not treat those as live until a
producer is proven.

Do not `andi 0xFC`.

## Verify

```text
python3 pc_port/tools/pe_btl6_3d834_callees_oracle.py
PE_TEST_FILTER=BTL6 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```

STOP: prove live `+0x158` payload on EE `0→11→12→13` (not EE=4).
Then `0x8003A3B4` if `obj+0x18>0`. Do not invent GTE.
