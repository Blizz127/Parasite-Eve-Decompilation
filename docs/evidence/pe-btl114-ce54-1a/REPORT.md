# PE-BTL114 — CE54 writer and +0x1A provenance

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Gate B — CE54

`D_8009CE54` (`gp+0xE4`) has one TEXT store: `sb $v0, 228(gp)` at
`0x80024F94` (`li 1` at `24F90`). The same case writes `CE55=2` at
`24F8C`.

That store sits in `24A3C` case 9 (`jtbl[9]=0x80024F48` at
`0x80010824`). `24A3C` is `lbu D_8009D25C` then the 17-way. There is
no `jal 24A40`. The only `jal 24A3C` is `22394` at `0x800229D8`,
gated by `rec+0x4C & 0x80000`. `22394` itself is `21DE0` at
`0x80021E6C` when `slot+4` is in `[387, 407)`.

Case 9 waits `Aya+0x0F == Aya+0x1A`, then `1A680((int8)CE48*2+9)`,
then CE54/CE55, then `body |= 0x2000` (same `~0x6000 | 0x2000` mask
as `236E8`). Do not plant CE54. Do not jal `236E8` just to advance.

`D25C` is the case index. Writers are the case 0–8 / 10–16
increments inside `24A3C` plus `26934` zero. Those cases stay
deferred; they are the remaining D25C producers.

## Gate C — +0x1A

`1A680` `sw 0` at `+0x18` zeros `+0x1A`. `1A4AC` at `0x8001A520`
stores `+0x14` into `+0x18` before adding `+0x1C` to `+0x14`.
`+0x1A` is therefore the previous tick's `+0x16`. Equality
`+0x0F == +0x1A` is the tick after `+0x16` first equals `+0x0F`,
or the same-walk `1A4AC` after `2B0E8` phase 0 sets `+0x98` bit
`0x100` (copy, no increment).

`2B0E8` phase 2 (`2B1E8` / `2B1EC`) waits that equality, then
`1A680(0x15)` (or `0x18` if `D1A0&0x1800`). Do not copy one field
into the other.

## Verify

```text
python3 pc_port/tools/pe_btl114_ce54_1a_oracle.py
PE_TEST_FILTER=BTL114 ./pc_port/build/pe-native-tests
```
