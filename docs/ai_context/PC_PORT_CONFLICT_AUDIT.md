# pc_port conflict audit — matched C vs hand-written ports

A *conflict* is a matched `c` leaf (byte-exact against retail, authority) whose
symbol is already defined by a hand-written `pc_port/` translation unit. The
hand port may be behaviourally equivalent, may be a legitimate host adaptation,
or may be a real bug. This document records the audit of those conflicts and
the divergences found.

## Tool

`tools/analysis/conflict_audit.py` (read-only) compares semantic signatures of
the matched leaf in `src/` against the hand port:

* return type
* guest data addresses touched (`D_XXXXXXXX` names and raw `0x8xxxxxxx` literals)
* numeric constants / bit masks
* callee set

It never writes to `src/`, `pc_port/`, or the manifest.

### Normalisation layers (why the first cut was noisy)

The raw comparison produced many false positives. Three normalisation passes
were added before a difference is reported:

1. **Macro expansion** — hand ports name constants through file-local
   `#define`s (`GA_D_8009D13C`, `TASK_STRIDE`), so object-like macros are
   expanded textually before scanning.
2. **Address-offset folding** — hand ports compose addresses as
   `base + offset` (`0x8009CD70u + 0x3CCu`) where the leaf names the symbol
   directly. Adjacent `0x8xxxxxxx ± 0xNN` pairs are folded into one literal, so
   base+offset composition no longer looks like a missing/extra global.
3. **Numeric literal normalisation** — all decimal and hex literals are
   parsed to integers, so `4095` and `0xFFF` compare equal, and the
   `u`/`U`/`l`/`L` suffixes are ignored.

After normalisation the tool reports candidates, not verdicts. A candidate is
then classified by hand as **AGREE**, **DIVERGE**, or **UNTESTABLE**.

### Candidate classification helper

`--json` output plus a small follow-up script classifies each candidate by
whether its src-only addresses/constants/callees are absent from the *entire*
hand file (not just the matched function body). A symbol reached through a
shared helper shows up file-wide, so it is not a divergence; a symbol absent
file-wide is a strong signal.

## Result

```
conflict audit: 296 compared, 178 signature-clean, 118 candidates
  on route: 202, candidates: 71
```

Of the 118 candidates, 49 are pure return-type *alias* differences
(`int` vs `int32_t`, `uint16_t` vs `unsigned short`) and are behavioural
no-ops. The remaining candidates were inspected individually; the sharpest
(non-alias, on-route, cross-referenced against the caller) are classified
below.

## DIVERGE

### `func_8006F39C` — missing overlay-handler prelude (fan-in 1, 206 words)

**Difference.** The matched leaf (`src/func_8006F39C.c`) runs a CD/XA prelude
for command ids `0x6C..0x72` when `D_800B0CD8` bit `0x10000` is clear: it
issues a read through `func_8006E6A8(D_800B0DD8 + D_80093162[0],
D_80011618, D_80093162[1] - D_80093162[0])`, polls `func_8006E7E8()` (`-1`
restarts the read), calls `func_80072714`/`func_800726C4`/`func_80072724`,
installs the seven handler descriptors `D_801F1BD8`/`…1C58/…1D00/…1D8C/…1E18/
…1EA4/…1EF0` into `D_800E10A0[0..6]`, and sets bit `0x10000`. The hand port had
no prelude at all, so ids `0x6C..0x72` silently skipped the descriptor install
and the flag never rose.

**Evidence.** `src/func_8006F39C.c` lines 53–81 (authority; byte-exact against
retail, `LINK_EXACT`) vs `pc_port/game/boot/func_8006F39C_port.c`, which had no
`D_800E10A0`/`D_80093162`/`D_800B0DD8` reference before the fix.

**Fix.** Added `otag_install_overlay_handlers()` to
`pc_port/game/boot/func_8006F39C_port.c` (verbatim transcription of the leaf's
prelude, including the `retry`/`goto` read-restart shape) and the guard
`if (code >= 0x6Cu && code <= 0x72u)` immediately before `func_8006914C(0)`,
matching the leaf's order. The seven descriptors are held in a static
`overlay_handlers[7]` table and stored into `D_800E10A0`.

