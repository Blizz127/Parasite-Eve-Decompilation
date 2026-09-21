# `func_8006E6D4` — CD sector-read issue with mode-mismatch exit

Outcome: **MATCHED** on era `-O2 -G0` (YAML default profile), pinless (no
`register asm`, no barrier, no maspsx knob). Integrated as matching-C leaf
564. The former `PARK.md` residual (extra `lui $v1,0x100` rematerialized
after `jal func_800719E4`) is closed by the callee's true `noreturn`
signature, substantiated below.

## Function hood and retail span

- File span `[0x5EED4,0x5EFE8)` = `0x114` bytes = 69 words.
- VRAM span `[0x8006E6D4,0x8006E7E8)`.
- Canonical `jr ra; nop` at `0x8006E7E0/0x8006E7E4`.
- Preceded by `func_8006E6A8` (`jr ra; nop` at `0x8006E6CC/0x8006E6D0`);
  followed by matched-C `func_8006E7E8` at `0x8006E7E8`.
- Callers: `jal 0C01B9B5` (`func_8006E6A8` wrapper) and the direct
  `func_8006E834` read loop (`src/func_8006E834.c`).

## Semantics (from retail bytes, SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`)

```text
8006e6d4  27bdffd0  addiu sp,sp,-48
8006e6d8  afb00018  sw    s0,24(sp)      ; s0 = lba
8006e6dc  00808021  move  s0,a0
8006e6e0  afb20020  sw    s2,32(sp)      ; s2 = off
8006e6e4  00a09021  move  s2,a1
8006e6e8  afb40028  sw    s4,40(sp)      ; s4 = dest
8006e6ec  00c0a021  move  s4,a2
8006e6f0  afb30024  sw    s3,36(sp)
8006e6f4  afb1001c  sw    s1,28(sp)
8006e6f8  3c11800b  lui   s1,0x800b      ; s1 = &D_800B0CD8
8006e6fc  26310cd8  addiu s1,s1,3288
8006e700  afbf002c  sw    ra,44(sp)
8006e704  8e220000  lw    v0,0(s1)
8006e708  3c030100  lui   v1,0x100
8006e70c  00431024  and   v0,v0,v1
8006e710  1440002b  bnez  v0,0x8006e7c0
8006e714  00e09821  move  s3,a3          ; delay: s3 = sectors
8006e718  0c01fdcb  jal   func_8007F72C
8006e71c  00000000  nop
8006e720  24030001  li    v1,1
8006e724  14430027  bne   v0,v1,0x8006e7c4
8006e728  2402ffff  li    v0,-1
8006e72c  0c01fdde  jal   func_8007F778
8006e730  00000000  nop
8006e734  14400023  bnez  v0,0x8006e7c4
8006e738  2402ffff  li    v0,-1
8006e73c  0c01fdea  jal   func_8007F7A8
8006e740  00000000  nop
8006e744  3c03800b  lui   v1,0x800b
8006e748  94630dd4  lhu   v1,3540(v1)    ; D_800B0DD4 (unsigned short)
8006e74c  00000000  nop
8006e750  10430003  beq   v0,v1,0x8006e760
8006e754  3c030100  lui   v1,0x100        ; delay: reused across the call
8006e758  0c01c679  jal   func_800719E4    ; B(38h) = exit(1)
8006e75c  24040001  li    a0,1
8006e760  34634000  ori   v1,v1,0x4000
8006e764  02128021  addu  s0,s0,s2
8006e768  02002021  move  a0,s0
8006e76c  8e220000  lw    v0,0(s1)
8006e770  27a50010  addiu a1,sp,16
8006e774  00431025  or    v0,v0,v1
8006e778  0c0202d1  jal   func_80080B44
8006e77c  ae220000  sw    v0,0(s1)        ; delay
8006e780  27a40010  addiu a0,sp,16
8006e784  02602821  move  a1,s3
8006e788  02803021  move  a2,s4
8006e78c  0c02038d  jal   func_80080E34
8006e790  24070080  li    a3,128
8006e794  1440000b  bnez  v0,0x8006e7c4    ; nonzero status returned
8006e798  3c03feff  lui   v1,0xfeff
8006e79c  3463bfff  ori   v1,v1,0xbfff
8006e7a0  3c048001  lui   a0,0x8001
8006e7a4  2484136c  addiu a0,a0,4972    ; D_8001136C
8006e7a8  02002821  move  a1,s0
8006e7ac  8e220000  lw    v0,0(s1)
8006e7b0  02603021  move  a2,s3
8006e7b4  00431024  and   v0,v0,v1
8006e7b8  0c01c69d  jal   func_80071A74
8006e7bc  ae220000  sw    v0,0(s1)        ; delay
8006e7c0  2402ffff  li    v0,-1
8006e7c4  8fbf002c  lw    ra,44(sp)      ; epilogue
```

C shape (`src/func_8006E6D4.c`):

