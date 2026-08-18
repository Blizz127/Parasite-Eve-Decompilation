# Phase 6E-B54C — `func_8006AD40` material-table path audit

## Verdict

**PE-6E-B54C AUDIT SUCCESS — MATERIAL-TABLE PATH AND NEXT RETAIL BOUNDARY PROVEN**

B54C is read-only. Production C is unchanged. The live frontier remains
`func_8006AD40_prefix_cut` at `0x8006AE68`.

Retail execution beginning at `0x8006AE68` is a straight-line prefix:

1. two-iteration `D_80091648` record packing (`a1 = 0x20, 0x30`);
2. already-translated `func_8006E498(s4, 0xABADC06C)`;
3. already-translated one-record `func_8007506C` payload walk;
4. `s0 = 1`; `s2 == 1` does **not** take the `s2 == -1` back-edge;
5. `jal func_8006E7E8` at `0x8006AF54`.

The helper at that site is translated. Its **return value** is live CD
state and is **not** derived here.

```text
canonical_poll_result=UNRESOLVED
```

The audit stops before that branch. `func_800718D0` and `func_80030894`
are not reached on the proven prefix.

Recommended next implementation rung:

```text
start_pc=0x8006AE68
end_pc=0x8006AF54
first_unimplemented_pc=0x8006AF54
implemented_callees_required=func_8006E498, func_8007506C
unresolved_dependency=func_8006E7E8 return value (live CD poll)
reason_this_is_narrowest_honest_cut=packing plus already-translated lookup/walk are a branch-free prefix; the first unresolved semantics are the poll at 0x8006AF54
```

## Checkout and authority

The path named in the B54C brief,
`/home/blizz/dev/parasite-eve-port-black`, is **not present** on this
host. B54B evidence was taken from the accepted local B54B artifacts
(`b54b-edit` / B54A snapshot). The matching executable is present and
was re-hashed before every word claim.

```text
claimed_canonical_checkpoint=c1efff529e1529893c2ae73f78637b3677c30f77
claimed_branch=phase6e-b-provider-frontier
claimed_frontier=func_8006AD40 @ 0x8006AE68
local_live_repo=ABSENT
b54b_evidence_base=f003e4aa5ecfd17c5b98aa466f8262f44c5d6d37
b54a_evidence_commit=d58ff9b88b71373fd34c3d3866e8c16220a9a5d0
accepted_b54b_cut=0x8006AE68
matching_EXE_SHA-1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
superseded_parallel=19d36a0  (not used)
```

Every instruction word cited below was re-read from that executable
(`taddr = 0x80010000`, file offset `pc - 0x80010000 + 0x800`). B54A
oracle 8/8 and B54B oracle 8/8 both pass against it.

Accepted B54B outcome used as input:

```text
PE-6E-B54B SUCCESS — COUNTED func_8006E1C0 LOOP COMPLETED TO 0x8006AE68
Native / ASan+UBSan 568/568
B54A / B54B oracles 8/8 each; oracle total 47/47
framebuffer fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
trace     42c1956e077a40fed5176653b6a18938a8a91e99e35fe9d7044f31581de785af
visible output remains Level 0
```

## State entering `0x8006AE68`

B54B exit locals (canonical Disc 1):

| Reg | Meaning | Value |
| --- | --- | --- |
| `s0` | delay-slot advance one record past entry 12 | `0x8012E05C` (overwritten later) |
| `s1` | completed-entry counter | `13` |
| `s2` | last CD-issue mode / prior poll | `1` |
| `s3` | metadata `base + [base+4]` | `0x8012DF18` |
| `s4` | channel-1 buffer | `0x801229A0` |
| `s5` | `&D_800B0CD8` | `0x800B0CD8` |
| `s6` | LBA base `D_800B0DD8` | `0x000003F5` |

Guest / subsystem captured at the earlier B54A cut (`0x8006AE50`) and
still valid for CD / `D_80091648` / archive bytes after B54B, because
the counted `func_8006E1C0` loop does not write those surfaces:

```text
D_800B0CD8              0x41004003
busy bits 0x01004000    still set (channel 2 issued, not yet polled)
base                    0x801229A0
base+0 / base+4         0x0000B6BC / 0x0000B578
dir header [s3+8]       0x0040B5AC   count=1, offset=0xB5AC
tex header [s3+0x28]    0x0340B5B8   count=13, offset=0xB5B8
producer / consumer     1 / 1   (B54A cut; not recaptured after 12 extra LoadImages)
DMA2                    idle at B54A cut; not recaptured after B54B
I_STAT / I_MASK         0 / 0x0009
D_800945E6              0
VBlank                  0
```

`s2 = 1` is the prefix store at `0x8006AE04` after a successful channel-2
**issue**. Channel 2 has not been polled. That is why `0x01004000` is
still set. The material-table prefix does not clear it.

---

# QUESTION A — material-table synthesis

Begins exactly at:

```text
0x8006AE68  addiu  a1, zero, 0x20     ; 0x24050020
```

## Record layout

`D_80091648` is a 4-record array. Each record is `0x10` bytes. `a1` is
the byte offset of the current record. Field symbols are the first
record's addresses; later records are those addresses plus `a1`.

| Record off | `+0` | `+2` | `+4` | `+6` | `+8` | `+A` |
| --- | --- | --- | --- | --- | --- | --- |
| symbol | `D_80091648` | `D_8009164A` | `D_8009164C` | `D_8009164E` | `D_80091650` | `D_80091652` |
| width | u16 | u16 | u16 | u16 | u16 | u16 |
| role | source A | source B | source C | source D | packed dest 1 | packed dest 2 |

Static EXE halfwords (also the B54A live dump; B54B does not mutate them):

| `a1` | A | B | C | D | dest1 | dest2 |
| --- | --- | --- | --- | --- | --- | --- |
| `0x00` | `0x0140` | `0x0000` | `0x0140` | `0x00FC` | `0` | `0` |
| `0x10` | `0x0180` | `0x0000` | `0x0150` | `0x00FC` | `0` | `0` |
| `0x20` | `0x0110` | `0x01D4` | `0x0130` | `0x01DE` | `0` | `0` |
| `0x30` | `0x011C` | `0x01D4` | `0x0130` | `0x01DD` | `0` | `0` |

This loop does **not** touch `a1 = 0x00` or `a1 = 0x10`. Those two
records are packed later at `0x8006AFB4` (`a1 < 0x20`), which is not
reached on this prefix. No table header is generated. No count field is
written. The only destinations are the two packed halfwords of records
2 and 3.

## Packing formulas (literal)

```text
packed1 = ((A & 0x3FF) >> 6)
        | 0x20
        | ((B & 0x100) >> 4)
        | ((B & 0x200) << 2)

packed2 = (D << 6) | ((C >> 4) & 0x3F)
```

Interpretation, not a renamed symbol: these bit operations are the
Psy-Q `GetTPage` page field (with bit 5 forced set) and `GetClut(C, D)`.
Confidence: **high** for the bit math, **medium** for the draw-use of
the resulting halfwords (no consumer of `D_80091650/52` is executed on
this prefix).

Canonical packed results:

| `a1` | dest1 address | dest1 | dest2 address | dest2 |
| --- | --- | --- | --- | --- |
| `0x20` | `0x80091670` | `0x0034` | `0x80091672` | `0x7793` |
| `0x30` | `0x80091680` | `0x0034` | `0x80091682` | `0x7753` |

## Writes