**Regression test.** `test_DECOMPPTR_overlay_handler_prelude` in
`pc_port/tests/test_decomp_ptr_params.h`. It builds the synthetic disc fixture,
runs `func_8007ED58()` (the CD reset the other CD tests run — without it the
drive lane reads idle and the prelude's retry loop spins), seeds
`D_800B0DD8`/`D_80011618`/`D_80093162`, calls `func_8006F39C(0x6C, 0)`, and
asserts:

* bit `0x10000` rises;
* all seven `D_800E10A0` slots hold `0x801F1BD8..0x801F1EF0` in order;
* a second call with the flag already set does not touch the table;
* a code below `0x6C` does not run the prelude.

## AGREE

Each of these was inspected against its leaf; the apparent difference is a
representation difference, not a behavioural one.

| leaf | hand port | why it agrees |
| --- | --- | --- |
| `func_800293F4` | `func_800293F4_port.c` | the leaf's sequence of `flags &= ~0x…` clears (`~0x1000`, `~0x2000`, …) is functionally `flags &= 0xC0C00000`; the hand port writes the folded mask. The remaining src-only literals (`0x100`, `0x200`, …) are the individual mask bits, and `D_8009D1D0` is addressed file-wide. |
| `func_8006FC18` | `func_80020F18_port.c` | the nine `0x800C7DC4..0x800D4850` addresses in the hand port are the *destruction callbacks* the leaf reaches indirectly through `D_800942E0[h]+0x14`. The hand port binds the proven identities directly and forwards unknown targets to the loud boundary; the guest-observable record clear (state `4`, then the +1..+8 wipe) is identical. |
| `func_80070064` / `func_8006FE14` / `func_800702DC` | `func_80020F18_port.c` | `D_800E0EF0[k]` for `k = 0x6C..0x72` equals `D_800E10A0[k-0x6C]` (they are the same seven slots); the hand port uses `D_800E10A0`, and `clear_effect_slot` carries the `&~0x10000` and the record wipe. Cross-checked against the `func_8006F39C` prelude, which writes `D_800E10A0` at `0x800E10A0`. |
| `func_80036DF8` / `func_80036E34` / `func_80036E58` | `func_80036DC8_port.c` | the leaf names `D_800A76A4`/`A8`/`B0`/`B4`/`BC`/`C0`; the hand port uses record bases `D_800A76A0`/`AC`/`B8` plus offsets, which resolve to the same addresses. Store order matches (including the two overwritten zero-stores retail keeps). |
| `func_8001266C` | `func_800125E0_port.c` | the leaf types both task tables as a 44-byte `Row`; the hand port uses `TASK_STRIDE = 0x2C` (44) with the link word at `+0x24`, so `&D_8009D310[i+1]` and `D_8009D310 + 0x24 + i*0x2C` reach the same addresses. |
| `func_8005F844` | `func_8005F844_port.c` | the leaf names `D_8009D13C/140/144`; the hand port names them as `0x8009CD70 + 0x3CC/3D0/3D4`, which is the same `$gp` base plus displacement. |
| `func_80065A60` / `func_80065A9C` | `func_800659F8_port.c` | `func_800B1624` is a pointer global in the leaf; the hand port reaches the same buffer through `sew3_rec16(index)`, a helper that folds `*D_800B1624 + 0x10` and the `a0 << 4` stride. Literal `D_800B1624` absent is expected. |
| `func_8006DED4` | `func_8006DE80_port.c` | the leaf's stack-local `func_8006DFA8(buf, &v0, &v1)` is the pan/volume computation; the hand port inlines an equivalent `spatial_sound()` returning `pan`/`volume` and forwards to the same `func_8006DF50`. |
| `func_80064C20` | `func_8004542C_port.c` | `~0u` and the leaf's `0xFFFFFFFFu` compare equal after integer normalisation; both store to `+0x70` and `+0x44`. |
| `func_800718D0` | `func_800718D0_port.c` | `loadimage_tim_chunk()` calls the same `func_8007506C` (via `func_80076C34`'s inline path); traversed from the helper. |
| `func_80085098` | `func_800850F4_port.c` | the leaf's `func_80085F44(0)` is the guarded `D_8009B434` setter; the hand port stores `0` directly. Equivalent when the slot is non-zero; the only divergence is the no-op case (storing `0` over `0`), which is unobservable. |
| `func_8006E6D4` / `func_800811E4` | `pe_libcd.c` | the leaf's issue/poll path (`func_80080B44`/`func_80080E34`/`func_8007F608`) is host hardware; the hand port does the transfer synchronously and records the same guest-observable post-issue state (`D_8009B6AC=0x200`, `D_8009B6B0=dest`, `D_8009B6C4=vsync`, `D_8009B6D4=1`) and the same timeout rule (`D_8009B6C4 + 1200 < vsync` → `-1`). The `D_8001136C` printf is diagnostic. |
| `func_8006ECEC` | `bootstrap/func_8006ECEC_port.c` | `D_800B0CD8` is written absolute in the hand port but via the scalar global in the leaf; same word. `func_8006E6D4`/`func_800811E4` are reached through `read_range()`, which is the leaf's read/poll loop factored once. |
| `func_8002F7D8` | `func_8002F7D8_port.c` | `D_800A5D5C` is reached as `GA_D_800A5D58 + 4`; the leaf uses `D_800A5D5C` (the body label) with the same 220-byte stride. The `func_8001A680` call is `func_8001A680_command_cut` here. |
| `func_80077CB4` / `func_80077AA4` / `func_80077A64` / `func_800428C4` / `func_80052F0C` / `func_80062CC4` / `func_800824C8` / `func_800824F0` | various | pure return-type alias differences (`int` vs `uint32_t`, `int *` vs `pe_addr_t`); the guest-address/constant/callee sets match, and the hand-port bodies compute the same expression. |
| `func_80036xxx`, `func_800471BC`, `func_8004732C`, `func_8004F950`, `func_8004F978`, `func_8004FFA8`, `func_8004FFD0`, `func_80050204`, `func_800474A8` | various `*_port.c` | the hand port passes the callback as the 32-bit guest address `0x800XXXXXu` (a data-symbol-style reloc), exactly as the leaf passes the function symbol; `func_800638D8` then dispatches on that identity. |
| `func_8007506C`, `func_80075424`, `func_800754E4` | `func_8007506C_port.c`, `pe_libgpu.c` | the JTB dispatch is reproduced through the proven `func_80076C34` identity with a loud boundary for any other target; the DR_ENV packet build and the `+0x1C` mask splice match the leaf. |
| `func_80073CF4`, `func_80073CC4`, `func_80073D24`, `func_80073D58`, `func_80073E10`, `func_80073C94` | `pe_libetc.c`, `func_80073CF4_port.c` | the `D_8009566C`/`D_80095674` jump-table slots are SDK callback-registration internals; the hand ports expose the same guest API surface and preserve the slot identity. |

## UNTESTABLE

These candidates need a subsystem or hardware model that does not exist yet, so
the hand port's behaviour cannot be exercised against the leaf at present.
They are recorded rather than "fixed", because changing them without a test
would trade a proven hand translation for an unproven one.

* `func_80064C54` — the hand port signs the parameter `uint32_t id`, the leaf
  `void`. Cross-checked against the four callers in `asm/disc1/33A4C.s`,
  `40A80.s`, `40F48.s`, `41470.s`: every site loads `$a0` before the `jal`, so
  the leaf's `(void)` prototype is the compressed form of the same call, not a
  zero-argument function. Behaviour is identical; only the host prototype
  differs.
* The `func_80073Dxx` / `func_80073Exx` SDK wrappers whose slot pointers live
  inside the driver's own jump table.
* `func_8007F7A8` — the leaf forwards `func_8007FCAC()` (`D_8009B590`); the hand
  port inlines the same load. `func_8007FCAC` is itself a derived port, so the
  hand port cannot reach the same symbol without a duplicate definition.
* `func_80076B98` — the leaf is a four-word DMA control-register store
  (`*D_80095854 = 0x4000002`, `*D_80095858 = a0`, `*D_8009585C = 0`,
  `*D_80095860 = 0x1000401`, i.e. GP1/DMA2 MADR/BCR/CHCR). The hand port
  instead implements the *linked-list DMA worker* (the `func_80076C34` chain
  walk) and deliberately performs no hardware-register write, because DMA
  registers do not exist on the host. This is a documented host adaptation
  (`pc_port/docs/drawotag_decision.md`), not a behavioural bug: the guest's
  observable ordering-table state is unchanged, and the retail targets are pure
  MMIO.
* `func_80075424` / `func_800754E4` — the leaf's `func_80071A34` is
  `B(0xA0)`, a BIOS *printf* (see `asm/disc1/621E4.s`: `li $t2,0xA0; jr $t2;
  li $t1,0x2A`), i.e. debug output only. The hand port routes it through the
  loud debug boundary instead of the C library. Behaviourally the retail path
  prints and continues.

## Reproduce

```bash
python3 tools/analysis/conflict_audit.py --route-json /tmp/route2.json \
    --json /tmp/audit.json --top 40
python3 tools/analysis/route_coverage.py --json /tmp/route2.json   # route set
python3 tools/analysis/pc_port_coverage.py                         # counts
python3 tools/analysis/gen_decomp_ports.py --verify                # 0 drifted
```

## Bottom line

296 conflicts compared. One real DIVERGE (`func_8006F39C`, a whole missing CD/XA
prelude) found and fixed, with a regression test pinning the seven installed
handler descriptors and the `0x10000` flag. The remaining candidates are
representation differences in address/constant/return-type spelling, or host
hardware adaptations that the hand port already models. No blind mass
replacement was performed.