```c
int func_8006E6D4(int lba, int off, unsigned char *dest, int sectors)
{
    unsigned char loc[8];
    int status;
    unsigned int *flagsPtr = &D_800B0CD8;

    if (*flagsPtr & 0x01000000) return -1;
    if (func_8007F72C() != 1) return -1;
    if (func_8007F778() != 0) return -1;
    if (func_8007F7A8() != D_800B0DD4) func_800719E4(1);
    *flagsPtr |= 0x01004000;
    lba += off;
    func_80080B44(lba, loc);
    status = func_80080E34(loc, sectors, dest, 0x80);
    if (status != 0) return status;
    *flagsPtr &= 0xFEFFBFFF;
    func_80071A74(D_8001136C, lba, sectors);
    return -1;
}
```

## The rematerialization lever (why the PARK residual is closed)

`func_800719E4` is the BIOS trampoline `li $t2,0xB0; jr $t2; li $t1,0x38`
(3 words, `0x800719E4`). Per the authoritative PSXSPX BIOS function list,
`B(38h) or A(06h) exit(exitcode)` — the call terminates the program and
does not return. `A(06h)=exit` is the same function via the A table.

- `__attribute__((noreturn))` on the prototype is therefore the **true**
  signature, not a codegen fiction.
- With the call modelled as returning, cc1 keeps `0x01000000` live across
  a call-clobbered register and emits a fall-through `lui $v1,0x100`
  rematerialization after the `jal` (plus reorg's delay-slot copy) → 70
  words, one long.
- With the no-return edge, the merge block has a single predecessor, so
  reorg threads retail's `lui` into the `beq` delay slot and deletes the
  rematerialize → 69 words, byte-exact. The merge block is still reached
  when the modes are equal, so the emitted stream is correct on the only
  reachable path; all 12 retail callers discard `$v0`.

Cross-check of the other 11 call sites (`func_800370DC`/`func_80037140` and
the `80037xxx` wrapper family) passing `a0=-1`: they are fatal error exits
(`exit(-1)`), consistent with the identity.

### Correction for `pc_port/platform/func_800719E4_port.c`

That port classifies B(38h) as "CD mode set: collapsed no-op" and returns 0.
The BIOS table says B(38h) = `exit(exitcode)`. The collapse is
guest-invisible only because every retail call site is an unreached fatal
path; the classification should be recorded as `exit`, not "CD mode set".

## Single-leaf object

```text
AS=tools/mipsel-host/usr/bin/mipsel-linux-gnu-as \
OBJDUMP=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objdump \
OBJCOPY=tools/mipsel-host/usr/bin/mipsel-linux-gnu-objcopy \
  tools/analysis/era_leaf_match.sh src/func_8006E6D4.c 0x8006E6D4 0x114 -O2 -G0
```

Result: `MISMATCHES=13`, every one a link-time relocation word with the
body bits identical modulo the relocated field:

| VRAM | retail | object | relocation |
|---|---|---|---|
| `0x8006E6F8` | `3c11800b` | `3c110000` | R_MIPS_HI16 `D_800B0CD8` |
| `0x8006E6FC` | `26310cd8` | `26310000` | R_MIPS_LO16 `D_800B0CD8` |
| `0x8006E718` | `0c01fdcb` | `0c000000` | R_MIPS_26 `func_8007F72C` |
| `0x8006E72C` | `0c01fdde` | `0c000000` | R_MIPS_26 `func_8007F778` |
| `0x8006E73C` | `0c01fdea` | `0c000000` | R_MIPS_26 `func_8007F7A8` |
| `0x8006E744` | `3c03800b` | `3c030000` | R_MIPS_HI16 `D_800B0DD4` |
| `0x8006E748` | `94630dd4` | `94630000` | R_MIPS_LO16 `D_800B0DD4` |
| `0x8006E758` | `0c01c679` | `0c000000` | R_MIPS_26 `func_800719E4` |
| `0x8006E778` | `0c0202d1` | `0c000000` | R_MIPS_26 `func_80080B44` |
| `0x8006E78C` | `0c02038d` | `0c000000` | R_MIPS_26 `func_80080E34` |
| `0x8006E7A0` | `3c048001` | `3c040000` | R_MIPS_HI16 `D_8001136C` |
| `0x8006E7A4` | `2484136c` | `24840000` | R_MIPS_LO16 `D_8001136C` |
| `0x8006E7B8` | `0c01c69d` | `0c000000` | R_MIPS_26 `func_80071A74` |

Extra `.text` bytes (`C=0x120` vs `ROM=0x114`) are GNU as alignment pad,
trimmed to the YAML span size by `tools/trim_elf_section_pad.py` during
`scripts/build_us.sh`.

## Registration

- YAML: `- [0x5EED4, c, func_8006E6D4]` in `configs/USA/disc1.yaml`
  (`Mid-5E39C` carve, replacing the former anonymous `0x5EED4` asm span).
- `python3 tools/build/disc1_plan.py --check` → 852 spans
  (564 c, 286 asm, 2 rodata).
- `python3 tools/build/test_disc1_plan.py` → 8 tests OK.
- `scripts/verify_us.sh --public` → `PUBLIC_VERIFY=PASS`, matching-C **564**,
  plan SHA-256
  `3778923deaeb30074862d07f5e0916cbe5fecf6663cc3f9bb4cd166a1fdab3a4`.
- Exact packed SHA-1 rebuild not run here (no `mipsel-linux-gnu-gcc`, no
  distrobox/docker); see ACTIVE_HANDOFF.
