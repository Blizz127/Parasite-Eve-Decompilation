# func_800339A0 — bounded GP load/store scheduling residual

Status: `PARKED-GP-LOAD-STORE-SCHEDULING-AND-REGISTER-HOME`.

Disposition: `ACCEPTED-RESIDUAL`. This is evidence disposition only, not a
byte-identity claim. The leaf remains assembly and does not increment the
matching-C count.

## Function hood and boundaries

The exact span is file `[0x241A0,0x24220)`, VRAM
`[0x800339A0,0x80033A20)`: `0x80` bytes / 32 words. It ends with `jr ra` at
`0x80033A18` and `nop` in the delay slot at `0x80033A1C`.

Six distinct raw direct calls target the exact start. Generated duplicate
assembly views were deduplicated by caller PC:

```text
caller PC   file off  target
80016CF0    0074F0    800339A0
80029998    01A198    800339A0
8005C15C    04C95C    800339A0
8005C190    04C990    800339A0
8005C55C    04CD5C    800339A0
8005C654    04CE54    800339A0
```

Each call is the direct `jal` encoding `0C00CE68` (little-endian file bytes
`68 CE 00 0C`). The preceding real `func_800334AC` ends at file
`0x24198/0x2419C` with `jr ra; nop`. The following matched
`func_80033A20` begins immediately at file `0x24220` with a real gp-relative
`lbu`. Both boundary sides are executable.

```text
FUNCTION_HOOD=PROVEN_BY_6_DIRECT_CALLERS_AND_CANONICAL_RETURN
```

## Screens and state provenance

The body is a no-call, no-loop leaf with a 16-byte local aggregate. It copies
the 16-byte constant table at `D_80010E38` to the stack, indexes one four-byte
pair by the low byte of the argument, and publishes the selected pair plus the
original selector to gp state.

| screen | retail result | consequence |
|---|---|---|
| callee buckets | zero `jal` in the body | no callee return/clobber bucket |
| frame | `0x10`, no saved registers | exactly the local table copy; no prologue-save lever |
| coloring pressure | stack base, indexed pair address, two halfword values, and three independent stores | selected-address/result homes are the central pressure |
| `$v0` liveness | `$v0` first holds copied words, then the scaled index, then each selected halfword | retail deliberately reuses `$v0`; candidate gives the pair address to `$v0` instead |
| address retention | retail keeps `sp + ((index & 0xff) << 2)` in `$v1` across both loads and the first store | candidate keeps that address in `$v0` and cannot reproduce the store interleave |
| `-O` signal | full aggregate copy, scheduled load-delay nop, and independent GP stores | era `-O2 -G8` is the justified probe |
| back-edge owner | none | straight-line leaf |

With `_gp = 0x8009CD70`, the written state is:

```text
gp+0x110  D_8009CE80  unsigned byte selector
gp+0x114  D_8009CE84  first selected halfword
gp+0x116  D_8009CE86  second selected halfword
```

A full executable exact-access census finds seven accesses to `gp+0x110`,
nineteen to `gp+0x114`, and twenty-two to `gp+0x116`. This function is the
sole writer in each bucket; the remaining 6/18/21 sites are readers. The
consumer cluster spans `func_800334BC`, `func_80033A20`, and the routines
through `0x80034760`; additional selector readers occur at `0x80034A30`,
`0x80034D90`, `0x80034E28`, and `0x80034EC8`.

The source table bytes are:

```text
dd 00 b1 00  0f 00 b1 00  0f 00 0f 00  dd 00 0f 00
```

or four pairs `{0x00DD,0x00B1}`, `{0x000F,0x00B1}`,
`{0x000F,0x000F}`, and `{0x00DD,0x000F}`. The index mask is preserved
exactly; retail supplies no range guard for values above three. Existing
read-only oracle `pc_port/tools/pe_btl6_339a0_oracle.py` independently
confirms the 32-word body, table contents, and three state writes. No pc-port
file was changed for this matching attempt.

## Retail body — all 32 words

```text
off  word      instruction
00   27BDFFF0  addiu sp,sp,-0x10
04   3C068001  lui   a2,0x8001
08   24C60E38  addiu a2,a2,0x0E38
0C   88C20003  lwl   v0,3(a2)
10   98C20000  lwr   v0,0(a2)
14   88C30007  lwl   v1,7(a2)
18   98C30004  lwr   v1,4(a2)
1C   88C5000B  lwl   a1,11(a2)
20   98C50008  lwr   a1,8(a2)
24   ABA20003  swl   v0,3(sp)
28   BBA20000  swr   v0,0(sp)
2C   ABA30007  swl   v1,7(sp)
30   BBA30004  swr   v1,4(sp)
34   ABA5000B  swl   a1,11(sp)
38   BBA50008  swr   a1,8(sp)
3C   88C2000F  lwl   v0,15(a2)
40   98C2000C  lwr   v0,12(a2)
44   00000000  nop
48   ABA2000F  swl   v0,15(sp)
4C   BBA2000C  swr   v0,12(sp)
50   308200FF  andi  v0,a0,0xff
54   00021080  sll   v0,v0,2
58   03A21821  addu  v1,sp,v0
5C   94620000  lhu   v0,0(v1)
60   00000000  nop
64   A7820114  sh    v0,0x114(gp)
68   94620002  lhu   v0,2(v1)
6C   A3840110  sb    a0,0x110(gp)
70   A7820116  sh    v0,0x116(gp)
74   27BD0010  addiu sp,sp,0x10
78   03E00008  jr    ra
7C   00000000  nop
```

