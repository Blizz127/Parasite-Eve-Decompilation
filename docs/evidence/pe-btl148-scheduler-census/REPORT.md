# PE-BTL148 — destination-writer and package-entry census

## Status

`EVIDENCE_BLOCKED=NEEDS_RETAIL_CAPTURE_OR_SAVE_ARTIFACT`

This rung does not implement a scheduler. The available Disc 1 executable
proves the ordinary field-script path and the executable destination-state
writers, but does not identify how a first-time retail execution of `m0360i`
is scheduled. No native scene-specific workaround was added.

## Proven destination state

The field loader consumes `D_8009D280` at `func_8003F074` →
`func_8006B4F8` (`0x8003F088`), where `func_8006E2D0` decodes the packed
token and `func_8006E454` derives the package index. The normal field VM
opcode `0x31` handler is `func_80017BB4`; it stores the immediate argument
through `D_8009D280` and returns to the task VM.

The all-script parser audit found 1,011 field-script `0x31` commands. Every
one uses argument mode `(0,)`, i.e. an immediate token. No script `0x31`
argument is actor-, persist-, table-, or callback-derived.

## Executable writer census

| Writer | PCs | Input / condition | Result |
|---|---|---|---|
| `func_8001220C` | `0x80012290`, `0x8001239C` | initial state and ordinary boot/field branch | `0xA9400048`, `0xA80830C8` |
| `func_80015790` | `0x80015834`, `0x80015908` | handler-table random/event selection; `D_800917E4` / `D_8009180C` tables | computed name token |
| `func_80015964` | `0x800159FC`, `0x80015A70` | second handler-table random/event selection; same tables | computed name token |
| `func_80017BB4` | `0x80017BF8` | VM opcode `0x31`, immediate script argument | loaded token |
| `func_8003EB04` | `0x8003ED9C` | system bit transition: counter reaches 9 and mask matches | `0xAA108448` |
| `func_8006A25C` | `0x8006A2C8` | special/death path | `0xA9400048` |
| `func_8006E9A0` | `0x8006EBB0` | selector values 1 or 3 | `0xA80830C8` or `0xA80651C8` |
| `func_8003FBD8` | `0x8003FCD0` | save restore | saved `D_8009D280` word |

The save writer `func_8003F800` copies `D_8009D280` into the save buffer at
offset `0x804`; `func_8003FBD8` restores that word. This is a persistence
path, not proof of the event that originally selected the saved token.

No other `lui/addiu` address formation or direct store targeting
`D_8009D280` occurs in `asm/disc1`. There is no literal `0xA8066048` store.

## Computed chooser result

The only executable name-packing writers are `func_80015790` and
`func_80015964`, both installed in the `D_800910A0` handler table. Their
tables are:

* `D_800917E4`: five copies of `m0295i`;
* `D_8009180C`: `m0291i` through `m0311i`, then `m0383i` through `m0424i`
  in the documented table order.

`m0360i` is absent from both tables. The packed name helper
`func_8006E3D4` is generic, but its only executable callers that store the
result into `D_8009D280` are these two handlers. Thus the chooser can derive
tokens, but the inspected Disc 1 table data cannot select `m0360i`.

## Package-entry audit

`func_8006B4F8` receives the current `D_8009D280` token, decodes it to a
seven-byte package name, indexes `D_80093378`, and loads the three PE.IMG
chunks. The package metadata/publish path begins only after this token has
already been selected. No callback, actor table, or module-local data path
was found that bypasses `D_8009D280` for package entry.

Therefore the available evidence establishes:

```text
known executable destination writers
  -> ordinary immediate 0x31, fixed computed tables, system/death/menu,
     or save restoration
known package loader
  -> D_8009D280 -> token decode -> PE.IMG package
```

It does not establish:

```text
retail event precondition -> scheduler decision -> first m0360i load
```

## Required artifact

A retail PCSX/PCSX-Redux trace or memory-card save that reaches the Day 2+
`m0360i` event is required. The watch must log, at minimum, writes to
`D_8009D280`, package-load entry, `persist[0]`, `persist[0x4A]`, and the
event/task state immediately before the `m0360i` load. A Disc 2 executable or
overlay, if the event scheduler is not in the inspected Disc 1 executable,
would also resolve the missing writer census.

Until that artifact exists, implementing a generic bridge would require
inventing its input table or selecting `m0360i` by name/address, both of
which are explicitly forbidden by the BTL148 contract.

```text
PRODUCTION_REACHABILITY=blocked_at_func_80030894_L2L3_cut
SEMANTIC_IMPLEMENTATION=not_started_without_scheduler_provenance
NATIVE_SUITE=unchanged (928/928 baseline)
```
