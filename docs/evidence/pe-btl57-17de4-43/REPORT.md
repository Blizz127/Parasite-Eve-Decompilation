# PE-BTL57 — opcode 0x43 choice read + FB 09 cursor

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
17DE4       15w  0x80017DE4..0x80017E20  sha 71be0cb3…c405
37864       3w   0x80037864..0x80037870  lb 0x134($gp)
```

Matching `src/` C was not added.

## 0x43

`*arg0 = (signed char)D_8009CEA4`. Live type-0 `local[5]` after
`0x22`. `375E0` writes `-1`. FB 09 copies `gp+0x130` into
`gp+0x134` on `D1F4&0x100`.

## FB 09

`flags |= 0x00200000`, count = next byte `& 7` into flags 22-24.
`D1F4&0x20` increments cursor; `D1F4&0x08` decrements. Cross
latches the cursor. MSG 0x07 operand `02` is Watch/Skip.
Watch (0) is the default cursor.
