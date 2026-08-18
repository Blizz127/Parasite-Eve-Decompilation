# PE-BTL10 — func_8001266C task pool; 35038 a1=0 empty-+0x1AC ctor

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_8001266C

37 words `0x8001266C..0x80012700`, SHA-256 `34d11f74…645d`.
Sole TEXT caller `3F074@0x8003F0B8`, immediately after `34FC4`.

```text
sw 0 → gp+0x590          # D_8009D300
sw D_8009D310 → gp+0x8C  # D_8009CDFC
for i in 0..70:          # sltiu 71
    slot[i]+0x24 = slot[i+1]   # stride 0x2C
sw 0 → D_8009DF68        # slot[71]+0x24
zero 64 words at D_800B6A80
```

`12700` pops `gp+0x8C` with no empty-list guard. Calling
`35038` after `34FC4` without `1266C` loads address 0.

## func_80035038 live cut

328 words `0x80035038..0x80035558`, SHA-256 `412e4f80…6ea7`.
TEXT callers: `125E0@0x80012628` (live `a1=0,a2=1`),
`0x80016C1C`, `0x80017394`.

```text
head = lw gp+0x53C            # D_8009D2AC
if head==0: return 0
pop; if a1==0: insert at D_8009D20C
else: insert as a1's predecessor
init motion/vtable/desc bytes
+0x1AC = D_800B0E70[type] if type<10 else 0
jal 12700(*( *D_800B161C + type*4 + 8 ), 0)
if +0x1AC==0: +0x98 |= 0xE0; return actor
```

`D_800B0E70` is overlay `+0x198`. EXE BSS is 0, so the host
live cut takes the `+0x1AC==0` arm. Type 0 publishes
`D_8009D254` and sets `+0x20=0x10000`; `jal 2F76C`
(`5218C`/`51980`/`51E64`) is not this cut. `+0x1AC!=0`
(`1A680`/`362B8`/`3D050`) is not this cut.

## Architecture

`1266C`, `35038`, `34FC4`, `125E0`, and `12700` are all
EXE-resident. Overlay supplies the spawn-list header at
`+0x944` (`D_800B161C`) and optional `+0x198` (`D_800B0E70`).
A persistent actor on `D_8009D20C` is not a recurring battle
tick. `M2_battle_overlay_entry` stays NO.

## Verify

```text
python3 pc_port/tools/pe_btl10_35038_oracle.py
PE_TEST_FILTER=BTL10 ./pc_port/build/pe-native-tests
```
