# PE-OTC1 — ClearOTagR translated (guest-side ordering-table clear)

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C (both bodies stay `nonmatching` asm; this rung is a
native translation, not a decomp leaf).

`func_800752AC` — 43 words `0x800752AC..0x80075358`,
SHA-256 `23468418…16e1dc`. ClearOTagR:
debug name `ClearOTagR(%08x,%d)...\n` at `D_80011910`, level-`<2` skip,
`jtb[11]` dispatch with `(ot, n)`, terminator tail
`D_8009580C = 0x004957F8`, `*ot = 0x0009580C`.

`func_80076354` — 56 words `0x80076354..0x80076434`,
SHA-256 `b7a681b4…0dcacf`. The jtb[11] worker programs DMA channel 6
(OTC); its static pointers prove it
(`D_80095864/68/6C/70` = `1F8010E0/E4/E8/F0` = D6 MADR/BCR/CHCR + DPCR).
`MADR = ot + n*4 - 4` (LAST entry), `BCR = n`, `CHCR = 0x11000002`,
DPCR `|= 0x08000000`, `jal 773D0`, busy-bit wait over `77404`'s ordinary
path. Sole D6 programmer in the executable; no other accessor exists.

Hardware contract (PSX-SPX DMA ch.6: each entry points to the previous;
MADR = last entry, BCR = count, CHCR = `11000002h`; `0xFFFFFF` end
marker): the channel writes terminators into guest RAM — no GPU
rasterization, list walk, or callback. Native performs the fill
synchronously with that spec-derived pattern
(`OT[i] = OT[i-1] & 0xFFFFFF`, `OT[0] = tail link`) and completes during
the first wait poll (`polls == 1` deterministically; retail's exact count
is bus-timing-dependent and diagnostic-only). DPCR goes through the
shared `PE_GPU` authority.

NOT a matching leaf. This rung adds no code under `src/` and claims no
matching progress: a native translation is behavior-verified (focused
tests + the retail-word oracle above), never byte-exact machine code.
The byte-exact gate for any future `src/func_800752AC.c` /
`src/func_80076354.c` leaf is unchanged: era `-O2 -G0` compile plus the
docker-gated `scripts/build_us.sh` EXACT SHA-1 rebuild and
`scripts/verify_us.sh`, neither runnable in this session (no mipsel
toolchain on host; `cc1`/`cpp` die under the sandbox seccomp; docker
denied). The oracle's window hashes plus its 15 decoded immediates are
the complete machine-checked retail ledger that leaf attempt must
reproduce.

Null-OT guard: retail always passes a real arena OT (`lookup[D_8009CDDC]`;
matched leaf `src/func_8006E9A0.c`), and the de-adapted 6E9A0 loop does
the same since Phase 6E-FD1. A null/unrepresentable OT performs the safe
non-memory effects and logs `func_800752AC_null_ot_skip` with NO stop.
Dirty `jtb[11]` and level-`>= 2` print stay honest boundaries.

## Verify

```text
python3 pc_port/tools/pe_otc1_clearotagr_oracle.py
PE_TEST_FILTER=OTC1 ./pc_port/build/pe-native-tests
```

Full normal suite: 1015 run / 998 passed / 1 pre-existing
environmental failure (`B54KY` missing `local/pe_disc1.path`, identical
on the base tree) / 16 skipped. ASan/UBSan (leak check off — LSAN is
fatal under this sandbox's ptrace): same 998/1/16, zero diagnostics.
