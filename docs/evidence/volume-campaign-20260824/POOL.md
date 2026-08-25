# Matching-C volume campaign pool — refreshed 2026-08-25

Generated from the active `[address, asm]` subsegments in `configs/USA/disc1.yaml`; stale generated asm outside active span geometry is excluded. Candidates are active `nonmatching` spans of 40 words or fewer. Screens are static triage and must be re-proven at C2 before an attempt. This refresh removes the six leaves matched by the 2026-08-24 campaign; the three COP2 helpers remain in SKIP.

Total: **1135** — TIER 1 88, TIER 2 214, TIER 3 98, SKIP 735.

## TIER 1

| file off | function | words | jr/tail | callers/refs | jal | gp | indexed symbolic/temp | loop/back-edge owner | repeated constant | boundaries | screen |
|---:|---|---:|---|---:|---:|---|---|---|---|---|---|
| 0x77FBC | `func_800877BC` | 6 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4E410 | `func_8005DC10` | 6 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x43CCC | `func_800534CC` | 6 | jr-ra | 1/1 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4E670 | `func_8005DE70` | 6 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x48518 | `func_80057D18` | 6 | jr-ra | 0/2 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x2788C | `func_8003708C` | 7 | jr-ra | 65/65 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x66AA0 | `func_800762A0` | 7 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x539C0 | `func_800631C0` | 7 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x77F28 | `func_80087728` | 7 | jr-ra | 2/2 | 0 | no | - | - | lui:16x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x77F0C | `func_8008770C` | 7 | jr-ra | 1/1 | 0 | no | - | - | lui:16x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x77F44 | `func_80087744` | 7 | jr-ra | 1/1 | 0 | no | - | - | lui:16x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x77F60 | `func_80087760` | 7 | jr-ra | 1/1 | 0 | no | - | - | lui:16x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x77F7C | `func_8008777C` | 7 | jr-ra | 1/1 | 0 | no | - | - | lui:16x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x77FD4 | `func_800877D4` | 7 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x77FF0 | `func_800877F0` | 7 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x74420 | `func_80083C20` | 7 | jr-ra | 0/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4F0A4 | `func_8005E8A4` | 8 | jr-ra | 247/247 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4F168 | `func_8005E968` | 8 | jr-ra | 16/16 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x569CC | `func_800661CC` | 8 | jr-ra | 7/7 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4C6E8 | `func_8005BEE8` | 8 | jr-ra | 6/6 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4E2DC | `func_8005DADC` | 8 | jr-ra | 5/5 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4E38C | `func_8005DB8C` | 8 | jr-ra | 4/4 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x69694 | `func_80078E94` | 8 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x74650 | `func_80083E50` | 8 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x74684 | `func_80083E84` | 8 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x746C4 | `func_80083EC4` | 8 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x746A4 | `func_80083EA4` | 8 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x77F98 | `func_80087798` | 9 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4E170 | `func_8005D970` | 9 | jr-ra | 1/1 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x69494 | `func_80078C94` | 9 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x69AD4 | `func_800792D4` | 10 | jr-ra | 6/6 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x68304 | `func_80077B04` | 10 | jr-ra | 5/5 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x68334 | `func_80077B34` | 10 | jr-ra | 4/4 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x7803C | `func_8008783C` | 10 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x78064 | `func_80087864` | 10 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x80D9C | `func_8009059C` | 10 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x80DC4 | `func_800905C4` | 10 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x467E0 | `func_80055FE0` | 11 | jr-ra | 12/12 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x69A44 | `func_80079244` | 11 | jr-ra | 6/6 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x2E8D0 | `func_8003E0D0` | 11 | jr-ra | 1/1 | 0 | no | - | - | addiu:0x2x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x467B4 | `func_80055FB4` | 11 | jr-ra | 1/1 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x732DC | `func_80082ADC` | 11 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x64C78 | `func_80074478` | 11 | jr-ra | 0/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0xBEC70 | `func_800CE470` | 11 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x69604 | `func_80078E04` | 12 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x7800C | `func_8008780C` | 12 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x780C0 | `func_800878C0` | 12 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x7808C | `func_8008788C` | 13 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0xB944C | `func_800C8C4C` | 13 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0xBA200 | `func_800C9A00` | 13 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0xBAD40 | `func_800CA540` | 13 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0xBC3BC | `func_800CBBBC` | 13 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x684B4 | `func_80077CB4` | 14 | jr-ra | 9/9 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x73F90 | `func_80083790` | 14 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x682C4 | `func_80077AC4` | 15 | jr-ra | 55/55 | 0 | no | - | - | lui:16x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x68264 | `func_80077A64` | 15 | jr-ra | 36/36 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x651D8 | `func_800749D8` | 15 | jr-ra | 4/4 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x53958 | `func_80063158` | 16 | jr-ra | 30/30 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4F188 | `func_8005E988` | 16 | jr-ra | 12/12 | 0 | yes | - | - | lui:16x3,ori:0xFFFFx3 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x2FF58 | `func_8003F758` | 16 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x53C28 | `func_80063428` | 17 | jr-ra | 59/59 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x5EC54 | `func_8006E454` | 17 | jr-ra | 2/2 | 0 | no | - | - | addiu:-0x30x3 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x55BB8 | `func_800653B8` | 18 | jr-ra | 1/1 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4E3AC | `func_8005DBAC` | 19 | jr-ra | 11/11 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x33C74 | `func_80043474` | 19 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x63A44 | `func_80073244` | 20 | jr-ra | 2/2 | 0 | no | - | - | addiu:-0x1x2,addiu:0x1x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x5E348 | `func_8006DB48` | 21 | jr-ra | 4/4 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x6EEB0 | `func_8007E6B0` | 21 | jr-ra | 4/4 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x4C4BC | `func_8005BCBC` | 21 | jr-ra | 2/2 | 0 | yes | - | - | addiu:0x8x2,addiu:0x10x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x7459C | `func_80083D9C` | 21 | jr-ra | 0/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x69978 | `func_80079178` | 22 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x699D0 | `func_800791D0` | 22 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x69434 | `func_80078C34` | 23 | jr-ra | 25/25 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x2CDD8 | `func_8003C5D8` | 24 | jr-ra | 26/26 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x6156C | `func_80070D6C` | 25 | jr-ra | 3/3 | 0 | no | - | - | lui:16x3,ori:0xFFFFx3,addiu:-0x4x2,ori:0x40x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x53C6C | `func_8006346C` | 26 | jr-ra | 4/4 | 0 | no | - | - | addiu:-0x1x3 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x2FF98 | `func_8003F798` | 26 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x2F00 | `func_80012700` | 29 | jr-ra | 7/7 | 0 | yes | - | - | addiu:0x1x2 | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x69B04 | `func_80079304` | 30 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x2E750 | `func_8003DF50` | 30 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x68D54 | `func_80078554` | 31 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0xBF070 | `func_800CE870` | 32 | jr-ra | 27/27 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x241A0 | `func_800339A0` | 32 | jr-ra | 6/6 | 0 | yes | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x71448 | `func_80080C48` | 32 | jr-ra | 6/6 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x66ABC | `func_800762BC` | 32 | jr-ra | 4/4 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x6B234 | `func_8007AA34` | 32 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x12050 | `func_80021850` | 34 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |
| 0x68BE4 | `func_800783E4` | 34 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | 0 jal; no indexed symbolic access; no loop |

## TIER 2

