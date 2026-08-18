# PE-BTL112 — 4B70C persist arms 534=1000

Authority: `build/disc1.candidate.exe` SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C. MATCHED yaml C leaves stay 227.

`2B0E8` phase 1 waits `lh gp+0x534 == 1000`. That halfword
is only stored from `5C498` (`29A68`). `51504` zeros
`D010` first. `512AC` case 10 (`514A4` `li 1000`) is the
writer. The TEXT `jal 512AC` with `a0=10` at `4BC4C` sits
inside `4BB80`.

## Install

`2B0E8` phase 0 (`2B194`) jals `4B70C(gp+0x594, gp+0x4AC,
0x800A7FF0)`. `4B70C` copies `C0E00` to `gp+0x278` /
`27C = 278+a0`, then jals `4B90C`.

`4B90C` synthesizes `0x8004BB80` (`lui 0x8005` /
`addiu 0xBB80`) into `obj+0x2C` and jals `62CB8(obj)`.
`62CB8` is `sw a0, gp+0x3EC`. `62CC4` is the matching load.

## Fire

`5C498` jals `5E30C` after `51504`. `5E30C` type-4 arm
(`5E4AC`) does `jal 62CC4` then `jalr obj+0x2C` with
`a1=0x10000`. `4BB80` returns 1 if `!(a1&0x10000)`.
If `gp+0x278 < gp+0x27C` (signed) it snaps `278=27C`
and skips `512AC`. Else `57ECC==0` jals `512AC(10)`.

Type-4 enqueue has no TEXT `jal`. This cut runs the type-4
jalr arm while mode is 2 and `4B90C` has installed `4BB80`.
`62D2C` heap, `57ECC` / `48654`, `703F4`, and `67CBC` stay
deferred.

## Verify

```text
python3 pc_port/tools/pe_btl112_4b70c_oracle.py
PE_TEST_FILTER=BTL112 ./pc_port/build/pe-native-tests
```
