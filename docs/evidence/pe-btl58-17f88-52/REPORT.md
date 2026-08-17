# PE-BTL58 — opcode 0x52 / 0x53 / 0xA6

## Handlers

| op | VA | words | SHA-256 |
|---|---|---|---|
| 0x52 | `0x80017F88` | 10 | `ed1c623197818f92f87ae1fc3b82baa3d14f84a7bb66e9472a1876bc1348bb2b` |
| 0x53 | `0x80017FB0` | 11 | `3594551d87ad74ca023d6f16c06a295719ee38c4a07dc9edaa698b6e315f259f` |
| 0xA6 | `0x80019484` | 11 | `731901885ba4878c7e07bcaf57d1106489875f5a4dcafb7b08a690dd78b64a3f` |

`D_800910A0[op]` matches. Both 0x52 and 0x53 write `D_8009D1A0` (`0x800A0000-0x2E60`). 0x52 ORs `*arg0`. 0x53 ANDs `~*arg0`. Delay-slot `addiu $v0,$zero,1`.

0xA6 `jal 0x800438C0(*arg0)` then `v0=1`. 438C0 is the existing masked `D_8009CEF0` setter.

## Live type-0 Watch arm

After 0x43 choice 0, persist[0] bit 1 is set and the script `goto +0xBD4`. Persist[0xA] is still 0 after 34F10, so `persist[0xA] & 0x80000000` is false and the script takes `+0xCF0` 0x53 (clear 0x800) then 0xA6 imm `0x7F`.

The persist-bit-set arm yields at `+0xC70` then hits 0x52 imm `0x800` on the next visit. Do not force persist[0xA] bit 31.

## Host scalar

`D_8009D1A0` is the host `unsigned int`, the same word 3E974 / 35558 / 0x3A already use. Do not `PE_LoadU32(0x8009D1A0)`.
