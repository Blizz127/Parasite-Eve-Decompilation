# PE-B54F — D_800930EE issue and live font atlas

```text
PE-B54F SUCCESS — D_800930EE ISSUE AND FONT ATLAS UPLOAD ON LIVE PREFIX
```

Base PE-B54E `81efe08`. Production now consumes
`0x8006AF68..0x8006B04C` and names the same
`func_8006AD40_prefix_cut` at **`0x8006B04C`**.

The second poll is not consumed. `poll` / `s2` are not assigned.
`func_80030894` is not taken.

## Isolation

```text
checkout=/var/home/blizz/dev/parasite-eve-port-black
branch=phase6e-b-provider-frontier
base_b54e_commit=81efe08
```

One local commit. Not pushed.

## Implemented window

57 words. Issue `D_800930EE` (`[197,200)`, dest `+0x180`) through
already-translated `func_8006E6A8`. Walk the previous TIM at
`dest+0x174` through `func_800718D0`. Pack records 0 and 1.
Stop before `jal func_8006E7E8` at `0x8006B04C`.

The font atlas is now on the live prefix: record 0 is
`0x0025` / `0x3F14`, image `{320,0,64,256}`, CLUT
`{320,252,16,1}`.

Record 1 packs `0x0026` / `0x3F15` from EXE `D_80091658`
`{384,0,336,252}`. It is not assumed to be a second font.

Host `D_8009B6B4=0` after the new issue is
`B54E-HOST-POLL-COLLAPSE`. Busy bits stay set because the
second poll is not consumed.

Framebuffer digest is unchanged: atlas VRAM x=320 is off the
presented display.

## Frontier

```text
frontier=func_8006AD40_prefix_cut @ 0x8006B04C
next_unresolved=second live func_8006E7E8 poll @ 0x8006B04C; first unresolved function remains func_80030894 @ 0x8006B0AC
```

---

```text
base_b54e_commit=81efe08
cut_implemented=0x8006B04C
words=57

d800930ee_issue_implemented=yes
func_800718D0_reached=yes
atlas_uploaded_on_live_path=yes
atlas_rect={320,0,64,256}
atlas_clut_rect={320,252,16,1}

record0_tpage=0x0025
record0_clut=0x3F14
record1_packs=0x0026/0x3F15
record1_source=EXE D_80091658 {384,0,336,252}

second_poll_consumed=no
poll_assigned=no

frontier=func_8006AD40_prefix_cut @ 0x8006B04C
next_unresolved=second live func_8006E7E8 poll @ 0x8006B04C; first unresolved function remains func_80030894 @ 0x8006B0AC

gates=native 576/576; ASan+UBSan 576/576; B54F 2/2; B54E 2/2; B54C 2/2; B54D 2/2; D 8/8; C 10/10; B2 15/15; B1 8/8; H 8/8; B53B 15/15; B54A/B/C oracles 8/8 each; B54C-718D0 8/8; B54E oracle 8/8; B54F oracle 8/8; exe-taking oracles 50/50; B49 PASS normal and ASan+UBSan
framebuffer=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
fnv=7D860391E1ED6C97
matching_exe=452fb033f2eaa4b18aa20a5bca60b8125af3a37b

hard_blockers=
unknowns=retail first sample at B04C; record-1 consumer; D_800930EE TIM layout beyond flag/CLUT header
warnings=host D_8009B6B4=0 after D_800930EE is B54E-HOST-POLL-COLLAPSE not retail completion; record 1 is not assumed to be a second font; atlas VRAM x=320 leaves the presented framebuffer digest unchanged

SUCCESS

PE-B54F SUCCESS — D_800930EE ISSUE AND FONT ATLAS UPLOAD ON LIVE PREFIX
```
