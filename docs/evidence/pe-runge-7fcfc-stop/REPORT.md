# PE-RUNGE — named-stop analysis: func_8007FCFC (CD completion dispatch)

Authority: retail Disc 1 executable `SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; disassembly
`asm/disc1/704BC.s:34` (`glabel func_8007FCFC`, 74 words,
`0x8007FCFC..0x8007FE24`).

## Caller chain (all translated, all real)

`func_8007F0C8` CdlReadS queue issue (CDQ1 + CDS1 tail) calls
`func_8007E8F4` (CDS1 predicate), which calls `func_8007FB44`
(CDS1 predicate), which would call `func_8007FCFC`. The port
records `func_8007FCFC` from `func_8007FB44` +
`PE_PORT_STOP_UNRESOLVED_BOUNDARY` (`pc_port/platform/pe_libcd.c`).
Stub site: `grep -rn "func_8007FCFC" pc_port --include=*.c`
hits only the CDS1 boundary in `func_8007FB44` and the migrated
test expectations.

## Why it is not translated in this run

74 words, control-heavy, three jal targets:

| jal | callee | port status |
|---|---|---|
| `func_8007B9EC` | CD hardware access | collapsed, not translated |
| `func_80080950` | 4-byte copy-or-clear | real |
| `func_8007B558` | controller dispatch (`asm/disc1/6B130.s:883`, ~200 words, 10 jal incl. `7B9EC`, `73A44`, `71A74`, `7B010`, `73C5C`, `73DE8`, `7AAB4`) | not translated |

Multi-branch drive-state machine over `D_8009B5xx` bytes; returns
0/1 into 7FB44's `sltu`. Translating 7FCFC alone only pushes the
stop into 7B558 (or the collapsed 7B9EC) with no field-ward
progress: the 7B558 controller and then the 583-word 7C564
delivery state machine are each their own rung. Per the rung rule
(not leaf-shaped, cannot be finished and tested in the box), the
run stops honestly here. No scheduler/m0360i/persist state
touched; nothing faked.

## Production evidence (this session, real Disc 1 image)

Strict:
`FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
func_8007FCFC / called from: func_8007FB44`.
Non-strict headless boot:
`[STUB:BOOTSTRAP_RET] func_8007FCFC (first invocation)`,
`[FB] vsyncs=486 drawsyncs=1445 presents=483 mask=0`,
`[HOST] stop_reason=unresolved-boundary`, stub summary
`implemented: 0, host-adapted: 0, bootstrap-return: 1,
unsupported: 0`, screenshot 3912 non-black of 76800 (bottom
32 rows, dim grays: direct-fb fills, not the display path — all
483 presents blanked with mask off). Zero `HOST_ADAPTED` lines.
The stop lands in boot media streaming (486 vsyncs in), before
any New-Game menu: field frames remain beyond the frontier.

## What unblocks field frames

1. `func_8007B558` controller rung (needs 7B9EC hardware
   semantics + `7B010`/`73C5C`/`73DE8`/`7AAB4` disposition).
2. `func_8007C564` 583-word delivery rung behind the completion
   callback.
3. Then the boot→New-Game→M0431I path re-evaluated against the new
   frontier.
