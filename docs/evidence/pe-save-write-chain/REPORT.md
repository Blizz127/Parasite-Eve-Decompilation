# Save-write chain: func_80042020/42170/5C25C/40B80 ported; route frontier moves into the card-write state machine

Branch `agent/save-write`, worktree `/tmp/pe-agent-savewrite`, base `d7d6e35b`
("docs: menu input cleared; save interaction runs to the Saving dialog").

## What the wall was

The Day-1 `--route-pad` autopilot reached the save-menu confirm and then the
`[STUB:BOOTSTRAP_RET] func_80042020` decomp boundary — the save-write handler.
`func_80042020`, `func_80042170`, `func_8005C25C` and `func_80040B80` were
unported nonmatching functions, so no save was ever assembled or written.

## Implemented (hand-translated from retail bytes, not matching leaves)

Authority: retail Disc1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

| function | span | file | what |
| --- | --- | --- | --- |
| `func_80042020(card,slot)` | 0x150 / 84w | `307CC.s` | save-write entry: formats the file name via `func_80071A84` (host `PE_FormatterFrame`) into `D_8009EE70`, assembles the block with `func_80040B80`, arms the record state |
| `func_80042170(card,slot)` | 0x0B8 / 46w | `307CC.s` | matching load entry |
| `func_8005C25C()` | 0x118 / 70w | `4C974.s` | copies six status words (0x20 stride from `D_800A1E6E`), gathers play-time/rank/name fields |
| `func_80040B80(record)` | 0x400 / 256w | `307CC.s` | builds the 0x2000-byte card block in `D_8009EED0` (0x100 header + 0x12E0 body + `func_8003F800` fields) and stores the CRC-16/CCITT (`0x1021`) as `~crc` |
| `func_8003F800()` | 0x3D8 / 246w | `2F174.s` | appends the record's extra fields to the block cursor |
| `func_8004006C(dest,fmt)` | 0x1A4 / 105w | `307CC.s` | Shift-JIS fullwidth `%d`/`%D`/`%s` formatter |
| `func_80040210(idx,time)` | 0x144 / 81w | `307CC.s` | H/M/S split of the play time and format into `D_8009EE8C` |
| `func_8005DE08(a0)` | 0x68 | `307CC.s` | name-table NUL skip |
| `func_80043474(a0)` | 0x4C | `33A4C.s` | play-time rank 1..6 |
| `func_8005D940()` | 0x30 | `4C974.s` | current-room name index (matching leaf; host impl returns `func_8006E454`'s value, which every retail caller consumes from `$v0`) |
| `func_8005DE70()` | leaf | — | `D_800A8044 + 0x800A8028` base accessor |
| `func_80071A14(dst,src)` | 0xC / 3w | `621E4.s` | BIOS A(19h) `strcpy` trampoline (psx-spx) |

Files: `pc_port/game/boot/func_80042020_port.c`,
`func_80040B80_port.c`, `func_8003F800_port.c`, `func_8005C25C_port.c`,
`func_80040210_port.c`; `pc_port/platform/func_80071A14_port.c`.

The retail unaligned `lwl`/`lwr` copies are plain byte copies and are expressed
as memcpy loops. `PE_FormatterFrame` needs a caller stack; the port has no guest
stack, so `GA_FMT_SP = 0x801FF600` is a dedicated scratch in the free
0x801FF040..0x801FFE00 band it carves `caller_sp-0x250..caller_sp+0x14` from.

## Generator fix (required for a correct call)

`tools/analysis/gen_decomp_ports.py` had a blanket textual rewrite that turned
**any** bare-first-argument data symbol into its address. That is only correct
when the callee parameter is a guest address; it produced
`func_80042020((pe_addr_t)0x8009CF44u, ...)` instead of
`func_80042020(D_8009CF44, ...)`. Removed the blanket rewrite (the
prototype/boundary-aware `rewrite_data_args` already covers the real cases).
21 generated TUs changed, all comment-only except the intended scalar-arg
fixes; `gen_decomp_ports.py --check --allow-orphans` is OK.

## Port robustness

`func_8004FE58` (slot-row predicate) assumed a non-null `func_800424B4` lookup
and faulted `PE_LoadU8(0)` once the save invalidated the selected slot while the
menu kept drawing. Added a guarded "missing entry -> row disabled" return,
logged once, because the card-write state machine (`func_80041108` states 3..11)
is not ported yet. Not a stub widening: `func_80041108` still stops at its own
named boundary.

## Verified

```text
./pc_port/build/pe-native-tests            -> 1384 run / 1384 passed / 0 failed / 0 skipped
ctest                                      -> 11/11 passed
card oracles (status/operation/operation_frame/driver) -> PASS (4096/8192/1280/96)
python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans -> check: OK
bash scripts/build_us.sh                   -> EXACT SHA-1 452fb033… (679 registered C leaves)
```

The matching build is unaffected (only `pc_port/` and the port-only generator
changed).

Live route, exact before/after (real Disc 1, present `.mcr`, headless,
`--route-pad --max-frames 42000`):

```text
BEFORE (d7d6e35b): [ROUTE] frame=38500 story=0x48
  [STUB:BOOTSTRAP_RET] func_80042020 (first invocation)
  [HOST] stop_reason=unresolved-boundary

AFTER:              [ROUTE] frame=38500 story=0x48
  [STUB:BOOTSTRAP_RET] card operation unresolved call   (func_80041108 format_name, target 0x80071A84)
  [HOST] stop_reason=unresolved-boundary
  PORT EXIT 0 (no abort)
```

The `func_80042020` boundary is gone: the save handler runs (formats the name,
assembles the 0x2000-byte block, arms the record) and the frontier moves into
`func_80041108` — the card-write state machine's `format_name` formatter call.
Story is still `0x48`; the write is not yet completed.

## Remaining frontier (precise, not faked)

`func_80041108` states 3..11 (and the live `format_name` path) still stop at the
formatter boundary. Completing the save needs those states wired to the ported
libcard file API (`func_80072734` open / `func_80072754` read / `func_80072774`
close / `func_80071A84` formatter), with the menu held/frozen while the slot is
invalidated. `func_80042020`/`func_80042170` and the block assembly are real.

## Matching leaves

None of the four target functions was added as a matching `src/` leaf; each is a
large hand-translation (84/46/70/256 words with the SD card ABI) and none was
proven byte-exact. No matching claim is made.