| retail PC | destination | width | value | source | interpretation | confidence |
| --- | --- | --- | --- | --- | --- | --- |
| `0x8006AEB0` (iter `a1=0x20`) | `0x80091670` | u16 | `0x0034` | `A=0x0110`, `B=0x01D4` | packed dest1 of record 2 | HIGH |
| `0x8006AEE4` (iter `a1=0x20`) | `0x80091672` | u16 | `0x7793` | `C=0x0130`, `D=0x01DE` | packed dest2 of record 2 | HIGH |
| `0x8006AEB0` (iter `a1=0x30`) | `0x80091680` | u16 | `0x0034` | `A=0x011C`, `B=0x01D4` | packed dest1 of record 3 | HIGH |
| `0x8006AEE4` (iter `a1=0x30`) | `0x80091682` | u16 | `0x7753` | `C=0x0130`, `D=0x01DD` | packed dest2 of record 3 | HIGH |

Loop bound: `a1` starts at `0x20`, adds `0x10`, continues while
`a1 < 0x40`. Exactly two iterations. The last iteration's delay slot
sets `a0 = s4 = 0x801229A0` for the lookup.

No MMIO. No DMA. No CD. No GPU command. Synthesis itself does not
require a provider or hardware event.

```text
material_table_status=LITERAL_PACK_PROVEN
D_80091648_semantics=four 0x10-byte records; this path writes packed dest1/dest2 of records 2 and 3 only
literal_range_audited=0x8006AE68..0x8006AF54
```

---

# QUESTION B — `func_8006E498`

Call site:

```text
0x8006AEF8  lui    a1, 0xABAD
0x8006AEFC  jal    func_8006E498          ; 0x0C01B926
0x8006AF00   ori   a1, a1, 0xC06C         ; delay
```

| register | value on canonical Disc 1 | source |
| --- | --- | --- |
| `a0` | `0x801229A0` | `s4` / `D_800B0CD8+0x160` channel-1 base |
| `a1` | `0xABADC06C` | immediate |

The helper is Phase 6E-B16, 31 words / `0x7C`,
`0x8006E498..0x8006E513` (file offset `0x5EC98`). All 31 words rechecked
against the matching EXE. No callees. No stores. Production
`func_8006E498_port.c` matches the retail walk:

```text
hdr   = lw(base + lw(base+4) + 8)
count = hdr >> 22
e     = base + (hdr & 0x3FFFFF)          ; 0xC-byte records
return base + (lw(e+4) & 0xFFFFFF)       ; on lw(e+8) == key
return 0                                 ; if count==0 or no match
```

Canonical directory (B54A RAM dump and Disc 1 channel-1 extract, both):

```text
hdr              0x0040B5AC
count            1
first entry      0x8012DF4C
dir[0]+0         0x00007F10     ; unused by this helper
dir[0]+4         0x00000008
dir[0]+8         0xABADC06C     ; exact key match at i=0
result           0x801229A0 + 8 = 0x801229A8
```

```text
func_8006E498_status=TRANSLATED_FAITHFUL
func_8006E498_args=a0=0x801229A0 a1=0xABADC06C
func_8006E498_result=0x801229A8
```

Production behavior matches retail on this input. The return is a guest
address, not a host pointer.

---

# QUESTION C — `func_8007506C` walk

After the lookup:

```text
0x8006AF04  s0 = v0                         ; 0x801229A8
0x8006AF08  v0 = lw(s0)                     ; 0x00007F0C
0x8006AF10  beqz v0, 0x8006AF44             ; not taken
0x8006AF14   a0 = s0 + 4                    ; delay
.L8006AF18:
0x8006AF18  jal  func_8007506C
0x8006AF1C   a1 = s0 + 0xC                  ; delay
0x8006AF20  v0 = lw(s0)
0x8006AF28  v0 = v0 & ~3                    ; srl 2 / sll 2
0x8006AF30  s0 = s0 + v0
0x8006AF34  v0 = lw(s0)
0x8006AF3C  bnez v0, 0x8006AF18
0x8006AF40   a0 = s0 + 4                    ; delay
.L8006AF44:
0x8006AF44  s0 = 1
```

Record layout at the lookup result (not a named type; observed bytes):

