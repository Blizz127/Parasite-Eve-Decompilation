# FIELD_ENTRY — how field code requests battle

## Producer

```text
opcode   0x89
table    D_800910A0 + 0x89*4
handler  0x80017FF0   func_80017FF0
argc     0
```

Retail body (matching leaf, already in `src/func_80017FF0.c`):

```text
addiu $v0, $zero, 6
lui   $at, %hi(D_8009D28C)
sw    $v0, %lo(D_8009D28C)($at)
jr    $ra
addiu $v0, $zero, 1
```

Effect: **`D_8009D28C = 6`**, return 1 (script continues).

This is a mode-word store, not an overlay load and not a dest token.
No fade, inhibit, or actor capture lives inside `0x89` itself.
m0005i issues `0x40` / `0xAA` **after** the post-battle poll, not
inside the request.

## Mode word

```text
D_8009D28C
$gp+0x51C     ($gp = D_8009CD70; 0x8009CD70+0x51C = 0x8009D28C)
type          int state (Phase 5EJ READY-FROM-READER)
```

Observed stores:

| Value | Writer | Role this rung |
|---|---|---|
| 0 | `0x95` / `func_800192B8`; also `sw $zero, 0x51C($gp)` at `0x80029834` and consume path `0x80029A08` | field / clear / consume-request |
| 3 | `0x8001F41C` | battle-cluster internal |
| 4 | `0x80021F04` | battle-cluster internal |
| 5 | `0x8A` / `func_80017FDC` | unused on first m0005i request |
| 6 | `0x89` / `func_80017FF0` | **battle request** |
| 7 | script immediate only | m0005i waits for this after `0x89` |
| 8 | `0x96` / `func_800192C8` | used on m0005i after some fights |

Do not name 3/4/5/8 beyond “observed store.”

## Consumer

`0x800299CC` (battle/field tick, frame `addiu $sp, -456`):

```text
lw    $a0, 0x508($gp)          ; current record
lw    $v0, 0x4C($a0)
andi  $v0, 0x00080000
beqz  -> skip
lw    $v1, 0x51C($gp)          ; D_8009D28C
addiu $v0, $zero, 6
bne   $v1, $v0, 0x80029A0C
sb    ..., 0x10C($gp)
sw    $zero, 0x51C($gp)        ; consume 6 -> 0
```

Later `0x80029A64`: if mode != 0 branch to `0x8002A7F8`; if mode == 0
the same function continues into the long body (active processing).

Init sibling `0x80029818` zeros the mode word and related gp/actor
state before the first tick.

## What is not the entry

| Candidate | Why rejected |
|---|---|
| `0x1A` | generic RNG (`func_800176FC` → `70D6C`/`70DD0`); used on many non-battle scripts |
| `0x31` | dest-token publisher (`D_8009D280` + `D_8009D1A0 \|= 0x2000`) |
| `0x12` | field task-list splice; used on m0002i sidewalk |
| `0x70` / `0xB7` / `0x5A` / `0x6F` | slot/formation **setup**, not the mode request |
| Function names | unused; all claims are table + matching leaf + consumer compare |

## Inhibit / fade / capture

On the first m0005i request (`+0x350C`):

```text
0x94 / wait mode != 0     already parked in field
0x89                      request
0x94 / wait mode == 7
0x40 0xAA 0x1C            inhibit + load bit + mailbox  AFTER return
```

Actor/player state is not snapshotted by `0x89`. The live Aya object
`D_8009D254` and the 7-slot table are the shared stores.
