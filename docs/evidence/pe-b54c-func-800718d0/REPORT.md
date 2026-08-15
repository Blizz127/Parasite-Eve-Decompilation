# PE-B54C — `func_800718D0` font atlas upload

```text
PE-B54C SUCCESS — FONT ATLAS UPLOAD TRANSLATED
```

Base PE-B54B `58eadcd` (counted loop live at `c1efff5`). Production
prefix is the B54D cut at `0x8006AF54`. This rung translates the
29-word TIM walker and the two record-0 pack stores. It does not
invent `poll=0`, does not enter `func_80030894`, and does not add
a third DMA checkpoint.

## Isolation

```text
checkout=/var/home/blizz/dev/parasite-eve-port-black
branch=phase6e-b-provider-frontier
base_b54b_commit=58eadcd
cut_live=c1efff5
b54d_prefix=630f5ac
```

One local commit. Not pushed.

## Function

`func_800718D0` (`0x800718D0..0x80071944`, 29 words) walks a TIM
at `a0`. Flag bit 3 selects a CLUT chunk. It calls the existing
`func_8007506C` for the **image first**, then the CLUT. Return is
the image pixel pointer. See `LITERAL_RECONSTRUCTION.md`.

## Packs

`0x8006AFF8` / `0x8006B02C` are the `sh` dests of the
`a1=0,0x10` loop. Record 0 becomes `0x0025` / `0x3F14`. Exposed
as `PE_func_8006AD40_PackFontRecords`. The live `func_8006AD40`
prefix still packs only records 2 and 3.

## Frontier

Live `func_8006AD40` cut stays `func_8006AD40_prefix_cut` at
`0x8006AF54`. Next unresolved **function** after the packs is
`func_80030894` at `0x8006B0AC`.

---

```text
base_b54b_commit=58eadcd
function=func_800718D0
words=29
callees=func_8007506C

atlas_loadimage_implemented=yes
atlas_rect={320,0,64,256}
clut_rect={320,252,16,1}
record0_pack_implemented=yes
tpage_written=0x0025
clut_written=0x3F14

frontier=func_8006AD40_prefix_cut @ 0x8006AF54
next_unresolved=func_80030894 @ 0x8006B0AC

gates=native 572/572; ASan+UBSan 572/572; B54C 2/2; B54D 2/2; D 8/8; C 10/10; B2 15/15; B1 8/8; H 8/8; B53B 15/15; B54A/B/C oracles 8/8 each; B54C-718D0 oracle 8/8; exe-taking oracles 49/49; B49 PASS
framebuffer=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
fnv=7D860391E1ED6C97
matching_exe=452fb033f2eaa4b18aa20a5bca60b8125af3a37b

txt1_prerequisites_met=YES

hard_blockers=
unknowns=func_80030894 body (0xC50); whether canonical poll at AF54 is ever 0 on this Disc 1 prefix
warnings=718D0+packs are not reached from the live 6AD40 prefix; poll=0 was not assigned; image LoadImage precedes CLUT (literal jal order)

SUCCESS

PE-B54C SUCCESS — FONT ATLAS UPLOAD TRANSLATED
```
