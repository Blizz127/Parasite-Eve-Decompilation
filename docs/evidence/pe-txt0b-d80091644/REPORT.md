# PE-TXT0-B — D_80091644 and the retail font atlas

```text
PE-TXT0-B SUCCESS — RETAIL FONT ATLAS SOURCE IDENTIFIED
```

Evidence only. No TXT1. Production C and the `0x8006AF54` cut
were not changed.

```text
base_txt0 = 1d47df3
b54b      = 58eadcd
tool      = python3 tools/research/pe_txt0b_d80091644.py --peimg PE.IMG
exe       = 5d94938ee752e81ef375bd4493c9883850c25a86895f9cb0732cf3622b44351b
```

## 1. What D_80091644 is

`func_80037870` does `lui/addiu $fp, 0x80091644` and reads:

```text
lhu  0xC($fp)   # 0x80091650  tpage
lhu  0xE($fp)   # 0x80091652  CLUT
```

`D_80091644` is the **address base**, not a pointer word. The word
at 91644 is 0 and has no store in the EXE.

`D_80091648` is a 4×0x10 array. Record 0 dests **are** 91650/91652.

## 2. Writers

| Dest | PC | Value | When |
|---|---|---|---|
| 91644 | none | 0 | link |
| 91648–4E A/B/C/D | none | 0x140/0/0x140/0xFC | link |
| **91650 tpage** | **0x8006AFF8** | **0x0025** | after 718D0, a1=0 |
| **91652 clut** | **0x8006B02C** | **0x3F14** | same |
| 91660/62 | same PCs | 0x26 / 0x3F15 | a1=0x10 |
| 91670/72, 91680/82 | 0x8006AEB0 / AEE4 | 0x34 / 0x7793, 0x34 / 0x7753 | B54D records 2–3 |

The font pair is only the a1=0 iteration of the pack loop at
`0x8006AFB4` (`a1 < 0x20`). That loop is **after** the live cut.

## 3. Atlas upload

Channel-2 range `D_800930EC=180` .. `D_800930EE=197` is PE.IMG
`[180,197)`, a TIM (`flag=8`). `func_800718D0` at `0x8006AFA8`
LoadImages it after `func_8006E7E8` returns 0.

```text
CLUT  {320, 252, 16, 1}
IMAGE {320, 0, 64, 256}     256x256 4bpp
tpage 0x0025
clut  0x3F14
```

256 ≥ 252. Packed dests from record 0 A/C/D **equal** those RECTs.
B54B's 13-entry table is sectors 157–180 and is not this TIM.

## 4. Adjacent

`D_80091694`: `func_80052594`. `D_800B1628/162C`: no store in
SLUS_006.62.

---

```text
base_txt0_commit=1d47df3
b54b_commit=58eadcd

d80091644_writer_count=1
d80091644_writer_pcs=0x8006AFF8,0x8006B02C
tpage_value=0x0025
clut_value=0x3F14

atlas_upload_site=func_800718D0@0x8006AFA8
font_atlas_rect={320,0,64,256}
font_clut_rect={320,252,16,1}
font_resource_status=PROVEN

d800b1628_writer=UNKNOWN_NOT_IN_SLUS
d80091694_runtime_writer=func_80052594

txt1_implementation_ready=YES

hard_blockers=none
unknowns=D_800B1628/162C writer (not in SLUS_006.62); 0x4B glyph
warnings=atlas LoadImage and record-0 pack sit after 0x8006AF54; this rung does not implement them; record 1 dests 0x26/0x3F15 are packed here but are not the 37870 $fp pair

SUCCESS

PE-TXT0-B SUCCESS — RETAIL FONT ATLAS SOURCE IDENTIFIED
```
