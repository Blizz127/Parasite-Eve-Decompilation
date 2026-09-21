# Field-VM opcode coverage on the boot → Day-2 transition path

This note narrows the open-ended statement "many opcodes ported; unported table
slots are explicit boundaries" (`docs/ai_context/BOOT_TO_DAY2_COVERAGE.md` §5)
to a **concrete, reproducible list of the opcodes the transition-critical
scripts actually use**, and marks exactly which of those the native VM
(`pc_port/game/boot/func_80017018_port.c`) implements.

New tool: `pc_port/tools/pe_field_vm_opcode_coverage.py`. Output:
`local/live/field-vm-opcode-coverage.json`.

## 1. Why the script source is not the whole story

`func_80017018` dispatches `op = word & 0x1FFF` through the table
`D_800910A0[op]`. The transition-critical rooms split into two kinds:

- **Fixed package slots** (the default type, as decoded by
  `pe_pst0_scan.extract_script_from_package`): the script bytes are in the
  `PE.IMG` package on the disc. Every room in §2 is of this kind.
- **Type-0 world-map slots**: their script bytes come from **player data**
  (the save file's 32 KB world-map arena published at `persist[0]`/`D_800A77F0
  + 0x50`), so there is no type-0 bytecode on the retail disc to census. The
  all-`0x20`-yield "scripts" the older `pe_btl147` scan shows for
  `m0004i`/`m0378i`/`m0005i` are that degenerate player-data form, not the
  authored scripts. This note therefore does not claim opcode coverage for
  player-authored type-0 content.

## 2. Census (fixed package slots)

| Room | modules | commands | distinct ops | unported ops |
| --- | ---: | ---: | ---: | --- |
| `m0004i` (field spawn) | 5 | 288 | 43 | — |
| `m0377i` | 6 | 117 | 37 | — |
| `m0378i` | 5 | 129 | 39 | — |
| `m0036i` (Day-1 marker) | 4 | 377 | 37 | — |
| `m0351i` (Day-1/Day-2 hop) | 2 | 385 | 28 | `E9` |
| `m0091i` (Day-2 terminal) | 5 | 913 | 49 | `9A` |
| `m0191i` (park, not on the M0042I route) | 10 | 1634 | 62 | `71` |
| `m0037i` (park, not on the M0042I route) | 5 | 722 | 38 | — |
| `m0374i` (park, on the M0042I route) | 6 | 750 | 52 | `97` |

Handler VAs are read from the SHA-1-exact EXE at
`D_800910A0 + (op & 0x1FFF)*4`; "ported" means the port's dispatch chain
compares against that exact VA.

## 3. The four unported opcodes (status as of this session)

When this report was first written all four were unported. **All four are now
ported** in `pc_port/` with native regression tests; the last one, `0xE9`, was
closed by opening the field-message subsystem
(`docs/evidence/field-message-subsystem/REPORT.md`).

| Opcode | Handler | Port status | Port artifact / test |
| --- | --- | --- | --- |
| `0x97` | `0x800192DC` | **ported** | `func_800192DC` in `game/boot/func_80017018_port.c`; `test_SEW19_opcode_97_effect_destroy` |
| `0x9A` | `0x800193D8` | **ported** | `func_800193D8` + `func_80065AD4` (`game/boot/func_80065AD4_port.c`); `test_SEW20_opcode_9A_slot_fill` |
| `0x71` | `0x80018A9C` | **ported** (park lane) | `func_80018A9C` + `func_800671C8` (`game/boot/func_800671C8_port.c`); `test_SEW21_opcode_71_pan_clamp` |
| `0xE9` | `0x80015C7C` | **ported** | `func_80015C7C` + `func_8005D2B4` (`game/boot/field_message_port.c`); `test_SEW23_opcode_E9_message_query` |

### `0xE9` — handler `0x80015C7C` (5 uses, all in `m0351i` module 1) — PORTED

Words `8F820590 27BDFFE8 AFBF0014 AFB00010 94420008 … 30420020 1440000D …`
The function starts `lw v0,0x590(gp)` and tests `lhu v0,8(v0) & 0x20`, i.e. an
owner/handle state gate. It then loads four `args` `lw`s and calls
`0x8005D2B4` (`0x0C0174AD` = `jal 0x8005D2B4`). Uses at `0x8018F5D0`,
`0x8018F5EC`, `0x8018F608`, `0x8018F624`, `0x80191038`.

`func_8005D2B4` is now ported for its five route-reachable arms
(`cmd` 0x453/0x454/0x456/0x457 query leaves and 0x45E → `func_8004BCE8`); its
other 16 arms are loud domain guards returning retail's default-miss `0`.
`func_8004BCE8` and its closure were found to be already ported, so the
Day-1/Day-2 hop room no longer hits a boundary. Full write-up:
`docs/evidence/field-message-subsystem/REPORT.md`.

### `0x9A` — handler `0x800193D8` (2 uses, `m0091i` module 3) — PORTED

`8C820000 8C830004 8C860008 8C440000 8C650000 8CC60000 0C0196B5` — three
pointer args dereferenced and passed to `jal 0x80065AD4`. Uses at
`0x801C4F70`, `0x801C546C`, both `args=[2,5,0]`, in the Day-2 terminal room.
Ported as `func_800193D8` (`game/boot/func_80017018_port.c`) plus
`func_80065AD4` (`game/boot/func_80065AD4_port.c`). The `slot+4` update is a
retail `lbu` — bits 8..15 are dropped; the test asserts that.

### `0x97` — handler `0x800192DC` (1 use, `m0374i` module 2) — PORTED

`8C820000 3C05800A 8CA5D2F0 8C440000 0C01BF06 00003021` — `arg0` dereferenced
into `D_8009D2F0`-relative state, then `jal 0x8006FC18` (the effect-destroy
leaf, already ported as `func_8006FC18`). Use at `0x801C5D00`, `args=[14]`.

### `0x71` — handler `0x80018A9C` (1 use, `m0191i` module 1) — PORTED

`3C03800B 8C631624 … 01034021 000510C0 00451023 000210C0 … 0C019C72` —
index/stride arithmetic over `0x800B1624` then `jal 0x800671C8`. Use at
`0x801B53F8`, `args=[2,0,0,128]`. `m0191i` is **not** on the `M0042I` route
(`docs/ai_context/DAY1_DAY2_TRANSITIONS.md` §6), so this opcode is a park-lane
item, not a boot → Day-2 seam item. Ported as `func_80018A9C` plus
`func_800671C8` (`game/boot/func_800671C8_port.c`).

None of the four handlers has an `src/*.c` matched leaf; they are all pure
`asm` in the current snapshot. (The native port is not a matching leaf and is
not counted as matching C.) The offline oracle now reports
`unported_opcodes = []`: every critical room reports `unported=[]` when re-run
with the ported VAs in the dispatch chain.

## 4. Disposition

- **All nine critical rooms now use zero unported opcodes** — the field VM
  reaches the Day-1 completion marker (`M0036I`'s `persist[74]=0x80`), the
  `m0377i` bounce, the Day-1/Day-2 hop (`M0351I`, including `0xE9`), the Day-2
  terminal room (`M0091I`) and the park scripts without hitting a boundary.
- **`0xE9` is ported.** Its dependency closure (`func_8004BCE8` and friends)
  was already ported; `func_8005D2B4` now has its five route-reachable arms
  implemented and the rest as loud guards. See
  `docs/evidence/field-message-subsystem/REPORT.md`.
- This is a *static* census. It proves which opcodes the decoded bytecode
  names; it does not prove that a live frame reaches every one of them, nor
  that an unported `func_8005D2B4` arm is never reached on another script.

## 5. Reproduce

```
python3 pc_port/tools/pe_field_vm_opcode_coverage.py
```

Requires the Disc 1 image (`local/pe_disc1.path`) and
`build/disc1.candidate.exe` (sha1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`).