| file off | function | words | jr/tail | callers/refs | jal | gp | indexed symbolic/temp | loop/back-edge owner | repeated constant | boundaries | screen |
|---:|---|---:|---|---:|---:|---|---|---|---|---|---|
| 0xB76D8 | `func_800C6ED8` | 4 | jr-ra | 6/6 | 0 | no | sh:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB76C0 | `func_800C6EC0` | 6 | jr-ra | 7/7 | 0 | no | sh:$at,sh:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40820 | `func_80050020` | 6 | jr-ra | 0/2 | 0 | no | lw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6CD44 | `func_8007C544` | 7 | jr-ra | 1/1 | 0 | no | sw:$at,sw:$at,sw:$at | - | lui:00x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x71AF4 | `func_800812F4` | 7 | jr-ra | 1/1 | 0 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x5371C | `func_80062F1C` | 8 | jr-ra | 57/57 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB32F0 | `func_800C2AF0` | 8 | jr-ra | 8/8 | 0 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x757C4 | `func_80084FC4` | 8 | jr-ra | 7/7 | 0 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x712E4 | `func_80080AE4` | 8 | jr-ra | 3/3 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3E484 | `func_8004DC84` | 8 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6AC88 | `func_8007A488` | 8 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6B08C | `func_8007A88C` | 8 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6FF88 | `func_8007F788` | 8 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x712C4 | `func_80080AC4` | 8 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x71304 | `func_80080B04` | 8 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x71EF4 | `func_800816F4` | 8 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x72D34 | `func_80082534` | 8 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40888 | `func_80050088` | 8 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40A60 | `func_80050260` | 8 | jr-ra | 0/12 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40EE8 | `func_800506E8` | 8 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40F08 | `func_80050708` | 8 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40F28 | `func_80050728` | 8 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x413E8 | `func_80050BE8` | 8 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x41450 | `func_80050C50` | 8 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x414F8 | `func_80050CF8` | 8 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x71BE8 | `func_800813E8` | 8 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4FD94 | `func_8005F594` | 9 | jr-ra | 7/7 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x72CF0 | `func_800824F0` | 9 | jr-ra | 7/7 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42D58 | `func_80052558` | 9 | jr-ra | 3/3 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42F90 | `func_80052790` | 9 | jr-ra | 3/3 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42D34 | `func_80052534` | 9 | jr-ra | 2/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6E514 | `func_8007DD14` | 9 | jr-ra | 2/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x27634 | `func_80036E34` | 9 | jr-ra | 1/1 | 0 | no | sw:$at,sw:$at,sw:$at | - | lui:6x3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x27658 | `func_80036E58` | 9 | jr-ra | 1/1 | 0 | no | sw:$at,sw:$at,sw:$at | - | lui:6x3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3DCA0 | `func_8004D4A0` | 9 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3F14C | `func_8004E94C` | 9 | jr-ra | 1/1 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x55430 | `func_80064C30` | 9 | jr-ra | 1/1 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x55690 | `func_80064E90` | 9 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6B0EC | `func_8007A8EC` | 9 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x70464 | `func_8007FC64` | 9 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x70488 | `func_8007FC88` | 9 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x765C4 | `func_80085DC4` | 9 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x41860 | `func_80051060` | 9 | jr-ra | 0/2 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x32F70 | `func_80042770` | 10 | jr-ra | 4/4 | 0 | no | lbu:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x33164 | `func_80042964` | 10 | jr-ra | 1/1 | 0 | no | lbu:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x379BC | `func_800471BC` | 10 | jr-ra | 0/4 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x37B2C | `func_8004732C` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x37CA8 | `func_800474A8` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3BD34 | `func_8004B534` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3BD5C | `func_8004B55C` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3F730 | `func_8004EF30` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3FAE4 | `func_8004F2E4` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40150 | `func_8004F950` | 10 | jr-ra | 0/16 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40178 | `func_8004F978` | 10 | jr-ra | 0/4 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40730 | `func_8004FF30` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40758 | `func_8004FF58` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40780 | `func_8004FF80` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x407A8 | `func_8004FFA8` | 10 | jr-ra | 0/6 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x407D0 | `func_8004FFD0` | 10 | jr-ra | 0/6 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x407F8 | `func_8004FFF8` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40838 | `func_80050038` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40860 | `func_80050060` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40A04 | `func_80050204` | 10 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40CF4 | `func_800504F4` | 10 | jr-ra | 0/4 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40D1C | `func_8005051C` | 10 | jr-ra | 0/2 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x68200 | `func_80077A00` | 10 | jr-ra | 0/8 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x75898 | `func_80085098` | 10 | jr-ra | 0/2 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB852C | `func_800C7D2C` | 10 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB9670 | `func_800C8E70` | 10 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBA368 | `func_800C9B68` | 10 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBAF00 | `func_800CA700` | 10 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBC70C | `func_800CBF0C` | 10 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBD6E8 | `func_800CCEE8` | 10 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBE0C8 | `func_800CD8C8` | 10 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBE944 | `func_800CE144` | 10 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x5EEA8 | `func_8006E6A8` | 11 | jr-ra | 26/26 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x777F8 | `func_80086FF8` | 11 | jr-ra | 6/6 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77824 | `func_80087024` | 11 | jr-ra | 6/6 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x2A170 | `func_80039970` | 11 | jr-ra | 5/5 | 0 | no | sb:$at,sb:$at,sb:$at,sw:$at | - | lui:0091x3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42F64 | `func_80052764` | 11 | jr-ra | 4/4 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6D680 | `func_8007CE80` | 11 | jr-ra | 3/3 | 0 | no | - | 8007CE9C->8007CE88 slot=addiu     $a0, $a0, 0x4 | addiu:0x4x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3FC64 | `func_8004F464` | 11 | jr-ra | 2/2 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x47414 | `func_80056C14` | 11 | jr-ra | 2/2 | 0 | no | lhu:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76CCC | `func_800864CC` | 11 | jr-ra | 1/1 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB8500 | `func_800C7D00` | 11 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB9644 | `func_800C8E44` | 11 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBA33C | `func_800C9B3C` | 11 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBAED4 | `func_800CA6D4` | 11 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBC6E0 | `func_800CBEE0` | 11 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBD6BC | `func_800CCEBC` | 11 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBE09C | `func_800CD89C` | 11 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBE918 | `func_800CE118` | 11 | jr-ra | 0/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4ED4C | `func_8005E54C` | 12 | jr-ra | 2/2 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6ED94 | `func_8007E594` | 12 | jr-ra | 2/2 | 0 | no | - | 8007E5AC->8007E5A4 slot=addiu     $v0, $v0, -0x1 | addiu:0x3x2,addiu:-0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x43094 | `func_80052894` | 12 | jr-ra | 1/1 | 0 | no | lw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x5AAE8 | `func_8006A2E8` | 12 | jr-ra | 1/1 | 0 | no | sh:$at,sh:$at,sb:$at | - | lui:00x3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76714 | `func_80085F14` | 12 | jr-ra | 1/1 | 0 | no | sw:$at,sw:$at | - | addiu:0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x72A7C | `func_8008227C` | 12 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x50D90 | `func_80060590` | 13 | jr-ra | 11/11 | 1 | yes | - | - | addiu:0x4x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x50DF8 | `func_800605F8` | 13 | jr-ra | 8/8 | 1 | yes | - | - | addiu:0x2x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x50D5C | `func_8006055C` | 13 | jr-ra | 6/6 | 1 | yes | - | - | addiu:0x5x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x67BD0 | `func_800773D0` | 13 | jr-ra | 5/5 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x50D28 | `func_80060528` | 13 | jr-ra | 3/3 | 1 | yes | - | - | addiu:0x6x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x615D0 | `func_80070DD0` | 13 | jr-ra | 3/3 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76C64 | `func_80086464` | 13 | jr-ra | 3/3 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x758C0 | `func_800850C0` | 13 | jr-ra | 2/2 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x3C480 | `func_8004BC80` | 13 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x50DC4 | `func_800605C4` | 13 | jr-ra | 1/1 | 1 | yes | - | - | addiu:0x3x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x58340 | `func_80067B40` | 13 | jr-ra | 1/1 | 0 | no | sb:$at,sb:$at | - | lui:00x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x75344 | `func_80084B44` | 13 | jr-ra | 1/1 | 0 | no | sw:$at,sw:$at,sw:$at | - | lui:0084x2,addiu:0084x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76C98 | `func_80086498` | 13 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76FB0 | `func_800867B0` | 13 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40A2C | `func_8005022C` | 13 | jr-ra | 0/2 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x40B08 | `func_80050308` | 13 | jr-ra | 0/2 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x71764 | `func_80080F64` | 13 | jr-ra | 0/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6634C | `func_80075B4C` | 14 | jr-ra | 2/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77734 | `func_80086F34` | 14 | jr-ra | 2/2 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x79A18 | `func_80089218` | 14 | jr-ra | 2/2 | 0 | no | - | 80089240->80089224 slot=addiu     $a0, $a0, 0x11C | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x7F978 | `func_8008F178` | 14 | jr-ra | 2/2 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77074 | `func_80086874` | 14 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x771AC | `func_800869AC` | 14 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x772E4 | `func_80086AE4` | 14 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x775AC | `func_80086DAC` | 14 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77670 | `func_80086E70` | 14 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x2F2C8 | `func_8003EAC8` | 15 | jr-ra | 20/20 | 0 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x45EE8 | `func_800556E8` | 15 | jr-ra | 19/19 | 0 | yes | lh:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x49608 | `func_80058E08` | 15 | jr-ra | 3/3 | 0 | yes | lh:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x7A82C | `func_8008A02C` | 15 | jr-ra | 3/3 | 0 | no | - | 8008A058->8008A038 slot=addiu     $v1, $v1, 0x40 | addiu:0x40x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x45F24 | `func_80055724` | 15 | jr-ra | 2/2 | 0 | yes | - | 8005574C->8005573C slot=addiu     $a0, $a0, 0x4 | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x275F8 | `func_80036DF8` | 15 | jr-ra | 1/1 | 0 | no | sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:6x5 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x486D8 | `func_80057ED8` | 15 | jr-ra | 1/1 | 0 | yes | lh:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4C708 | `func_8005BF08` | 15 | jr-ra | 1/1 | 0 | yes | - | 8005BF34->8005BF1C slot=addiu     $a0, $a0, 0x1 | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4A6C8 | `func_80059EC8` | 16 | jr-ra | 4/4 | 0 | yes | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x7741C | `func_80086C1C` | 16 | jr-ra | 3/3 | 1 | no | sw:$at,sw:$at,sw:$at | - | lui:0x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x33478 | `func_80042C78` | 16 | jr-ra | 2/2 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x704BC | `func_8007FCBC` | 16 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76F70 | `func_80086770` | 16 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x775E4 | `func_80086DE4` | 16 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x776A8 | `func_80086EA8` | 16 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x7776C | `func_80086F6C` | 16 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x770AC | `func_800868AC` | 17 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x771E4 | `func_800869E4` | 17 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x7731C | `func_80086B1C` | 17 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x405A4 | `func_8004FDA4` | 17 | jr-ra | 0/4 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x72C00 | `func_80082400` | 17 | jr-ra | 0/2 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42DEC | `func_800525EC` | 18 | jr-ra | 49/49 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42E34 | `func_80052634` | 18 | jr-ra | 35/35 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42EC4 | `func_800526C4` | 18 | jr-ra | 23/23 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42E7C | `func_8005267C` | 18 | jr-ra | 22/22 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x7745C | `func_80086C5C` | 18 | jr-ra | 7/7 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | lui:0x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x534E4 | `func_80062CE4` | 18 | jr-ra | 6/6 | 0 | yes | - | 80062D0C->80062CF8 slot=nop | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x71198 | `func_80080998` | 18 | jr-ra | 6/6 | 0 | no | - | 800809BC->800809A8 slot=addiu     $a0, $a0, 0x1 | addiu:0x1x3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x71150 | `func_80080950` | 18 | jr-ra | 5/5 | 0 | no | - | 80080974->80080960 slot=addiu     $a0, $a0, 0x1 | addiu:0x1x3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76FE4 | `func_800867E4` | 18 | jr-ra | 2/2 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x7702C | `func_8008682C` | 18 | jr-ra | 2/2 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76F28 | `func_80086728` | 18 | jr-ra | 1/1 | 1 | no | sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x76EA4 | `func_800866A4` | 19 | jr-ra | 9/9 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x436C0 | `func_80052EC0` | 19 | jr-ra | 1/1 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x43724 | `func_80052F24` | 19 | jr-ra | 1/1 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77624 | `func_80086E24` | 19 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x776E8 | `func_80086EE8` | 19 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x777AC | `func_80086FAC` | 19 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4F0C4 | `func_8005E8C4` | 20 | jr-ra | 9/9 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x539DC | `func_800631DC` | 20 | jr-ra | 3/3 | 0 | yes | - | 8006321C->800631F0 slot=nop | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77890 | `func_80087090` | 20 | jr-ra | 2/2 | 1 | no | - | 800870BC->800870B4 slot=addu      $a0, $s0, $zero | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4F114 | `func_8005E914` | 21 | jr-ra | 9/9 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB76F8 | `func_800C6EF8` | 21 | jr-ra | 6/6 | 0 | no | - | 800C6F38->800C6F1C slot=addiu     $a3, $a3, 0x4 | addiu:0x4x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB774C | `func_800C6F4C` | 21 | jr-ra | 6/6 | 0 | no | - | 800C6F8C->800C6F70 slot=addiu     $a2, $a2, 0x4 | addiu:0x4x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x774A4 | `func_80086CA4` | 21 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:0x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42D94 | `func_80052594` | 22 | jr-ra | 5/5 | 0 | no | - | 800525CC->800525B0 slot=addiu     $a0, $a0, 0x1 | addiu:0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x42F0C | `func_8005270C` | 22 | jr-ra | 3/3 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x770F0 | `func_800868F0` | 22 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77228 | `func_80086A28` | 22 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77360 | `func_80086B60` | 22 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6E4AC | `func_8007DCAC` | 23 | jr-ra | 16/16 | 0 | no | - | 8007DCF8->8007DCC0 slot=nop | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBEC9C | `func_800CE49C` | 23 | jr-ra | 1/1 | 1 | no | lw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x5373C | `func_80062F3C` | 24 | jr-ra | 149/149 | 1 | yes | - | 80062F7C->80062F54 slot=nop | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x27CE8 | `func_800374E8` | 24 | jr-ra | 12/12 | 0 | no | lw:$at,sb:$at,sw:$at | 80037538->800374F4 slot=nop | lui:00x3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x269F4 | `func_800361F4` | 24 | jr-ra | 3/3 | 1 | yes | sw:$at | 80036234->80036210 slot=addiu     $s0, $s0, 0x4 | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6A9E0 | `func_8007A1E0` | 24 | jr-ra | 0/2 | 1 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x43408 | `func_80052C08` | 25 | jr-ra | 8/8 | 0 | no | - | 80052C24->80052C1C slot=addiu     $a0, $a0, 0x1; 80052C5C->80052C50 slot=addiu     $a0, $a0, 0x1 | addiu:0xFFx4,addiu:0x1x6 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x26A54 | `func_80036254` | 25 | jr-ra | 4/4 | 0 | no | - | 80036298->8003626C slot=nop; 800362A8->8003625C slot=addiu     $a0, $a0, 0x4 | addiu:0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x641C4 | `func_800739C4` | 25 | jr-ra | 4/4 | 1 | no | sw:$at,sw:$at | - | lui:16x2,ori:0xFFFFx2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77148 | `func_80086948` | 25 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:0x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x77280 | `func_80086A80` | 25 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:0x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x773B8 | `func_80086BB8` | 25 | jr-ra | 1/1 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:0x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x74E44 | `func_80084644` | 26 | jr-ra | 0/2 | 0 | no | - | 8008469C->80084694 slot=addiu     $v1, $v1, 0x1 | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x2D74 | `func_80012574` | 27 | jr-ra | 1/1 | 0 | yes | - | 800125C8->800125AC slot=addiu     $v1, $v1, 0x4 | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x55164 | `func_80064964` | 27 | jr-ra | 1/1 | 1 | no | sb:$at,sb:$at,sb:$at,sb:$at,sb:$at,sb:$at,sb:$at,sb:$at | - | lui:0x5 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x62B34 | `func_80072334` | 27 | jr-ra | 1/1 | 0 | no | - | 80072360->8007234C slot=sb        $v0, 0x0($a0); 8007238C->80072378 slot=addiu     $a3, $a3, 0x1 | addiu:-0x1x4,addiu:0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB85E4 | `func_800C7DE4` | 27 | jr-ra | 0/1 | 1 | no | sh:$at,sh:$at,sw:$at,sh:$at | - | lui:34x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xB9728 | `func_800C8F28` | 27 | jr-ra | 0/1 | 1 | no | sh:$at,sh:$at,sw:$at,sh:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBA420 | `func_800C9C20` | 27 | jr-ra | 0/1 | 1 | no | sh:$at,sh:$at,sw:$at,sh:$at | - | lui:35x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBAFB8 | `func_800CA7B8` | 27 | jr-ra | 0/1 | 1 | no | sh:$at,sh:$at,sw:$at,sh:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xC4E98 | `func_800D4698` | 27 | jr-ra | 0/1 | 0 | no | sw:$at,sw:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x44720 | `func_80053F20` | 28 | jr-ra | 8/8 | 1 | yes | - | 80053F68->80053F48 slot=nop | addiu:0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x63D54 | `func_80073554` | 28 | jr-ra | 4/4 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x412D8 | `func_80050AD8` | 28 | jr-ra | 0/8 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4C0A8 | `func_8005B8A8` | 29 | jr-ra | 5/5 | 0 | no | - | 8005B8F8->8005B8B4 slot=sll       $v0, $a3, 2 | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x257C4 | `func_80034FC4` | 29 | jr-ra | 1/1 | 0 | yes | sw:$at,sw:$at,sw:$at | 80034FFC->80034FE4 slot=addiu     $v1, $v1, 0x280; 80035024->80035010 slot=nop | addiu:0x280x3,lui:4x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x551D0 | `func_800649D0` | 30 | jr-ra | 3/3 | 1 | yes | sb:$at,sb:$at,sb:$at,sb:$at,sb:$at,sb:$at,sb:$at,sb:$at | - | lui:0x5 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xC4E20 | `func_800D4620` | 30 | jr-ra | 0/1 | 0 | no | - | 800D4658->800D4650 slot=addiu     $a1, $a1, -0x2; 800D4688->800D4670 slot=addiu     $a1, $a1, 0xC | addiu:0xCx3,addiu:0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x27C6C | `func_8003746C` | 31 | jr-ra | 1/1 | 0 | no | lh:$at,lbu:$at,sb:$at | 800374D8->8003747C slot=andi      $v0, $a1, 0xFF | lui:00x3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x43630 | `func_80052E30` | 32 | jr-ra | 44/44 | 1 | yes | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4481C | `func_8005401C` | 32 | jr-ra | 2/2 | 1 | yes | - | 80054080->8005406C slot=addu      $v0, $s0, $zero | addiu:0x2x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4C510 | `func_8005BD10` | 33 | jr-ra | 1/1 | 0 | yes | - | 8005BD40->8005BD28 slot=addiu     $v1, $v1, 0x1 | addiu:0xFFx3 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x55480 | `func_80064C80` | 34 | jr-ra | 5/5 | 1 | yes | - | - | lui:16x4,ori:0xFFFFx4,addiu:-0x2x2,addiu:0x2x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x4C594 | `func_8005BD94` | 34 | jr-ra | 2/2 | 0 | yes | - | 8005BDC4->8005BDAC slot=addiu     $a0, $a0, 0x1 | addiu:0xFFx2,addiu:-0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x7A758 | `func_80089F58` | 34 | jr-ra | 1/1 | 1 | no | - | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x44790 | `func_80053F90` | 35 | jr-ra | 8/8 | 1 | no | - | 80053FF0->80053FBC slot=addiu     $s0, $s0, 0x2 | addiu:0x1x2 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x6F388 | `func_8007EB88` | 35 | jr-ra | 3/3 | 1 | no | sw:$at,sb:$at | - | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0xBE884 | `func_800CE084` | 37 | jr-ra | 0/1 | 1 | no | sw:$at,sw:$at,sw:$at,sb:$at,sh:$at,sh:$at,sh:$at,sh:$at,sb:$at,sb:$at,sb:$at | - | lui:2x11 | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x2681C | `func_8003601C` | 38 | jr-ra | 1/1 | 0 | yes | - | 800360A4->80036030 slot=nop | - | real/real | 0-1 jal / handled indexed access or simple loop |
| 0x685C4 | `func_80077DC4` | 40 | jr-ra | 73/73 | 0 | no | lh:$at,lh:$at,lh:$at,lh:$at | - | lui:009589x2 | real/real | 0-1 jal / handled indexed access or simple loop |

## TIER 3

