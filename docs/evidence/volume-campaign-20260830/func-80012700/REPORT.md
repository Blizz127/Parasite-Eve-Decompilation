# func_80012700 — matched Tier-1 leaf

Status: MATCHED, leaf 338. Two bounded natural-C phrasings. No pins, inline
assembly, file-scope assembly, volatile scheduling tricks, or forged padding.

## Function hood and boundaries

The exact span is file `[0x2F00,0x2F74)`, VRAM
`[0x80012700,0x80012774)`: `0x74` bytes / 29 words. It ends with the
canonical return `jr ra` at file `0x2F6C`, with the live
`move v0,a2` return value in its delay slot at `0x2F70`.

Seven distinct instructions call the exact start:

```text
caller PC   file off
8003526C    025A6C
800366BC    026EBC
80036784    026F84
80036A60    027260
80036B2C    02732C
80065484    055C84
8006553C    055D3C
```

Both boundaries are executable instructions. The preceding real function,
`func_8001266C`, ends with `jr ra; nop` at file `0x2EF8/0x2EFC`. The
following real function, `func_80012774`, starts immediately at file `0x2F74`
with `lui a3,%hi(D_8009D20C)` (`3C07800A`). Therefore:

```text
FUNCTION_HOOD=PROVEN_BY_7_DIRECT_CALLERS_AND_CANONICAL_RETURN
```

This is a genuine callable function, not padding, data, or a mislabeled tail.

## Retail semantics and screens

The function pops the first 44-byte task record from the global freelist. If
`parent` is non-null, it inserts the popped task at the head of the parent's
doubly linked child list; otherwise it clears the task links. It initializes
the task's entry/state fields, assigns the old 16-bit serial, increments the
global serial with halfword width, and returns the popped task.

| screen | retail result | implication |
|---|---|---|
| callee buckets | no `jal`; no callees | leaf; no call-result or clobber bucket |
| frame / save pressure | no stack frame or saved registers | caller-saved allocation only |
| Stage-0: `D_8009CDFC` | target reads at `80012700` and writes at `80012710`; `func_8001266C` seeds it at `80012680`; `func_80012774` drains/recycles through it at `8001280C/80012810`; sibling allocator `func_800131E8` reads/writes it at `80013210/8001325C` and `8001327C/80013294` | proven task-freelist cursor, not an isolated scalar |
| Stage-0: `D_8009D308` | target `lhu/sh` at `80012744/80012768`; sibling allocator `func_800131E8` uses the same halfword serial at `80013214/80013258` and `800132D4/800132F0`; initialization/reset writers include `func_800124F8` and `8003F0A8` | unsigned 16-bit serial counter; preserve postincrement and halfword access |
| coloring pressure | popped task remains in `a2`; parent in `a1`; link temporary in `v0`; retail serial old value in `v0`, constant/new value in `v1`; return copies `a2` to `v0` | serial expression and store ordering determine the final register homes |
| `$v0` liveness | `v0` is reused for freelist/child links, then the old serial; it is overwritten by the returned task only in the `jr` delay slot | do not introduce an early long-lived result accumulator |
| address retention | globals are direct gp-relative accesses; record accesses remain argument-relative | no symbolic absolute-address or assembler-temp lever |
| optimization signal | scheduled branch/store slots, required load-delay nops, compact caller-saved allocation, and gp-relative small data | era GCC 2.7.2 `-O2 -G8` |
| loop/back-edge owner | none | acyclic list insertion and initialization |

The native translation and independent focused tests already exercise the
null-parent path, linked insertion, two sequential pops, halfword serial
width, and write footprint in `pc_port/game/boot/func_80012700_port.c` and
`pc_port/tests/test_native.c`. They were read as an independent semantic
oracle; this matching leaf changes no `pc_port` file.

## Bounded phrasing attempts

Attempt 1 split the serial read, task assignment, and global increment:

```c
unsigned short serial;

serial = D_8009D308;
/* task initialization */
task->serial = serial;
D_8009D308 = serial + 1;
```

The first 17 words through offset `0x40` match. The explicit local keeps the
old serial in `v1`, allocates constant one to `v0`, hoists the active store,
and produces this nonmatching tail:

