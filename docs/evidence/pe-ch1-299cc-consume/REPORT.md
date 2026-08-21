# PE-CH1 — func_800299CC_consume_cut BTL1 mode 6→0

Native named cut of the battle/field tick consume edge. Matching
`src/` C was not added: this worktree has no `asm/`, no era `cc1`,
and no extracted SLUS (`scripts/verify_us.sh` cannot take a new carve).
Does not port ATB, the 456-byte dispatcher, or the 0x80029A20 inverse.

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
window      0x800299CC..0x80029A08  (16 words, 0x40)
exclusive   0x80029A0C  lw $v0, 0x4C($a0)  = 0x8C82004C
file        0x1A1CC
yaml        inside [0x11718, asm]  (before matching 2F970 @ 0x20170)
jal         0x800355E8 (full tick, not this cut)
init twin   0x80029838  sb $zero, 0x10C($gp)
```

## EXE dump (every cut word)

| VA | Word | Instruction |
|---|---|---|
| `0x800299CC` | `8F840508` | `lw $a0, 0x508($gp)` |
| `0x800299D0` | `27BDFE38` | `addiu $sp, $sp, -456` |
| `0x800299D4` | `AFB101BC` | `sw $s1, 444($sp)` |
| `0x800299D8` | `AFBF01C0` | `sw $ra, 448($sp)` |
| `0x800299DC` | `AFB001B8` | `sw $s0, 440($sp)` |
| `0x800299E0` | `8C82004C` | `lw $v0, 0x4C($a0)` |
| `0x800299E4` | `3C050008` | `lui $a1, 0x8` (`0x00080000`) |
| `0x800299E8` | `00451024` | `and $v0, $v0, $a1` |
| `0x800299EC` | `1040000C` | `beqz → 0x80029A20` |
| `0x800299F0` | `24110001` | `addiu $s1, 1` (delay; not a guest store) |
| `0x800299F4` | `8F83051C` | `lw $v1, 0x51C($gp)` |
| `0x800299F8` | `24020006` | `addiu $v0, 6` |
| `0x800299FC` | `14620003` | `bne → 0x80029A0C` |
| `0x80029A00` | `24020006` | `addiu $v0, 6` (delay; sb source) |
| `0x80029A04` | `A382010C` | **`sb $v0, 0x10C($gp)`** (byte, value 6) |
| `0x80029A08` | `AF80051C` | **`sw $zero, 0x51C($gp)`** |

`$gp = D_8009D28C - 0x51C = 0x8009CD70`:

| Off | Address | Role |
|---|---|---|
| `+0x508` | `D_8009D278` | current record pointer |
| `+0x51C` | `D_8009D28C` | mode word (consume 6→0) |
| `+0x10C` | `D_8009CE7C` | consume-edge **byte** set to 6 |

## Contract

```text
record = *(u32*)D_8009D278
if ((*(u32*)(record+0x4C) & 0x00080000) == 0) return   # no stores
if (*(u32*)D_8009D28C != 6) return                     # no stores
*(u8*)D_8009CE7C = 6
*(u32*)D_8009D28C = 0
```

Skip paths in this cut leave mode and `gp+0x10C` untouched. The
bit-clear branch target `0x80029A20` (inverse: if edge==6, sb 0 and
restore mode 6) is past the exclusive end and is not implemented.

## Verify

```text
python3 pc_port/tools/pe_ch1_299cc_oracle.py
# from pc_port/build: PE_TEST_FILTER=299CC ./pe-native-tests
```

Oracle: 16/16 ROM words + `gp+0x51C`/`D_8009D28C` + `sb gp+0x10C=6` + exclusive `0x80029A0C`.
Native tests: 5 focused `299CC_*` (`PE_TEST_FILTER=299CC` → 5/5) plus full suite **646/646**.
