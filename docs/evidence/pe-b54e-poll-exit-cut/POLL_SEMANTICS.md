# PE-B54E — AF54 poll semantics

## Live helper

`func_8006E7E8` (Phase 6E-B16, 19 words) is already translated:

```text
st = func_800811E4(scratch)
if ((uint32_t)(st + 1) < 2)     /* st ∈ {-1, 0} */
    D_800B0CD8 &= 0xFEFFBFFF    /* clears 0x01004000 */
return st
```

`func_800811E4`: `-1` if `vsync > issue_vsync + 1200`, else
`D_8009B6B4` (pending bytes). `0` = complete.

B54E calls this helper. It does not write `s2 = 0` and does not
write `D_8009B6B4`.

## Host first sample

Channel 2 issued at `0x8006ADF4` (`D_800930EC..EE` = `[180,197)`,
17 sectors, dest `lw(D_800B0CD8+0x174)`). `pe_libcd.c` completes
that read before `func_8006E6A8` returns and stores
`D_8009B6B4 = 0`. VBlank at the cut is 0, so the timeout arm is
not taken.

The first host sample at `AF54` is therefore **0**. One poll, then
`AF68`. `D_800B0CD8` loses `0x01004000` through the real RMW.
Focused tests treat that bit-clear as proof the helper ran: an
assigned `s2 = 0` would leave the busy bits set.

```text
host_first_sample_recorded=0
retail_first_sample=UNKNOWN
poll_assigned=no
s2_assigned=no
```

## Fidelity item — host collapse is not retail timing

```text
id=B54E-HOST-POLL-COLLAPSE
class=NONBLOCKING_FIDELITY
surface=boot CD poll at 0x8006AF54
retail_does=asynchronous 17-sector read; first sample may be 1, then 0
implemented_instead=host issue-time D_8009B6B4=0; first sample is 0
observable_difference_today=host takes AF68 after one poll; retail may
  iterate AE08/AF4C while pending, then the same AF68 fallthrough
promotion_trigger=when a later rung claims retail poll-count or
  mid-wait guest state at AF54, or when CD completion is no longer
  collapsed at issue
```

This is recorded, not "fixed" by inventing a busy-then-0 sequence
and not "resolved" by treating host 0 as the retail first sample.

Retail first-play still requires some later sample to be 0
(B54D: `6AD40` has one `jr $ra` after `B0AC`; `1220C` always
calls it). That is the ever-0 fact. It is not a first-sample fact.

## What the wait still does correctly

| sample | production |
| --- | --- |
| `0` | fall through to `AF68` |
| pending `!= 0` | `AE08` → `AF4C` → poll again |
| `-1` | reissue `D_800930EC` via existing `func_8006E6A8`, then poll |

Canonical Disc 1 never takes the `-1` arm on the first visit
(`s2 = 1`). The arm is implemented so a later live `-1` is not
swallowed.
