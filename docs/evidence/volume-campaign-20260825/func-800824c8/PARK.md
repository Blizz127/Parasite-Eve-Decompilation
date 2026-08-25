# func_800824C8 — refreshed volume attempt 3

Outcome: PARKED-ADDRESS-RETENTION after two allowed phrasing iterations. No matching-C leaf claim; count remains 288.

## C1 / pool row

0x72CC8 | func_800824C8 | 5 words | jr-ra | 12 direct callers | 0 jal in body | global D_800B8AB4 | no loop | real/real boundaries | TIER 1

## C2 / function hood

- Span: file [0x72CC8,0x72CDC), VRAM [0x800824C8,0x800824DC), five words.
- Body ends in jr ra with store a0,0(v1) in the delay slot.
- Previous boundary at 0x800824C4 is the real sw a0,0(v1) delay-slot instruction ending func_800824B4.
- Following boundary at 0x800824DC is the real first instruction of adjacent func_800824DC.
- Twelve unique direct jal callsites target the exact start:

- file 0x6AAD0 / VA 8007A2D0
- file 0x71B6C / VA 8008136C
- file 0x71BA4 / VA 800813A4
- file 0x725B8 / VA 80081DB8
- file 0x72620 / VA 80081E20
- file 0x727F8 / VA 80081FF8
- file 0x7280C / VA 8008200C
- file 0x728DC / VA 800820DC
- file 0x729B8 / VA 800821B8
- file 0x72A14 / VA 80082214
- file 0x72A94 / VA 80082294
- file 0x72AE4 / VA 800822E4

FUNCTION_HOOD=PROVEN. This is callable code, not padding or a mislabeled span.

## C3 / retail and screens

```text
800824C8: 3C03800B  lui   v1,%hi(D_800B8AB4)
800824CC: 24638AB4  addiu v1,v1,%lo(D_800B8AB4)
800824D0: 8C620000  lw    v0,0(v1)
800824D4: 03E00008  jr    ra
800824D8: AC640000  sw    a0,0(v1)
```

| Screen | Result |
|---|---|
| Frame decomposition | args 0 + locals 0 + saves 0 = frame 0 |
| Callee buckets | none; no jal in the body |
| Stage-0 written global | D_800B8AB4 is read before it is overwritten; return value is the old word |
| Coloring pressure | old global value must remain in v0; address must remain in v1 until the delay-slot store |
| $v0 liveness | loaded old value is live through jr and returned |
| Address retention | PROVEN: v1 address is retained from lui/addiu through lw to store delay slot |
| -O signal | no loop or repeated constant; -O2 -G0 is the normal straight-line era baseline |
| Indexed symbolic gate | none; scalar symbolic global, absolute G0 form |
| Loop/back-edge owner | none |

Flags selected for both attempts: era -O2 -G0. G0 is required by the absolute scalar address form; no 3W, division, dispatch-fold, sched2, pin, or inline-asm mechanism applies.

## C4 / two bounded C phrasings

Attempt 1, assignment-expression shape:

```c
extern unsigned int D_800B8AB4;
unsigned int func_800824C8(unsigned int a0) {
    return D_800B8AB4 = a0;
}
```

Object result:

```text
move v0,a0
lui at,%hi(D_800B8AB4)
sw v0,0(at)
jr ra
nop
```

Attempt 2, explicit old-value shape:

```c
extern unsigned int D_800B8AB4;
unsigned int func_800824C8(unsigned int a0) {
    unsigned int old = D_800B8AB4;
    D_800B8AB4 = a0;
    return old;
}
```

Object result:

```text
lui v0,%hi(D_800B8AB4)
lw v0,0(v0)
lui at,%hi(D_800B8AB4)
sw a0,0(at)
jr ra
nop
```

The second phrasing has the correct old-value semantics but still fails the retail address-retention shape: it reconstructs the address and leaves the body at seven instructions rather than five. A pointer-local or hard-register pin would be the next lever, but pins and further phrasing iterations are forbidden by R6/R7. This is parked rather than misrepresented as matching C.

## Disposition

No YAML, build, verifier, or count integration was made. The current candidate is preserved in stash: park volume func_800824C8 address-retention residual. Evidence is this report; the pool removes the row from TIER 1 and classifies it as PARKED-ADDRESS-RETENTION.