| file off | function | words | jr/tail | callers/refs | jal | gp | indexed symbolic/temp | loop/back-edge owner | repeated constant | boundaries | screen |
|---:|---|---:|---|---:|---:|---|---|---|---|---|---|
| 0x60BF4 | `func_800703F4` | 10 | jr-ra | 6/6 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3D5AC | `func_8004CDAC` | 10 | jr-ra | 1/1 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x55454 | `func_80064C54` | 11 | jr-ra | 9/9 | 2 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3E1D8 | `func_8004D9D8` | 11 | jr-ra | 4/4 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3CDDC | `func_8004C5DC` | 11 | jr-ra | 1/1 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x4C944 | `func_8005C144` | 12 | jr-ra | 2/2 | 2 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3FFD8 | `func_8004F7D8` | 12 | jr-ra | 0/2 | 2 | yes | - | - | addiu:0x6x2 | real/real | function hood passes; higher control/call complexity |
| 0x6E574 | `func_8007DD74` | 13 | jr-ra | 2/2 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3C4B4 | `func_8004BCB4` | 13 | jr-ra | 0/2 | 3 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x6E640 | `func_8007DE40` | 14 | jr-ra | 1/1 | 4 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x40CBC | `func_800504BC` | 14 | jr-ra | 0/2 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x411A8 | `func_800509A8` | 14 | jr-ra | 0/2 | 2 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x684F4 | `func_80077CF4` | 15 | jr-ra | 76/76 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x32A28 | `func_80042228` | 15 | jr-ra | 1/3 | 4 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3F43C | `func_8004EC3C` | 15 | jr-ra | 0/2 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3F478 | `func_8004EC78` | 15 | jr-ra | 0/2 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x40568 | `func_8004FD68` | 15 | jr-ra | 0/2 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x409C8 | `func_800501C8` | 15 | jr-ra | 0/4 | 3 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x40D44 | `func_80050544` | 15 | jr-ra | 0/2 | 3 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x758F4 | `func_800850F4` | 16 | jr-ra | 8/8 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x375BC | `func_80046DBC` | 16 | jr-ra | 0/2 | 3 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3FF98 | `func_8004F798` | 16 | jr-ra | 0/2 | 3 | yes | - | - | addiu:0x4x2 | real/real | function hood passes; higher control/call complexity |
| 0x400D0 | `func_8004F8D0` | 16 | jr-ra | 0/10 | 2 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x40110 | `func_8004F910` | 16 | jr-ra | 0/2 | 2 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3DA98 | `func_8004D298` | 17 | jr-ra | 1/1 | 2 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x35BE8 | `func_800453E8` | 17 | jr-ra | 0/2 | 4 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3DE90 | `func_8004D690` | 17 | jr-ra | 0/2 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x4043C | `func_8004FC3C` | 17 | jr-ra | 0/2 | 3 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x41470 | `func_80050C70` | 17 | jr-ra | 0/2 | 3 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x414B4 | `func_80050CB4` | 17 | jr-ra | 0/2 | 3 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3CD94 | `func_8004C594` | 18 | jr-ra | 5/5 | 2 | no | - | - | addiu:0x13x2 | real/real | function hood passes; higher control/call complexity |
| 0x594E0 | `func_80068CE0` | 18 | jr-ra | 1/1 | 6 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0xB6D3C | `func_800C653C` | 18 | jr-ra | 1/1 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x41408 | `func_80050C08` | 18 | jr-ra | 0/2 | 2 | no | - | - | addiu:-0x2x2 | real/real | function hood passes; higher control/call complexity |
| 0x6FF2C | `func_8007F72C` | 19 | jr-ra | 11/11 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x7EFF4 | `func_8008E7F4` | 19 | jr-ra | 1/1 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x41348 | `func_80050B48` | 19 | jr-ra | 0/2 | 4 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x402F8 | `func_8004FAF8` | 20 | jr-ra | 0/2 | 3 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x40978 | `func_80050178` | 20 | jr-ra | 0/4 | 5 | yes | - | - | addiu:0x68x2 | real/real | function hood passes; higher control/call complexity |
| 0x7D354 | `func_8008CB54` | 21 | jr-ra | 2/2 | 4 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x39808 | `func_80049008` | 21 | jr-ra | 1/1 | 6 | no | - | - | addiu:0x1x3 | real/real | function hood passes; higher control/call complexity |
| 0x714DC | `func_80080CDC` | 21 | jr-ra | 1/1 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3985C | `func_8004905C` | 21 | jr-ra | 0/4 | 4 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3D5D4 | `func_8004CDD4` | 21 | jr-ra | 0/2 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3D830 | `func_8004D030` | 21 | jr-ra | 0/6 | 2 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x41394 | `func_80050B94` | 21 | jr-ra | 0/2 | 4 | yes | - | - | addiu:-0x2x2 | real/real | function hood passes; higher control/call complexity |
| 0x41710 | `func_80050F10` | 21 | jr-ra | 0/2 | 4 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x41764 | `func_80050F64` | 21 | jr-ra | 0/2 | 4 | yes | - | - | addiu:-0x2x2 | real/real | function hood passes; higher control/call complexity |
| 0x417B8 | `func_80050FB8` | 21 | jr-ra | 0/2 | 4 | yes | - | - | addiu:-0x2x2 | real/real | function hood passes; higher control/call complexity |
| 0x4180C | `func_8005100C` | 21 | jr-ra | 0/2 | 4 | yes | - | - | addiu:-0x2x2 | real/real | function hood passes; higher control/call complexity |
| 0x5E750 | `func_8006DF50` | 22 | jr-ra | 19/19 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3BD84 | `func_8004B584` | 22 | jr-ra | 1/1 | 3 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x389FC | `func_800481FC` | 22 | jr-ra | 0/2 | 6 | yes | - | - | addiu:0x4x2 | real/real | function hood passes; higher control/call complexity |
| 0x40E90 | `func_80050690` | 22 | jr-ra | 0/2 | 6 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x43770 | `func_80052F70` | 23 | jr-ra | 61/61 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x72518 | `func_80081D18` | 23 | jr-ra | 3/3 | 3 | no | - | 80081D58->80081D50 slot=sltiu     $v0, $v0, 0x1 | addiu:0x10x2 | real/real | function hood passes; higher control/call complexity |
| 0x3E178 | `func_8004D978` | 24 | jr-ra | 3/3 | 3 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x68934 | `func_80078134` | 24 | jr-ra | 3/3 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x383EC | `func_80047BEC` | 25 | jr-ra | 1/1 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3C10C | `func_8004B90C` | 25 | jr-ra | 1/1 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x408A8 | `func_800500A8` | 25 | jr-ra | 0/2 | 3 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3B73C | `func_8004AF3C` | 26 | jr-ra | 1/1 | 3 | no | - | - | addiu:0x21x2 | real/real | function hood passes; higher control/call complexity |
| 0x3B83C | `func_8004B03C` | 26 | jr-ra | 1/1 | 3 | no | - | - | addiu:0x23x2 | real/real | function hood passes; higher control/call complexity |
| 0x3FF30 | `func_8004F730` | 26 | jr-ra | 0/4 | 2 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3CCB4 | `func_8004C4B4` | 27 | jr-ra | 1/1 | 3 | yes | - | - | addiu:0x1Ex2 | real/real | function hood passes; higher control/call complexity |
| 0x6E5D4 | `func_8007DDD4` | 27 | jr-ra | 1/1 | 8 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x4090C | `func_8005010C` | 27 | jr-ra | 0/2 | 8 | yes | - | - | addiu:0x68x3,addiu:0x10x2 | real/real | function hood passes; higher control/call complexity |
| 0x6F0F4 | `func_8007E8F4` | 28 | jr-ra | 3/3 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x3F3CC | `func_8004EBCC` | 28 | jr-ra | 0/2 | 4 | yes | - | 8004EC20->8004EC08 slot=nop | - | real/real | function hood passes; higher control/call complexity |
| 0x401A0 | `func_8004F9A0` | 28 | jr-ra | 0/6 | 4 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x404F8 | `func_8004FCF8` | 28 | jr-ra | 0/2 | 4 | yes | - | 8004FD4C->8004FD34 slot=nop | - | real/real | function hood passes; higher control/call complexity |
| 0x405E8 | `func_8004FDE8` | 28 | jr-ra | 0/4 | 4 | yes | - | 8004FE3C->8004FE24 slot=nop | - | real/real | function hood passes; higher control/call complexity |
| 0x620D0 | `func_800718D0` | 29 | jr-ra | 7/7 | 2 | no | - | - | addiu:0x8x2,addiu:0x4x2,addiu:0xCx2 | real/real | function hood passes; higher control/call complexity |
| 0x3BDDC | `func_8004B5DC` | 29 | jr-ra | 0/2 | 9 | no | - | - | addiu:0x7Bx2 | real/real | function hood passes; higher control/call complexity |
| 0x3CD20 | `func_8004C520` | 29 | jr-ra | 0/2 | 9 | yes | - | - | addiu:0x2x2,addiu:0xEBx2 | real/real | function hood passes; higher control/call complexity |
| 0x41004 | `func_80050804` | 29 | jr-ra | 0/2 | 5 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x37F14 | `func_80047714` | 30 | jr-ra | 0/2 | 8 | yes | - | - | addiu:0xAx2,lui:16x2,ori:0xFFFFx2 | real/real | function hood passes; higher control/call complexity |
| 0x40480 | `func_8004FC80` | 30 | jr-ra | 0/2 | 6 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x40E18 | `func_80050618` | 30 | jr-ra | 0/2 | 4 | yes | - | - | addiu:-0x2x3 | real/real | function hood passes; higher control/call complexity |
| 0x5E6D4 | `func_8006DED4` | 31 | jr-ra | 4/4 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x66384 | `func_80075B84` | 32 | jr-ra | 4/4 | 2 | no | - | - | addiu:-0x1x2 | real/real | function hood passes; higher control/call complexity |
| 0x3B59C | `func_8004AD9C` | 32 | jr-ra | 1/1 | 4 | no | - | - | addiu:0x20x3 | real/real | function hood passes; higher control/call complexity |
| 0x73DC0 | `func_800835C0` | 33 | jr-ra | 1/1 | 3 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x40C38 | `func_80050438` | 33 | jr-ra | 0/2 | 8 | no | - | - | addiu:-0x20x2 | real/real | function hood passes; higher control/call complexity |
| 0x37B54 | `func_80047354` | 36 | jr-ra | 0/2 | 8 | yes | - | - | addiu:0x1x2 | real/real | function hood passes; higher control/call complexity |
| 0x37CD0 | `func_800474D0` | 36 | jr-ra | 0/2 | 6 | yes | - | - | addiu:0x1x2 | real/real | function hood passes; higher control/call complexity |
| 0xBDD0C | `func_800CD50C` | 36 | jr-ra | 0/1 | 5 | no | sw:$at,sw:$at | - | addiu:0x3x2,addiu:0x20x2 | real/real | function hood passes; higher control/call complexity |
| 0x5E538 | `func_8006DD38` | 37 | jr-ra | 3/3 | 2 | no | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x40658 | `func_8004FE58` | 37 | jr-ra | 0/2 | 3 | yes | - | - | addiu:-0x25x2 | real/real | function hood passes; higher control/call complexity |
| 0x38748 | `func_80047F48` | 38 | jr-ra | 0/2 | 6 | no | - | - | addiu:0x2x2,addiu:-0x1x2 | real/real | function hood passes; higher control/call complexity |
| 0x3B7A4 | `func_8004AFA4` | 38 | jr-ra | 0/2 | 7 | no | - | - | addiu:0x1x3 | real/real | function hood passes; higher control/call complexity |
| 0x3B8A4 | `func_8004B0A4` | 38 | jr-ra | 0/2 | 7 | no | - | - | addiu:0x1x3 | real/real | function hood passes; higher control/call complexity |
| 0x3E204 | `func_8004DA04` | 38 | jr-ra | 0/4 | 6 | yes | - | 8004DA7C->8004DA58 slot=slt       $v0, $s0, $s1 | addiu:0x10x2 | real/real | function hood passes; higher control/call complexity |
| 0x40038 | `func_8004F838` | 38 | jr-ra | 0/4 | 6 | yes | - | 8004F88C->8004F874 slot=nop | - | real/real | function hood passes; higher control/call complexity |
| 0x40D80 | `func_80050580` | 38 | jr-ra | 0/2 | 6 | yes | - | - | - | real/real | function hood passes; higher control/call complexity |
| 0x76E08 | `func_80086608` | 39 | jr-ra | 4/4 | 2 | no | sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:0x2 | real/real | function hood passes; higher control/call complexity |
| 0x372BC | `func_80046ABC` | 39 | jr-ra | 1/1 | 6 | no | - | - | addiu:0x8x2 | real/real | function hood passes; higher control/call complexity |
| 0x37E78 | `func_80047678` | 39 | jr-ra | 1/1 | 13 | yes | - | - | addiu:0x2x2 | real/real | function hood passes; higher control/call complexity |
| 0x48370 | `func_80057B70` | 40 | jr-ra | 1/1 | 6 | yes | - | - | addiu:0x1x3 | real/real | function hood passes; higher control/call complexity |

## SKIP

