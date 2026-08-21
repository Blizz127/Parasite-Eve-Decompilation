# PE-BTL94 — M0367I persist `0x09` second visit

Source of resource-lifecycle facts:
`docs/evidence/pe-battle-data-precovery/`.

Type-1 parks at `+0x2C` after dest-ready. The next visit is four
`0x09` equality groups on `persist[0x4A]` (kind 2, tag 74), each
followed by `0x05` skip-if-false:

| persist[0x4A] | listed `0x08` types | skip if false |
|---|---|---|
| `0x26` | 2 | `+0x70` |
| `0x5E` | 2, 4 | `+0xD0` |
| `0xCD` | 3, 4 | `+0x130` |
| `0x29E` | 2, 3, 4 | `+0x1AC` |

Watch already wrote `persist[0x4A]=0x26` and `persist[1]=5` via
`0x0A` before the `0x31` hop (PE-BTL60). Dest-ready does not write
`persist[0x4A]`. That incoming value opens **only the first**
`0x08`. The other seven stay behind unproven writers for `0x5E` /
`0xCD` / `0x29E`. Do not poke those values.

Closed (`persist[0x4A]==0`): all four groups skip; still one actor;
type-1 stores PC at `+0x1B8` after the `+0x1AC` `0x02`.

`3F074` jals `1A918` after `6C5BC==0` and before `125E0`.
M0367I `B1620` is `chunk2+0x1C6D8` (`lhu(+2)=1`, `+0x20!=0`),
so `1AA78` on the nonempty `+0x1AC` type-2 takes the `D1D8!=0`
arm. Do not invent a collision hit.

Open (Watch `0x0A`): one type-2 sibling. First visit is
`0x9B` / `0xED(0xA29)` / `0x14` / `0x1E` / `0x79` / `0xC1` /
`0x0B` / `0x0B` / `0x2E(0x09)` / `0x02`. `0xC1` (`0x80019AC0`,
9 words) is `actor+0x98 |= 0x400`, `v0=1`. Subsequent type-2 visit
is `0x20` (task+8 bit `0x10` park). Type-1 continues
`0xAA` / `0x40` / `0x03` / `0x02(10)` into the `0x26` message arm
and parks on `0x9C` while `CFEE&3 >= 2`. `B0E38` / `68E24` fade
tick is not dest-ready published. Do not hop `+0x2D8` `0x31`
M0005I. Do not force type-6 `scratch[0]&4`.

## Commands

```
python3 pc_port/tools/pe_btl94_m0367i_persist09_oracle.py
# PASS: persist 0x09 / 0x05 / 0x08 / 0x0A and 0xC1 19AC0 |=0x400

python3 pc_port/tools/pe_battle_data_precovery_oracle.py
# PASS: battle-data precovery; … M0367I WA 36

PE_TEST_FILTER=BTL94_m0367i ./pc_port/build/pe-native-tests
# Results: 821 run, 1 passed, 0 failed, 820 skipped

./pc_port/build/pe-native-tests
# Results: 821 run, 821 passed, 0 failed, 0 skipped
```
