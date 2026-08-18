# PE-BTL115 — 24A3C cases 4–8

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

`24A3C` jtbl at `0x80010824`. Cases 4–8 increment `D25C`
only under the retail waits. Do not plant `D25C=9`.

| Case | Entry | Wait / work | Advance |
|------|-------|-------------|---------|
| 4 | `24C6C` | `+0x0F==+0x16` (lhu 22); walk `D20C` skip Aya, zero `+0x68/6C/70`; `6FC18` / `6F39C(0x6D)` deferred | `D25C=5` |
| 5 | `24CF8` | `1A680(6)`; `3C5D8` deferred; `+0x250\|=2` | `D25C=6` same tick |
| 6 | `24D14` | `Aya+0x252==0`; camera / `6DE80` deferred; `CE4C=30` | `D25C=7` |
| 7 | `24E78` | if `+0x0F==+0x1A`: `1A680(7)`, `+0x14=+0x0F<<15`; always drain `CE4C` | timer 0 → `+0x252=1`, `D25C=8` |
| 8 | `24F04` | `+0x0F==+0x1A`; `+0x250\|=0x20`; `1A680((int8)CE48*2+8)&~1` | `D25C=9` |

Case 4 equality is current-frame `+0x16`, not Gate C `+0x1A`.
`1A4AC` must run after case 3 clears `+0x98` bit `0x100`.
Do not copy `+0x0F` into `+0x16` or `+0x1A`.

`6C1CC` jals are only at `24A78` / `24B68` / `24BA8` /
`25388` (cases 0, 1-gate, 2, and 10+). Cases 4–8 do not
call it.

## +0x252

TEXT `sb` sites at `+0x252` are all stores of 1
(`24BD8`, `24ED8`, `25268`, `253D8`, `2AC78`). No
`sb $0, 594(rs)`. Case 2 stores 1 then jals
`6F39C(0x6C)`. Case 6 waits for 0. A 0→5 prefix
therefore parks at case 6 until the `0x6C` event
completes.

`6F39C` `0x6C..0x72` prelude (`6F3D4`) CD-reads
`*(0x80011618)=0x801ED7F8` when overlay bit
`0x10000` is clear, then installs `0x801F1BD8`.. at
`D_800E1044[0x17..]`. `D4698` jalrs `*(rec+0x30)`.
That overlay handler is the remaining clearer.

## Verify

```text
python3 pc_port/tools/pe_btl114_ce54_1a_oracle.py
PE_TEST_FILTER=BTL115 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