| file off | function | words | jr/tail | callers/refs | jal | gp | indexed symbolic/temp | loop/back-edge owner | repeated constant | boundaries | screen |
|---:|---|---:|---|---:|---:|---|---|---|---|---|---|
| 0x64CC8 | `func_800744C8` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x64F70 | `func_80074770` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x65238 | `func_80074A38` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x6824C | `func_80077A4C` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x68378 | `func_80077B78` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x68398 | `func_80077B98` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x683B8 | `func_80077BB8` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x683D8 | `func_80077BD8` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x683F8 | `func_80077BF8` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x68418 | `func_80077C18` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x68438 | `func_80077C38` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x68458 | `func_80077C58` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x68478 | `func_80077C78` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x746F8 | `func_80083EF8` | 1 | no | 0/0 | 0 | no | - | - | - | real/real | no jr-ra/tail; no caller/ref |
| 0x62254 | `func_80071A54` | 3 | no | 205/205 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62274 | `func_80071A74` | 3 | no | 46/46 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62EF4 | `func_800726F4` | 3 | no | 24/24 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62244 | `func_80071A44` | 3 | no | 19/19 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62EC4 | `func_800726C4` | 3 | no | 13/13 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6445C | `func_80073C5C` | 3 | no | 13/13 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62224 | `func_80071A24` | 3 | no | 10/10 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62EE4 | `func_800726E4` | 3 | no | 10/10 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62F74 | `func_80072774` | 3 | no | 9/9 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62234 | `func_80071A34` | 3 | no | 8/8 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x64474 | `func_80073C74` | 3 | no | 7/7 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x64484 | `func_80073C84` | 3 | no | 7/7 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62F34 | `func_80072734` | 3 | no | 6/6 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62204 | `func_80071A04` | 3 | no | 5/5 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x63254 | `func_80072A54` | 3 | no | 4/4 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E9F4 | `func_8007E1F4` | 3 | no | 4/4 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62264 | `func_80071A64` | 3 | no | 3/3 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62F04 | `func_80072704` | 3 | no | 3/3 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62F54 | `func_80072754` | 3 | no | 2/2 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62FA4 | `func_800727A4` | 3 | no | 2/2 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x64234 | `func_80073A34` | 3 | no | 2/2 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x64B84 | `func_80074384` | 3 | no | 2/2 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x64BA4 | `func_800743A4` | 3 | no | 2/2 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E9E4 | `func_8007E1E4` | 3 | no | 2/2 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x75FE4 | `func_800857E4` | 3 | no | 2/2 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x75FF4 | `func_800857F4` | 3 | no | 2/2 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62214 | `func_80071A14` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62B14 | `func_80072314` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62B24 | `func_80072324` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62EB4 | `func_800726B4` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62ED4 | `func_800726D4` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62F44 | `func_80072744` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62F64 | `func_80072764` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62F84 | `func_80072784` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x62F94 | `func_80072794` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x63264 | `func_80072A64` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x64B6C | `func_8007436C` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x64B94 | `func_80074394` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x68254 | `func_80077A54` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E544 | `func_8007DD44` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E554 | `func_8007DD54` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E564 | `func_8007DD64` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E5B4 | `func_8007DDB4` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E5C4 | `func_8007DDC4` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E9A4 | `func_8007E1A4` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E9B4 | `func_8007E1B4` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E9C4 | `func_8007E1C4` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6E9D4 | `func_8007E1D4` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6EB14 | `func_8007E314` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6EB24 | `func_8007E324` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x76004 | `func_80085804` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x7DFB0 | `func_8008D7B0` | 3 | no | 1/1 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x6972C | `func_80078F2C` | 3 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x697A0 | `func_80078FA0` | 3 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x278BC | `func_800370BC` | 4 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x278CC | `func_800370CC` | 4 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x696B4 | `func_80078EB4` | 4 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x696C4 | `func_80078EC4` | 4 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x696D4 | `func_80078ED4` | 4 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0xB76E8 | `func_800C6EE8` | 4 | jr-ra | 0/0 | 0 | no | sh:$at | - | - | real/real | no caller/ref |
| 0x6EA04 | `func_8007E204` | 5 | no | 1/1 | 0 | no | lw:destination | - | - | real/real | no jr-ra/tail; destination-as-temp |
| 0x6EA18 | `func_8007E218` | 5 | no | 1/1 | 0 | no | lw:destination | - | - | real/real | no jr-ra/tail; destination-as-temp |
| 0x67344 | `func_80076B44` | 5 | jr-ra | 0/0 | 0 | no | lbu:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x69704 | `func_80078F04` | 5 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69718 | `func_80078F18` | 5 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69738 | `func_80078F38` | 5 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69764 | `func_80078F64` | 5 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69778 | `func_80078F78` | 5 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6978C | `func_80078F8C` | 5 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x7CF0C | `func_8008C70C` | 5 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xB3310 | `func_800C2B10` | 6 | jr-ra | 15/15 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x703F0 | `func_8007FBF0` | 6 | jr-ra | 11/11 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x64610 | `func_80073E10` | 6 | jr-ra | 9/9 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0xB3328 | `func_800C2B28` | 6 | jr-ra | 3/3 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x5F46C | `func_8006EC6C` | 6 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6974C | `func_80078F4C` | 6 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69804 | `func_80079004` | 6 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69A28 | `func_80079228` | 6 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x621E4 | `func_800719E4` | 7 | no | 10/10 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x7A708 | `func_80089F08` | 7 | jr-ra | 1/1 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x7AE0 | `func_800172E0` | 7 | jr-ra | 0/0 | 0 | yes | - | - | - | real/real | no caller/ref |
| 0x7BF4 | `func_800173F4` | 7 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x7EE0 | `func_800176E0` | 7 | jr-ra | 0/0 | 0 | yes | - | - | - | real/real | no caller/ref |
| 0x7FAC | `func_800177AC` | 7 | jr-ra | 0/0 | 0 | yes | - | - | - | real/real | no caller/ref |
| 0x82A4 | `func_80017AA4` | 7 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9954 | `func_80019154` | 7 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xAB74 | `func_8001A374` | 7 | jr-ra | 0/0 | 0 | no | sb:$at | - | - | real/real | no caller/ref |
| 0x2EDF0 | `func_8003E5F0` | 7 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x8004C | `func_8008F84C` | 7 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x7A160 | `func_80089960` | 8 | jr-ra | 12/12 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | destination-as-temp |
| 0x7A4F0 | `func_80089CF0` | 8 | jr-ra | 12/12 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | destination-as-temp |
| 0x7A328 | `func_80089B28` | 8 | jr-ra | 11/11 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | destination-as-temp |
| 0x66950 | `func_80076150` | 8 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x7AFC | `func_800172FC` | 8 | jr-ra | 0/0 | 0 | yes | - | - | - | real/real | no caller/ref |
| 0x8128 | `func_80017928` | 8 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x8148 | `func_80017948` | 8 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x8168 | `func_80017968` | 8 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x853C | `func_80017D3C` | 8 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x855C | `func_80017D5C` | 8 | jr-ra | 0/0 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x857C | `func_80017D7C` | 8 | jr-ra | 0/0 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x8F54 | `func_80018754` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x9A98 | `func_80019298` | 8 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9BB8 | `func_800193B8` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x9E18 | `func_80019618` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x9E38 | `func_80019638` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x9F28 | `func_80019728` | 8 | jr-ra | 0/0 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x9F48 | `func_80019748` | 8 | jr-ra | 0/0 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0xA524 | `func_80019D24` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x62144 | `func_80071944` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x621C4 | `func_800719C4` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x696E4 | `func_80078EE4` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x697C4 | `func_80078FC4` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x697E4 | `func_80078FE4` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69934 | `func_80079134` | 8 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6ABCC | `func_8007A3CC` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6AC68 | `func_8007A468` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6B0AC | `func_8007A8AC` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6B0CC | `func_8007A8CC` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6B110 | `func_8007A910` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6D854 | `func_8007D054` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6F664 | `func_8007EE64` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6FFC8 | `func_8007F7C8` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x71324 | `func_80080B24` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x72D14 | `func_80082514` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x72D54 | `func_80082554` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x75320 | `func_80084B20` | 8 | jr-ra | 0/2 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x7C96C | `func_8008C16C` | 8 | jr-ra | 0/0 | 0 | no | sh:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x7CA70 | `func_8008C270` | 8 | jr-ra | 0/0 | 0 | no | sh:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x81880 | `func_80091080` | 8 | jr-ra | 0/0 | 1 | no | - | - | - | real / ? | no caller/ref; boundary not mapped |
| 0x29AEC | `func_800392EC` | 9 | jr-ra | 5/5 | 0 | no | lbu:destination,lbu:destination | - | lui:0091x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x4E428 | `func_8005DC28` | 9 | jr-ra | 5/5 | 0 | no | lw:destination,lbu:$at | - | - | real/real | destination-as-temp |
| 0x5F3E4 | `func_8006EBE4` | 9 | jr-ra | 2/2 | 0 | no | lbu:destination,lh:destination | - | lui:00x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x76744 | `func_80085F44` | 9 | jr-ra | 2/2 | 0 | no | lw:destination,sw:$at | - | lui:34x2 | real/real | destination-as-temp |
| 0x75F8 | `func_80016DF8` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x761C | `func_80016E1C` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x7ABC | `func_800172BC` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x84C4 | `func_80017CC4` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x8518 | `func_80017D18` | 9 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x859C | `func_80017D9C` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x85C0 | `func_80017DC0` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9E58 | `func_80019658` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9E7C | `func_8001967C` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9EA0 | `func_800196A0` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9EC4 | `func_800196C4` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA104 | `func_80019904` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA128 | `func_80019928` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA1F8 | `func_800199F8` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA29C | `func_80019A9C` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA2C0 | `func_80019AC0` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA2E4 | `func_80019AE4` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA404 | `func_80019C04` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA428 | `func_80019C28` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA9F0 | `func_8001A1F0` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xAB2C | `func_8001A32C` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xAB50 | `func_8001A350` | 9 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x44A1C | `func_8005421C` | 9 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x67320 | `func_80076B20` | 9 | jr-ra | 0/0 | 0 | no | lw:destination,sb:$at | - | - | real/real | no caller/ref; destination-as-temp |
| 0x67410 | `func_80076C10` | 9 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x698C0 | `func_800790C0` | 9 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69954 | `func_80079154` | 9 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6EBB4 | `func_8007E3B4` | 9 | no | 0/8 | 0 | no | - | - | - | real/real | no jr-ra/tail |
| 0x806D8 | `func_8008FED8` | 9 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x807C0 | `func_8008FFC0` | 9 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80854 | `func_80090054` | 9 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x808E4 | `func_800900E4` | 9 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80978 | `func_80090178` | 9 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x177D0 | `func_80026FD0` | 10 | jr-ra | 10/10 | 0 | yes | lb:destination | - | - | real/real | destination-as-temp |
| 0x569A4 | `func_800661A4` | 10 | jr-ra | 8/8 | 0 | no | lhu:destination,lhu:destination | - | - | real/real | destination-as-temp |
| 0x4E28C | `func_8005DA8C` | 10 | jr-ra | 2/2 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x6E484 | `func_8007DC84` | 10 | jr-ra | 2/2 | 0 | no | lw:destination | - | lui:16x2 | real/real | destination-as-temp |
| 0x73D78 | `func_80083578` | 10 | jr-ra | 2/2 | 0 | no | lw:destination | 80083590->80083584 slot=nop | - | real/real | destination-as-temp |
| 0x4E2B4 | `func_8005DAB4` | 10 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x6E45C | `func_8007DC5C` | 10 | jr-ra | 1/1 | 0 | no | lw:destination | - | lui:16x2 | real/real | destination-as-temp |
| 0x7A728 | `func_80089F28` | 10 | jr-ra | 1/1 | 0 | no | lw:destination | - | lui:009x2 | real/real | destination-as-temp |
| 0x7A94 | `func_80017294` | 10 | jr-ra | 0/0 | 0 | yes | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x7EB8 | `func_800176B8` | 10 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x8250 | `func_80017A50` | 10 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x82C0 | `func_80017AC0` | 10 | jr-ra | 0/0 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x9154 | `func_80018954` | 10 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x294E4 | `func_80038CE4` | 10 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x2F11C | `func_8003E91C` | 10 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0x66444 | `func_80075C44` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6646C | `func_80075C6C` | 10 | jr-ra | 0/0 | 0 | no | - | - | lui:16x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x69834 | `func_80079034` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6985C | `func_8007905C` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x698E4 | `func_800790E4` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6990C | `func_8007910C` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6E678 | `func_8007DE78` | 10 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0x71534 | `func_80080D34` | 10 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x803D4 | `func_8008FBD4` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x804BC | `func_8008FCBC` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80D74 | `func_80090574` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80DEC | `func_800905EC` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80E14 | `func_80090614` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80E3C | `func_8009063C` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80E64 | `func_80090664` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80E8C | `func_8009068C` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x81148 | `func_80090948` | 10 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0xB3368 | `func_800C2B68` | 10 | jr-ra | 0/0 | 0 | no | lw:destination | - | lui:16x2 | real/real | no caller/ref; destination-as-temp |
| 0x68484 | `func_80077C84` | 11 | jr-ra | 29/29 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x11854 | `func_80021054` | 11 | jr-ra | 7/7 | 0 | yes | lw:destination | - | - | real/real | destination-as-temp |
| 0x2E8A4 | `func_8003E0A4` | 11 | jr-ra | 1/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x5619C | `func_8006599C` | 11 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | destination-as-temp |
| 0x7578C | `func_80084F8C` | 11 | jr-ra | 1/3 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x8020 | `func_80017820` | 11 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x81F8 | `func_800179F8` | 11 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x8224 | `func_80017A24` | 11 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x8278 | `func_80017A78` | 11 | jr-ra | 0/0 | 0 | no | lw:destination,sw:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x8D6C | `func_8001856C` | 11 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA1CC | `func_800199CC` | 11 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x430C4 | `func_800528C4` | 11 | jr-ra | 0/0 | 0 | no | sw:$at | - | - | real/real | no caller/ref |
| 0x70160 | `func_8007F960` | 11 | jr-ra | 0/2 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x7B9D0 | `func_8008B1D0` | 11 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x7FCE8 | `func_8008F4E8` | 11 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x803FC | `func_8008FBFC` | 11 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x804E4 | `func_8008FCE4` | 11 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x81330 | `func_80090B30` | 11 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x813A0 | `func_80090BA0` | 11 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x644F4 | `func_80073CF4` | 12 | jr-ra | 11/11 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x644C4 | `func_80073CC4` | 12 | jr-ra | 5/5 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x64494 | `func_80073C94` | 12 | jr-ra | 4/4 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x673E0 | `func_80076BE0` | 12 | jr-ra | 3/3 | 0 | no | lw:destination,lw:destination | - | lui:16x2 | real/real | destination-as-temp |
| 0x4E140 | `func_8005D940` | 12 | jr-ra | 2/2 | 2 | no | lw:destination | - | addiu:0x10x2 | real/real | destination-as-temp; address-retention |
| 0x64558 | `func_80073D58` | 12 | jr-ra | 2/2 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x41E84 | `func_80051684` | 12 | jr-ra | 1/1 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x50044 | `func_8005F844` | 12 | jr-ra | 1/1 | 0 | yes | - | - | - | real/real | $v0-steal pattern |
| 0x760E8 | `func_800858E8` | 12 | jr-ra | 1/1 | 0 | no | lw:destination,lw:$at | - | lui:009x2 | real/real | destination-as-temp |
| 0x804C | `func_8001784C` | 12 | jr-ra | 0/0 | 0 | yes | - | - | - | real/real | no caller/ref |
| 0x84E8 | `func_80017CE8` | 12 | jr-ra | 0/0 | 1 | no | lw:destination | - | addiu:-0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x9064 | `func_80018864` | 12 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x9094 | `func_80018894` | 12 | jr-ra | 0/0 | 1 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x9398 | `func_80018B98` | 12 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x9684 | `func_80018E84` | 12 | jr-ra | 0/0 | 0 | no | sh:$at,sh:$at | - | - | real/real | no caller/ref |
| 0x9ADC | `func_800192DC` | 12 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9F68 | `func_80019768` | 12 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x275C8 | `func_80036DC8` | 12 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0x29110 | `func_80038910` | 12 | jr-ra | 0/0 | 0 | yes | - | - | - | real/real | no caller/ref |
| 0x2F144 | `func_8003E944` | 12 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref; address-retention |
| 0x561C8 | `func_800659C8` | 12 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | no caller/ref; destination-as-temp |
| 0x62164 | `func_80071964` | 12 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x8x2 | real/real | no caller/ref |
| 0x62194 | `func_80071994` | 12 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x8x2 | real/real | no caller/ref |
| 0x64588 | `func_80073D88` | 12 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x645B8 | `func_80073DB8` | 12 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x6631C | `func_80075B1C` | 12 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x69634 | `func_80078E34` | 12 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69664 | `func_80078E64` | 12 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6E7E0 | `func_8007DFE0` | 12 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0x4F050 | `func_8005E850` | 13 | jr-ra | 4/4 | 1 | no | lb:destination,lb:destination | - | lui:00x2 | real/real | destination-as-temp |
| 0x75974 | `func_80085174` | 13 | jr-ra | 4/4 | 0 | no | lw:destination,lw:destination | 80085198->8008518C slot=nop | lui:4x2,addiu:0x1x2 | real/real | destination-as-temp |
| 0x6CC44 | `func_8007C444` | 13 | jr-ra | 3/3 | 0 | no | lw:destination | 8007C468->8007C44C slot=sw        $zero, 0x0($v1) | - | real/real | destination-as-temp |
| 0x6ECE0 | `func_8007E4E0` | 13 | jr-ra | 1/1 | 0 | no | - | 8007E504->8007E4F4 slot=addiu     $v0, $v0, 0x4 | addiu:0x4x2 | real/real | 3+ loop scratch regs |
| 0x76118 | `func_80085918` | 13 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | lui:009x2 | real/real | destination-as-temp |
| 0x7C10 | `func_80017410` | 13 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x8668 | `func_80017E68` | 13 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9C50 | `func_80019450` | 13 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA584 | `func_80019D84` | 13 | jr-ra | 0/0 | 1 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x4ECE4 | `func_8005E4E4` | 13 | jr-ra | 0/0 | 1 | yes | - | - | - | real/real | no caller/ref |
| 0x4ED18 | `func_8005E518` | 13 | jr-ra | 0/0 | 1 | yes | - | - | - | real/real | no caller/ref |
| 0x64524 | `func_80073D24` | 13 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x65DBC | `func_800755BC` | 13 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x662E8 | `func_80075AE8` | 13 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6AC00 | `func_8007A400` | 13 | jr-ra | 0/0 | 0 | no | lw:$at | - | - | real/real | no caller/ref |
| 0x6AC34 | `func_8007A434` | 13 | jr-ra | 0/0 | 0 | no | lw:$at | - | - | real/real | no caller/ref |
| 0x7614C | `func_8008594C` | 13 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x774F8 | `func_80086CF8` | 13 | jr-ra | 0/0 | 1 | no | sw:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x5E3E0 | `func_8006DBE0` | 14 | jr-ra | 3/3 | 0 | no | - | 8006DC04->8006DBEC slot=addiu     $a1, $a1, 0x2 | - | real/real | $v0-steal pattern |
| 0x760B0 | `func_800858B0` | 14 | jr-ra | 2/2 | 0 | no | lw:destination | - | - | real/real | destination-as-temp; $v0-steal pattern |
| 0x41DC0 | `func_800515C0` | 14 | jr-ra | 1/1 | 0 | no | lw:destination,sh:$at | - | - | real/real | destination-as-temp |
| 0x6E8C0 | `func_8007E0C0` | 14 | jr-ra | 1/1 | 3 | no | - | - | addiu:0x1x2 | real/real | address-retention |
| 0x730F4 | `func_800828F4` | 14 | jr-ra | 1/1 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x62B8 | `func_80015AB8` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x77E0 | `func_80016FE0` | 14 | jr-ra | 0/0 | 0 | no | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x8454 | `func_80017C54` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x848C | `func_80017C8C` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x9330 | `func_80018B30` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x9A60 | `func_80019260` | 14 | jr-ra | 0/0 | 1 | no | lw:destination,lw:destination | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x9BD8 | `func_800193D8` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x9F98 | `func_80019798` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0xA4EC | `func_80019CEC` | 14 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xAC3C | `func_8001A43C` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0xAC74 | `func_8001A474` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x5629C | `func_80065A9C` | 14 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | no caller/ref; destination-as-temp |
| 0x6E810 | `func_8007E010` | 14 | jr-ra | 0/0 | 3 | no | sw:$at | - | - | real/real | no caller/ref |
| 0x752E8 | `func_80084AE8` | 14 | jr-ra | 0/2 | 0 | no | - | 80084B0C->80084AF8 slot=addiu     $v1, $v1, 0xF0 | addiu:0x10x2 | real/real | $v0-steal pattern |
| 0x76EF0 | `func_800866F0` | 14 | jr-ra | 0/0 | 1 | no | sw:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x7BD80 | `func_8008B580` | 14 | jr-ra | 0/0 | 1 | no | sh:$at,sw:$at | - | lui:009x2 | real/real | no caller/ref |
| 0x7FF84 | `func_8008F784` | 14 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80EE4 | `func_800906E4` | 14 | jr-ra | 0/0 | 0 | no | lbu:$at | - | - | real/real | no caller/ref |
| 0x80F1C | `func_8009071C` | 14 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0xBD240 | `func_800CCA40` | 14 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBD278 | `func_800CCA78` | 14 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xD0780 | `func_800DFF80` | 14 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x62CF4 | `func_800724F4` | 15 | jr-ra | 4/4 | 0 | no | - | 80072520->80072504 slot=sb        $a1, 0x0($a0) | - | real/real | 3+ loop scratch regs |
| 0x6EB34 | `func_8007E334` | 15 | jr-ra | 1/3 | 0 | no | - | 8007E360->8007E35C slot=nop | - | real/real | 3+ loop scratch regs |
| 0x76584 | `func_80085D84` | 15 | jr-ra | 1/1 | 1 | no | lw:destination,sw:$at | - | lui:38x2 | real/real | destination-as-temp |
| 0x85E4 | `func_80017DE4` | 15 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x8F18 | `func_80018718` | 15 | jr-ra | 0/0 | 0 | no | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0xA5B8 | `func_80019DB8` | 15 | jr-ra | 0/0 | 0 | no | lw:$at | - | - | real/real | no caller/ref |
| 0xAAF0 | `func_8001A2F0` | 15 | jr-ra | 0/0 | 0 | no | - | 8001A310->8001A308 slot=addiu     $a1, $a1, 0x1 | addiu:0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x330D4 | `func_800428D4` | 15 | jr-ra | 0/2 | 0 | no | lw:destination,sb:$at | - | - | real/real | destination-as-temp |
| 0x33128 | `func_80042928` | 15 | jr-ra | 0/2 | 1 | no | lw:destination,sw:$at,sw:$at | - | lui:860x2 | real/real | destination-as-temp |
| 0x56260 | `func_80065A60` | 15 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | no caller/ref; destination-as-temp |
| 0x69884 | `func_80079084` | 15 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x6E3C8 | `func_8007DBC8` | 15 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | - | - | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0x76D68 | `func_80086568` | 15 | jr-ra | 0/0 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x7752C | `func_80086D2C` | 15 | jr-ra | 0/0 | 1 | no | sw:$at,sw:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x7CED0 | `func_8008C6D0` | 15 | jr-ra | 0/0 | 0 | no | sw:$at | 8008C6FC->8008C6E8 slot=addiu     $v1, $v1, 0x11C | - | real/real | no caller/ref |
| 0x80478 | `func_8008FC78` | 15 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x8110C | `func_8009090C` | 15 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0xB9480 | `func_800C8C80` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xB94BC | `func_800C8CBC` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xB94F8 | `func_800C8CF8` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBA234 | `func_800C9A34` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBC3F0 | `func_800CBBF0` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBC42C | `func_800CBC2C` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBC468 | `func_800CBC68` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBD36C | `func_800CCB6C` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBDDB0 | `func_800CD5B0` | 15 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xD0444 | `func_800DFC44` | 15 | jr-ra | 0/0 | 0 | no | - | 800DFC70->800DFC4C slot=nop | - | real/real | no caller/ref |
| 0xD0824 | `func_800E0024` | 15 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref; $v0-steal pattern |
| 0x70304 | `func_8007FB04` | 16 | jr-ra | 1/1 | 2 | no | sw:$at,sw:$at,sw:$at | - | lui:009x2 | real/real | address-retention |
| 0x7B1C | `func_8001731C` | 16 | jr-ra | 0/0 | 0 | yes | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x8334 | `func_80017B34` | 16 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x8374 | `func_80017B74` | 16 | jr-ra | 0/0 | 0 | yes | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9C10 | `func_80019410` | 16 | jr-ra | 0/0 | 0 | yes | lbu:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x9EE8 | `func_800196E8` | 16 | jr-ra | 0/0 | 0 | no | lw:destination,sw:$at,sh:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0xA0C4 | `func_800198C4` | 16 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA14C | `func_8001994C` | 16 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0xA18C | `func_8001998C` | 16 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0xA544 | `func_80019D44` | 16 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0xABFC | `func_8001A3FC` | 16 | jr-ra | 0/0 | 0 | no | lw:destination,sw:$at,sh:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x66404 | `func_80075C04` | 16 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x67358 | `func_80076B58` | 16 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | 80076B88->80076B74 slot=sw        $v1, 0x0($v0) | addiu:-0x1x3 | real/real | no caller/ref; destination-as-temp |
| 0x6E960 | `func_8007E160` | 16 | jr-ra | 0/2 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x74704 | `func_80083F04` | 16 | jr-ra | 0/0 | 1 | no | lw:destination,sw:$at | - | - | real/real | no caller/ref; destination-as-temp |
| 0x75934 | `func_80085134` | 16 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0x77850 | `func_80087050` | 16 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | 80087068->80087058 slot=addu      $v0, $zero, $zero | lui:009x2 | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0x7FC30 | `func_8008F430` | 16 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x812AC | `func_80090AAC` | 16 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0xB891C | `func_800C811C` | 16 | jr-ra | 0/1 | 0 | no | lhu:destination,lw:destination,lhu:destination | - | - | real/real | destination-as-temp |
| 0xBB388 | `func_800CAB88` | 16 | jr-ra | 0/1 | 0 | no | lhu:destination,lw:destination,lhu:destination | - | - | real/real | destination-as-temp |
| 0xBCA44 | `func_800CC244` | 16 | jr-ra | 0/1 | 0 | no | lhu:destination,lhu:destination,lhu:destination | - | - | real/real | destination-as-temp |
| 0xBCA84 | `func_800CC284` | 16 | jr-ra | 0/1 | 0 | no | lhu:destination,lhu:destination,lhu:destination | - | - | real/real | destination-as-temp |
| 0xBCC40 | `func_800CC440` | 16 | jr-ra | 0/1 | 0 | no | lhu:destination,lhu:destination,lhu:destination | - | - | real/real | destination-as-temp |
| 0xBD87C | `func_800CD07C` | 16 | jr-ra | 0/1 | 0 | no | lhu:destination,lw:destination,lhu:destination | - | lui:7x2 | real/real | destination-as-temp |
| 0x5E39C | `func_8006DB9C` | 17 | jr-ra | 7/7 | 0 | no | - | 8006DBCC->8006DBA8 slot=addiu     $v1, $v1, 0x2 | - | real/real | $v0-steal pattern |
| 0x336DC | `func_80042EDC` | 17 | jr-ra | 2/2 | 0 | yes | lbu:destination | - | - | real/real | destination-as-temp |
| 0x36B34 | `func_80046334` | 17 | jr-ra | 2/2 | 1 | yes | lbu:destination | - | - | real/real | destination-as-temp; $v0-steal pattern |
| 0x6E2E0 | `func_8007DAE0` | 17 | jr-ra | 2/2 | 0 | no | lw:destination,lw:destination,lw:destination | - | lui:009x2 | real/real | destination-as-temp |
| 0x42CD0 | `func_800524D0` | 17 | jr-ra | 1/1 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x735BC | `func_80082DBC` | 17 | jr-ra | 1/1 | 4 | no | - | - | - | real/real | address-retention |
| 0x8744 | `func_80017F44` | 17 | jr-ra | 0/0 | 0 | no | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x406EC | `func_8004FEEC` | 17 | jr-ra | 0/2 | 3 | yes | lw:destination | - | - | real/real | destination-as-temp; address-retention |
| 0x48410 | `func_80057C10` | 17 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0x6EB70 | `func_8007E370` | 17 | jr-ra | 0/0 | 0 | no | - | 8007E390->8007E384 slot=nop | - | real/real | no caller/ref; $v0-steal pattern |
| 0x77568 | `func_80086D68` | 17 | jr-ra | 0/0 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x7B840 | `func_8008B040` | 17 | jr-ra | 0/0 | 1 | no | sw:$at | - | - | real/real | no caller/ref; $v0-steal pattern |
| 0x7B884 | `func_8008B084` | 17 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x7B924 | `func_8008B124` | 17 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0x7FEB0 | `func_8008F6B0` | 17 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x81220 | `func_80090A20` | 17 | jr-ra | 0/0 | 0 | no | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x812EC | `func_80090AEC` | 17 | jr-ra | 0/0 | 1 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x8135C | `func_80090B5C` | 17 | jr-ra | 0/0 | 1 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0xC47D8 | `func_800D3FD8` | 17 | jr-ra | 0/0 | 0 | no | lw:destination | - | addiu:0x80x2 | real/real | no caller/ref; destination-as-temp |
| 0x4E344 | `func_8005DB44` | 18 | jr-ra | 74/74 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x44A40 | `func_80054240` | 18 | jr-ra | 4/4 | 0 | yes | lb:destination,lb:destination | - | - | real/real | destination-as-temp; $v0-steal pattern |
| 0x54530 | `func_80063D30` | 18 | jr-ra | 4/4 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x56154 | `func_80065954` | 18 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | destination-as-temp |
| 0x67398 | `func_80076B98` | 18 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination,lw:destination,lw:destination | - | lui:16x2,ori:0xFFFFx2 | real/real | destination-as-temp |
| 0x7312C | `func_8008292C` | 18 | jr-ra | 1/1 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x73174 | `func_80082974` | 18 | jr-ra | 1/1 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x7A6B8 | `func_80089EB8` | 18 | jr-ra | 1/1 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x7D40 | `func_80017540` | 18 | jr-ra | 0/0 | 0 | yes | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x7F64 | `func_80017764` | 18 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x8620 | `func_80017E20` | 18 | jr-ra | 0/0 | 0 | yes | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9410 | `func_80018C10` | 18 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x9CB0 | `func_800194B0` | 18 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x9CF8 | `func_800194F8` | 18 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0xA8CC | `func_8001A0CC` | 18 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0xA914 | `func_8001A114` | 18 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0xA9A8 | `func_8001A1A8` | 18 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x355CC | `func_80044DCC` | 18 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0x4E2FC | `func_8005DAFC` | 18 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x5610C | `func_8006590C` | 18 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | no caller/ref; destination-as-temp |
| 0x81264 | `func_80090A64` | 18 | jr-ra | 0/0 | 0 | no | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0xBD12C | `func_800CC92C` | 18 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBED60 | `func_800CE560` | 19 | jr-ra | 28/28 | 0 | no | - | 800CE58C->800CE580 slot=addu      $v1, $v1, $a1 | addiu:0xCx2 | real/real | 3+ loop scratch regs |
| 0x7D308 | `func_8008CB08` | 19 | jr-ra | 13/13 | 0 | no | lw:destination,lw:destination,sw:$at | - | lui:009x3 | real/real | destination-as-temp |
| 0x7A7E0 | `func_80089FE0` | 19 | jr-ra | 2/2 | 0 | no | - | 8008A01C->80089FEC slot=addiu     $a0, $a0, 0x11C | addiu:0x1x2 | real/real | 3+ loop scratch regs |
| 0x64CD4 | `func_800744D4` | 19 | jr-ra | 1/1 | 2 | no | lw:destination | - | lui:00956x2 | real/real | destination-as-temp; address-retention |
| 0x6C3B0 | `func_8007BBB0` | 19 | jr-ra | 1/1 | 2 | no | sw:$at,sw:$at,sw:$at,sw:$at | - | lui:009x4 | real/real | address-retention |
| 0x5DFC | `func_800155FC` | 19 | jr-ra | 0/0 | 0 | yes | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x82E8 | `func_80017AE8` | 19 | jr-ra | 0/0 | 1 | no | lw:destination,lw:destination | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x8F74 | `func_80018774` | 19 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x9018 | `func_80018818` | 19 | jr-ra | 0/0 | 1 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x9860 | `func_80019060` | 19 | jr-ra | 0/0 | 1 | yes | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0xA95C | `func_8001A15C` | 19 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x55B6C | `func_8006536C` | 19 | jr-ra | 0/0 | 0 | yes | - | 80065394->80065384 slot=addiu     $v1, $v1, 0x4; 800653A4->8006537C slot=addiu     $a1, $a1, 0xC | addiu:0x1x2 | real/real | no caller/ref |
| 0x72D74 | `func_80082574` | 19 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x811C0 | `func_800909C0` | 19 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x4E44C | `func_8005DC4C` | 20 | jr-ra | 39/39 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x4E49C | `func_8005DC9C` | 20 | jr-ra | 16/16 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x4E4EC | `func_8005DCEC` | 20 | jr-ra | 11/11 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x5379C | `func_80062F9C` | 20 | jr-ra | 6/6 | 0 | yes | sw:$at | 80062FC0->80062FB4 slot=addiu     $v0, $v1, 0x90 | lui:2x2,addiu:2x2,addiu:0x90x2 | real/real | $v0-steal pattern |
| 0x72ABC | `func_800822BC` | 20 | jr-ra | 3/3 | 2 | no | - | - | - | real/real | address-retention |
| 0x7B3F0 | `func_8008ABF0` | 20 | jr-ra | 3/3 | 0 | no | lw:destination | 8008AC30->8008AC10 slot=sll       $a0, $a0, 1 | - | real/real | destination-as-temp |
| 0x7DFD0 | `func_8008D7D0` | 20 | jr-ra | 2/2 | 1 | no | lhu:destination,sw:$at,sh:$at,sh:$at | - | lui:00x3 | real/real | destination-as-temp |
| 0x32C64 | `func_80042464` | 20 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | - | real/real | destination-as-temp |
| 0x337E8 | `func_80042FE8` | 20 | jr-ra | 1/1 | 1 | yes | lw:destination | - | - | real/real | destination-as-temp; $v0-steal pattern |
| 0x425F8 | `func_80051DF8` | 20 | jr-ra | 1/1 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x4E53C | `func_8005DD3C` | 20 | jr-ra | 1/1 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x52D68 | `func_80062568` | 20 | jr-ra | 1/1 | 0 | yes | - | 8006258C->80062580 slot=addiu     $v0, $v1, 0x90 | addiu:0x90x2 | real/real | $v0-steal pattern |
| 0x332D8 | `func_80042AD8` | 20 | jr-ra | 0/0 | 0 | no | lbu:$at | - | - | real/real | no caller/ref; $v0-steal pattern |
| 0x3D7D4 | `func_8004CFD4` | 20 | jr-ra | 0/2 | 4 | no | - | - | - | real/real | address-retention |
| 0x68B44 | `func_80078344` | 20 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x68B94 | `func_80078394` | 20 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x693E4 | `func_80078BE4` | 20 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x7FA24 | `func_8008F224` | 20 | jr-ra | 0/0 | 0 | no | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x80428 | `func_8008FC28` | 20 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x81170 | `func_80090970` | 20 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x5E680 | `func_8006DE80` | 21 | jr-ra | 22/22 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x7B39C | `func_8008AB9C` | 21 | jr-ra | 14/14 | 0 | no | lw:destination | 8008ABE0->8008ABC0 slot=sll       $v1, $v1, 1 | - | real/real | destination-as-temp |
| 0x5E4E4 | `func_8006DCE4` | 21 | jr-ra | 11/11 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0xB7898 | `func_800C7098` | 21 | jr-ra | 6/6 | 0 | no | - | 800C70D8->800C70B8 slot=addiu     $t0, $t0, 0x4 | addiu:0x4x2 | real/real | 3+ loop scratch regs |
| 0x11880 | `func_80021080` | 21 | jr-ra | 1/1 | 0 | yes | lw:destination,lw:destination | - | - | real/real | destination-as-temp; $v0-steal pattern |
| 0x118D4 | `func_800210D4` | 21 | jr-ra | 1/1 | 0 | yes | lw:destination,lw:destination | - | - | real/real | destination-as-temp; $v0-steal pattern |
| 0x42C7C | `func_8005247C` | 21 | jr-ra | 1/1 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x76204 | `func_80085A04` | 21 | jr-ra | 1/1 | 0 | no | lw:destination,sw:$at,sw:$at,sw:$at | - | lui:16x2 | real/real | destination-as-temp |
| 0x9248 | `func_80018A48` | 21 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x3384C | `func_8004304C` | 21 | jr-ra | 0/0 | 5 | no | - | - | addiu:0x2x3 | real/real | no caller/ref |
| 0x41A58 | `func_80051258` | 21 | jr-ra | 0/0 | 1 | yes | - | 80051290->8005127C slot=nop | - | real/real | no caller/ref |
| 0x47FE0 | `func_800577E0` | 21 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref; $v0-steal pattern |
| 0x57E78 | `func_80067678` | 21 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | no caller/ref; destination-as-temp |
| 0x66494 | `func_80075C94` | 21 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0x69A74 | `func_80079274` | 21 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x7FB28 | `func_8008F328` | 21 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,sw:$at | - | lui:009x3 | real/real | no caller/ref; destination-as-temp |
| 0x80300 | `func_8008FB00` | 21 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x45E10 | `func_80055610` | 22 | jr-ra | 1/1 | 0 | yes | lw:destination | 80055640->80055624 slot=sra       $a1, $a1, 1 | lui:00x2,addiu:00x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x64BB4 | `func_800743B4` | 22 | jr-ra | 1/1 | 2 | no | lw:destination,sw:$at | - | lui:00956x2 | real/real | destination-as-temp; address-retention |
| 0x7FC8 | `func_800177C8` | 22 | jr-ra | 0/0 | 1 | yes | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x8FC0 | `func_800187C0` | 22 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref |
| 0x6CAA0 | `func_8007C2A0` | 22 | jr-ra | 0/0 | 2 | no | lw:destination,lw:destination | - | - | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x6E404 | `func_8007DC04` | 22 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,lw:destination | - | lui:0x3,lui:16x3 | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0x80610 | `func_8008FE10` | 22 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0xD0320 | `func_800DFB20` | 22 | jr-ra | 0/0 | 0 | no | - | - | addiu:-0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x584BC | `func_80067CBC` | 23 | jr-ra | 13/13 | 0 | no | - | - | lui:8x2,addiu:8x2 | real/real | $v0-steal pattern |
| 0x4E688 | `func_8005DE88` | 23 | jr-ra | 11/11 | 0 | yes | - | 8005DEAC->8005DEA0 slot=addiu     $v0, $v1, 0xC | addiu:0xCx2 | real/real | $v0-steal pattern |
| 0x766B4 | `func_80085EB4` | 23 | jr-ra | 7/7 | 1 | no | sh:$at,lhu:destination,lw:destination | - | lui:14x2 | real/real | destination-as-temp |
| 0x65B58 | `func_80075358` | 23 | jr-ra | 2/2 | 0 | no | lw:destination,lw:destination | - | lui:0095744x2 | real/real | destination-as-temp |
| 0x57FA0 | `func_800677A0` | 23 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | destination-as-temp |
| 0x653B8 | `func_80074BB8` | 23 | jr-ra | 1/1 | 0 | no | lw:destination | - | - | real/real | destination-as-temp; $v0-steal pattern |
| 0x765F4 | `func_80085DF4` | 23 | jr-ra | 1/1 | 1 | no | lw:destination,sw:$at | - | lui:16x2,ori:0xFFFFx2 | real/real | destination-as-temp |
| 0x76654 | `func_80085E54` | 23 | jr-ra | 1/1 | 1 | no | lw:destination,sw:$at | - | lui:16x2,ori:0xFFFFx2 | real/real | destination-as-temp |
| 0x61510 | `func_80070D10` | 23 | jr-ra | 0/0 | 0 | no | - | 80070D40->80070D2C slot=addiu     $t5, $t5, -0x1 | lui:16x3,ori:0xFFFFx3 | real/real | no caller/ref; 3+ loop scratch regs |
| 0x7B8C8 | `func_8008B0C8` | 23 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0xC04F4 | `func_800CFCF4` | 23 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x79F24 | `func_80089724` | 24 | jr-ra | 16/16 | 0 | no | - | 80089768->8008972C slot=sll       $t0, $t0, 1 | addiu:0x1x2 | real/real | 3+ loop scratch regs |
| 0x6586C | `func_8007506C` | 24 | jr-ra | 14/14 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x658CC | `func_800750CC` | 24 | jr-ra | 2/2 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x7B0CC | `func_8008A8CC` | 24 | jr-ra | 2/2 | 0 | no | lw:destination | 8008A91C->8008A8F0 slot=addiu     $a0, $a0, 0x11C | addiu:0x1x2 | real/real | destination-as-temp; 3+ loop scratch regs |
| 0x6AA44 | `func_8007A244` | 24 | jr-ra | 1/1 | 1 | no | lw:destination,sw:$at,sw:$at,sw:$at,sw:$at,sw:$at,sh:$at,sw:$at | - | lui:00x3 | real/real | destination-as-temp |
| 0x654C8 | `func_80074CC8` | 24 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0x68CF4 | `func_800784F4` | 24 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x745F0 | `func_80083DF0` | 24 | jr-ra | 0/2 | 0 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0xB6948 | `func_800C6148` | 24 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0xB6D84 | `func_800C6584` | 24 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x5F408 | `func_8006EC08` | 25 | jr-ra | 5/5 | 0 | no | lb:destination,lhu:destination,lb:destination | - | lui:00x3 | real/real | destination-as-temp; $v0-steal pattern |
| 0xBEDAC | `func_800CE5AC` | 25 | jr-ra | 4/4 | 0 | no | lw:destination | 800CE5F0->800CE5E4 slot=addu      $a1, $a1, $a2 | addiu:0xCx2 | real/real | destination-as-temp; 3+ loop scratch regs |
| 0x725F8 | `func_80081DF8` | 25 | jr-ra | 3/3 | 3 | no | - | - | - | real/real | address-retention |
| 0x57ECC | `func_800676CC` | 25 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | destination-as-temp |
| 0x6E27C | `func_8007DA7C` | 25 | jr-ra | 1/1 | 3 | no | lhu:destination,lw:destination | - | - | real/real | destination-as-temp; address-retention |
| 0x8B00 | `func_80018300` | 25 | jr-ra | 0/0 | 0 | yes | lw:destination | 80018344->80018324 slot=nop; 80018354->80018310 slot=addiu     $a0, $a0, 0x4 | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x929C | `func_80018A9C` | 25 | jr-ra | 0/0 | 1 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | no caller/ref; destination-as-temp |
| 0x531BC | `func_800629BC` | 25 | jr-ra | 0/0 | 0 | yes | - | 800629EC->800629D4 slot=addiu     $v1, $v1, 0x4; 80062A10->800629D0 slot=addu      $a1, $zero, $zero | - | real/real | no caller/ref |
| 0x6505C | `func_8007485C` | 25 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0x650C0 | `func_800748C0` | 25 | jr-ra | 0/0 | 2 | no | - | - | addiu:0x10x2 | real/real | no caller/ref |
| 0x72C44 | `func_80082444` | 25 | jr-ra | 0/2 | 3 | no | sw:$at | - | addiu:0x2x3 | real/real | address-retention |
| 0x76DA4 | `func_800865A4` | 25 | jr-ra | 0/0 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:0x2 | real/real | no caller/ref |
| 0x655C0 | `func_80074DC0` | 26 | jr-ra | 13/13 | 0 | no | lbu:destination,lw:destination,lw:destination | - | - | real/real | destination-as-temp |
| 0x5EAD0 | `func_8006E2D0` | 26 | jr-ra | 3/3 | 0 | no | lbu:$at | 8006E324->8006E2DC slot=addiu     $a2, $a2, 0x1 | addiu:0x1x2 | real/real | 3+ loop scratch regs; $v0-steal pattern |
| 0x4E608 | `func_8005DE08` | 26 | jr-ra | 2/2 | 0 | no | lw:destination | 8005DE60->8005DE4C slot=addu      $v0, $v1, $zero | - | real/real | destination-as-temp |
| 0x6EAA4 | `func_8007E2A4` | 26 | jr-ra | 2/2 | 3 | no | sw:$at,lw:destination | 8007E2E0->8007E2D4 slot=nop | lui:4x2 | real/real | destination-as-temp; address-retention; 3+ loop scratch regs |
| 0x80BA0 | `func_800903A0` | 26 | jr-ra | 2/2 | 1 | no | lw:destination,lw:destination,sw:$at | - | lui:0x2 | real/real | destination-as-temp |
| 0x743B8 | `func_80083BB8` | 26 | jr-ra | 1/1 | 0 | no | lw:destination | - | addiu:0x1x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0xB35A0 | `func_800C2DA0` | 26 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination,lw:destination | - | lui:248x2,lui:16x2 | real/real | destination-as-temp |
| 0x7EFC | `func_800176FC` | 26 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0x8964 | `func_80018164` | 26 | jr-ra | 0/0 | 2 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x9774 | `func_80018F74` | 26 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,sh:$at | - | - | real/real | no caller/ref; destination-as-temp |
| 0xA864 | `func_8001A064` | 26 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | 8001A0BC->8001A094 slot=nop | lui:16x2,ori:0xFFFFx2 | real/real | no caller/ref; destination-as-temp |
| 0x561F8 | `func_800659F8` | 26 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | 80065A48->80065A3C slot=addiu     $v1, $v1, 0x2 | lui:624x2 | real/real | no caller/ref; destination-as-temp |
| 0x5F484 | `func_8006EC84` | 26 | jr-ra | 0/0 | 1 | no | - | 8006ECC8->8006ECAC slot=sll       $v0, $s0, 16 | - | real/real | no caller/ref |
| 0x62E4C | `func_8007264C` | 26 | jr-ra | 0/0 | 0 | no | lw:destination | 80072694->80072684 slot=nop | - | real/real | no caller/ref; destination-as-temp |
| 0x6E8F8 | `func_8007E0F8` | 26 | jr-ra | 0/2 | 0 | no | lw:destination | 8007E14C->8007E134 slot=addu      $v0, $zero, $zero | addiu:-0x1x4 | real/real | destination-as-temp |
| 0x7155C | `func_80080D5C` | 26 | jr-ra | 0/0 | 2 | no | - | 80080DA0->80080D94 slot=addu      $a0, $s0, $zero | - | real/real | no caller/ref |
| 0x715C4 | `func_80080DC4` | 26 | jr-ra | 0/0 | 2 | no | - | 80080E08->80080DFC slot=addu      $a0, $s0, $zero | - | real/real | no caller/ref |
| 0x73308 | `func_80082B08` | 26 | jr-ra | 0/2 | 0 | no | lw:destination,lw:destination | - | - | real/real | destination-as-temp; $v0-steal pattern |
| 0x7B968 | `func_8008B168` | 26 | jr-ra | 0/0 | 1 | no | - | - | ori:0xFFFFx2,addiu:0x4x2 | real/real | no caller/ref |
| 0xBECF8 | `func_800CE4F8` | 26 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x114E4 | `func_80020CE4` | 27 | jr-ra | 1/1 | 1 | no | lw:destination,lw:destination,sw:$at,lw:destination | - | lui:009x2 | real/real | destination-as-temp |
| 0x55254 | `func_80064A54` | 27 | jr-ra | 1/1 | 0 | yes | - | - | - | real/real | $v0-steal pattern |
| 0x5747C | `func_80066C7C` | 27 | jr-ra | 1/1 | 0 | no | lhu:destination,lhu:destination,lhu:destination,sb:$at,sh:$at,sh:$at,sh:$at,sh:$at,sh:$at,sh:$at,sh:$at,sh:$at | - | lui:00x12 | real/real | destination-as-temp |
| 0x5DA4C | `func_8006D24C` | 27 | jr-ra | 1/1 | 1 | no | sb:$at,sb:$at,sb:$at,sb:$at,sb:$at,sb:$at,lw:destination,sw:$at | - | lui:00x8 | real/real | destination-as-temp |
| 0xBF53C | `func_800CED3C` | 27 | jr-ra | 1/1 | 1 | no | lw:destination,lw:destination,sh:$at | - | - | real/real | destination-as-temp |
| 0xD0860 | `func_800E0060` | 27 | jr-ra | 1/1 | 0 | no | lw:destination,lh:destination,sw:$at,lh:destination,sh:$at | 800E00B0->800E0088 slot=addiu     $a0, $a0, -0x14 | lui:1x3,addiu:-0x14x2 | real/real | destination-as-temp |
| 0xA378 | `func_80019B78` | 27 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | - | lui:009x2 | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0xAB90 | `func_8001A390` | 27 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x64C0C | `func_8007440C` | 27 | jr-ra | 0/2 | 0 | no | lw:destination,sw:$at | 8007445C->8007443C slot=addiu     $s0, $s0, 0x4 | lui:00956x2,addiu:0x1x2 | real/real | destination-as-temp |
| 0x6AB60 | `func_8007A360` | 27 | jr-ra | 0/0 | 3 | no | - | - | addiu:0x1x3 | real/real | no caller/ref; $v0-steal pattern |
| 0x7BDB8 | `func_8008B5B8` | 27 | jr-ra | 0/0 | 0 | no | lw:destination,sh:$at,sw:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x7C98C | `func_8008C18C` | 27 | jr-ra | 0/0 | 0 | no | lw:destination,sh:$at,sw:$at | - | - | real/real | no caller/ref; destination-as-temp |
| 0x7CA90 | `func_8008C290` | 27 | jr-ra | 0/0 | 0 | no | lw:destination,sh:$at,sw:$at | - | - | real/real | no caller/ref; destination-as-temp |
| 0x7FD9C | `func_8008F59C` | 27 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0x80878 | `func_80090078` | 27 | jr-ra | 0/0 | 0 | no | lw:$at | - | addiu:0x1x3 | real/real | no caller/ref; $v0-steal pattern |
| 0x813CC | `func_80090BCC` | 27 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0xD07B8 | `func_800DFFB8` | 27 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x719E4 | `func_800811E4` | 28 | jr-ra | 7/7 | 3 | no | - | - | addiu:-0x1x2 | real/real | address-retention |
| 0x4C9EC | `func_8005C1EC` | 28 | jr-ra | 4/4 | 1 | yes | - | - | lui:00x2,addiu:00x2 | real/real | $v0-steal pattern |
| 0x43E48 | `func_80053648` | 28 | jr-ra | 3/3 | 2 | no | - | - | - | real/real | address-retention |
| 0x65BB4 | `func_800753B4` | 28 | jr-ra | 2/2 | 0 | no | lbu:destination,lw:destination,lw:destination | - | - | real/real | destination-as-temp |
| 0x6D95C | `func_8007D15C` | 28 | jr-ra | 2/2 | 5 | no | lw:destination,sw:$at,sw:$at | - | lui:009x2 | real/real | destination-as-temp; address-retention; $v0-steal pattern |
| 0x6EA2C | `func_8007E22C` | 28 | jr-ra | 2/2 | 2 | no | sw:$at,sw:$at,sw:$at,lw:destination | 8007E278->8007E26C slot=nop | lui:4x4 | real/real | destination-as-temp; address-retention; 3+ loop scratch regs |
| 0xB7320 | `func_800C6B20` | 28 | jr-ra | 2/2 | 2 | no | lw:destination | - | - | real/real | destination-as-temp; address-retention |
| 0x57F30 | `func_80067730` | 28 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | destination-as-temp |
| 0x62DDC | `func_800725DC` | 28 | jr-ra | 1/1 | 0 | no | lw:destination,sw:$at | 8007262C->8007261C slot=nop | lui:0094538x2 | real/real | destination-as-temp |
| 0x6EC70 | `func_8007E470` | 28 | jr-ra | 1/1 | 2 | no | sw:$at,lw:destination | 8007E4BC->8007E4AC slot=addiu     $v0, $v0, 0x4 | lui:4x2,addiu:0x4x2 | real/real | destination-as-temp; address-retention; 3+ loop scratch regs |
| 0x6ED14 | `func_8007E514` | 28 | jr-ra | 1/1 | 3 | no | sw:$at,lw:destination | 8007E558->8007E548 slot=addiu     $v0, $v0, 0x4 | lui:4x2,addiu:0x4x2 | real/real | destination-as-temp; address-retention; 3+ loop scratch regs |
| 0x708F4 | `func_800800F4` | 28 | jr-ra | 1/1 | 2 | no | lw:destination | - | addiu:0x1x2 | real/real | destination-as-temp |
| 0x76CF8 | `func_800864F8` | 28 | jr-ra | 1/1 | 2 | no | sw:$at,sw:$at,sw:$at | - | lui:0x2,lui:4x2 | real/real | address-retention |
| 0x8188 | `func_80017988` | 28 | jr-ra | 0/0 | 0 | yes | lw:destination | 800179D4->800179B4 slot=nop; 800179E8->80017998 slot=nop | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0xA308 | `func_80019B08` | 28 | jr-ra | 0/0 | 0 | yes | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0x11474 | `func_80020C74` | 28 | jr-ra | 0/0 | 2 | yes | lw:destination,lw:destination,sw:$at,lw:destination | - | lui:009x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x30BC8 | `func_800403C8` | 28 | jr-ra | 0/0 | 4 | no | lw:destination,lw:destination,lw:destination,lw:destination,sw:$at,sw:$at,sw:$at | - | lui:00x4 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x30C38 | `func_80040438` | 28 | jr-ra | 0/0 | 4 | no | lw:destination,lw:destination,lw:destination,lw:destination,sw:$at,sw:$at,sw:$at | - | lui:00x4 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x58EA0 | `func_800686A0` | 28 | jr-ra | 0/0 | 0 | no | - | 800686FC->800686B8 slot=nop | - | real/real | no caller/ref; 3+ loop scratch regs |
| 0x61834 | `func_80071034` | 28 | jr-ra | 0/0 | 2 | no | lw:destination | - | addiu:0x15x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x7CB74 | `func_8008C374` | 28 | jr-ra | 0/0 | 2 | no | lw:destination,sw:$at,lw:destination,sw:$at | - | lui:009x4 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x80668 | `func_8008FE68` | 28 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x807E4 | `func_8008FFE4` | 28 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x80908 | `func_80090108` | 28 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0xB8554 | `func_800C7D54` | 28 | jr-ra | 0/1 | 3 | no | - | - | - | real/real | address-retention |
| 0xB9698 | `func_800C8E98` | 28 | jr-ra | 0/1 | 3 | no | - | - | lui:9x2,addiu:9x2 | real/real | address-retention |
| 0xBA390 | `func_800C9B90` | 28 | jr-ra | 0/1 | 3 | no | - | - | lui:00x2,addiu:00x2 | real/real | address-retention |
| 0xBAF28 | `func_800CA728` | 28 | jr-ra | 0/1 | 3 | no | - | - | lui:00x2,addiu:00x2 | real/real | address-retention |
| 0xBC734 | `func_800CBF34` | 28 | jr-ra | 0/1 | 3 | no | - | - | - | real/real | address-retention |
| 0xBD710 | `func_800CCF10` | 28 | jr-ra | 0/1 | 3 | no | - | - | lui:8x2,addiu:8x2 | real/real | address-retention |
| 0xBE0F0 | `func_800CD8F0` | 28 | jr-ra | 0/1 | 3 | no | - | - | lui:8x3,addiu:8x3 | real/real | address-retention |
| 0xBE96C | `func_800CE16C` | 28 | jr-ra | 0/1 | 3 | no | - | - | lui:00x3,addiu:00x3 | real/real | address-retention |
| 0x7F9B0 | `func_8008F1B0` | 29 | jr-ra | 5/5 | 0 | no | lw:destination,lw:destination,sw:$at,lw:destination,sw:$at,lw:destination,sw:$at,sw:$at | - | lui:0x3,lui:00x2,lui:4x4 | real/real | destination-as-temp |
| 0x710BC | `func_800808BC` | 29 | jr-ra | 2/2 | 1 | no | - | - | addiu:0xBx2 | real/real | $v0-steal pattern |
| 0x76184 | `func_80085984` | 29 | jr-ra | 1/1 | 5 | no | lw:destination,sw:$at,sw:$at,sw:$at,lw:destination,lw:destination | - | lui:009x2,lui:84x2 | real/real | destination-as-temp; address-retention |
| 0x7B694 | `func_8008AE94` | 29 | jr-ra | 1/1 | 2 | no | lhu:destination,lw:destination | - | - | real/real | destination-as-temp; address-retention |
| 0x97DC | `func_80018FDC` | 29 | jr-ra | 0/0 | 2 | yes | - | - | addiu:0x1x3 | real/real | no caller/ref |
| 0x30B54 | `func_80040354` | 29 | jr-ra | 0/0 | 0 | no | - | 800403A0->80040380 slot=andi      $v0, $v1, 0x8000; 800403B4->80040364 slot=nop | addiu:0x1x2 | real/real | no caller/ref |
| 0x7BE24 | `func_8008B624` | 29 | jr-ra | 0/0 | 0 | no | sh:$at,sw:$at,sw:$at | - | lui:009x2 | real/real | no caller/ref |
| 0x7C494 | `func_8008BC94` | 29 | jr-ra | 0/0 | 0 | no | lw:destination | 8008BCF8->8008BCB0 slot=sll       $a3, $a3, 1 | - | real/real | no caller/ref; destination-as-temp; 3+ loop scratch regs |
| 0x7C830 | `func_8008C030` | 29 | jr-ra | 0/0 | 0 | no | lw:destination | 8008C094->8008C04C slot=sll       $a3, $a3, 1 | - | real/real | no caller/ref; destination-as-temp; 3+ loop scratch regs |
| 0xC4764 | `func_800D3F64` | 29 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xD0620 | `func_800DFE20` | 29 | jr-ra | 0/0 | 0 | no | - | 800DFE84->800DFE60 slot=nop | - | real/real | no caller/ref |
| 0xBF5A8 | `func_800CEDA8` | 30 | jr-ra | 41/41 | 1 | no | lh:destination,lw:destination,lw:destination,sh:$at | - | lui:4x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0xBEE10 | `func_800CE610` | 30 | jr-ra | 35/35 | 0 | no | - | 800CE64C->800CE634 slot=addu      $a0, $a0, $a3 | addiu:0x1x2 | real/real | 3+ loop scratch regs |
| 0x4C974 | `func_8005C174` | 30 | jr-ra | 3/3 | 6 | yes | - | - | - | real/real | address-retention |
| 0x6E848 | `func_8007E048` | 30 | jr-ra | 2/2 | 4 | no | sw:$at,sw:$at | - | addiu:0x1x3,lui:4x3 | real/real | address-retention |
| 0x72A04 | `func_80082204` | 30 | jr-ra | 2/2 | 7 | no | sw:$at | - | - | real/real | address-retention |
| 0x43AB4 | `func_800532B4` | 30 | jr-ra | 1/7 | 1 | no | - | - | - | real/real | $v0-steal pattern |
| 0x80B28 | `func_80090328` | 30 | jr-ra | 1/1 | 1 | no | lw:destination,lw:destination,sw:$at | - | lui:4x2 | real/real | destination-as-temp |
| 0x8BE8 | `func_800183E8` | 30 | jr-ra | 0/0 | 0 | no | lw:destination | 8001843C->80018408 slot=nop; 8001844C->800183F4 slot=addiu     $a2, $a2, 0x4 | addiu:0x1x3 | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0x57360 | `func_80066B60` | 30 | jr-ra | 0/0 | 0 | no | lhu:destination,lhu:destination,lhu:destination,sh:$at,sh:$at,sh:$at,sb:$at,sb:$at,sh:$at,sh:$at,sh:$at,sh:$at,sh:$at | - | lui:00x13 | real/real | no caller/ref; destination-as-temp |
| 0x64AB8 | `func_800742B8` | 30 | jr-ra | 0/0 | 2 | no | lw:destination,lw:destination | - | - | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x7C100 | `func_8008B900` | 30 | jr-ra | 0/0 | 0 | no | lw:destination | 8008B968->8008B91C slot=sll       $a3, $a3, 1 | - | real/real | no caller/ref; destination-as-temp; 3+ loop scratch regs |
| 0x7C9F8 | `func_8008C1F8` | 30 | jr-ra | 0/0 | 0 | no | sw:$at,sh:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x7CAFC | `func_8008C2FC` | 30 | jr-ra | 0/0 | 0 | no | sw:$at,sh:$at,sw:$at | - | - | real/real | no caller/ref |
| 0x7FC70 | `func_8008F470` | 30 | jr-ra | 0/0 | 0 | no | lw:destination | - | addiu:0x3x2 | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0xC0694 | `func_800CFE94` | 30 | jr-ra | 0/0 | 1 | no | - | - | - | real/real | no caller/ref; $v0-steal pattern |
| 0x5EC98 | `func_8006E498` | 31 | jr-ra | 15/15 | 0 | no | - | 8006E4FC->8006E4D4 slot=addiu     $a2, $a2, 0xC | lui:16x2,ori:0xFFFFx2 | real/real | 3+ loop scratch regs |
| 0xB74E0 | `func_800C6CE0` | 31 | jr-ra | 14/14 | 0 | no | lw:destination | - | addiu:0x4x2 | real/real | destination-as-temp |
| 0x70344 | `func_8007FB44` | 31 | jr-ra | 3/3 | 1 | no | - | - | - | real/real | $v0-steal pattern |
| 0x334C4 | `func_80042CC4` | 31 | jr-ra | 2/2 | 0 | yes | - | 80042D1C->80042CF0 slot=nop | lui:878x2,addiu:878x2,addiu:0x1x2 | real/real | 3+ loop scratch regs; $v0-steal pattern |
| 0x569EC | `func_800661EC` | 31 | jr-ra | 2/2 | 0 | no | lw:destination,sh:$at,sh:$at,sh:$at,sh:$at,sw:$at | - | lui:8x2,lui:00x5 | real/real | destination-as-temp; $v0-steal pattern |
| 0x23C30 | `func_80033430` | 31 | jr-ra | 1/1 | 1 | yes | lw:destination,lhu:$at,sh:$at,lw:$at | - | lui:009x3,lui:8x2 | real/real | destination-as-temp |
| 0x33048 | `func_80042848` | 31 | jr-ra | 1/1 | 1 | no | lbu:$at,lw:destination,sw:$at | - | lui:860x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x4E58C | `func_8005DD8C` | 31 | jr-ra | 1/1 | 0 | no | lw:destination,lbu:$at,lw:destination | - | lui:028x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x5ED14 | `func_8006E514` | 31 | jr-ra | 1/1 | 0 | no | - | 8006E578->8006E550 slot=addiu     $a2, $a2, 0xC | lui:16x2,ori:0xFFFFx2 | real/real | 3+ loop scratch regs |
| 0x80AAC | `func_800902AC` | 31 | jr-ra | 1/1 | 1 | no | lw:destination,lw:destination,sw:$at | - | lui:4x2 | real/real | destination-as-temp |
| 0x8804 | `func_80018004` | 31 | jr-ra | 0/0 | 2 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x15D68 | `func_80025568` | 31 | jr-ra | 0/0 | 3 | no | lw:destination,lw:destination,sw:$at | - | - | real/real | no caller/ref; destination-as-temp; address-retention; $v0-steal pattern |
| 0x565CC | `func_80065DCC` | 31 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,lbu:destination,lhu:destination,lhu:destination,lhu:destination,lhu:destination | - | lui:624x2,lui:00x5 | real/real | no caller/ref; destination-as-temp |
| 0x5ED90 | `func_8006E590` | 31 | jr-ra | 0/0 | 0 | no | - | 8006E5F4->8006E5CC slot=addiu     $a2, $a2, 0xC | lui:16x2,ori:0xFFFFx2 | real/real | no caller/ref; 3+ loop scratch regs |
| 0x7CD5C | `func_8008C55C` | 31 | jr-ra | 0/0 | 3 | no | sw:$at,lw:destination,sw:$at,lw:destination,sw:$at | - | lui:009x5 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x7CDD8 | `func_8008C5D8` | 31 | jr-ra | 0/0 | 3 | no | sw:$at,lw:destination,sw:$at,lw:destination,sw:$at | - | lui:009x5 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x7CE54 | `func_8008C654` | 31 | jr-ra | 0/0 | 3 | no | sw:$at,lw:destination,sw:$at,lw:destination,sw:$at | - | lui:009x5 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x5EBD4 | `func_8006E3D4` | 32 | jr-ra | 4/4 | 0 | no | - | 8006E444->8006E3E0 slot=addiu     $a0, $a0, 0x1 | addiu:0x1x2 | real/real | 3+ loop scratch regs |
| 0x11550 | `func_80020D50` | 32 | jr-ra | 2/2 | 2 | yes | lw:destination,lw:destination,lw:destination,lw:destination | - | lui:78x2,lui:54x2 | real/real | destination-as-temp; address-retention |
| 0x45E68 | `func_80055668` | 32 | jr-ra | 2/2 | 2 | no | - | 8005569C->80055684 slot=addiu     $v1, $v1, 0x4 | addiu:0x1x2 | real/real | $v0-steal pattern |
| 0xB1F8 | `func_8001A9F8` | 32 | jr-ra | 1/1 | 2 | no | lw:destination,lw:destination,lw:destination | 8001AA5C->8001AA38 slot=nop | - | real/real | destination-as-temp; address-retention |
| 0x5C64C | `func_8006BE4C` | 32 | jr-ra | 1/1 | 0 | no | lbu:destination,lbu:destination,lbu:destination,sb:$at | - | lui:00x5,addiu:-0xAx2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x8099C | `func_8009019C` | 32 | jr-ra | 1/1 | 1 | no | lw:destination,lw:destination,sw:$at,lw:destination,sw:$at | - | lui:009x3,lui:00x2 | real/real | destination-as-temp |
| 0x8CEC | `func_800184EC` | 32 | jr-ra | 0/0 | 1 | yes | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0xA21C | `func_80019A1C` | 32 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,lw:destination,lw:destination | - | lui:009x4 | real/real | no caller/ref; destination-as-temp |
| 0x6877C | `func_80077F7C` | 32 | jr-ra | 0/0 | 1 | no | sw:$at,lw:destination | - | lui:00960x2,lui:16x2 | real/real | no caller/ref; destination-as-temp |
| 0x6AAA4 | `func_8007A2A4` | 32 | jr-ra | 0/0 | 6 | no | lw:destination,lw:destination,lw:destination | - | lui:009x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x76334 | `func_80085B34` | 32 | jr-ra | 0/0 | 0 | no | lw:destination | 80085BA0->80085B60 slot=addiu     $a1, $a1, 0x8 | lui:16x3,addiu:0x1x2 | real/real | no caller/ref; destination-as-temp; 3+ loop scratch regs |
| 0x80354 | `func_8008FB54` | 32 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x68804 | `func_80078004` | 33 | jr-ra | 6/6 | 0 | no | lh:destination | - | - | real/real | destination-as-temp |
| 0x32CB4 | `func_800424B4` | 33 | jr-ra | 4/4 | 0 | no | lbu:$at,lbu:$at | - | lui:00x3 | real/real | $v0-steal pattern |
| 0x3318C | `func_8004298C` | 33 | jr-ra | 3/3 | 1 | no | sw:$at | - | - | real/real | $v0-steal pattern |
| 0x42BF8 | `func_800523F8` | 33 | jr-ra | 2/2 | 2 | no | lhu:destination,lw:destination | - | lui:00x2 | real/real | destination-as-temp; address-retention |
| 0x72574 | `func_80081D74` | 33 | jr-ra | 2/2 | 2 | no | - | - | lui:0x2,addiu:0x2,addiu:0x1x2 | real/real | address-retention |
| 0x6E1F8 | `func_8007D9F8` | 33 | jr-ra | 1/1 | 4 | no | lw:destination,lhu:destination,lw:destination | - | - | real/real | destination-as-temp; address-retention |
| 0x7D284 | `func_8008CA84` | 33 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination,sw:$at | 8008CAE8->8008CAB0 slot=addiu     $s0, $s0, 0x24 | lui:009x4 | real/real | destination-as-temp |
| 0x8B64 | `func_80018364` | 33 | jr-ra | 0/0 | 0 | no | lw:destination | 800183C8->80018388 slot=nop; 800183D8->80018374 slot=addiu     $a2, $a2, 0x4 | addiu:0x1x3 | real/real | no caller/ref; destination-as-temp; 3+ loop scratch regs |
| 0x9550 | `func_80018D50` | 33 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,lw:destination | - | lui:009x3 | real/real | no caller/ref; destination-as-temp |
| 0x95D4 | `func_80018DD4` | 33 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,lw:destination | - | lui:009x3 | real/real | no caller/ref; destination-as-temp |
| 0xA7E0 | `func_80019FE0` | 33 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,lw:destination | 8001A030->8001A00C slot=nop | lui:16x2,lui:009x2,ori:0xFFFFx2,addiu:0x1x2 | real/real | no caller/ref; destination-as-temp; $v0-steal pattern |
| 0x35614 | `func_80044E14` | 33 | jr-ra | 0/12 | 5 | no | - | - | - | real/real | address-retention |
| 0x6CB04 | `func_8007C304` | 33 | jr-ra | 0/0 | 1 | no | sw:$at,sw:$at,sw:$at,sw:$at,sw:$at,sh:$at,sw:$at,sw:$at | - | lui:00x3 | real/real | no caller/ref |
| 0x6C164 | `func_8007B964` | 34 | jr-ra | 2/2 | 0 | no | lw:destination,lw:destination,lw:destination,lw:destination,lw:destination,lw:destination,lw:destination | - | lui:7x2,lui:84x2,lui:88x2 | real/real | destination-as-temp |
| 0xB090 | `func_8001A890` | 34 | jr-ra | 0/0 | 0 | yes | sh:$at,sh:$at | 8001A8B8->8001A898 slot=addu      $a0, $zero, $zero; 8001A8D8->8001A8CC slot=addiu     $v1, $v1, 0x4 | lui:009x3,addiu:0x4x2 | real/real | no caller/ref |
| 0x29154 | `func_80038954` | 34 | jr-ra | 0/0 | 2 | no | lw:$at | - | lui:0010x2,addiu:0010x2 | real/real | no caller/ref; address-retention |
| 0x40A80 | `func_80050280` | 34 | jr-ra | 0/2 | 6 | no | lh:$at | - | addiu:0x2x2,lui:00922x2 | real/real | address-retention |
| 0x4E6E4 | `func_8005DEE4` | 34 | jr-ra | 0/0 | 1 | yes | - | - | - | real/real | no caller/ref |
| 0x56544 | `func_80065D44` | 34 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination,lbu:destination,sh:$at,sh:$at,sh:$at,sh:$at | - | lui:624x2,lui:00x5 | real/real | no caller/ref; destination-as-temp |
| 0x617AC | `func_80070FAC` | 34 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x68C6C | `func_8007846C` | 34 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x69B84 | `func_80079384` | 34 | jr-ra | 0/0 | 0 | no | - | - | - | real/real | no caller/ref |
| 0x7B7B8 | `func_8008AFB8` | 34 | jr-ra | 0/0 | 3 | no | lw:destination,lw:destination | - | lui:009x2,addiu:0x68x2,addiu:0x1AA0x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x7CBE4 | `func_8008C3E4` | 34 | jr-ra | 0/0 | 2 | no | lw:destination,sw:$at,lw:destination,sw:$at | - | lui:009x4 | real/real | no caller/ref; destination-as-temp; address-retention; $v0-steal pattern |
| 0x7FD14 | `func_8008F514` | 34 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x80F54 | `func_80090754` | 34 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref |
| 0xB7390 | `func_800C6B90` | 34 | jr-ra | 0/0 | 0 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x41DF8 | `func_800515F8` | 35 | jr-ra | 6/6 | 0 | yes | lw:destination | 80051660->8005163C slot=nop | - | real/real | destination-as-temp; 3+ loop scratch regs |
| 0x71A68 | `func_80081268` | 35 | jr-ra | 5/5 | 5 | no | sw:$at | - | lui:009x2,addiu:0x1x3 | real/real | address-retention |
| 0x5AA5C | `func_8006A25C` | 35 | jr-ra | 3/3 | 7 | no | lw:destination,sw:$at,sw:$at | - | lui:16x2,lui:80x2 | real/real | destination-as-temp |
| 0x763B4 | `func_80085BB4` | 35 | jr-ra | 3/3 | 0 | no | lw:destination,lw:destination | 80085C2C->80085BEC slot=addiu     $a1, $a1, 0x8 | lui:16x3,addiu:0x1x2 | real/real | destination-as-temp; 3+ loop scratch regs |
| 0x6CA14 | `func_8007C214` | 35 | jr-ra | 1/3 | 0 | no | lw:destination,lw:destination,lw:destination,lw:destination,sw:$at,sw:$at,sw:$at | - | lui:00x4 | real/real | destination-as-temp |
| 0x8C60 | `func_80018460` | 35 | jr-ra | 0/0 | 0 | no | lw:destination | 800184CC->80018484 slot=nop; 800184DC->80018470 slot=addiu     $a2, $a2, 0x4 | addiu:0x1x3 | real/real | no caller/ref; destination-as-temp; 3+ loop scratch regs |
| 0x2E8FC | `func_8003E0FC` | 35 | jr-ra | 0/0 | 0 | no | - | 8003E170->8003E124 slot=addiu     $a3, $a3, 0xC | addiu:0x1x2,addiu:0xCx2 | real/real | no caller/ref; 3+ loop scratch regs |
| 0x68894 | `func_80078094` | 35 | jr-ra | 0/0 | 0 | no | lh:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x748DC | `func_800840DC` | 35 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0x7FE08 | `func_8008F608` | 35 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2,lui:16x2 | real/real | no caller/ref; $v0-steal pattern |
| 0xB93C0 | `func_800C8BC0` | 35 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBA174 | `func_800C9974` | 35 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBACB4 | `func_800CA4B4` | 35 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBC330 | `func_800CBB30` | 35 | jr-ra | 0/1 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0xBDDEC | `func_800CD5EC` | 35 | jr-ra | 0/1 | 0 | no | - | 800CD640->800CD5FC slot=andi      $v0, $a3, 0xFFFF | addiu:0x1x2 | real/real | 3+ loop scratch regs |
| 0x51CAC | `func_800614AC` | 36 | jr-ra | 8/8 | 0 | yes | - | - | lui:16x2 | real/real | $v0-steal pattern |
| 0x65744 | `func_80074F44` | 36 | jr-ra | 7/7 | 1 | no | lw:destination | - | - | real/real | destination-as-temp |
| 0x73CE8 | `func_800834E8` | 36 | jr-ra | 4/4 | 1 | no | lw:destination,lw:destination,lw:destination,lw:destination | 80083544->8008351C slot=nop | lui:88x3 | real/real | destination-as-temp; $v0-steal pattern |
| 0x7F040 | `func_8008E840` | 36 | jr-ra | 3/3 | 0 | no | - | - | - | real/real | $v0-steal pattern |
| 0x68530 | `func_80077D30` | 36 | jr-ra | 2/2 | 0 | no | lh:$at,lh:$at,lh:$at,lh:$at | - | lui:009589x3 | real/real | $v0-steal pattern |
| 0x493BC | `func_80058BBC` | 36 | jr-ra | 1/1 | 1 | yes | - | - | - | real/real | $v0-steal pattern |
| 0x59E60 | `func_80069660` | 36 | jr-ra | 1/1 | 4 | no | lw:destination,lw:destination | 80069694->80069684 slot=nop; 800696D0->800696C0 slot=nop | lui:009x2,addiu:0xBx2,addiu:0x1x2 | real/real | destination-as-temp; address-retention |
| 0x6FE9C | `func_8007F69C` | 36 | jr-ra | 1/1 | 3 | no | sw:$at,sw:$at,sw:$at | 8007F6E8->8007F6E0 slot=addiu     $v0, $v0, -0x1; 8007F704->8007F6D0 slot=addiu     $a0, $a0, 0x18 | addiu:0x3x2,addiu:-0x1x2,addiu:0x18x2 | real/real | address-retention |
| 0x80A1C | `func_8009021C` | 36 | jr-ra | 1/1 | 1 | no | lw:destination,lw:destination,sw:$at,lw:destination,sw:$at | - | lui:009x3,lui:00x2 | real/real | destination-as-temp |
| 0xBF4AC | `func_800CECAC` | 36 | jr-ra | 1/1 | 2 | no | - | 800CECE8->800CECC8 slot=addiu     $s1, $s1, 0x4; 800CED1C->800CECFC slot=addiu     $s1, $s1, 0x4 | addiu:0x100x2,addiu:0x1x2,addiu:0x4x2 | real/real | address-retention |
| 0x55A0 | `func_80014DA0` | 36 | jr-ra | 0/0 | 1 | no | - | 80014DE8->80014DBC slot=addiu     $a0, $a0, 0x8 | addiu:0x10x2,addiu:0x1x2,addiu:0x8x2 | real/real | no caller/ref |
| 0x90C4 | `func_800188C4` | 36 | jr-ra | 0/0 | 1 | no | lw:destination | 8001890C->800188E0 slot=addiu     $a0, $a0, 0x8 | addiu:0x10x2,addiu:0x1x2,addiu:0x8x2 | real/real | no caller/ref; destination-as-temp |
| 0x6E6C0 | `func_8007DEC0` | 36 | jr-ra | 0/0 | 7 | no | sw:$at | - | - | real/real | no caller/ref |
| 0x6E750 | `func_8007DF50` | 36 | jr-ra | 0/0 | 7 | no | sw:$at | - | - | real/real | no caller/ref |
| 0x7FEF4 | `func_8008F6F4` | 36 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x7FFBC | `func_8008F7BC` | 36 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2 | real/real | no caller/ref; $v0-steal pattern |
| 0x80FDC | `func_800907DC` | 36 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2,addiu:0x3x2 | real/real | no caller/ref |
| 0x1254C | `func_80021D4C` | 37 | jr-ra | 6/6 | 1 | yes | lhu:$at,lw:destination,sw:$at | 80021DA4->80021D64 slot=sll       $v0, $v1, 3 | addiu:-0x3x2,lui:009x2 | real/real | destination-as-temp |
| 0x41898 | `func_80051098` | 37 | jr-ra | 5/5 | 0 | yes | - | 80051100->800510DC slot=addiu     $a3, $a3, 0x10; 8005111C->800510D4 slot=addu      $a3, $t0, $zero | addiu:0x24x3,addiu:0x10x2 | real/real | 3+ loop scratch regs |
| 0x54578 | `func_80063D78` | 37 | jr-ra | 1/1 | 0 | no | - | - | addiu:-0x1x2 | real/real | $v0-steal pattern |
| 0x6EBDC | `func_8007E3DC` | 37 | jr-ra | 1/3 | 2 | no | sw:$at,lw:destination | 8007E448->8007E438 slot=addiu     $v0, $v0, 0x4 | lui:4x2,addiu:0x4x2 | real/real | destination-as-temp; address-retention; 3+ loop scratch regs |
| 0x6FE08 | `func_8007F608` | 37 | jr-ra | 1/1 | 2 | no | lw:destination,lw:destination | - | lui:530x2,addiu:0x1x2,addiu:0x4x2,lui:520x2,addiu:0x5x2 | real/real | destination-as-temp |
| 0xB350C | `func_800C2D0C` | 37 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination,lw:destination,lw:destination | - | lui:4x3,addiu:0x1x2 | real/real | destination-as-temp |
| 0x70F78 | `func_80080778` | 37 | jr-ra | 0/2 | 1 | no | lw:destination,lw:destination | - | lui:6x3 | real/real | destination-as-temp; $v0-steal pattern |
| 0xBE74C | `func_800CDF4C` | 37 | jr-ra | 0/1 | 0 | no | - | 800CDFB8->800CDF74 slot=andi      $v0, $a3, 0xFFFF | - | real/real | 3+ loop scratch regs |
| 0x65528 | `func_80074D28` | 38 | jr-ra | 10/10 | 1 | no | lw:destination,lw:destination | - | lui:16x2 | real/real | destination-as-temp |
| 0x66970 | `func_80076170` | 38 | jr-ra | 3/3 | 0 | no | lh:destination,lhu:destination,lh:destination,lhu:destination | - | lui:0095750x2,addiu:-0x1x4,lui:0095752x2 | real/real | destination-as-temp |
| 0x66A08 | `func_80076208` | 38 | jr-ra | 3/3 | 0 | no | lh:destination,lhu:destination,lh:destination,lhu:destination | - | lui:0095750x2,addiu:-0x1x4,lui:0095752x2 | real/real | destination-as-temp |
| 0x4EEF0 | `func_8005E6F0` | 38 | jr-ra | 2/2 | 1 | yes | lw:destination,lw:$at,lw:$at | - | lui:1x2 | real/real | destination-as-temp |
| 0x643BC | `func_80073BBC` | 38 | jr-ra | 2/2 | 3 | no | lw:destination,lw:destination | 80073C3C->80073BE4 slot=nop | lui:00956x2,addiu:-0x1x2 | real/real | destination-as-temp; address-retention |
| 0x70194 | `func_8007F994` | 38 | jr-ra | 1/1 | 5 | no | sw:$at,lw:destination,sw:$at,sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:6x4,lui:009x3 | real/real | destination-as-temp; address-retention |
| 0x74504 | `func_80083D04` | 38 | jr-ra | 1/1 | 0 | no | lw:destination | - | addiu:0x1x2,lui:0083x2,addiu:0083x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x80C08 | `func_80090408` | 38 | jr-ra | 1/1 | 1 | no | lw:destination,lw:destination,sw:$at | - | lui:0x2 | real/real | destination-as-temp |
| 0x7B5C | `func_8001735C` | 38 | jr-ra | 0/0 | 2 | no | lw:destination | - | addiu:0x1x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x98BC | `func_800190BC` | 38 | jr-ra | 0/0 | 2 | no | lw:destination,lw:destination,lw:destination,lhu:destination,sh:$at | - | lui:009x2,ori:0x2x2,lui:8x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x657D4 | `func_80074FD4` | 38 | jr-ra | 0/0 | 1 | no | lw:destination | - | - | real/real | no caller/ref; destination-as-temp |
| 0xB3898 | `func_800C3098` | 39 | jr-ra | 29/29 | 2 | no | sb:$at,sb:$at,lbu:destination,lbu:destination,lhu:destination,lhu:destination,sh:$at | - | lui:3x3 | real/real | destination-as-temp; address-retention |
| 0x437CC | `func_80052FCC` | 39 | jr-ra | 1/1 | 0 | no | - | 80053048->80053008 slot=nop | addiu:0x20x2 | real/real | 3+ loop scratch regs |
| 0x562D4 | `func_80065AD4` | 39 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | lui:624x2 | real/real | destination-as-temp |
| 0x5EE0C | `func_8006E60C` | 39 | jr-ra | 1/1 | 3 | no | lbu:destination,sb:$at | - | lui:00x3,lui:16x2 | real/real | destination-as-temp; address-retention; $v0-steal pattern |
| 0x76014 | `func_80085814` | 39 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | - | lui:009x2 | real/real | destination-as-temp |
| 0x5EB38 | `func_8006E338` | 39 | jr-ra | 0/0 | 0 | no | lb:$at | 8006E378->8006E368 slot=sll       $v0, $a1, 2; 8006E3BC->8006E34C slot=addu      $t0, $t0, $t3 | addiu:0x1x4,addiu:0x4x2 | real/real | no caller/ref; 3+ loop scratch regs |
| 0xB6EC8 | `func_800C66C8` | 39 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0xB6F64 | `func_800C6764` | 39 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0xB7000 | `func_800C6800` | 39 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0x757E4 | `func_80084FE4` | 40 | jr-ra | 4/4 | 0 | no | lw:destination,lw:destination,lw:destination,lw:destination,lw:destination | - | lui:16x4,ori:0xFFFFx3,lui:6x3,lui:2x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x55DD4 | `func_800655D4` | 40 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination | 8006565C->80065614 slot=addiu     $a0, $a0, 0x10 | lui:624x2,addiu:0x1x2 | real/real | destination-as-temp; 3+ loop scratch regs |
| 0x67D48 | `func_80077548` | 40 | jr-ra | 1/1 | 0 | no | lw:destination,lw:destination,lw:destination,lw:destination,lw:destination | - | lui:16x4,lui:0095854x3,ori:0xFFFFx4,lui:0095850x2,addiu:0x2x2 | real/real | destination-as-temp; $v0-steal pattern |
| 0x75F44 | `func_80085744` | 40 | jr-ra | 1/1 | 8 | no | lw:destination,lw:destination,lw:destination | 80085780->80085778 slot=lui       $a0, (0xF2000002 >> 16); 800857A4->80085794 slot=nop; 800857BC->800857AC slot=lui       $a0, (0xFFFFFF >> 16) | lui:16x3,ori:0xFFFFx3,lui:009x2 | real/real | destination-as-temp; address-retention |
| 0x83B4 | `func_80017BB4` | 40 | jr-ra | 0/0 | 2 | no | lw:destination,sw:$at,sw:$at | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0xA44C | `func_80019C4C` | 40 | jr-ra | 0/0 | 1 | no | lw:destination,lw:destination | - | lui:009x2 | real/real | no caller/ref; destination-as-temp |
| 0x307CC | `func_8003FFCC` | 40 | jr-ra | 0/0 | 0 | no | - | 80040044->80040018 slot=nop; 8004005C->8003FFF4 slot=nop | addiu:0x418x3 | real/real | no caller/ref; 3+ loop scratch regs |
| 0x41520 | `func_80050D20` | 40 | jr-ra | 0/2 | 5 | yes | lw:$at,lw:$at | - | addiu:0x1x2,lui:8x2 | real/real | address-retention |
| 0x41670 | `func_80050E70` | 40 | jr-ra | 0/2 | 8 | no | lw:$at | - | addiu:0x2x2,addiu:0x1x2 | real/real | address-retention |
| 0x4332C | `func_80052B2C` | 40 | jr-ra | 0/0 | 0 | yes | lbu:$at,lbu:$at,lbu:$at | 80052B74->80052B54 slot=addiu     $a1, $a1, 0x1; 80052BA8->80052B88 slot=addiu     $a1, $a1, 0x1 | addiu:0x1x5,lui:0x3 | real/real | no caller/ref |
| 0x5FA24 | `func_8006F224` | 40 | jr-ra | 0/0 | 0 | no | lw:destination,lw:destination | 8006F26C->8006F250 slot=addiu     $a0, $a0, 0x10C; 8006F2A0->8006F284 slot=nop; 8006F2B0->8006F294 slot=addiu     $a0, $a0, 0xA0C | addiu:-0x1x2,lui:00942x2,addiu:0x1x2 | real/real | no caller/ref; destination-as-temp |
| 0x64A18 | `func_80074218` | 40 | jr-ra | 0/0 | 2 | no | lw:destination,lw:destination,lw:destination,lw:destination | - | lui:0095678x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x7976C | `func_80088F6C` | 40 | jr-ra | 0/0 | 2 | no | - | - | - | real/real | no caller/ref |
| 0x7D1DC | `func_8008C9DC` | 40 | jr-ra | 0/0 | 3 | no | lw:destination,lw:destination,sw:$at,sw:$at,lw:destination,sw:$at | 8008CA1C->8008C9FC slot=addiu     $a0, $a0, 0x11C | lui:0x4,lui:009x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0x7EA3C | `func_8008E23C` | 40 | jr-ra | 0/2 | 3 | no | lw:destination,lw:destination,lw:destination,sw:$at,sw:$at,sw:$at,sw:$at,sw:$at | - | lui:16x2,ori:0xFFFFx2,lui:009x8 | real/real | destination-as-temp |
| 0x8106C | `func_8009086C` | 40 | jr-ra | 0/0 | 0 | no | - | - | addiu:0x1x2,addiu:0x3x2 | real/real | no caller/ref |
| 0xBFE58 | `func_800CF658` | 40 | jr-ra | 0/0 | 3 | no | - | - | - | real/real | no caller/ref |
| 0xC070C | `func_800CFF0C` | 40 | jr-ra | 0/0 | 3 | no | lhu:destination,sh:$at,lhu:destination | - | lui:1x3,addiu:0x100x2 | real/real | no caller/ref; destination-as-temp; address-retention |
| 0xC07AC | `func_800CFFAC` | 40 | jr-ra | 0/0 | 3 | no | lhu:destination,sh:$at,lhu:destination | - | lui:1x3,addiu:0x100x2 | real/real | no caller/ref; destination-as-temp; address-retention |

