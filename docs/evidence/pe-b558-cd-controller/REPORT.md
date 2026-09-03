# PE-B558 — CD command-issue controller translated (second attempt)

Authority: retail Disc 1 executable `SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; disassembly
`asm/disc1/6B130.s:506` (`func_8007B010`, 160 words,
`0x8007B010..0x8007B290`), `asm/disc1/6B130.s:883`
(`func_8007B558`, 259 words, `0x8007B558..0x8007B964`),
`asm/disc1/704BC.s:34` (`func_8007FCFC`, 74 words,
`0x8007FCFC..0x8007FE24`).

Status: second attempt (G6). The first attempt drafted all three
translations but misread four retail details; this attempt corrects
them word by word against the asm above. No `src/` or YAML changes;
matching count stays 560. NOT matching leaves.

## First-attempt errors corrected here

1. `func_8007FCFC` always latches `[B558] = cmd`: the FD28 `sb`
   sits in the `beqz s2` delay slot, so it executes on the
   data == 0 arm too. The draft only latched on data != 0.
2. `func_8007FCFC` 8-arm is conditional: the FDC0 `beq` skips the
   `[B558] = 1` relatch when `[B58B] == 1` (then `[B558]` stays 8
   and the controller issues command 8, not 1); the FDC8 store
   falls into FDCC (`[B560] = 0`). The draft relatched
   unconditionally and never zeroed `[B560]` on that arm.
3. `func_8007B010` entry runs once: B044-B08C (VSync(-1),
   `[A3478] = VsRet+0x3C0`, `[A347C] = 0`, `[A3480] = 0x80011BA0`)
   precede the B090 poll loop; the B258 restart returns to the
   B090 poll, not to the entry stores. The draft re-ran the entry
   stores every iteration (resetting the deadline and countdown).
4. `func_8007B558` B74C branch: after the B714 stores, `bnez
   [B294] -> B8EC` skips the setup + B76C wait (unreachable past
   the B674 zeroing on every path here; kept retail-exact instead
   of the draft's backwards comment).
5. `func_8007B558` timeout-print a2 is `AFDC[[AFD5]]` (B7F0-B7FC
   via s5 = &AFDC), not `B05C[[B295]]`. The B010 timeout print
   already had this right.

Verified-but-kept details (delay slots that look like bugs): the
FDF0/FE00 `addu $v0,$zero,$zero` zeroes the nonzero controller
return, so the taken path returns 0; the B918 `bne` delay zeroes
`$v0`, so the fall-through returns `[B294]` only via the B930-B938
`-1` (i.e. `([B294] == 5) ? -1 : 0`); the B14C/B828 delay `v0 = -1`
is overwritten before any consumer.

## Port changes (`pc_port/platform/pe_libcd.c`, `pe_sdk.h`)

- New `func_80073DE8()` (halfword getter), `func_8007B9EC()`
  (latch block; the BA14 poll's set-tag arm stays an honest
  hardware-wait stop), `func_8007AAB4()` (named stop, unwinds 0),
  `func_8007B010(cmdi, buf)`, `func_8007B558(cmd, data, dst,
  mode)`, `func_8007FCFC(cmd, data)` — all exactly per the asm
  above. Print/BIOS sites (`71A74`, `73C5C`) and guest-code
  callbacks (`AFB4`/`AFB8`) record + stop and continue in retail
  order; unknowable continuations unwind 0.
- VSync(-1)'s consumed `$v0` is the host vsync-counter query
  (`PE_GPU_VSyncQuery`), the honest host equivalent of the
  hardware frame counter (documented at both call sites).
- The CDS1 stop is untouched: `func_8007FB44` still records
  `func_8007FCFC` + `PE_PORT_STOP_UNRESOLVED_BOUNDARY`. Wiring the
  live head into this chain is a later rung (needs the 7C564
  delivery machine behind the completion callback).

## Oracle

`pc_port/tools/pe_b558_cd_controller_oracle.py`: the six windows
(74/259/160/3/4/53 words) by SHA-256, the full jal chain (7FCFC ->
7B9EC/80950/7B558; 7B558 -> 71A74 x3 incl. the B608 table print,
73C5C, 7B9EC, 7B010, 73A44 x2, 73DE8, 7AAB4; 7B010 likewise), and
the load-bearing immediates as true LE words — now including the
full B558 prologue/gate (`0x8007B558..0x8007B598`: lui/lw pair,
`addiu sp,-0x38`, the sw-spill block, `slti/bne` gate). Exit 0 =
green; proven to fail (perturbed `0x8007B560`/`0x8007B590` ->
`FAIL: ... word`, exit 1) and restored to exit 0.

## Tests

10 new `B558_*`: 73DE8 unsigned-halfword pin; 7B9EC full-store
transcription (no stop on clear tag); 7AAB4 boundary pin; 7B010
B21C/copy/B258 arms incl. the entry deadline/countdown words and
the store-2-not-5 pin; 7B558 B5BC `-2` gate (no print, no stop);
print-then-`-2` with both `71A74` indirect-arg sets pinned
(0x80011BB4/0x80011BBC formats, AFDC table word); full dispatch
returning 0 with the AFD5/S1 latches; 7FCFC zero-data wrapper
end-to-end (returns 1, `[B580] = [B558]`, no stop); 8-arm guarded
vs firing (`[B58B] == 1` keeps `[B558] = 8`); 7-arm firing vs
cold (`[B558] = 1` vs `7`). No existing test touched: the CDS1
boundary expectations still hold because 7FB44 still stops.

## Strict run

Unchanged frontier (by design — the switch is not flipped):
`FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
func_8007FCFC / called from: func_8007FB44`.

## Verify

```
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
```

Both lines with `PE_DISC1_BIN` (gateless first run: 1064 run,
1046 passed, 1 failed = pre-existing B54KY environmental case
missing `local/pe_disc1.path`, 17 skipped). Normal CTest:
`100% tests passed, 0 tests failed out of 2`. Fresh ASan/UBSan
CTest with the disc env: `100% tests passed, 0 tests failed out
of 2`, zero sanitizer diagnostics.