| off | width | canonical | role |
| --- | --- | --- | --- |
| `+0` | u32 | `0x00007F0C` | size; terminator when 0; step = size & ~3 |
| `+4` | RECT | `{448, 0, 64, 254}` | `a0` for `func_8007506C` |
| `+C` | pixels | `0x801229B4` | `a1` for `func_8007506C` (inline, not a loaded pointer) |

`64 * 254 * 2 = 0x7F00` and `0x7F0C - 0xC = 0x7F00`. The size field is
header plus pixel bytes.

Walk result:

```text
record 0: func_8007506C(rect@0x801229AC, data@0x801229B4)
          RECT = {448, 0, 64, 254}
          step = 0x7F0C
next s0   0x8012A8B4
[next]    0
walk ends; exactly one LoadImage
s0 overwritten with 1 at 0x8006AF44
```

The walk is fully deterministic from state at `0x8006AE68`. The archive
lives in the already-complete channel-1 buffer. The B54B `func_8006E1C0`
loop reads the 13 texture entries at `base + 0xB5B8` and does not write
the directory or this payload. Disc 1 user sectors at
`LBA = 0x3F5 + 0x9D = 0x492`, 23 sectors, reproduce the B54A RAM words
and the single-record walk.

`func_8007506C` is Phase 6E-B52, 24 words / `0x60`,
`0x8007506C..0x800750CC`. Rechecked against the EXE. It is the Psy-Q
LoadImage wrapper: validator `func_80074E28("LoadImage", rect)`, then
`jalr jtb[2]` with `a0 = jtb[8]`, `a2 = 8`, `a3 = data`. Canonical
`jtb[2] = 0x80076C34`, `jtb[8] = 0x80076664`. Those GPU/DMA effects use
**existing** B52/B53 authority. This audit does not add a checkpoint.

Return of `func_8007506C` is discarded. After the walk, `s0` is forced
to `1`.

```text
func_8007506C_status=TRANSLATED_FAITHFUL
func_8007506C_walk=1 record; RECT{448,0,64,254}; terminator 0x8012A8B4
```

---

# QUESTION D — `func_8006E7E8` poll

Exact call site (first unimplemented PC of the recommended rung):

```text
0x8006AF4C  beq    s2, v0, 0x8006ADD8     ; v0 was set to -1 at 0x8006AF48
0x8006AF50   nop
0x8006AF54  jal    func_8006E7E8          ; 0x0C01B9FA
0x8006AF58   nop
0x8006AF5C  s2 = v0
0x8006AF60  bnez   s2, 0x8006AE08
```

On the canonical path `s2 = 1`, so `0x8006AF4C` is **not taken**. The
poll is reached.

The helper is Phase 6E-B16, 19 words / `0x4C`,
`0x8006E7E8..0x8006E833`. Rechecked against the EXE:

```text
st = func_800811E4(sp+0x10)     ; host uses PE_6E7E8_SYNC_ADDR
if ((uint32_t)(st + 1) < 2)     ; st ∈ {-1, 0}
    D_800B0CD8 &= 0xFEFFBFFF    ; clears 0x01004000
return st
```

`func_800811E4` (host `pe_libcd.c`): `0` = done, `-1` = vsync timeout,
else pending byte count `D_8009B6B4`.

Entering the call:

```text
D_800B0CD8 still has 0x01004000
channel 2 was issued at 0x8006ADE4 and has not been polled
material-table prefix does not call CD helpers
VBlank was 0 at the B54A cut; B54B added no VSync
```

The result **cannot** be derived from the suffix alone. It is live CD
completion. Retail CD is asynchronous (17 channel-2 sectors still
outstanding at issue). The host issue path writes `D_8009B6B4 = 0`
synchronously; that collapse is **not** a retail timing proof and is
**not** used as `poll_result`.

No B54B-frontier observation of `0x8006AF54` exists on this host. This
audit does not invent `poll=0` or `poll!=0`.

