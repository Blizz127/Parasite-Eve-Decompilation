# PST1_IMPLEMENTATION_CONTRACT

Read-only contract for a later persist-runtime rung.
This file does **not** authorize implementation in PE-PST0.

## Frozen identity

```text
bank        = D_800A77F0
count       = 512 words
bytes       = 0x800
element     = u32 word
binder      = field VM mode 2
zero        = func_80034F10 only (boot / new-game)
field load  = do not zero
save/load   = memcpy 0x800 via func_8003F800 / func_8003FBD8
```

## Must do (PST1)

1. Allocate exactly 512 words at one host authority that *is*
   `D_800A77F0`. No second “storyFlags” object.
2. Implement binder mode 2 as `base + slot*4` word load/store.
3. Implement `0x0A` as `*dst = *src` and `0x09` with the 24
   ALU cases in `BINDER.md` (at least 0x02/03/09/0A/0B/0C used
   on Day 1).
4. Keep the bank across `func_80034FC4` field rebuilds.
5. On new game only, run the 512-word zero (and the separate
   64-word scratch zero). Do not call that from room change.
6. First-play writers, in order (re-derived this rung):

   | Scene | PC | Op |
   |---|---|---|
   | m0002i mod0 | `+0x0268` | `persist[0] = cond` (0 on first play) |
   | m0002i mod0 | `+0x0278` | `persist[0x4A] = 9` |
   | m0002i mod5 | `+0x0EE4` | `persist[1] = 2` then `0x31 m0003i` |
   | m0003i mod1 | `+0x0718` | `persist[1] = 3` |
   | m0372i mod1 | `+0x04E4` | `persist[0x4A] = 0x12` then `0x31 m0004i` |
   | m0372i mod3 | `+0x0D48` | `persist[0x4A] = 0x12` (same value) |
   | m0004i mod0 | `+0x0534` | `persist[0] = cond` (0) |
   | m0004i mod0 | `+0x0544` | `persist[0x4A] = 0x18` |
   | m0004i mod4 | `+0x0F30` | `persist[1] = 4` then `0x31 m0378i` |
   | m0378i mod4 | `+0x0724` | `persist[1] = 0x17A` then `0x31 m0377i` |

7. Dest spawn must consume `persist[1]` with the dest module’s
   own `0x09 ==` table. Do not invent a pose table.

## Must not

- Name `persist[0x4A]` `storyProgress`.
- Treat 0 and 9 as the same gate.
- Shrink the bank to the observed index set.
- Zero persist on battle entry without a proven caller.
- Serialize a subset of indices.
- Implement memory-card files (that is SAV0).
- Merge scratch / cond / actor locals into persist[].

## SAV0 (later)

Allowed only after PST1 owns the 512-word bank:

- memcpy 0x800 persist
- keep the 0x12E4 prefix and 0x8A8 tail as research
- close CRC-16/`0x1021` span and card header before load tests

## UE / native parity

A UE persist component is ready only when it is a 512-word
array with the same indices, the same first-play writes, and
no parallel convenience flags. That is **not** ready in PST0.

## Ready flags

```text
pst1_implementation_ready = NO   # contract only; no runtime change
sav0_research_ready       = PARTIAL  # persist range closed; header/CRC span open
ue_persist_model_ready    = NO
```
