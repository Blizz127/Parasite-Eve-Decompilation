# PE-BTL121 — 26824 publishes D2A4 to BE830 slot+4

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## Consumer

`21DE0` reads `BE830[D1D4].tid` at `slot+4`. `D1D4==0` is
`BE834`. tid in `[387,407)` jals `22394` → `24250(tid-387)`.
tid 406 is `jtbl[19]`, the only TEXT OR of `rec+0x4C` bit
`0x80000`.

## Rejected direct writers

| Site | Why not the publisher |
|------|------------------------|
| `20F7C` / `26FA0` `sh $0, BE834` | table zero only |
| TEXT `li 406` at `16828` / `22C64` / `22CA8` / `27A58` | compares |
| `512AC` `addiu 387` | writes `D010`, not `BE834` |
| `21938` `A76DC` | copies into a different dest+4 |
| `15DAC` `sh` to `+4(a0)` | script record, not proven BE830 |

## Publisher

Indexed `sh rt, 4(BE830+i*8)` family lives in `25E00..26C58`.
`26824(a0==1)` is the D2A4 path:

```
lh s0, D_8009D2A4
if tid < 387:          one slot, actor=D254, tid=D2A4
if tid in [387,407):   jtbl[tid-393]
  tid 406 = jtbl[13] @ 2692C
if tid >= 407:         deferred
```

`2692C`: `26FD0`, `D25C=0`, seven slots
`actor=AE000[(71A54() % D2B0)*12]`, `tid=lhu D2A4`.
`269F4` (387–392, 396, 398–405): one slot, same tid.

`D1F0!=0` at `26230` jals `26824` with `a0=D1F0`. `a0==1`
is this path.

## D2A4 mailbox

`D_8009D2A4` is `gp+0x534`. Sole halfword stores:

- `299CC` `29A68` `sh v0, 534(gp)` after `jal 5C498`
- `35558` `35684` `sh v0, D2A4` after `jal 5C498`

`5C498` returns `D010` (`514F8`). `51504` zeros `D010`
first. `512AC` case 1 (`5130C`) stores `*a1+387` to `D010`.
`*a1==19` → `406`. Case 10 still stores `1000` (BTL112).

Natural `5C498` → `406` still needs the `512AC(1)` enqueue
(`57B70` / `46DBC`; no TEXT `jal 46DBC`). Do not plant
`BE834`.

## Verify

```text
python3 pc_port/tools/pe_btl121_tid406_oracle.py
PE_TEST_FILTER=BTL121 ./pc_port/build/pe-native-tests
PE_TEST_FILTER=BTL116 ./pc_port/build/pe-native-tests
```
