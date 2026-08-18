# PE-BTL83 — Retail dynamic battle-transition capture

Research/capture lane. No production gameplay was patched
to force combat. Authority is original Disc 1 + retail EXE
running in PCSX-Redux. The native port supplied watch
addresses only.

This directory name is the user-requested evidence pack.
It is not the camera rung `PE-BTL83 — 70E54 CDDC flip`.

## Identity

| Item | Value |
|---|---|
| Disc 1 BIN | `Parasite Eve (USA) (Disc 1).bin` |
| Image size | 495,531,120 |
| Image SHA-1 | `c339455d5b1dae04f77c2ee847d0932adaf2e84b` |
| `SYSTEM.CNF` | `BOOT=cdrom:\SLUS_006.62;1` |
| `SLUS_006.62` SHA-1 | `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` |
| `SLUS_006.62` SHA-256 | `5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b` |
| In-RAM 64 KiB at `0x80010000` SHA-1 | `668f4a900da0f5d581a35b533f177cb521706edf` (matches file `[0x800:0x10800]`) |
| Known words | `1D340=27BDFCD8` `293F4=8F850508` `72534=3C02800A` `2CEE0=0C01A453` `33A2C=24020001` `144FC=27BDFFE0` |
| BIOS | SCPH-1001 / DTL-H1001 Version 2.0 05/07/95 A |
| BIOS SHA-1 | `649895efd79d14790eabb362e94eb0622093dfb9` |
| PCSX-Redux | AppImage `PCSX-Redux-HEAD-x86_64.AppImage` |
| BUILD_ID | `ff4033ac` |
| AppImage SHA-256 | `cf99f1e9eb287bb56ed0908b7b5ccd153905ad7a64f1ff7f429de20841a6bc1f` |
| Source | `https://distrib.app/pub/org/pcsx-redux/project/dev-linux-x64/latest` |
| Flags | `-interpreter -debugger -gdb -gdb-port 3334 -softgpu -fastboot` |

Retail identity matches the project EXE. Capture proceeded.

## Watch map (live guest)

| Signal | Address | Width |
|---|---|---|
| `4D4` | `D_8009D244` `0x8009D244` | byte (watch 4) |
| mode | `D_8009D28C` `0x8009D28C` | word |
| dest token | `D_8009D280` `0x8009D280` | word |
| Aya HP record ptr | `D_8009D278` `0x8009D278` | word |
| scratch[0] | `D_800B6A80` `0x800B6A80` | word; bit 2 = value 4 |
| script cursor | `gp+0x90` `0x8009CE00` | word |
| `D2E8` | `0x8009D2E8` | word |
| `1D340` | `0x8001D340` | exec |

## Retail route used

A retail memcard save was loaded through the game's Continue
menu. That is normal game interaction, not a RAM poke.

| Field | Value |
|---|---|
| Card | raw 128 KiB `MC` |
| Product | `BASLUS-006620000000A` |
| SJIS title | `PE-01 / 00:11:17 / DAY1 Theater` |
| Load menu | Aya Lv1 HP **40/45** (`0x28`/`0x2D`) |
| Card SHA-1 | `7e2055b15173054a99ba0a6942ae16e5ccf3662e` |

DuckStation `.sav` states exist on this machine and were
**not** loaded. They are a different emulator format.

## Observed dest tokens

Token formula for the low Carnegie names:
`(name << 7) + 0x48` under `0xA8000000`.
`M0004I=0xA8000248`, `M0005I=0xA80002C8`,
`M0064I=0xA8002048`.

| Token | Meaning | Writer PC | RA |
|---|---|---|---|
| `0xA9400048` | boot/title special (`func_8001220C`) | `0x80012290` then later `0x8003FCE0` | `0x8001228C` / `0x80042350` |
| `0xA8002048` | Day-1 Theater field (`M0064I`) | `0x8003FCE4` | `0x80042350` |
| `0xA80651C8` | Tutorial menu (`func_8006E9A0`) | (missed width/alias on one run) | — |

`0xA8002048` is **not** `M0005I`. The save lands in a
Carnegie backstage dressing room (Aya in the evening gown,
scripted burnt corpse, no battle HUD). Field `gp+0x90`
was `0x801A1E98` after the load.

## Field snapshot after Theater load (run1)

Taken while dest stayed `0xA8002048` and the 3D room was
visible. No combat HUD.

| Signal | Value |
|---|---|
| mode `D28C` | `0` |
| `4D4` | `0` |
| scratch[0] | `0` (bit 2 clear) |
| `D278` | `0` (Aya HP record pointer not published) |
| HP watch | not armed (`D278==0`) |
| `1D340` | never entered |
| `0x55` | never entered |
| `293F4` | never entered after boot |

Boot/title writes of `4D4`/`mode`/`scratch0` were **zeros**
(BSS / `func_80034F10` kind-4 clear at `0x80034F4C`,
`0x800126E0`, EXE entry `0x80072544`, BIOS `0xBFC065FC`).
Those are not the combat `0 → nonzero` events.

## Completed capture (local redux outdir)

`/var/home/blizz/Applications/pcsx-redux/captures/pe-btl83`
after Theater load and a live M0036I (`0xA8001248`) encounter.

| First | PC / RA | Note |
|---|---|---|
| dest combat | heartbeat dest `0xA8001248` | token `(36<<7)+0x48` |
| mode 7 | `2CF24` inside `2CEE0` | `6914C(0)==0` and `s1=1` |
| `4D4=1` | `33A34` `ra=19D34` | opcode `0xCF` jal `33A2C` |
| `1D340` | `ra=2A504` | `a0=1` hp=40 |
| HP sub | `1F704` `ra=1F5F0` | 40→39 then 39→34 |

`192BC` is `0x95` `sw $zero` (mode 0), not mode 7.

Native rung PE-BTL97 recovers `6914C(0)` v0=0, `2A7F8`
mode-6 → `2BC90`/`2CEE0`/`2CF24`, and opcode `0xCF`.
PE-BTL98 recovers opcode `0x95` mode 0, `299CC` jal
`1D340` @ `2A4FC`, and `1F704` 40→39. PE-BTL99
recovers `1F814` / the 39→34 loop / `1F080` death.
Do not poke `4D4`, mode 7, `1D340`, or HP.

## Harness

- `capture.lua` — write/exec watchpoints, CSV sinks, optional pad
- `launch.sh` — Disc 1 + SCPH-1001 + portable `portable-pe-btl83`
- `PLAYBOOK.md` — human drive after Theater load

## SUCCESS OUTPUT

```
retail_first_4D4_nonzero_writer=0x80033A34 ra=0x80019D34
retail_mode7_writer=0x8002CF24 via 2CEE0
retail_first_hp_subtractive_writer=0x8001F704 ra=0x8001F5F0 40→39
retail_damage_entry_call_chain=2A4FC jal 1D340 ra=2A504
native_first_divergence=recovered 1F814 + 39→34 loop + 1F080 death
next_native_rung=encounter complete / teardown / field return
```

GOAL_STATUS=CONTINUE

