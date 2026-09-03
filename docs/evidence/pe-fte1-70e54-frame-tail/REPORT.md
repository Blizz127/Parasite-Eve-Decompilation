# PE-FTE1 — func_80070E54 frame tail matched and translated; field tick routed through it

Authority: retail Disc 1 executable `SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`; disassembly
`asm/disc1/60C1C.s:724` (70E54) and `asm/disc1/33744.s:63` (42FE8).

## Matching leaves (src/, era gate)

Two new byte-exact C leaves, both on era `-O2 -G8`
(`configs/USA/disc1_build_profiles.json`, `era_o2_g8`):

| Leaf | VRAM | Size | File offset | Carve |
|---|---|---|---|---|
| `func_80070E54` | `0x80070E54..0x80070FAC` | 0x158 (86 words) | `0x61654` | `[0x61654, c]`, resume `[0x617AC, asm]` |
| `func_80042FE8` | `0x80042FE8..0x80043038` | 0x50 (20 words) | `0x337E8` | `[0x337E8, c]`, `43038` follows directly |

Shape levers that mattered:
- `-G8` puts every ≤8-byte extern in sdata. Retail addresses `D_800B0CD8`
  and `D_800B0E54` absolutely (`lui/lw`), so both are declared as
  unknown-size arrays (`D_800B0CD8[0]`, `D_800B0E54[0]`) to keep them out
  of small data while `D_8009CDDC`/`D_8009CED8`/`D_8009CEDC` stay
  `$gp`-relative (`+0x6C`, `+0x168`, `+0x16C`).
- Retail loads the OT pointer as `lw $a0, 0x160($v0)` with `$v0 =
  &D_800B0CD8 + 4*CDDC`, reusing the `$a0` base it materialized for the
  second `& 0x200` test. The source therefore indexes
  `D_800B0CD8[0x58 + D_8009CDDC]` rather than naming `D_800B0E38`.
- The 6EC08 status test is `sll 24; bnez`: a `(signed char)` cast, not a
  byte mask (`andi 0xFF` was one word longer and mis-placed the branch).
- `(short)func_8006EBE4() >= 3` reproduces `sll 16 / sra 16 / slti 3`.

Fast loop: `tools/analysis/era_leaf_match.sh src/func_80070E54.c
0x80070E54 0x158 -O2 -G8` and `... src/func_80042FE8.c 0x80042FE8 0x50
-O2 -G8` differ from ROM only in relocation words (jal/lui/lo16/gp16);
the docker link resolves them.

## Native translation (pc_port, NOT a matching leaf)

`pc_port/game/boot/func_80070E54_port.c` translates all three functions:
- `func_80070E54`: `DrawSync(0)`; `42FE8()`; if `D_800B0CD8 & 0x200`:
  `VSync(4)`, `if ((short)6EBE4() >= 3) SetDispMask(1)`; else `VSync(2)`.
  `74A44(1)`; `PutDispEnv(D_800BCE80 + 20*CDDC)`; if `(char)6EC08() ||
  flag`: `PutDrawEnv(D_800BCDC8 + 92*CDDC)` else
  `DrawOTagEnv(B0E38[CDDC] + 0x3FFC, D_800BCDC8 + 92*CDDC)`; `CDDC =
  (CDDC == 0)`. `D_8009CDDC` is read and flipped in guest RAM (the word
  the 3F3C4 port already used); the 6E9A0 loop now reads the same word
  (`bootstrap/func_8006E9A0_port.c`), closing the host-int/guest split
  for this double-buffer index.
- `func_80042FE8`: `if (D_8009CED8 == 6) LoadImage({0,0x1E0,0x100,
  D_8009CEDC}, D_800B0E54)`; the RECT is a retail stack temporary and
  is built on the host stack because `func_8007506C` takes the host view.
- `func_8006EBE4`: `D_800B0DBA ? (short)D_800B0DBC : -1`.

The `psx_compat.h` stub `Bootstrap_ReturnVoid("func_80070E54",
"func_8006E9A0")` is removed. The 6E9A0 fade loop now executes the real
tail every poll; termination is unchanged (CFEE countdown in 68E24, two
iterations in the fixtures — see PE-FD1).

### 3F3C4 routing (Rung D)

`pc_port/game/boot/func_8003F3C4_port.c` no longer inlines a "70E54
live prefix": the retail `jal func_80070E54 @ 0x8003F590` is a call to the
real function. The field tick's draw therefore goes DrawOTagEnv →
`754E4` → `76C34(76B98)` chain walk (PE-DRW1) exactly as retail, and the
guest CDDC flip happens inside the tail. BTL78/79/80 (the old prefix
tests) pass unchanged against the real tail.