```text
func_8006E7E8_status=TRANSLATED_FAITHFUL_RESULT_LIVE
func_8006E7E8_semantics=return func_800811E4; clear 0x01004000 iff st in {-1,0}
canonical_poll_result=UNRESOLVED
```

The audit stops before `0x8006AF60`.

---

# QUESTION E — `func_800718D0`

```text
func_800718D0_reachable_on_canonical_path=NOT_YET
func_800718D0_callsite=0x8006AFA8
func_800718D0_args=a0 = lw(D_800B0CD8+0x174) = 0x8012F1A0   ; only if reached
```

Which branch reaches it: `func_8006E7E8` must return **0** at
`0x8006AF54`, then the suffix issues the next CD range
(`D_800930EE` / `buf+0x180` via translated `func_8006E6A8`) and only
then, with `s0` still 0, calls `func_800718D0`.

Required poll result: `0`. That result is UNRESOLVED, so canonical
Disc 1 execution is **not** proven to reach `0x8006AFA8`.

Function (re-read from EXE, not implemented):

```text
size           29 words / 0x74
range          0x800718D0 .. 0x80071944 (exclusive)
jr / delay     0x8007193C / nop at 0x80071940
callees        func_8007506C @ 0x80071910 and 0x80071920 only
```

It is **not** the next unresolved execution dependency. The next
unresolved dependency is the poll result at `0x8006AF54`.

`func_80030894_reached=no`.

---

# QUESTION F — hardware / DMA / CD

| surface | at `0x8006AE68` | mutated by packing? | mutated by 6E498? | mutated by 7506C walk? |
| --- | --- | --- | --- | --- |
| `D_800B0CD8` | `0x41004003`; busy `0x01004000` set | no | no | no |
| CD busy | set; channel 2 in flight | no | no | no |
| DMA2 CHCR | not recaptured after B54B's 12 extra LoadImages | no | no | existing GPU/DMA authority only |
| DICR stored/physical | `0x00800000` at B54A cut; not recaptured | no | no | existing path only |
| I_STAT / I_MASK | `0` / `0x0009` at B54A cut | no | no | existing path only |
| `D_800945E6` | `0` | no | no | no on this prefix |
| producer / consumer | `1` / `1` at B54A cut; not recaptured | no | no | existing queue path |
| work marker | `1` at B54A cut | no | no | existing path |

Material-table synthesis requires **no** provider or hardware event.

The recommended rung's `func_8007506C` call uses the existing LoadImage
/ DMA2 authority. No new DMA checkpoint is required or invented. Further
completions, if any, belong to later host opportunities, not to this
audit.

Resolving the poll would require live CD progression. That is why the
cut is before `0x8006AF54`.

```text
dma_checkpoint_required=no
hardware_progression_required=no   ; for the recommended prefix
                                   ; yes to resolve the poll, which is out of scope
```

---

# QUESTION G — presentation relevance

```text
presentation_relevance=INDIRECTLY_PRESENTATION_RELEVANT
```

What this prefix actually does:

- packs two draw-page / CLUT-style halfwords into `D_80091650/52`;
- uploads one `64 x 254` rectangle to VRAM at `(448, 0)`.

`(448, 0)` sits outside a 320-wide display window. The accepted
framebuffer hash is unchanged through B54B and this prefix does not
call `PutDispEnv` / `SetDispMask` / `ResetGraph` / `VSync`. There is no
evidence this path selects the uploaded background for presentation or
emits a display list.

Classification of the pieces:

| piece | class |
| --- | --- |
| `D_80091648` packing | INDIRECTLY_PRESENTATION_RELEVANT (draw-table fields) |
| `func_8006E498` lookup | NOT_PRESENTATION_RELEVANT by itself |
| `func_8007506C` `{448,0,64,254}` | INDIRECTLY_PRESENTATION_RELEVANT (VRAM material) |
| camera / display setup | NOT_PRESENTATION_RELEVANT (not executed) |
| GPU command generation for the presented FB | NOT_PRESENTATION_RELEVANT (not executed) |
| selecting uploaded background for presentation | NOT_PRESENTATION_RELEVANT (not executed) |

