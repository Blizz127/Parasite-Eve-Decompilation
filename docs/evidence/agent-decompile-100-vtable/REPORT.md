# D_8009566C vtable dispatch family — 6 matching C leaves (620–625)

- **Span**: `asm/disc1/621E4.s`, file `0x64494`–`0x645E8`, VRAM
  `0x80073C94`–`0x80073DB8`.
- **Members** (0x30 bytes each), differing only in the vtable slot:

| leaf | file | slot |
|---|---|---|
| func_80073C94 | 0x64494 | 0xC |
| func_80073CC4 | 0x644C4 | 8 |
| func_80073CF4 | 0x644F4 | 4 |
| func_80073D58 | 0x64558 | 0x14 |
| func_80073D88 | 0x64588 | 0x10 |
| func_80073DB8 | 0x645B8 | 0x18 |

- **Body**: `((void (*)(void)) *(unsigned int *)(D_8009566C + SLOT))();`
  under default `era_o2_g0`. The prologue-hoisted `lui`/`lw D_8009566C`
  falls out of the pointer global being read before the frame.
- `func_80073D24` (0x64524, 0x34) sits between members and is left `asm`.

## Proven by the harness

```sh
bash scripts/split_us.sh
podman run --rm --userns=keep-id -v /tmp/pe-agent-decomp:/workspace:Z \
  -w /workspace localhost/pe-mipsel-img:latest bash scripts/build_us.sh
```

```
Plan:      OK (921 YAML spans; no manual span lists)
Compile:   OK (625 generated C entries)
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (625 registered C leaves)
BUILD EXIT: 0
```

## Tried and left unmatched (recorded honestly)

- `func_80085DF4`/`func_80085E54` (clamp pair, 0x5C each): body matches except
  the epilogue, where retail fills the `jr` delay slot with
  `addiu sp,sp,0x18` while cc1 emits it before `jr` plus a nop. No flag rung
  tried (`-O1`, `-fschedule-insns2`, ASPSX 2.30, `MASPSX_FILL_STORE_DELAY_SLOT`)
  moves it. Sources were **not** committed.
- `func_800525EC` family (5-arg display call, 4 members): arg-store scheduling
  around the 5th stack argument differs; not committed.
- `func_80085F14` required the `era_o2_g0_fill_store_delay_slot` profile.
