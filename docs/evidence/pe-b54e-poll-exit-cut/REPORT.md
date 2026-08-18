# PE-B54E — AF54 poll-exit cut

```text
PE-B54E SUCCESS — AF54 POLL EXIT CUT TAKEN
```

Base PE-B54D `7d84397` (parent `fc5b5cd`). Production now consumes
the live wait at `0x8006AF54..0x8006AF68` and names the same
`func_8006AD40_prefix_cut` at **`0x8006AF68`**.

`poll=0` and `s2=0` are not assigned. `func_800718D0`,
`func_80030894`, and the `D_800930EE` issue are not taken.

## Isolation

```text
checkout=/var/home/blizz/dev/parasite-eve-port-black
branch=phase6e-b-provider-frontier
base_b54d_commit=7d84397
```

One local commit. Not pushed.

## Implemented window

5 words / `0x14`. Canonical entry `s0=1`, `s2=1`. Live
`func_8006E7E8`; busy returns to `AE08`/`AF4C`; `-1` reissues
channel 2 through the existing `0x8006ADD8` `func_8006E6A8`; `0`
stops at `AF68`. See `LITERAL_RECONSTRUCTION.md`.

## Host timing

The first host sample is 0 (`D_8009B6B4` issue-time collapse).
That is recorded as `B54E-HOST-POLL-COLLAPSE`, not used as a
retail first-sample constant. Retail first sample remains
UNKNOWN. See `POLL_SEMANTICS.md`.

## Frontier

```text
frontier=func_8006AD40_prefix_cut @ 0x8006AF68
next_unresolved=D_800930EE issue via func_8006E6A8 @ 0x8006AF88 (translated); first unresolved function remains func_80030894 @ 0x8006B0AC
```

---

```text
base_b54d_commit=7d84397
cut_implemented=0x8006AF68
words=5

poll_assigned=no
s2_assigned=no
host_first_sample_recorded=0
retail_first_sample=UNKNOWN

frontier=func_8006AD40_prefix_cut @ 0x8006AF68
next_unresolved=D_800930EE issue via func_8006E6A8 @ 0x8006AF88 (translated); first unresolved function remains func_80030894 @ 0x8006B0AC

gates=native 574/574; ASan+UBSan 574/574; B54E 2/2; B54C 2/2; B54D 2/2; D 8/8; C 10/10; B2 15/15; B1 8/8; H 8/8; B53B 15/15; B54A/B/C oracles 8/8 each; B54C-718D0 8/8; B54E oracle 8/8; exe-taking oracles 49/49; B49 PASS normal and ASan+UBSan
framebuffer=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
fnv=7D860391E1ED6C97
matching_exe=452fb033f2eaa4b18aa20a5bca60b8125af3a37b

hard_blockers=
unknowns=retail first sample at AF54 (0 vs 1-then-0); D_800930EE payload after AF68
warnings=host D_8009B6B4=0 is issue-time collapse not retail timing; recorded as B54E-HOST-POLL-COLLAPSE; poll=0 and s2=0 were not assigned

SUCCESS

PE-B54E SUCCESS — AF54 POLL EXIT CUT TAKEN
```