## Post-campaign SKIP overlay: handwritten libGTE/COP2 family

The following three rows were removed from TIER 1 after the campaign and are
now included in the `SKIP` total as `SKIP-SDK-LIBRARY-COP2`:

| file off | function | words | disposition |
|---:|---|---:|---|
| `0x697AC` | `func_80078FAC` | 3 | `SKIP-SDK-LIBRARY-COP2` |
| `0x697B8` | `func_80078FB8` | 3 | `SKIP-SDK-LIBRARY-COP2` |
| `0x69824` | `func_80079024` | 3 | `SKIP-SDK-LIBRARY-COP2` |

The complete contiguous family (`func_80078E04` through `func_80079024`, 23
handwritten COP2/register helpers) is screened in
`COP2_SDK_SCREEN.md`. Exact matching remains optional SDK work; these entries
must not consume matching-C campaign attempts.

## Post-campaign SKIP overlay: address-retention family

The static family screen in `docs/evidence/volume-campaign-20260825/ADDRESS_RETENTION_SCREEN.md`
proves the exact scalar-global exchange shape and records the indexed getter
variant. These spans are removed from matching-C scheduling without duplicate
phrasing attempts:

| file off | function | evidence | disposition |
|---:|---|---|---|
| `0x72CB4` | `func_800824B4` | exact five-word scalar exchange; no exact-start caller/reference | `SKIP-ADDRESS-RETENTION-FAMILY` |
| `0x72CC8` | `func_800824C8` | exact five-word scalar exchange; 12 callers; two bounded C attempts already parked | `PARKED-ADDRESS-RETENTION` |
| `0x72CDC` | `func_800824DC` | exact five-word scalar exchange twin; screened with 824C8 | `PARKED-ADDRESS-RETENTION-FAMILY` |
| `0x7265C` | `func_80081E5C` | exact five-word scalar exchange; two callers; screened with 824C8 | `PARKED-ADDRESS-RETENTION-FAMILY` |
| `0x703F0` | `func_8007FBF0` | indexed getter; 11 callers; `$v0` address-temp residual after two retries | `PARKED-ASSEMBLER-TEMP` |

