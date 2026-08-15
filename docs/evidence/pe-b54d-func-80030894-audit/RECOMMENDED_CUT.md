# PE-B54D — recommended B54E cut

B54A style: a prefix whose next work is an already-translated
helper, or a bounded sub-range. Do not leap to the first
unresolved function. Do not invent `poll=0`.

## Selected cut

```text
recommended_b54e_cut=0x8006AF68
cut_kind=prefix_to_already_translated_helper
helper=func_8006E7E8
range=0x8006AF54 .. 0x8006AF68   ; live wait, exclusive end
new_named_cut=func_8006AD40_prefix_cut @ 0x8006AF68
```

Implement only the wait/reissue already decoded at the live
frontier:

```text
/* s0 == 1, s2 == 1 at today's cut */
for (;;) {
    if (s2 == -1)
        reissue channel 2;          /* existing prefix at 0x8006ADD8 */
    s2 = func_8006E7E8();           /* TRANSLATED; live result */
    if (s2 == 0)
        break;
    /* s0 == 1 skips the texture loop: AE08 -> AF4C */
}
/* STOP at 0x8006AF68. Do not issue D_800930EE. */
```

That is the B54A analog. B54A finished the remaining
`func_8006E1C0` iterations and stopped at the loop exit
`0x8006AE68`. B54E should finish the remaining `func_8006E7E8`
samples at this site and stop at the wait exit `0x8006AF68`.

## Why this is the narrowest correct cut

| id | cut | verdict |
| --- | --- | --- |
| A | stay at `0x8006AF54` | leaves the already-translated poll unconsumed |
| **B** | **`0x8006AF68` after the live wait** | **selected** |
| C | `0x8006B04C` after `6E6A8` + `718D0` + packs | correct later, but a second rung; B54A did not also take packing |
| D | `0x8006B0AC` before `30894` | leaps over a second live poll; not narrowest |
| E | prefix of `func_80030894` | first jal is unresolved `GetTPage`; no translated prefix |
| F | assign `poll=0` and jump to `718D0`/`30894` | forbidden |

`func_800718D0` is translated (B54C) and sits on the `poll==0`
path. It is the next already-translated helper **after** the wait
exit, the same way packing sat after B54A's loop exit. Consume it
on a later rung, not this one.

## What B54E must not do

- Invent `s2 = 0` or skip `func_8006E7E8`.
- Enter `func_80030894`.
- Issue `D_800930EE` / `D_800930F0` or call `func_800718D0` from
  the live prefix.
- Add a DMA checkpoint or a periodic CD scheduler.

## Why not a bounded sub-range of `30894`

`func_80030894` is 788 words. Its first jal (`0x800308EC`) is
unresolved `func_80077A64` (`GetTPage`). Five later GPU-header
callees have matching C and no native port. Seven callees have
neither. There is no honest prefix inside the body.

## After B54E

If the live wait reports 0, the next audit/rung is the
already-translated `func_8006E6A8` + `func_800718D0` + record-0/1
pack stretch, stopping before the second live poll at
`0x8006B04C`. `func_80030894` remains the first unresolved
*function* on that later path.
