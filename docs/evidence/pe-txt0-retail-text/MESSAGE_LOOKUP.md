# MESSAGE_LOOKUP — opcode 0x0D to encoded bytes

## Authority

USA Disc 1 SHA-256 `7f20fce99a7ff18accebf3156419b24d4c0145c5c0f8168d5e86005ccf28f9c4`.
EXE SHA-256 `5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b`.
Dispatch table `0x800910A0 + opcode*4`.

## Opcode 0x0D

Handler `0x80017410`. One bound argument: the message ID
(`lh` of the first binder pointer). `$a1` is hardcoded to 0. `$a2`
is a stack `s16` initialized to `-1`. The handler calls
`func_800375E0(id, 0, &minus_one)` and returns 1 (non-blocking).

Script form on the current slice is always `argc=1`:

```text
m0372i module1 +0x025C..+0x037C  0x0D  id=0x14..0x20
m0004i module0 +0x0434/+0x044C/+0x0464  0x0D  id=0x21/0x22/0x23
```

There is no second script operand. The “speaker parameter = 0”
observed on these opens is the handler store of `$a1`, not a
script field.

## Allocation — func_800375E0

Does **not** look up text bytes. It scans four 56-byte records at
`D_800BCEA8` for `byte0==0`, then writes:

| offset | write |
|---|---|
| `+0x00` | state `1` |
| `+0x08` | `$a1` (0 on the 0x0D path) |
| `+0x10` | message ID as `s16` |
| `+0x0C` flags | clear bits `0x00100000` and `0x00200000` |

`$a2` is a `-1`-terminated list of `s16` values converted to
decimal digits at `record+0x1A`. Opcode 0x0D passes only `-1`, so
that loop is not taken.

## First conversion of ID to source bytes

`func_80037870` (field loop `jal` at `0x8003F568`), when
`record+0x00 == 1`:

1. copies `*(gp+0x120)` (`D_8009CD90`) into `record+0x04`
2. scans that stream for `(0xFF or 0xF9) 0xFE <id>`
3. stores `record+0x04 = marker + 3`

That scan is the first function that turns a message ID into a
pointer to encoded text bytes.

`gp+0x120` is published by `func_800371B0` (`sw $a0, 0x120($gp)`).
Its caller at `0x8003F244` selects:

```text
if (D_800B0CD8 & 0x40000000)
    a0 = D_800B162C;   /* slot7 store 1, English */
else
    a0 = D_800B1628;   /* slot7 store 0 */
```

USA boot at `0x8005286C` does `D_800B0CD8 |= 0x40000000` and then
`func_800371A4(1)`. First-play therefore uses stream 1.

## Proven chain (m0004i 0x21)

```text
script +0x0434  opcode 0x0D  argc=1  id=0x21
  -> func_80017410
  -> func_800375E0(0x21, 0, &-1)
  -> record at D_800BCEA8  state=1  half10=0x21
  -> func_80037870
  -> D_800B162C  (USA bit set)
  -> slot7[1] +0x0604  bytes F9 FE 21
  -> body FA 4A 0F 2B 2B FF
```
