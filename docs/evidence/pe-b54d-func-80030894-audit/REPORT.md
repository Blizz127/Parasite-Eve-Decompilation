# PE-B54D — `func_80030894` audit

```text
PE-B54D SUCCESS — FUNC_80030894 AUDITED AND B54E CUT RECOMMENDED
```

Audit only. Production C is unchanged. The live named cut stays
`func_8006AD40_prefix_cut` at `0x8006AF54`. `poll=0` is not
assigned.

## Isolation

```text
checkout=/var/home/blizz/dev/parasite-eve-port-black
branch=phase6e-b-provider-frontier
base_b54c_commit=fc5b5cd
exe=build/disc1.candidate.exe
exe_sha1=452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

Words cited below were re-read from that executable
(`taddr = 0x80010000`, file offset `pc - 0x80010000 + 0x800`).
B54C oracles re-run against it: `718D0` 8/8, material-table 8/8.

## Function

```text
func_80030894
range          0x80030894 .. 0x800314E4 (exclusive)
bytes          0xC50
words          788
ABI            void func_80030894(void)
               first a0 write is addu a0, zero, zero at 0x800308E0
prologue       addiu sp,-0x58; sw ra/fp/s7..s0
return_sites   1
               0x800314DC jr $ra / 0x800314E0 nop
indirect_calls 0 jalr, 0 j, 0 bltzal/bgezal
mmio_cop       0
next_func      0x800314E4 addiu sp,-0x20
```

Body is a two-pass (`lbu sp+0x18`; `sltiu < 2`; back-edge
`0x800314A8 → 0x80030910`) GPU-primitive table builder. It copies
three bytes from `D_8009CD90`, then:

```text
0x800308EC  jal func_80077A64   GetTPage(0, 1, 0x100, 0x1E0) = 0x0034
0x800308FC  jal func_80077AA4   GetClut(0x130, 0x1F8)        = 0x7E13
0x80030910  jal func_8005DADC   table id 0x8B
0x80030938  jal func_80077BA4   SetPolyFT4 at 0x800BE9F0 + i*40
```

Those tpage/CLUT values are **not** the B54C font pair
`0x0025` / `0x3F14`. This is not the atlas uploader.

Eleven internal branches stay inside the body. See `CALLEES.csv`.

## Callees

42 direct `jal` to 12 unique targets. None is a native-port
translation.

| class | n | names |
| --- | --- | --- |
| UNRESOLVED | 7 | `370DC`(18), `37140`(1), `5DADC`(2), `77A64` GetTPage(5), `77AA4` GetClut(2), `77B04` SetSemiTrans(2), `77B34` SetShadeTex(3) |
| MATCHING_C_LEAF_NO_PORT | 5 | `77B64` SetPolyF3, `77BA4` SetPolyFT4, `77BC4` SetPolyG4, `77C44` SetTile, `77C64` SetSprt |

First jal is unresolved `GetTPage`. There is no prefix of
already-translated helpers inside `30894`.

## Poll at `0x8006AF54`

B54C left this open. Resolved: the site is **ever 0** on this
Disc 1 prefix. It is not assigned.

`func_8006E7E8` is translated. Canonical `s0=1`, `s2=1` makes
`AF54` a wait/reissue loop (busy → poll again; `-1` → reissue
channel 2; `0` → `AF68`). `func_8006AD40` has one `jr $ra`
(`0x8006B354`); `func_8001220C` always `jal`s it at
`0x800122C4`; first-play field starts only after that return.
A completed Disc 1 boot is therefore the observation that some
`AF54` sample was 0. Host `D_8009B6B4` is already 0 from the
issued 17-sector channel-2 read (`D_800930EC..EE` = `[180,197)`),
so the first host sample is 0. Retail may sample `1` first. Full
argument: `REACHABILITY.md`.

```text
poll_at_AF54_ever_zero=yes
poll_assigned=no
```

## Reachability

```text
reachable_from_canonical_prefix=yes
```

Sole caller `0x8006B0AC` in boot `func_8006AD40`.
`func_8001220C @ 0x800122C4` is the sole, unconditional caller
of `6AD40`. `B0AC` is the success continuation after `AF54==0`
and a second live poll at `0x8006B04C` (`s0` is 0 on the first
visit to `B098`). First-play field starts only after `1220C`
returns. Today's native run still stops at `AF54`, so `30894`
is not executed yet.

## Recommended B54E cut

B54A style: finish the already-translated helper that the live
cut is sitting on, stop at the wait exit.

```text
recommended_b54e_cut=0x8006AF68
cut_rationale=AF54 is a wait/reissue loop around translated func_8006E7E8; consume the live poll (do not assign 0) and stop at the poll==0 fallthrough; do not leap to 718D0 or 30894
```

`func_80030894` remains the first unresolved *function* on the
later path. Its own body is not the next rung.

## Preservation

This rung adds only:

- `docs/evidence/pe-b54d-func-80030894-audit/`
- the handoff paragraph above

No production C. Frontier not moved. No DMA checkpoint. No CD
progression.

Production binaries are the B54C tree. Gates below are inherited
from `fc5b5cd` and were not re-run; matching EXE SHA-1 and the
two B54C oracles were rechecked.

---

```text
base_b54c_commit=fc5b5cd
function=func_80030894
words=788
callees=12
unresolved_callees=7
indirect_calls=0
return_sites=1

reachable_from_canonical_prefix=yes
poll_at_AF54_ever_zero=yes
poll_assigned=no

recommended_b54e_cut=0x8006AF68
cut_rationale=AF54 is a wait/reissue loop around translated func_8006E7E8; consume the live poll (do not assign 0) and stop at the poll==0 fallthrough; do not leap to 718D0 or 30894

gates=inherited B54C fc5b5cd native 572/572; ASan+UBSan 572/572; B54C 2/2; B54D 2/2; D 8/8; C 10/10; B2 15/15; B1 8/8; H 8/8; B53B 15/15; B54A/B/C oracles 8/8 each; B54C-718D0 oracle 8/8 rechecked; B49 PASS
framebuffer=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
fnv=7D860391E1ED6C97
matching_exe=452fb033f2eaa4b18aa20a5bca60b8125af3a37b

hard_blockers=
unknowns=retail first sample at AF54 (0 vs 1-then-0); D_800A8030[0x8B] payload; full 30894 primitive layout
warnings=host D_8009B6B4=0 is issue-time collapse not retail timing; poll=0 was not assigned; 30894 tpage/clut 0x0034/0x7E13 is not the font atlas 0x0025/0x3F14; five matching GPU-header leaves have no native port

SUCCESS

PE-B54D SUCCESS — FUNC_80030894 AUDITED AND B54E CUT RECOMMENDED
```
