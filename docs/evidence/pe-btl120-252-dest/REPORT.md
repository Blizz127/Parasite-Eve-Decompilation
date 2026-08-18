# PE-BTL120 — Aya+0x252 is dest+0x9E; 3C818 clears it

Retail EXE SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`Aya+0x1B4` is an embedded dest. Therefore:

| Actor | Dest |
|-------|------|
| `+0x250` | `dest+0x9C` |
| `+0x252` | `dest+0x9E` |

`24A3C` case 2 `sb 1, Aya+0x252` is `dest+0x9E=1`.
Case 0 `+0x250|=2` is `dest+0x9C` bit 1.

TEXT has no `sb $0, 594(actor)`. The zero store is
`sb $0, 158(dest)` at `0x8003C87C` inside `3C818`
when `dest+0x8C==1`.

`35558` dest walk (`359A0` `lw D20C`) sets `s0=actor+0x1B4`
and jals `3AF14`. `3AF14` (`dest+0` and `lh +0xBA` nonzero)
takes `dest+0x9C&2` → `3C818`.

`3C818` +0x8C machine:

| +0x8C | Action |
|-------|--------|
| 0 | `sb -1`, return |
| 1 | `dest+0x9E=0`, then `--` |
| <0 | copy `+0x8D` → `+0x8C`, then `--` |
| ≥2 | fade deferred, then `--` |

Case 2 `3C5D8(dest, 30)` writes `+0x8D=30`. After the
0→-1 arm, one copy tick, then 29 decrements reach 1
and clear `+0x252` during case 3's 30-tick `CE4C` drain
if `35558` runs each frame.

`dest+0==0` or `+0xBA==0` is fail-closed (no clear).
That is the BTL115 prefix negative: BSS dest does not
clear. Do not plant `+0x252=0`.

`3C2E0` / `3B144` / `3C638` / `3CCB0` / `3CEF8` stay
deferred. `6F39C(0x6C)` overlay is not this clearer.

## Verify

```text
python3 pc_port/tools/pe_btl120_252_dest_oracle.py
PE_TEST_FILTER=BTL120 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
