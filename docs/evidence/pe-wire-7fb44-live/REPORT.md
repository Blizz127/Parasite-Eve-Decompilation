# PE-WIRE — 7FB44 wired into the live 7FCFC chain; new frontier at 7B9EC

Authority: retail Disc 1 executable `SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; disassembly
`asm/disc1/7018C.s:144` (`func_8007FB44`, 31 words),
`asm/disc1/704BC.s:34` (`func_8007FCFC`), `asm/disc1/6B130.s`
(`func_8007B9EC`, 53 words).

Status: the only product-metric item this session. One provider
registration removed a boundary; the 493 translated words did the
rest. No new transcription.

## The wiring

`func_8007FB44`'s passing arm now calls the real
`func_8007FCFC(cmd & 0xFF, data)` and returns its value directly
(retail has no `sltu` in 7FB44 — the `sltu $v0,$zero,$v0` sits in
7E8F4, whose port already propagates `!= 0`). The 0x1F/lane-2
latches and the 0xB store in the `jal` delay slot are unchanged.

## The WIRE finding (why the frontier is 7B9EC, not 7C564)

The first live run died at
`FATAL: PE_StoreU8: invalid guest address 0x1F801800 size 1`.
Read-back from the SHA-1-exact EXE shows why — the image
pre-initializes the pointer tables to CD hardware registers:

| table | initializer |
|---|---|
| `[B27C]` | `0x1F801800` |
| `[B280]` | `0x1F801801` |
| `[B284]` | `0x1F801802` |
| `[B288]` | `0x1F801803` |
| `[B28C]` | `0x1F801020` |

`func_8007B9EC`'s very first effect (B9F8 `sb 1 @ [[B27C]]`) pokes
a CD register, and the BA14 spin loop rewrites the tags it tests
until a CD interrupt releases it. Only the BA70 tail touches RAM
(`[B296]/[B295]/[B294] = 0/0/2`), but it runs last, so no prefix
is transcribable without reordering effects. The B558
transcription is therefore reverted to a named stop at 7B9EC
entry; no caller consumes its `0x1325` return (7FCFC ignores $v0;
both timeout arms overwrite $v0 = -1 in their jump delay slots),
so unwinding void loses nothing observable. The next rung is the
CD-register handshake disposition — not the 583-word 7C564
machine.

## New frontier

Strict, real Disc 1, exit 1:
`FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
func_8007B9EC / called from: func_8007FCFC/8007B010/8007B558`.
(The caller label lists all three static call sites.)

Non-strict production continues past the stop (harness semantic)
and aborts at the first hardware touch:
`FATAL: PE_StoreU8: invalid guest address 0x1F801800`
(B558 B6A0 through the real table). Loud and exact — no silent
corruption, no guards invented, zero `HOST_ADAPTED` lines.
Screenshot still written: 320x240, non-black 3912 of 76800 —
identical to the pre-wire baseline. CD commands don't draw, so
the pixel metric doesn't move; the frontier moves from a stub to
a hardware handshake instead.

## FDC0 answer (live arms)

All three 7FCFC arms (7/8/other) now run through the live B558
in tests with returns asserted: the 8-arm guarded-vs-firing
distinction (`[B58B] == 1` keeps `[B558] = 8`, else relatches 1)
is pinned end-to-end with `[B580]` latches; the CDS1 live-head
test additionally pins the 80950 data copy (`[B559] =
0x44332211`) through the live chain.

## Test migrations (intent intact, nothing relaxed)

- `B558_PlantChain()` helper (scratch pointers — a harness
  accommodation, since the EXE points them at hardware —
  `[B294] = 2` for the fast B010 arm, `[AFC0] = 0`,
  73DE8-source 0). Used by all 5 synthetic dispatch sites.
- 7 boundary assertions move `func_8007FCFC == 1` → the 7B9EC
  entry stop (B54KAD/AE, CDQ1 queue + 81314, CDS1 live-head +
  negative-flag). Return asserts move with the live values
  (7E8F4/7FB44 now return 1 on dispatch).
- CDS1 live-head + negative-flag data args move to mapped
  scratch (80950 copies 4 bytes live; wild pointers would fault
  before the chain).
- B54KY (real disc, state words stay real): pointers only. The
  801918F8 arg4 assert is scoped by caller (`func_801918F8`
  made no indirect calls itself) instead of log-empty, since the
  live chain legitimately prints. 7B9EC count is pinned == 2:
  the FD14 entry plus the B010-timeout re-entry (73C5C in the
  trail proves the timeout arm fired on real `[B294] == 0`).
- `B558_7b9ec_transcribe` becomes `B558_7b9ec_boundary`
  (entry stop, zero effects). The three wrapper tests plant
  `[B294] = 2` and cross the boundary keeping every latch
  assert.

## Verify

```
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
```

Both with `PE_DISC1_BIN` (gateless first: B54KY env case only).
Normal CTest with disc: `100% tests passed, 0 tests failed out
of 2`. Fresh ASan/UBSan CTest with disc: `100% tests passed, 0
tests failed out of 2`, zero sanitizer diagnostics. B558 oracle
still green (retail bytes unchanged). Leaf count 560; no
src/YAML changes.
