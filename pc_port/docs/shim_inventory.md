# Shim Inventory — Phase 6D-R

Bootstrap stubs invoked in the `func_8001220C` (main) → first-clear path.
All stubs are explicitly classified. No anonymous empty stubs.

## Real translated functions (not stubs)

| Function | Size | Words | Port file |
|----------|------|-------|-----------|
| `func_8001220C` | - | 187 | `bootstrap/func_8001220C_port.c` |
| `func_800698D4` | - | - | `bootstrap/func_800698D4_port.c` |
| `func_8006E834` | - | 91 | `bootstrap/func_8006E834_port.c` |
| `func_8006E9A0` | - | - | `bootstrap/func_8006E9A0_port.c` |
| `func_8003E610` | 0x70 | 28 | `game/boot/func_8003E610_port.c` |
| `func_8003E680` | 0xD4 | 53 | `game/boot/func_8003E680_port.c` |
| `func_8006A5BC` | 0x90 | 36 | `game/boot/func_8006A5BC_port.c` |
| `func_8006A64C` | 0x28 | **10** | `game/boot/func_8006A64C_port.c` |
| `func_8006A674` | 0x260 | 152 | `game/boot/func_8006A674_port.c` |
| `func_8006A8D4` | 0x110 | **68** | `game/boot/func_8006A8D4_port.c` |

## IMPLEMENTED (real host equivalents)

| Function | PS1 role | Host implementation |
|----------|----------|---------------------|
| `func_80073A44` | VSync | `HostFB_VSync()` |
| `func_80074D28` | SetDispMask | `HostFB_SetDispMask()` |
| `func_80074DC0` | DrawSync | `HostFB_DrawSync()` |
| `func_80074F44` | ClearImage | `HostFB_ClearImage()` |
| `func_800755F0` | PutDispEnv/Present | `HostFB_Present()` |

## HOST_ADAPTED

| Function | Adaptation |
|----------|------------|
| `scratchpad_stack_handoff` | Native stack call, no MIPS scratchpad manipulation |

## BOOTSTRAP_RET (51 stubs invoked in first-clear path)

### func_8001220C callees
- `func_800725DC`
- `func_8006A9E4`
- `func_8006AD40`
- `func_8006ECEC`
- `func_8006F044`
- `func_80069B08(int)`
- `func_8003F3C4`
- `func_801235DC`
- `func_8019234C`
- `func_801909B4`

### func_8003E610 callees (10)
- `func_80073C94`
- `func_8003E754(int, int)`
- `func_8007D054`
- `func_80077F7C`
- `func_80079004(int, int)`
- `func_80079024(int)`
- `func_800409B4`
- `func_8003E944`
- `func_8007EC14`
- `func_80080CC8(int)`

### func_8006A5BC callees (7)
- `func_80085644`
- `func_80086FF8`
- `func_80087024`
- `func_8008682C(int)`
- `func_8007ED58` (returns 1)
- `func_8007F72C` (CdReady — strict-mode gate)
- `func_8007F7A8` (returns 0)

### func_800698D4 callees (5)
- `func_8007F72C` (shared with 6A5BC)
- `func_8007F778`
- `func_80082314`
- `DsSearchFile(PEDISC01.IDF)`
- `DsSearchFile(PE.IMG)`
- `func_80080C48`

### func_8003E680 callees (2000 + 16)
- ~~`func_80070D10`~~ — TRANSLATED (Phase 6E-B1): lagged-Fibonacci RNG table
  init, `game/boot/func_80070D10_port.c`
- ~~`func_80070D6C`~~ (×2000) — TRANSLATED (Phase 6E-B2): lagged-Fibonacci
  RNG advance, `game/boot/func_80070D6C_port.c`; read cursor cycles through
  the 2 index words + 14 retail code words below the table (verbatim
  `|= 0x40` wrap), so retail-exact output needs the exe image in guest RAM
  (`pe_guest_image`); ranged wrapper `func_80070DD0` also translated
- ~~`func_8003E974`~~ — TRANSLATED (Phase 6E-B3): state zeroing
  (5 $gp-relative globals + 32-word array D_800A76F0) + ordered series of 20
  `func_8003EAC8(mask,value)` registration calls, `game/boot/func_8003E974_port.c`
- `func_8003EAC8`  ← current strict-mode frontier (from func_8003E974):
  GTE LZCS/LZCR highest-set-bit-index leaf (0x80000000 → slot 31 special
  case), writes D_800A76F0[idx]=a1, return unused; 63 call sites exe-wide;
  `game/boot/func_8003EAC8_port.c` stays an unresolved provider with bounded
  test-only call recording (PE_3EAC8_RecordReset/Count/At)
- `func_80036DC8`
- `func_80073D24` (×2 — callback register: reset + register)
- `func_800371A4(int)`
- `func_80029388`
- `func_8005BCA8`
- `func_80068D28`
- `func_800124F8`
- `func_8001A890`
- `func_80034F10`
- `func_8006536C`
- `func_80038D1C`

### func_8006E834 callees (9)
- `func_80086FF8` (shared with 6A5BC)
- `func_8006E6D4(int,int,uchar*,int)`
- `func_800811E4(void*)`
- `func_80072714`
- `func_800726C4`
- `func_80072724`
- `func_800749D8(SetDefDispEnv)`

### func_8006E9A0 callees (7)
- `func_8005E588`
- `func_80066B60(int)`
- `ClearOTagR`
- `func_80068E24`
- `func_80070E54`
- `func_80038D1C` (shared with 3E680)

## Remaining bootstrap providers

func_8007F72C (CdReady) is the first unresolved provider in strict mode.
It gates the transition to Phase 6E (real Disc 1 + PE.IMG reads).

Additional unresolved providers required for boot-to-logo:
- `func_8007F778`, `func_80082314` — disc identity
- `DsSearchFile`, `func_80080C48` — file search and mount
- `func_8006E6D4`, `func_800811E4` — image read and completion poll
- `func_800749D8` — display environment setup (currently memset stub)
- `func_800752AC` (ClearOTagR) — ordering table clear
