# PE-BTL9 — func_800371B0 window init; 125E0 DrawSync+35038 walk

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_800371B0

169 words `0x800371B0..0x80037454`, SHA-256 `83a0b015…ba366`.
All eight callees are already-ported GPU/BIOS leaves. TEXT callers:
`3F074@0x8003F274` (live) and `0x80069C2C` (`a0=lw D_800B0E6C`).

```text
clear 4 x 56B records at D_800BCEA8
sb 0 → gp+0x130 / gp+0x160; sw 0 → gp+0x164
sw a0 → gp+0x120          # D_8009CE90
for i in 0..1:
    DR_MODE+setSprt+append at D_800AEC70 + i*28
    TILE 320x54 at y=170, x=0, rgb=2, SetSemiTrans
```

3F074 selects a0:

```text
if (D_800B0CD8 & 0x40000000)  # USA boot 527C8 ORs this
    a0 = D_800B162C           # overlay+0x954
else
    a0 = D_800B1628           # overlay+0x950
```

Live m0005i therefore uses `+0x954`. gp+0x120 is the 37870
scan base (TXT0). Not a battle-actor name.

## func_80012574 / func_800125E0

12574: 27 words, SHA-256 `bf4a0017…e124`. Sole caller
`6B4F8@0x8006B8BC`. Publishes a0 to `gp+0x94` and rebases the
pointer table. Result also lands at overlay+0x944.

125E0: 35 words, SHA-256 `7c30399d…434c`. Sole caller
`3F074@0x8003F27C`.

```text
DrawSync(0)
count = lbu(**(gp+0x94))
for i in 0..count-1:
    func_80035038(desc + 1 + i*2, a1=0, a2=1)
```

DrawSync is not battle rendering.

## func_80034FC4 / func_80035038

`34FC4`: 29 words `0x80034FC4..0x80035038`. Sole caller
`3F074@0x8003F0B0` (before the poll). Builds the 14-slot
freelist at `D_800BEA90` stride `0x280`, publishes the head to
`gp+0x53C` (`D_8009D2AC`), clears `gp+0x536` / `gp+0x49C` /
`gp+0x4E4`.

`35038`: 328 words, EXE-resident. First load is that head.
Empty head returns `v0=0`. Live `a1=0` pop/insert/init/`12700`
is ported. `D_800B0E70[type]` is EXE BSS 0, so `+0x1AC==0` and
the ctor ORs `0xE0` into `+0x98` and returns the actor. Type 0
`jal 2F76C` and `+0x1AC!=0` (`1A680`/`362B8`/`3D050`) are not
this cut.

`1266C`: 37 words, SHA-256 `34d11f74…645d`. Sole caller
`3F074@0x8003F0B8`. 72 task blocks at `D_8009D310` stride
`0x2C` → `gp+0x8C`. `12700` pops this list.

## Architecture

371B0, 12574, 125E0, 35038, and E0060 are all inside EXE
`taddr=0x80010000 tsize=0x1EE000`. Overlay package data
(`+0x944/+0x948/+0x950/+0x954`) is loaded; the code is not.
This is EXE-resident runtime plus loaded data, not a battle
code overlay. `M2_battle_overlay_entry` stays NO. Do not rename
to `M2_battle_runtime_entry` until a recurring battle tick /
actor-update authority is proven. Window init and DrawSync are
not that.

## Verify

```text
python3 pc_port/tools/pe_btl9_371b0_oracle.py
PE_TEST_FILTER=BTL9 ./pc_port/build/pe-native-tests
```
