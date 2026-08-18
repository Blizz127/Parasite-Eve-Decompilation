# PE-BTL53 — 35558 → 299CC idle + 69594 pump; rec=4 producer

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
69594       50w  0x80069594..0x8006965C  sha b182bcd8…ac47
6F8EC       65w  0x8006F8EC..0x8006F9F0  sha 659a9a3e…6563
D4704       83w  0x800D4704..0x800D4850  sha 4090a71a…62ef
299CC jal   35558 @ 0x800355E8
69594 jal   35558 @ 0x80035B2C
```

Matching `src/` C was not added. Do not invent rec=4, command 7,
scratch[0]&4, pad, persist==39, or a type-3 hit.

## Live 35558 path (D1A0 bit 1 set by 0x3A)

```text
walk D20C
if D1A0&2: jal 299CC          # 355E8
… mid-body not this cut …
jal 6C5BC                     # 35B24, not this cut
jal 69594                     # 35B2C
```

Host walk_cut now issues the 299CC consume + after-consume idle
gate when `D1A0&2`, then 69594. 5C498 / 2A7F8 mode switch /
2A470 / 1D340 are not this cut.

## 299CC after consume (0x80029A0C..)

Inverse at 29A20: flag clear && edge==6 → edge=0, mode=6.
Then `D278 = *D254`, `D230 = 0`. `5C498` not this cut.
`mode!=0` or `gp+0x4D4==0` → 2A7F8. Live `293F4(0)` clears
4D4 at 29488, so the mode-0 body does **not** reach 2A470.

## Rec-byte-4 producer (not reached live)

`2FAF8` waits until `*(slot+0x18)` byte 0 == 4, or
`slot+code*16+0x1C` byte 0 == 4. Writers of that byte:

| Site | Function | Gate |
|---|---|---|
| 1F5A4 | 1F4D4 inside 1D340 | sole jal 2A4FC; needs 4D4!=0 |
| 27DE0 / 27E5C | 27D14 | 2A53C (same 4D4 path), 2A994 (mode 3), 2CEC8 (exit) |
| 2FCC4 / 2FD08 | 2FAF8 FC54 | not live `+0x0E==2` |
| D427C | D413C (`table[0x55]+0x10`) | zero jal; jalr +0x10 not on 6F8EC |

`293F4(1)` at 2AE60 (2AA98 JT `gp+0x104==7`) is the only `sb 1, 4D4`.
2AA98 is jal'd from the mode==3 arm. Mode 3 is stored at 1F41C
inside 1D340. That is a 4D4-gated cycle. `33A2C` also writes
4D4=1 but its only jal (19D20) has zero callers.

Do not force 4D4. 0x64 wait is authentic while 4D4==0.

## 69594 / 6F8EC / D4704

69594: if `D1A0&0x80`, loop `6F8EC(0..10)`. 661A4/661CC and
6F9F0 are not this cut. 6F8EC jalrs `table[remap]+0xC`. Live
0x75 → D4704. D4704 writes F32D0/E2368; eight halfwords at
`slot+0x2C` are 0xFFFF after D4620, so the GTE/jalr body is
skipped. This array is **not** the 2FAF8 clip rec at
`slot+0x1C+i*16`. 69594 does not write rec=4.

## Other waits (unchanged)

Type-6 `scratch[0]&4`: EXE has only 126CC/34F10 zeros and
17018 kind-4 decode at 6A80. No type-0..6 script dest-kind-4
index 0 store. Writer still RESEARCH_REQUIRED.

Type-2 fork `+0x0E==7`: 1A680(a1=7) sites in 24A3C target
`*D254` (type 0), not type 2. Do not invent command 7.

Type-3 `0x85`: still hit-gated. Type-0 X=16 misses the rects.

## Verify

```text
python3 pc_port/tools/pe_btl53_69594_oracle.py
python3 pc_port/tools/pe_ch1_299cc_oracle.py
PE_TEST_FILTER=BTL53 ./pc_port/build/pe-native-tests
```
