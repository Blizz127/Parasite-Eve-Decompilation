# PE-B54G — 6AD40 sequence park

After the second poll's `poll==0` fallthrough at `0x8006B060`,
the `func_8006AD40` prefix is **PARKED**.

## Why

The next retail work is already-translated `func_8006E6A8`
(`D_800930F0` / dest+0x14C) plus another `func_800718D0`, then
`jal func_80030894` at `0x8006B0AC`.

`func_80030894` is the first real wall:

- 788 words / `0xC50`
- one `jr $ra`, no `jalr`
- twelve direct callees; seven still unresolved
- first jal is unresolved `GetTPage` (`func_80077A64`)
- no honest translated prefix inside the body (B54D)

PE-GPU1 already ported the five matching-C SET leaves
(`SetPolyF3`/`FT4`/`G4`, `SetTile`, `SetSprt`). That workstream
landed. It does not create a prefix inside `30894`.

Continuing `6AD40` past `B060` only to stop on that wall does not
advance a faithful frontier.

```text
sequence_parked=yes
park_reason=func_80030894 is 788 words with no translated prefix; first jal is unresolved GetTPage; PE-GPU1 ported the five SET leaves but 30894 remains the wall
frontier=func_8006AD40_prefix_cut @ 0x8006B060
next_unresolved=func_80030894 @ 0x8006B0AC
```

B54G does not issue `D_800930F0`, does not call the later
`718D0`, and does not enter `30894`.