## Incidental fix — func_8007F0C8 packet loop (CDQ1 residual)

Fresh ASan/UBSan flagged a stack-buffer-overflow in
`pc_port/platform/pe_libcd.c:func_8007F0C8` (reached by the pre-existing
`B54KAD_fmv2_filename_threshold`). Three transcription errors from the
CDQ1 rung, all the same `sp+0x10` frame-base confusion:
1. The descriptor loop started at `fr + 0x30` and walked four packets,
   reading past the 0x60-byte frame. Retail (`0x8007F1AC`) sets `$s4 =
   sp+0x10` (= `fr[0]`) and walks `+0x10` per pass for `$s6 = 4`
   packets: command 9, SetMode 0x0E, SetLoc 2, then the ReadS command.
2. The `swl 0x34($sp) / swr 0x31($sp)` location bytes land at
   `fr[0x21..0x24]` (SetLoc packet bytes 1..4), not `fr[0x31..]`.
3. The `jal func_80080C48` delay slot (`0x8007F150`) stores `$v0 =
   sp+0x21` (address of the mode byte) into packet 1's gate word; it is
   nonzero, so the 80950 copy arm runs for SetMode. The port had
   documented that word as a "pre-call zero"; it now keeps a nonzero
   marker (the stack address has no guest meaning; only its zero test is
   consumed).
`CDQ1_7F0C8_queue_and_selector` now asserts the retail descriptor layout
(desc0 = 9, desc1 = 0x0E with mode copied to `+5` and `+0xC = desc+5`,
desc2 = 2 with the loc bytes copied, desc3 = 27 with a3). The CDQ1
report's "descriptor 0 carries the real packet" sentence is superseded
by this section. The completion-selector boundary is unchanged.

## Tests

7 focused `FTE1_*` tests: flag-clear path (VSync(2), present,
DrawOTagEnv splices the OT tail into the env link, DMA programmed,
CDDC→1); flag-set path (VSync(4), SetDispMask(1) at status ≥3,
PutDrawEnv link terminal); status <3 keeps the mask; 6EC08≠0 takes
PutDrawEnv; CDDC double-buffer flip 0→1→0 with env[1]/OT[1]; 42FE8 gate
(≠6 dispatches nothing, ==6 reaches the LoadImage dispatch boundary);
6EBE4 signed halfword. The stub guard test now also pins
`func_80070E54`/`func_80042FE8`/`func_8006EBE4`. The five 6E9A0-calling
fixtures seed `jtb[2]`/`jtb[6]` so the production DrawOTagEnv arm runs.

## Verify

```
python3 pc_port/tools/pe_fte1_70e54_oracle.py                 # PASS (exit 0)
tools/analysis/era_leaf_match.sh src/func_80070E54.c 0x80070E54 0x158 -O2 -G8
tools/analysis/era_leaf_match.sh src/func_80042FE8.c 0x80042FE8 0x50 -O2 -G8
scripts/split_us.sh
docker run --rm -v "$PWD:/workspace" -w /workspace --user "$(id -u):$(id -g)" \
  pe-mipsel-img:latest bash scripts/build_us.sh   # Compare: EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
scripts/verify_us.sh                              # VERIFY_US=PASS, matching-C count: 560
cmake --build pc_port/build -j
PE_TEST_FILTER=FTE1 ./pc_port/build/pe-native-tests   # 7 passed, 0 failed
./pc_port/build/pe-native-tests                       # Results: 1043 run, 1025 passed, 1 failed, 17 skipped
PE_DISC1_BIN=<Disc 1 .bin> ./pc_port/build/pe-native-tests   # Results: 1043 run, 1043 passed, 0 failed, 0 skipped
cmake -S pc_port -B pc_port/build-asan -DPE_PORT_SANITIZERS=ON && cmake --build pc_port/build-asan -j
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir pc_port/build-asan --output-on-failure   # see ASan line below
```
The single failure without a disc is the documented `B54KY_192CE8`
environment case (`missing local/pe_disc1.path`).

Docker gate this session: exit 0, `Compare: EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `sha1sum
build/disc1.candidate.exe` = `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`,
`grep -c ', c,' configs/USA/disc1.yaml` = 560 (was 558).

Strict production run (`--headless --strict-stubs --disc-image`) still
stops at `func_8007F0C8_completion_selector` (the selector is reached
before any New-Game frame; 70E54 sits on the New-Game path after it).
ASan/UBSan (fresh `build-asan`, leak check off): zero diagnostics; the ctest
"failure" is the same `B54KY_192CE8` disc-path environment case.