The 824C8/824DC/81E5C/703F0 historical evidence remains authoritative; this
overlay only prevents repeated scheduling.

## Post-campaign SKIP overlay: address-register coloring

`func_8005DBF8` is removed from Tier 1 after its two bounded attempts. Its
retail body keeps the symbolic address in `$v0`, loads through `$v1`, and then
reuses `$v0` for the offset; the two natural expressions did not reproduce
that allocation. It is `PARKED-ADDRESS-REGISTER-COLORING` and must not consume
another campaign attempt without a new compiler/tooling hypothesis.

## Post-refresh SKIP overlay: handwritten syscall wrappers

The adjacent 4-word wrappers `func_80072714` and `func_80072724` are removed
from TIER 1 after the first refreshed-campaign screen. Both contain a literal
MIPS `syscall 0` instruction between an immediate `$a0` setup and the canonical
`jr ra; nop`; ordinary C has no sanctioned spelling for that architectural
side effect, and inline assembly is forbidden by R7. They are classified
`SKIP-SDK-LIBRARY-SYSCALL` and must not consume matching-C attempts. Evidence:
`docs/evidence/volume-campaign-20260825/func-80072714/PARK.md`.

| file off | function | words | disposition |
|---:|---|---:|---|
| `0x62F14` | `func_80072714` | 4 | `SKIP-SDK-LIBRARY-SYSCALL` |
| `0x62F24` | `func_80072724` | 4 | `SKIP-SDK-LIBRARY-SYSCALL` |

