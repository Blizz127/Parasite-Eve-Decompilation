# func_80017294 — pb-small (slice C)

Landed as a matching retail C leaf. Leaf count 768 -> 774 (six-leaf batch).

## Span math

- VRAM `0x80017294`; file offset `0x7A94`; size `0x28`; unit `7818`.
- Carve: split the enclosing `[0x7818, asm]` run:
  `[0x7818, asm]` / `[0x7A94, c, func_80017294]` / `[0x7ABC, c, func_800172BC]`.
- `0x7A94 + 0x28 = 0x7ABC`, exactly the next existing span; no asm resume needed.

## Semantics

`D_8009CE00 (0x90($gp)) = *(int *)((char *)D_8009D2F0[0] + 0x9C) + (**arg0 << 1); return 1;`

The `pc_port` behavioural body (`func_80017018_port.c:175`) gives the same
fields (`GA_D_8009D2F0`, `+0x9C`, `GA_D_8009CE00`).

## Build profile

`era_o2_g8` (added to `configs/USA/disc1_build_profiles.json`).

## Matching lever

The 4-byte store is gp-relative (`sw $v1, 0x90($gp)`), so `-G8` is required.
Retail keeps `D_8009D2F0` **absolute** (`lui`/`lw`); under `-G8` a complete
`extern int *` would be placed in small data. Declaring it as an *incomplete
pointer array* (`extern int *D_8009D2F0[];`, then `D_8009D2F0[0]`) suppresses
the `.extern` size so cc1 emits the absolute form — the same lever documented
for `func_80021054`. No other divergence.

## Evidence

- Triage: `try_leaf.py src/func_80017294.c 0x7A94 0x28 --flags "-O2 -G8"` -> `WORDS MATCH`.
- Authority (fresh, this session): split plan `1120 spans (774 c, 344 asm, 2 rodata)`;
  `build_us.sh` -> `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
  `Matching claim: YES (774 registered C leaves)`; `verify_us.sh` ->
  `VERIFY_US=PASS`, all 774 packed C spans equal retail.
