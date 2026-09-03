# PE-FD1 — func_8006E9A0 adapter removed (real fade loop)

Authority is the 141-word matched C leaf `src/func_8006E9A0.c`
(VRAM 0x8006E9A0 / file 0x5F1A0 / size 0x234, era `-O2 -G0`,
byte-exact), transliterated to guest-address native style in
`pc_port/bootstrap/func_8006E9A0_port.c`. This is the first adapter
removed rather than catalogued: the HOST_ADAPTED single-pass stand-in
(one `ClearOTagR(NULL)`, no arena, faked poll) is gone.

Sections, ROM order: display init (unchanged providers), the 19-store
pointer arena republished exactly as retail does (same cursor chain as
the 6A8D4 port), `5E588` + `66B60(2)`, the real
`do { ClearOTagR(lookup[CDDC], 0x1000); 68E24(); 70E54(); }
while ((D_800BCFEE & 3) != 1)`, post-loop `B0DC6 = 0` + `38D1C`, and
the arg-1 `0xA80830C8` / arg-3 `0xA80651C8` dispatch.

Termination proof (no iteration cap exists in retail, none added):
`66B60` arms `CFEE=2` / `CFF6=arg` / `CFF8=0` unconditionally;
`68E24` advances `CFF8` to `CFF6` then publishes `CFEE=1`. Test
fixtures (`66B60(2)`) terminate in exactly 2 iterations.

`70E54` remains a stub boundary inside the loop (logs, returns
off-strict; strict stops there honestly — the exposed next rung on the
New Game path, no longer masked by the adapter). Strict production is
unaffected today: its frontier stops inside `801909B4`, before any
`6E9A0` call.

NOT a matching leaf (no `src/` change; same standard as OTC1/EV1).

## Verify

```text
PE_TEST_FILTER="6E9A0_calls_66B60" ./pc_port/build/pe-native-tests
PE_TEST_FILTER="FD1_" ./pc_port/build/pe-native-tests
```

`BTL41_6E9A0` migrated to the retail contract (`CFEE 2->1`, two ticks).
New `FD1_6E9A0_real_loop_fills_arena_ot` proves the loop's OT is the
arena entry (`0x801229A0`, self-referential at `CDDC=0`) with the OTC1
terminator tail on it — never the adapter NULL. Full normal suite:
1019 run / 1002 passed / 1 pre-existing environmental failure
(`B54KY` missing `local/pe_disc1.path`, identical on the base tree) /
16 skipped.