## 2026-08-25 campaign match overlay

The first eligible post-refresh Tier 1 candidate, `func_80062A20` at
`0x53220`, matched 5/5 words under era `-O2 -G0` and was integrated as leaf
288. Its evidence is `docs/evidence/volume-campaign-20260825/func-80062a20/REPORT.md`.

The next screened candidate, `func_800824C8` at `0x72CC8`, is removed from
TIER 1 as `PARKED-ADDRESS-RETENTION` after two allowed phrasings. Retail keeps
the scalar-global address in `$v1` through the load and store-in-`jr` delay
slot; ordinary era C did not. Evidence:
`docs/evidence/volume-campaign-20260825/func-800824c8/PARK.md`.

`func_80078120` at `0x68920` is removed as `SKIP-SDK-LIBRARY-GTE-TAIL`:
its three loads branch into the adjacent wrapper’s handwritten COP2 path.
Evidence: `docs/evidence/volume-campaign-20260825/func-80078120/SKIP.md`.

The next candidate, `func_80085084` at `0x75884`, matched 5/5 words under
era `-O2 -G0` and was integrated as leaf 289. Evidence:
`docs/evidence/volume-campaign-20260825/func-80085084/REPORT.md`.
The adjacent `func_800824DC` is the same five-word exchange over
`D_800B8AB8` and is screened into the same family without a second attempt.

`func_80081E5C` at `0x7265C` is removed as the same
`PARKED-ADDRESS-RETENTION-FAMILY` as 824C8/824DC; no duplicate attempt was
spent. Evidence:
`docs/evidence/volume-campaign-20260825/func-80081e5c/SKIP.md`.