Do not stretch this into a first-visible-output rung.

---

# Literal control-flow map (`0x8006AE68` .. `0x8006AF54`)

Delay slots are indented. Canonical taken/not-taken is marked.

```text
8006AE68  addiu  a1, zero, 0x20              ; a1 = 0x20
.L8006AE6C:                                  ; packing body
8006AE6C  lui    at, 0x8009
8006AE70  addu   at, at, a1
8006AE74  lhu    a0, 0x164A(at)              ; B
8006AE78  lui    at, 0x8009
8006AE7C  addu   at, at, a1
8006AE80  lhu    v0, 0x1648(at)              ; A
8006AE84  andi   v1, a0, 0x100
8006AE88  srl    v1, v1, 4
8006AE8C  andi   v0, v0, 0x3FF
8006AE90  srl    v0, v0, 6
8006AE94  ori    v0, v0, 0x20
8006AE98  or     v1, v1, v0
8006AE9C  andi   a0, a0, 0x200
8006AEA0  sll    a0, a0, 2
8006AEA4  or     v1, v1, a0
8006AEA8  lui    at, 0x8009
8006AEAC  addu   at, at, a1
8006AEB0  sh     v1, 0x1650(at)              ; WRITE dest1
8006AEB4  lui    at, 0x8009
8006AEB8  addu   at, at, a1
8006AEBC  lhu    v1, 0x164E(at)              ; D
8006AEC0  lui    at, 0x8009
8006AEC4  addu   at, at, a1
8006AEC8  lhu    v0, 0x164C(at)              ; C
8006AECC  sll    v1, v1, 6
8006AED0  srl    v0, v0, 4
8006AED4  andi   v0, v0, 0x3F
8006AED8  or     v1, v1, v0
8006AEDC  lui    at, 0x8009
8006AEE0  addu   at, at, a1
8006AEE4  sh     v1, 0x1652(at)              ; WRITE dest2
8006AEE8  addiu  a1, a1, 0x10
8006AEEC  sltiu  v0, a1, 0x40
8006AEF0  bnez   v0, 0x8006AE6C              ; taken once (a1=0x30); not taken (a1=0x40)
8006AEF4   addu  a0, s4, zero                ; delay: a0 = 0x801229A0
8006AEF8  lui    a1, 0xABAD
8006AEFC  jal    func_8006E498               ; TRANSLATED
8006AF00   ori   a1, a1, 0xC06C              ; delay: a1 = 0xABADC06C
8006AF04  addu   s0, v0, zero                ; s0 = 0x801229A8
8006AF08  lw     v0, 0(s0)                   ; 0x00007F0C
8006AF0C  nop
8006AF10  beqz   v0, 0x8006AF44              ; NOT taken canonically
8006AF14   addiu a0, s0, 4                   ; delay
.L8006AF18:
8006AF18  jal    func_8007506C               ; TRANSLATED; one canonical iter
8006AF1C   addiu a1, s0, 0xC                 ; delay
8006AF20  lw     v0, 0(s0)
8006AF24  nop
8006AF28  srl    v0, v0, 2
8006AF2C  sll    v0, v0, 2
8006AF30  addu   s0, s0, v0                  ; s0 = 0x8012A8B4
8006AF34  lw     v0, 0(s0)                   ; 0
8006AF38  nop
8006AF3C  bnez   v0, 0x8006AF18              ; NOT taken
8006AF40   addiu a0, s0, 4                   ; delay
.L8006AF44:
8006AF44  addiu  s0, zero, 1
8006AF48  addiu  v0, zero, -1
8006AF4C  beq    s2, v0, 0x8006ADD8          ; NOT taken (s2=1)
8006AF50   nop
8006AF54  jal    func_8006E7E8               ; FIRST UNIMPLEMENTED / CUT
8006AF58   nop
          ; NOT AUDITED PAST THIS POINT
```