```text
off   attempt 1  retail    difference
44    97830598   97820598  serial home v1 vs v0 after gp normalization
48    24020001   24030001  constant one v0 vs v1
4C    ACC20010   ACC0000C  active store hoisted ahead of zero field
50    ACC0000C   ACC40000  shifted initialization order
54    ACC40000   ACC00004  shifted initialization order
58    ACC00004   ACC30010  shifted initialization order
5C    A4C00008   A4C00008  rejoins for flags store
60    A4C3000A   24430001  serial store precedes increment
64    24630001   A4C2000A  increment follows serial store
68    A7830598   A7830598  same global write after gp normalization
```

Attempt 2 preserves the source-level postincrement as one expression:

```c
task->serial = D_8009D308++;
```

This keeps the old value in `v0`, reuses `v1` for one and the incremented
value, and recovers retail's initialization/store order. All 29 words match.

Reusable lever:

```text
POSTINCREMENT-STORE-ORDER: when retail loads an unsigned scalar, stores the
old value into a record, and writes old+1 back later, preserve the operation
as `record->field = global++`. Splitting it into a local plus explicit write
can extend the old value's live range, swap v0/v1 homes, and hoist an unrelated
store.
```

## Final C

```c
typedef struct Task {
    unsigned int entry;
    unsigned int field_04;
    unsigned short flags;
    unsigned short serial;
    unsigned int field_0C;
    unsigned int active;
    unsigned char pad_14[0x10];
    struct Task *next;
    struct Task *prev;
} Task;

extern Task *D_8009CDFC;
extern unsigned short D_8009D308;

Task *func_80012700(unsigned int entry, Task *parent)
{
    Task *task;
    Task *next;

    task = D_8009CDFC;
    D_8009CDFC = task->next;

    if (parent != 0) {
        task->prev = parent;
        next = parent->next;
        task->next = next;
        if (next != 0) {
            next->prev = task;
        }
        parent->next = task;
    } else {
        task->prev = 0;
        task->next = 0;
    }

    task->field_0C = 0;
    task->entry = entry;
    task->field_04 = 0;
    task->active = 1;
    task->flags = 0;
    task->serial = D_8009D308++;

    return task;
}
```

The exact invocation is era GCC 2.7.2 `-O2 -G8`, followed by the ordinary
maspsx/ASPSX 2.21 and little-endian MIPS-I assembler path. `-G8` is required
by retail's `$gp+0x8C` and `$gp+0x598` forms. No opt-in maspsx gate is enabled:

```sh
era_compile src/func_80012700.c build/src/func_80012700.c.o -O2 -G8
```

## Full 29-word object comparison

The object has four `R_MIPS_GPREL16` text relocations and one local
`R_MIPS_26 .text` relocation. With `$gp=0x8009CD70`,
`D_8009CDFC-$gp=0x008C` and `D_8009D308-$gp=0x0598`. The raw local jump
`08000011` at offset `0x34` resolves from the linked base to `080049D1`,
targeting `0x80012744`. After those ordinary normalizations:

```text
off   object normalized  retail    instruction
00    8F86008C           8F86008C  lw    a2,0x8C(gp)
04    00000000           00000000  nop
08    8CC20024           8CC20024  lw    v0,0x24(a2)
0C    00000000           00000000  nop
10    AF82008C           AF82008C  sw    v0,0x8C(gp)
14    10A00009           10A00009  beqz  a1,0x3C
18    00000000           00000000  nop
1C    ACC50028           ACC50028  sw    a1,0x28(a2)
20    8CA20024           8CA20024  lw    v0,0x24(a1)
24    00000000           00000000  nop
28    10400002           10400002  beqz  v0,0x34
2C    ACC20024           ACC20024  sw    v0,0x24(a2)
30    AC460028           AC460028  sw    a2,0x28(v0)
34    080049D1           080049D1  j     0x80012744 (raw 08000011 + R_MIPS_26)
38    ACA60024           ACA60024  sw    a2,0x24(a1)
3C    ACC00028           ACC00028  sw    zero,0x28(a2)
40    ACC00024           ACC00024  sw    zero,0x24(a2)
44    97820598           97820598  lhu   v0,0x598(gp)
48    24030001           24030001  li    v1,1
4C    ACC0000C           ACC0000C  sw    zero,0x0C(a2)
50    ACC40000           ACC40000  sw    a0,0(a2)
54    ACC00004           ACC00004  sw    zero,4(a2)
58    ACC30010           ACC30010  sw    v1,0x10(a2)
5C    A4C00008           A4C00008  sh    zero,8(a2)
60    24430001           24430001  addiu v1,v0,1
64    A4C2000A           A4C2000A  sh    v0,0x0A(a2)
68    A7830598           A7830598  sh    v1,0x598(gp)
6C    03E00008           03E00008  jr    ra
70    00C01021           00C01021  move  v0,a2
```

