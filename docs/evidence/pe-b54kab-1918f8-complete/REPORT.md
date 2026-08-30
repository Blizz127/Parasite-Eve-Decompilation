# PE-B54K-AB — complete `func_801918F8`

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

Retail `func_801918F8` is the overlay display-pair initializer at
`[0x801918F8,0x80191B64)`: 155 words, SHA-256
`86c8721a16712363ff166438380acd4ca4b2f732b7c8aecb917a08298b050109`.
It ends in `jr ra; nop`, has four exact-start callers (`0x80191F74`,
`0x8019256C`, `0x8019257C`, `0x80192990`), and is bounded by real
instructions. Its only callees are the authenticated generic
`SetDefDispEnv` and `SetDefDrawEnv` providers, twice each.

The low byte of argument 0 selects one of the 20-byte DISP_ENV and 92-byte
DRAW_ENV pairs and chooses the 0/240 page. A zero low byte in argument 1 uses
320x240 and publishes mode 2. A nonzero low byte initializes 480x240, folds
both width fields to 320 with retail's `(width*2)/3`, sets `isrgb24`, and
publishes mode 3. Both paths set the six draw flags at offsets `0x16..0x1B`.

Both branches are exercised in the retained B54K-Y contracts. The caller now
executes both calls and advances from the 29-word to the 35-word prefix:
`[0x801924F8,0x80192584)`, SHA-256
`9f6476d633f517cd6e17fee8a76167180a9f87d320ecf0e62ef4e4f3b45114b1`.

```text
normal full suite: 985 run, 985 passed, 0 failed, 0 skipped
fresh ASan/UBSan:  985 run, 985 passed, 0 failed, 0 skipped
strict frontier:   func_801924F8_80192584_cut from func_801924F8
normal telemetry:  vsyncs=486 drawsyncs=1445 presents=483
DMA telemetry:     calls=27 queries=26 services=26 captured=26 serviced=26
EXE SHA-1:         452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

```text
FUNC_801918F8=COMPLETE_155_WORDS
FUNC_801924F8=AUTHENTICATED_271_WORDS_PREFIX_35_WORDS
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192584_cut
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
NEXT_ARTIFACT_FREE_RUNG=continue_func_801924F8_at_80192584
```