Unreachable on canonical `s2=1` / lookup-hit / nonzero first word:

- `0x8006AF44` via the `beqz` at `0x8006AF10` (would skip the walk);
- `0x8006ADD8` via `s2 == -1`;
- `0x8006AE08` via `poll != 0`;
- `0x8006AF6C` .. `func_800718D0` via `poll == 0`;
- `func_80030894`.

The `beqz` skip-walk path is real retail but not the Disc 1 archive.

---

# Next-boundary selection

Outcomes considered:

| id | cut | why accepted or rejected |
| --- | --- | --- |
| A | stop after packing, before `0x8006AEFC` | correct but leaves an already-translated helper as the next step |
| B | packing + translated helpers | the honest prefix |
| C | stop before unresolved poll | the honest end of that prefix |
| D | stop before `func_800718D0` | requires an invented `poll=0` |
| E | other | none found |

Selected: **B + C**. Implement `0x8006AE68..0x8006AF54` (exclusive).
Stop on the `jal func_8006E7E8`. Do not implement `func_800718D0` or
`func_80030894`. Do not invent a poll. Do not add a DMA checkpoint.

```text
recommended_next_rung=C. synthesis + already-translated helpers, stop before unresolved poll
recommended_start_pc=0x8006AE68
recommended_end_pc=0x8006AF54
recommended_new_cut=0x8006AF54
first_unresolved_dependency=func_8006E7E8 return value at 0x8006AF54
```

---

# Report block

```text
base_commit=c1efff529e1529893c2ae73f78637b3677c30f77 (claimed; live checkout ABSENT on this host)
branch=phase6e-b-provider-frontier (claimed)
head=EVIDENCE_COMMIT_CONTAINING_THIS_REPORT

audit_start=0x8006AE68

literal_range_audited=0x8006AE68..0x8006AF54 (59 words; first unimplemented 0x8006AF54)

material_table_status=LITERAL_PACK_PROVEN
D_80091648_semantics=four 0x10-byte records; this path writes packed dest1/dest2 of records 2 and 3 only

func_8006E498_status=TRANSLATED_FAITHFUL
func_8006E498_args=a0=0x801229A0 a1=0xABADC06C
func_8006E498_result=0x801229A8

func_8007506C_status=TRANSLATED_FAITHFUL
func_8007506C_walk=1 record RECT{448,0,64,254} data=0x801229B4 terminator=0x8012A8B4

func_8006E7E8_status=TRANSLATED_FAITHFUL_RESULT_LIVE
func_8006E7E8_semantics=return func_800811E4; clear D_800B0CD8 0x01004000 iff st in {-1,0}
canonical_poll_result=UNRESOLVED

func_800718D0_reachable_on_canonical_path=NOT_YET
func_800718D0_callsite=0x8006AFA8
func_800718D0_args=a0=0x8012F1A0 if reached

func_80030894_reached=no

dma_checkpoint_required=no
hardware_progression_required=no

presentation_relevance=INDIRECTLY_PRESENTATION_RELEVANT

recommended_next_rung=C. synthesis + already-translated helpers, stop before unresolved poll
recommended_start_pc=0x8006AE68
recommended_end_pc=0x8006AF54
recommended_new_cut=0x8006AF54

first_unresolved_dependency=func_8006E7E8 return value at 0x8006AF54

production_files_changed=0
frontier_moved=no
poll_result_invented=no
new_dma_checkpoint_added=no

tests/oracles=B54A 8/8; B54B 8/8; B54C 8/8 against EXE SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
warnings=live repo path /home/blizz/dev/parasite-eve-port-black ABSENT; claimed checkpoint c1efff52 not locally verifiable; DMA2/queue after B54B extra LoadImages not recaptured; host CD sync collapse not used as poll_result
hard_blockers=none for this audit; poll result remains UNRESOLVED by design
```

Independent contract: `pc_port/tools/b54c_6ad40_material_table_oracle.py`.

STOP AFTER B54C.