## Two bounded C phrasings

Both attempts used the era compiler with `-O2 -G8`, maspsx ASPSX 2.21 with
the repository's standard flags, and GNU MIPS-I `as`. The aggregate and field
types follow the observed copy and halfword accesses:

```c
typedef struct Pair {
    unsigned short first;
    unsigned short second;
} Pair;

typedef struct PairTable {
    Pair pairs[4];
} PairTable;

extern const PairTable D_80010E38;
extern unsigned char D_8009CE80;
extern unsigned short D_8009CE84;
extern unsigned short D_8009CE86;

void func_800339A0(unsigned int index)
{
    PairTable table = D_80010E38;
    unsigned int selected = index & 0xFF;

    D_8009CE84 = table.pairs[selected].first;
    D_8009CE80 = index;
    D_8009CE86 = table.pairs[selected].second;
}
```

Attempt 1 put the selector store first (`CE80`, `CE84`, `CE86`). Attempt 2,
shown above, follows retail source order for the first two stores
(`CE84`, `CE80`, `CE86`). cc1 emitted byte-identical output for both, proving
that the residual is not controlled by natural assignment order. No pin,
inline asm, volatile ordering fiction, mismatch-hiding macro, or forged nop
was used.

## Complete candidate object and normalized comparison

Both attempts emit 31 content words (`0x7C`) followed by one zero alignment
word. Relocations are shown normalized to linked values below:

```text
off  candidate  retail     result
00   27BDFFF0   27BDFFF0   =
04   3C068001   3C068001   =  HI16 D_80010E38
08   24C60E38   24C60E38   =  LO16 D_80010E38
0C   88C20003   88C20003   =
10   98C20000   98C20000   =
14   88C30007   88C30007   =
18   98C30004   98C30004   =
1C   88C5000B   88C5000B   =
20   98C50008   98C50008   =
24   ABA20003   ABA20003   =
28   BBA20000   BBA20000   =
2C   ABA30007   ABA30007   =
30   BBA30004   BBA30004   =
34   ABA5000B   ABA5000B   =
38   BBA50008   BBA50008   =
3C   88C2000F   88C2000F   =
40   98C2000C   98C2000C   =
44   00000000   00000000   =
48   ABA2000F   ABA2000F   =
4C   BBA2000C   BBA2000C   =
50   308200FF   308200FF   =
54   00021080   00021080   =
58   03A21021   03A21821   address home v0 != v1
5C   94430000   94620000   first result v1 != v0
60   94420002   00000000   second load hoisted; retail load-delay nop
64   A3840110   A7820114   selector store != first-half store
68   A7830114   94620002   first-half store != deferred second load
6C   A7820116   A3840110   second-half store != selector store
70   27BD0010   A7820116   epilogue starts one word early
74   03E00008   27BD0010   shifted
78   00000000   03E00008   shifted
7C   00000000   00000000   alignment zero vs retail return-slot nop
```

In the relocatable object, the two table words are `3C060000/24C60000` with
`HI16/LO16 D_80010E38`; the three GP stores carry `GPREL16` relocations and
link to offsets `0x110/0x114/0x116`. Those ordinary relocations normalize to
the retail words above and are not residuals.

The first mismatch is therefore offset `0x58`. Retail retains the selected
pair address in `$v1`, loads the first halfword into `$v0`, leaves the required
load-delay nop, stores that result, then performs the second load before the
selector and second-half stores. cc1 instead gives the address to `$v0`, the
first result to `$v1`, hoists the independent second load, and emits all three
GP stores consecutively. Source-order reversal did not alter any decision.

```text
RESIDUAL_MECHANISM=GP_LOAD_STORE_SCHEDULING_AND_REGISTER_HOME
FUTURE_UNBLOCKER=a compiler allocation/scheduler lever that retains the
  selected-pair address in v1, keeps the first result in v0, and declines to
  hoist the second lhu across the first gp-relative halfword store
```

The final candidate is preserved in labeled stash
`park func_800339A0 gp-load-store scheduling residual`; recover the untracked
source from that stash's third parent. There is no YAML, build, or verifier
integration. The exact matching-C count remains 340.
