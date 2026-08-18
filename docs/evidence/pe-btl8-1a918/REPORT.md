# PE-BTL8 — func_8001A918 rebase; 0x800E0060 is EXE-resident

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_8001A918

56 words `0x8001A918..0x8001A9F8`, SHA-256 `c38eb3c9…6496`.
Zero `jal`/`jalr`. Sole TEXT caller: `3F074` @ `0x8003F23C`
after the 6C5BC poll exits.

```text
obj = lw D_800B1620          # overlay+0x948
sw obj, gp+0x48C             # D_8009D1FC
if obj+0x18 > 0x80000000:    # already rebased
    sw obj+0x28, gp+0x98     # D_8009CE08
    sw obj+0x20, gp+0x468    # D_8009D1D8
    return
obj+0x18 += obj
obj+0x1C += obj
obj+0x24 += obj
if obj+0x20: obj+0x20 += obj
sw obj+0x28, gp+0x98
sw obj+0x20, gp+0x468
sw obj+0x24, gp+0xA4         # D_8009CE14
sh (lhu(+8)>>5)+1, +8        # delay of count==0
rebase lhu(+2) words at +0x28
```

`D_800B1620` is not a lui+sw 0x1620 site. Writer is
`0x8006B8E8` `sw $v0, 0x948($s6)` inside `0x8006B4F8`.
Zeroer is `0x8006B3F0` in `func_8006B35C` (3F074's first jal).
EXE default is 0. Host obj=0 maps Kuseg `0x80000000`
(APPROXIMATION until a post-boot image exists).

Sibling `func_8001A890` clears the same gp slots at boot.

## func_800E0060 — REJECTED loaded-overlay

27 words `0x800E0060..0x800E00CC`, SHA-256 `cfa139eb…750e`.
EXE `taddr=0x80010000` `tsize=0x1EE000` covers
`0x80010000..0x801FE000`. Bytes are in SLUS. Zero `jal`/`jalr`.
Sole caller `3F074` @ `0x8003F284`.

```text
cur = D_800B0E5C - 0x14
sw cur, D_800E2800
for i, n=lh D_800E21A4; walk cur -= 0x14:
    if lbu(cur): sb 0; i++
sh 0, D_800E21A4
```

Not a battle-overlay entry. Not M2.

## After-poll still pending

```text
jal 0x800371B0   # 169w sha 83a0b015…; sw a0, gp+0x120
jal 0x800125E0   # 35w; DrawSync(0) then 35038 loop
```

`371B0` a0 is overlay bit `0x40000000` ? `D_800B162C` :
`D_800B1628` (also overlay+0x950/+0x954; same 6B4F8 writer
family). Do not stub those to a battle start.

## Verify

```text
python3 pc_port/tools/pe_btl8_1a918_oracle.py
PE_TEST_FILTER=BTL8 ./pc_port/build/pe-native-tests
```
