# PE-B54G — B04C poll semantics

## Live helper

Same translated helper as B54E:

```text
st = func_800811E4(scratch)
if ((uint32_t)(st + 1) < 2)     /* st ∈ {-1, 0} */
    D_800B0CD8 &= 0xFEFFBFFF    /* clears 0x01004000 */
return st
```

B54G calls this helper. It does not write `s2 = 0` and does not
write `D_8009B6B4`.

## Host first sample

`D_800930EE` issued at `0x8006AF88` (`[197,200)`, dest
`lw(D_800B0CD8+0x180)`). `pe_libcd.c` completes that read before
`func_8006E6A8` returns and stores `D_8009B6B4 = 0`. VBlank at
the cut is 0, so the timeout arm is not taken.

The first host sample at `B04C` is therefore **0**. One poll, then
`B060`. `D_800B0CD8` loses `0x01004000` through the real RMW.
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
surface=boot CD poll at 0x8006B04C
retail_does=asynchronous 3-sector read; first sample may be 1, then 0
implemented_instead=host issue-time D_8009B6B4=0; first sample is 0
observable_difference_today=host takes B060 after one poll; retail may
  iterate AF9C/B044 while pending, then the same B060 fallthrough
promotion_trigger=when a later rung claims retail poll-count or
  mid-wait guest state at B04C, or when CD completion is no longer
  collapsed at issue
```

This is the same item recorded at AF54. It is not resolved by
treating host 0 as the retail first sample.

## What the wait still does correctly

| sample | production |
| --- | --- |
| `0` | fall through to `B060` |
| pending `!= 0` | `AF9C` → `B044` → poll again |
| `-1` | reissue `D_800930EE` via existing `func_8006E6A8`, then poll |

Canonical Disc 1 never takes the `-1` arm on the first visit
(`s2 = 1`). The arm is implemented so a later live `-1` is not
swallowed.
