# BINDER — field script operand targeting

Evidence-only. Bytes from `SLUS_006.62` at:

- fetch/bind: `0x800170F0` … `0x80017244`
- mode table: `D_80010690`
- ALU: `func_80012850` + `jtbl_80010000` (24 entries)
- assign: `func_800173F4`
- skip-if-false: `func_8001731C`

Handler table `D_800910A0` verified this rung for
`0x00/0x05/0x09/0x0A/0x31`.

## Command header

Each command is 8 bytes plus `argc` argument words:

```text
bits  0..12  opcode          (mask 0x1FFF)
bits 13..16  argc            (0..15)
bits 17..    3 bits × argc   binder mode per argument
```

If `argc > 5`, modes 5+ are taken from the next header word
(`$t1` / `0x80017214`). Observed Day-1 persist ops use `argc ≤ 4`.

The binder writes a **pointer** for each argument into a small
frame array, then `jalr`s `D_800910A0[opcode]`. Handlers
dereference those pointers (`lw`/`sw` of words).

## Mode cases (`sltiu` 5, table `0x80010690`)

| Mode | Case VA | Pointer written |
|---:|---|---|
| 0 | `0x80017154` | address of this command’s immediate word |
| 1 | `0x8001718C` | `*(D_8009D2F0) + 0xAC + slot*4` (actor locals) |
| 2 | `0x800171BC` | `D_800A77F0 + slot*4` (**persist[]**) |
| 3 | `0x8001716C` | `D_8009DF70 + slot*4` (condition bank) |
| 4 | `0x800171DC` | `D_800B6A80 + slot*4` (scratch) |

`slot` is the 32-bit argument word, then `sll 2`. No width other
than word is used. No sign-extend of the index.

This is how field scripts target persistent state: opcode `0x0A`
or `0x09` with a mode-2 argument.

## Opcode `0x0A` — `func_800173F4`

```text
*arg0 = *arg1
return 1
```

Read and write are the same instruction. Typical first-play store:

```text
modes [2, 0]  args [1, 2]   → persist[1] = 2
modes [2, 0]  args [0x4A, 9] → persist[0x4A] = 9
modes [2, 3]  args [0, 0]    → persist[0] = cond[0]
```

## Opcode `0x09` — `func_80012850`

`sltiu subop, 0x18` then `jtbl_80010000[subop]`.

Argument roles: `[subop, dst, a, b]` (b optional).

| sub | VA | Operation | Signedness |
|---:|---|---|---|
| 0x00 | `0x80012894` | `dst = a + b` | wrap |
| 0x01 | `0x800128B4` | `dst = a - b` | wrap |
| 0x02 | `0x800128D4` | `dst = a \| b` | bits |
| 0x03 | `0x800128F4` | `dst = a & b` | bits |
| 0x04 | `0x80012914` | `dst = a ^ b` | bits |
| 0x05 | `0x80012934` | `(a!=0) \|\| (b!=0)` | bool |
| 0x06 | `0x8001296C` | `(a!=0) && (b!=0)` | bool |
| 0x07 | `0x800129A0` | `a == 0` | bool |
| 0x08 | `0x800129B8` | `dst = ~a` | bits |
| 0x09 | `0x800129D0` | `dst = (b < a)` | **slt signed** |
| 0x0A | `0x800129EC` | `dst = (a < b)` | **slt signed** |
| 0x0B | `0x80012A0C` | `dst = (a == b)` | bit-exact |
| 0x0C | `0x80012A30` | `dst = (a >= b)` | slt xor 1 |
| 0x0D | `0x80012A54` | `dst = (a <= b)` | slt xor 1 |
| 0x0E | `0x80012A74` | `dst = (a != b)` | bit-exact |
| 0x0F | `0x80012A98` | `dst = a * b` | lo |
| 0x10 | `0x80012AC0` | `dst = a / b` | signed div |
| 0x11 | `0x80012B08` | `dst = a << b` | sllv |
| 0x12 | `0x80012B28` | `dst = a >> b` | **srav** |
| 0x13 | `0x80012B48` | `dst = a` | copy |
| 0x14 | `0x80012B5C` | `func_8003708C(a,b)` | UNKNOWN helper |
| 0x15 | `0x80012B80` | `func_800370A8(a,b)` | UNKNOWN helper |
| 0x16 | `0x80012BA4` | `dst = a % b` | signed rem |
| 0x17 | `0x80012BF0` | `dst = -a` | neg |

Day-1 persist compares are almost all:

- `0x0B` equality (`persist[1] == N`)
- `0x0A` signed `<` (`persist[0x4A] < imm`)
- `0x0C` signed `>=`
- `0x09` signed `imm < persist[0x4A]` (written `persist[0x4A] > imm`)
- `0x03` / `0x02` bit and/or on `persist[0]`

Result is typically stored to **mode 3 cond[0]**, then consumed by
opcode `0x05`.

## Opcode `0x05` — `func_8001731C`

If `*arg0 == 0`, set the script pointer to
`actor+0x9C + (*arg1 << 1)` (module-relative halfword units).
If nonzero, fall through. This is skip-if-false.

## Width / signedness summary

- persist cells are 32-bit words
- stores are full-word `sw`
- relational gates on `persist[0x4A]` are **signed `slt`**
- flag tests on `persist[0]` are bitwise
- `persist[1]` identity tests are bit-exact equality

Do not infer an enum from one assignment. A slot that sees both
`andi` and `==` is not a single type.
