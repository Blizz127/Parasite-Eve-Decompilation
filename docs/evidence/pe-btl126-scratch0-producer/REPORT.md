# PE-BTL126 — post-1266C scratch[0]|=4 provenance

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
m0005i chunk2 SHA-256 `01a64ba3…7e3b`. No matching `src/` C.
Do not poke `D_800B6A80`. Do not force type-2 PC / `2F7D8` / D20C.

## EXE / disc C stores

Every TEXT instruction with imm `0x6A80`:

| VA | Site | Effect |
|---|---|---|
| `0x800126D0` | `1266C` | addiu; zeros 64 words at `D_800B6A80` |
| `0x800171F4` | `17018` | kind-4 decode `D_800B6A80 + imm*4` |
| `0x80034F3C` | `34F10` | boot zero of word 0 |

Disc-wide `lui 0x800B` + imm `0x6A80` is those three EXE sites
only (SLUS LBAs 29/39/98). No PE.IMG overlay writer.

`3F074@0x8003F0B8` is the sole `jal 1266C`. dest-ready runs it
before `125E0`.

## Script stores of scratch[0] bit 2

`0x2A` is `*arg0 |= (1 << *arg1)`. Linear walk of m0005i types
0–6 finds **one** `0x2A[scratch[0], 2]`:

```text
type-6 +0x1850  word=0x0008402A  kinds [4,0]  imms [0,2]
```

That site is after `0xAE` (`+0x1100`) / `0x55` (`+0xFAC`).
It is the battle-start setter, not a dest-enter unlock.
Type-0 `+0x1150` is the same opcode with **bit 4**.
No `0x0A` / `0x09` dest `scratch[0]` on this map.

## Type-6 +0x190 wait (unchanged polarity)

```text
+0x180  0x14 code 2 rel 1668  → actor+0x19C = base+0xD08
+0x190  0x09 AND scratch[0]&4 → cond[0]     word=0x02308009
+0x1A8  0x09 is_zero cond[0]  → cond[1]
+0x1C0  0x05 skip-if-false imm 0xF4         → +0x1E8
+0x1D0  0x02 yield
+0x1DC  0x00 goto +0x190
```

Clear bit waits. Set bit skips to `+0x1E8` `0x12` then `+0x394`
`0x89` and `+0x3F8` `0x1C(2,0,0x7D)`.

New-game type-0 never reaches `+0x035C` `0x1C(2,0,0xB)`
(persist `[0x4A]==39`). persist `[1]==4` only selects a pose
and rejoins the same persist-39 gate.

## Type-2 does not unpark 0x20

```text
+0x03C  0x14 code 2 rel 84    → actor+0x19C = base+0xA8
+0x098  0x20                 → this task parks (task+8|=0x10)
+0x0A8  0x1F local[4]        → mailbox task starts here
+0x4B8  local[4]==0x7D       → 0x98 / 0x2E(2) / 0x04 / 0x1C(6,0,0x84)
+0x484  local[4]==0x7F       → 0x00 +0x70C → +0x804 0x6F
+0x804  0x6F                 → 2F7D8 body
```

`65400` delivers a **new** task at `+0x19C`. The parked `0x20`
task is not resumed. `0x7D` (type-6 after the wait) is the
new-game handshake, not the `0x6F` arm. `0x7F` (type-0
`+0x1128`) is the body spawn.

First-visit dest-ready ticks leave `scratch[0]&4` clear, type-6
on the `+0x1DC` wait, type-2 on `0x20` with `+0x19C` live.
No planted mailbox.

## Verify

```text
python3 pc_port/tools/pe_btl126_scratch0_producer_scan.py
PE_TEST_FILTER=BTL126 ./pc_port/build/pe-native-tests
```
