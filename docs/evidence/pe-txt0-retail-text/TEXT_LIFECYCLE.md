# TEXT_LIFECYCLE

## Open

Opcode `0x0D` → `func_800375E0` → state 1, id at `+0x10`.
Non-blocking; the script continues into `0x22` (m0004i) or an
authored `0x02` wait (m0372i).

## Bind and parse

Once per field frame, `0x8003F568` calls `func_80037870`.

On state 1 the updater copies `D_8009CD90` into `record+0x04`
and walks until `(FF|F9) FE <id>`. The cursor is left on the
first body byte.

The inner loop at `0x80037B68` consumes bytes until
`0x50($sp)` (parser_stop) is set. **Generic glyphs do not set
stop.** One invocation therefore emits every glyph up to the
next yielding control.

Yielding controls:

| byte | stop? | extra |
|---|---|---|
| `0xF7` | yes | newline, continue next frame at the new y |
| `0xF8` | yes | page; wait for `D_8009D1F4 & 0x100`, then skip to next `0xF8` |
| `0xF9` | yes | **`sb 0` to state** — immediate close / chain |
| `0xFB 06` | yes | wait-for-`0x100` skip |
| `0xFB 07 nn` | yes while `record+0x0D < nn` | pause; increment `+0x0D` once per matching tick |
| `0xFF` | yes | terminal; see below |

## Reveal

There is **no per-glyph delay** on the generic path. Current-slice
bodies are short and contain no `0xF8`. They appear in one
update, then sit on their terminator.

`0x14..0x20` terminate with `FB 07 nn` then `F9` (next-id
marker). They pause `nn` ticks and auto-close.

`0x21..0x23` terminate with `0xFF`. They use the RD5-X close
rule.

Pad bit `0x100` on `D_8009D1F4` is the newly-pressed word from
`func_8003EB04`: `(held ^ previous) & held`. Held-from-before
does not dismiss. That ruling is not reopened.

## Terminal 0xFF

`0xFF` is a **control token in the message stream**, not a C
string NUL and not a window-state enum. After the marker search
has passed, the parser treats a body `0xFF` as:

```text
if state != 2:
    parser_stop = 1          /* later: 1 -> 2 */
else if (D_8009D1F4 & 0x100) and !(flags & 0x02000000):
    state = 0
    parser_stop = 1
else:
    parser_stop = 1
```

So “terminal 0xFF” is the stream token that *creates* text-engine
state 2, and on a later frame *allows* the 0x100 edge to clear
the record.

## Opcode 0x22

`func_800177C8` → `func_80037548(id)`. While `byte0 != 0` it
rewinds `gp+0x90` by 12, sets `current_task+0x10 = 1`, returns 0.
When `byte0 == 0` it returns 1 and the script advances.

m0004i `0x21..0x23` use this poll. m0372i `0x14..0x20` do not;
those IDs auto-close on F9 and the script uses `0x02` waits.

## Same system

`0x21..0x23` use the same `func_80037870` generic path as
`0x14..0x20`. Difference is terminator (FF vs FB07+F9) and the
script wait style (0x22 vs 0x02).
