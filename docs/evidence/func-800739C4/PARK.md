# func_800739C4 — 0x64 bytes — PARKED

Retail VRAM `0x800739C4` (file offset `0x641C4`, size `0x64`), in
`asm/disc1/621E4.s`. Not registered as a `c` span; stays `asm`.

## Retail semantics (proven)

```c
int func_800739C4(int a0, int a1) {
    D_80094564 = a0;
    D_80094568 = a1;
    if (a0 == 0x21)
        func_80073A34(0xF4000002, 0x301);
    else if (a0 == 0x22)
        func_80073A34(0xF4000002, 0x302);
    return 0;
}
```

## Residual mechanism

12 of 25 words match. `-O2 -G0` cc1 hoists the `$ra` save above the two global
stores (retail stores first, then `sw $ra`), and emits **one shared `jal`** with
a `j` into it from the first arm:

```
bne  $4,$2,$L2 ; li $5,0x301
li   $4,-201326592
j    $L3 ; ori $4,$4,2
$L2: li $2,0x22 ; bne $4,$2,$L7 ; li $4,-201326592
ori  $4,$4,2 ; li $5,0x302
$L3: jal func_80073A34
```

Retail instead **duplicates the materialization** and emits the call in each
arm (the `jal` for the 0x21 arm, `$a1=0x301` set in its `beq` delay slot;
a second value set in the 0x22 arm's `beq` delay slot). This is a
tail-duplication / cross-jump decision of the era assembler-reorder path; five
phrasings (if/else, shared tail, duplicated literal, `switch`, split locals with
a merged call) all produce the shared-`jal` form. Invariant under the default
rung. No new lever available yet.

Next-possible-unblocker: an aspsx reorder lever that duplicates a
single-callee call across both arms of a 2-way compare, or evidence tying retail
to a distinct psx frontend.

### Retry log (this session)

Re-attacked per the "retry scheduling-only near-misses" mandate with a
control-flow restructure: an early-`return` per arm and a `v0`-carried compare
ladder, so that each arm owns its own call site and literal materialisation.
Both phrasings gave `word mismatches=18` — worse than the shared-tail form's 12,
which confirms the era frontend is collapsing the two arms regardless of the
restructure. The decision is downstream of C control flow (asm-reorder /
cross-jump), so no C spelling reaches it. PARK stands unchanged.
