# PE-RD5-F9 naming — MSG_ vs OP_

Message ids and field-script opcodes collide by value on this
slice. The same integer is a **record id** in the slot7 stream and
a **dispatcher opcode** in the module bytecode.

This rung uses prefixes in every table, test name, and claim.

| Prefix | Meaning | Handler / consumer | Slice examples |
|---|---|---|---|
| `MSG_0xNN` | dialogue record id written by `OP_0x0D` into `D_800BCEA8+0x10` | `func_800375E0` / `func_80037870` | `MSG_0x14`..`MSG_0x20` (m0372i reel); `MSG_0x21` `MSG_0x22` `MSG_0x23` (m0004i reel) |
| `OP_0xNN` | field-script opcode, dispatch `D_800910A0[NN]` | see below | `OP_0x0D` open; `OP_0x22` poll; `OP_0x02` tick wait; `OP_0x23` explicit clear; `OP_0x3F` control restore |

Opcode identities used here:

```text
OP_0x0D  func_80017410  open record (non-blocking)
OP_0x22  func_800177C8  poll record byte0; rewind while nonzero
OP_0x02  authored tick wait (script, not window)
OP_0x23  explicit record clear (not used on MSG_0x14..MSG_0x23 first-play)
OP_0x3F  func_80017D5C  control restore (m0004i module0+0x05B0)
```

Bare `0x21` / `0x22` / `0x23` are forbidden in this folder. Write
`MSG_0x22` or `OP_0x22`. Stream bytes `0xF9` / `0xFF` / `FB 07` are
window-parser controls, not field opcodes, and keep their hex form.

m0372i first-play reel is `MSG_0x14`..`MSG_0x20` only. `MSG_0x21` /
`MSG_0x22` / `MSG_0x23` belong to m0004i and are named here so the
collision is visible, not so this rung reopens that reel.

The only `OP_0x22` in the m0372i package is `module3+0x0A00`
`OP_0x22 MSG_0x07` on the `persist[0]&4` arm. First play
`persist[0]=0` skips it. It is not a reel row.
