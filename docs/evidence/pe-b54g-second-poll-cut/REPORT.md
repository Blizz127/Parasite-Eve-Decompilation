# PE-B54G — second poll cut; 6AD40 sequence parked

```text
PE-B54G SUCCESS — SECOND POLL CONSUMED; 6AD40 SEQUENCE PARKED
```

Base PE-B54F `f900b97`. Production now consumes
`0x8006B04C..0x8006B060` and names the same
`func_8006AD40_prefix_cut` at **`0x8006B060`**.

`poll` / `s2` are not assigned. `func_80030894` is not taken.
`D_800930F0` is not issued.

## Isolation

```text
checkout=/var/home/blizz/dev/parasite-eve-port-black
branch=phase6e-b-provider-frontier
base_b54f_commit=f900b97
```

One local commit. Not pushed.

## Implemented window

5 words / `0x14`. B54E-shaped wait around live `func_8006E7E8`
for the already-issued `D_800930EE` range. Canonical entry
`s0=1`, `s2=1`. Busy returns to `AF9C`/`B044`; `-1` reissues
through the existing `0x8006AF6C` `func_8006E6A8`; `0` stops at
`B060`. See `LITERAL_RECONSTRUCTION.md`.

## Host timing

The first host sample is 0 (`D_8009B6B4` issue-time collapse).
That is `B54E-HOST-POLL-COLLAPSE`, not a retail first-sample
constant. Retail first sample remains UNKNOWN. See
`POLL_SEMANTICS.md`.

## Park

After this cut the `6AD40` sequence is parked. Next retail work
is `D_800930F0` then `func_80030894` @ `0x8006B0AC`: 788 words,
7 unresolved callees, no translated prefix. Five matching-C
GPU-header leaves have no native port and must land first.
See `PARK.md`.

## Coverage

HostFB is a 320×240 host RGB buffer, proven unaliased to PSX
VRAM by `test_B53B_authority_separation_guard`. The framebuffer
digest cannot observe any LoadImage. Focused LoadImage/pack
assertions remain the upload gate, not `fb28dc21…`.

---

```text
base_b54f_commit=f900b97
cut_implemented=0x8006B060
words=5
poll_assigned=no
s2_assigned=no
host_first_sample_recorded=0
retail_first_sample=UNKNOWN

sequence_parked=yes
park_reason=func_80030894 is 788 words with no translated prefix; first jal is unresolved GetTPage; PE-GPU1 ported the five SET leaves but 30894 remains the wall

frontier=func_8006AD40_prefix_cut @ 0x8006B060
next_unresolved=func_80030894 @ 0x8006B0AC

gates=native 579/579; ASan+UBSan 579/579; B54G 2/2; B54F 2/2; B54E 2/2; B54C 2/2; B54D 2/2; D 8/8; C 10/10; B2 15/15; B1 8/8; H 8/8; B53B 15/15; B54A/B/C oracles 8/8 each; B54C-718D0 8/8; B54E oracle 8/8; B54F oracle 8/8; B54G oracle 8/8; exe-taking oracles 51/51; B49 PASS normal and ASan+UBSan
framebuffer=fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb
fnv=7D860391E1ED6C97
matching_exe=452fb033f2eaa4b18aa20a5bca60b8125af3a37b

hard_blockers=
unknowns=retail first sample at B04C (0 vs 1-then-0)
warnings=host D_8009B6B4=0 is B54E-HOST-POLL-COLLAPSE not retail timing; poll=0 and s2=0 were not assigned; framebuffer digest is blind to LoadImage — focused tests are the upload gate; 6AD40 sequence is parked before 30894

SUCCESS

PE-B54G SUCCESS — SECOND POLL CONSUMED; 6AD40 SEQUENCE PARKED
```