Result: `29/29` after ordinary relocation normalization, zero mismatches.

## Carve geometry

The former active assembly subsegment was exactly `[0x2F00,0x2F74)`, because
the already-matched `func_80012774` begins at the next boundary. Sizes come
only from file-boundary arithmetic:

```text
prefix asm:  0x2F00 - 0x2F00 = 0x0000
C leaf:      0x2F74 - 0x2F00 = 0x0074
resume asm:  0x2F74 - 0x2F74 = 0x0000
closure:     0x0000 + 0x0074 + 0x0000 = 0x0074
```

The trim guard reduces the aligned object `.text` to the proven `0x74` body
only after checking that all removed tail bytes are zero alignment padding.
No object-alignment size participates in the carve arithmetic.

## Full packed-span comparison

This independently compares the rebuilt executable against retail from the
preceding function's return through the first word of the following function:

```text
file    VRAM       candidate  retail    status
02EF8   800126F8   03E00008   03E00008  MATCH
02EFC   800126FC   00000000   00000000  MATCH
02F00   80012700   8F86008C   8F86008C  MATCH
02F04   80012704   00000000   00000000  MATCH
02F08   80012708   8CC20024   8CC20024  MATCH
02F0C   8001270C   00000000   00000000  MATCH
02F10   80012710   AF82008C   AF82008C  MATCH
02F14   80012714   10A00009   10A00009  MATCH
02F18   80012718   00000000   00000000  MATCH
02F1C   8001271C   ACC50028   ACC50028  MATCH
02F20   80012720   8CA20024   8CA20024  MATCH
02F24   80012724   00000000   00000000  MATCH
02F28   80012728   10400002   10400002  MATCH
02F2C   8001272C   ACC20024   ACC20024  MATCH
02F30   80012730   AC460028   AC460028  MATCH
02F34   80012734   080049D1   080049D1  MATCH
02F38   80012738   ACA60024   ACA60024  MATCH
02F3C   8001273C   ACC00028   ACC00028  MATCH
02F40   80012740   ACC00024   ACC00024  MATCH
02F44   80012744   97820598   97820598  MATCH
02F48   80012748   24030001   24030001  MATCH
02F4C   8001274C   ACC0000C   ACC0000C  MATCH
02F50   80012750   ACC40000   ACC40000  MATCH
02F54   80012754   ACC00004   ACC00004  MATCH
02F58   80012758   ACC30010   ACC30010  MATCH
02F5C   8001275C   A4C00008   A4C00008  MATCH
02F60   80012760   24430001   24430001  MATCH
02F64   80012764   A4C2000A   A4C2000A  MATCH
02F68   80012768   A7830598   A7830598  MATCH
02F6C   8001276C   03E00008   03E00008  MATCH
02F70   80012770   00C01021   00C01021  MATCH
02F74   80012774   3C07800A   3C07800A  MATCH
```

`cmp` returns zero for all `0x80` displayed bytes. Candidate and retail slice
SHA-256 are both
`1dc8802565728406a9138f3355ce37e05538e925822a02791b70c672d3c5ec04`.
The packed leaf therefore agrees with both retail and the normalized isolated
object.

## Full gates

```text
$ sha1sum build/disc1.candidate.exe
452fb033f2eaa4b18aa20a5bca60b8125af3a37b  build/disc1.candidate.exe

$ scripts/build_us.sh  # raw summary tail
=== Summary ===
Assemble: OK
Compile:  OK
Pad trim: OK
Link:     OK
Pack:     OK (build/disc1.candidate.exe, size 0x1EE800)
Compare:  EXACT SHA-1 MATCH

$ scripts/verify_us.sh  # raw summary
=== Summary ===
Split verification (Phase 4E): OK.
candidate: build/disc1.candidate.exe SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
compare: EXACT MATCH to original
C conversion: Phase 5HD-12850 — 338 leaves

$ grep -cE ',[[:space:]]*c,' configs/USA/disc1.yaml
338
```

After integration, the active pool is 1,091 spans: Tier 1 10, Tier 2 214,
Tier 3 99, and SKIP/suppressed 768. This match resets consecutive bounded
parks to zero; three leaves have matched in this refreshed campaign.
