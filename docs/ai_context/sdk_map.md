# PE1 PsyQ SDK Map (PsyCross redirect targets — do NOT decompile for the port)

Confirmed by string cross-references to debug strings embedded in the
statically linked PsyQ libraries.

## libGPU (segments `65238.s` / `654C8.s`, approximately `0x80074A44–0x800755F0`)

| Function | SDK name | Evidence |
| --- | --- | --- |
| `func_800749D8` | `SetDefDrawEnv` **(PROBABLE — INFERENCE, not string-proven)** | Position in the libGPU segment just before `ResetGraph` + five-arg call shape `(&env,0,0,320,240)` from `func_8006E834` (5th arg on the stack); the same env pointer is later passed to `PutDispEnv`, so `SetDefDispEnv` is also possible |
| `func_80074A44` | `ResetGraph` | `"ResetGraph:jtb=%08x,env=%08x\n"` and `"ResetGraph(%d)...\n"` |
| `func_80074CC8` | `DrawSyncCallback` | `"DrawSyncCallback(%08x)...\n"` |
| `func_80074D28` | `SetDispMask` | `"SetDispMask(%d)...\n"` |
| `func_80074DC0` | `DrawSync` | `"DrawSync(%d)...\n"` |
| `func_80074F44` | `ClearImage` | `"ClearImage"` |
| `func_80074FD4` | `ClearImage2` | `"ClearImage2"` |
| `func_8007506C` | `LoadImage` | `"LoadImage"` |
| `func_800750CC` | `StoreImage` | `"StoreImage"` |
| `func_8007512C` | `MoveImage` | `"MoveImage"` |
| `func_800751E4` | `ClearOTag` | `"ClearOTag(%08x,%d)...\n"` |
| `func_800752AC` | `ClearOTagR` | `"ClearOTagR(%08x,%d)...\n"` |
| `func_800753B4` | `DrawOTag` | `"DrawOTag(%08x)...\n"` |
| `func_80075424` | `PutDrawEnv` | `"PutDrawEnv(%08x)...\n"` |
| `func_800754E4` | `DrawOTagEnv` | `"DrawOTagEnv(%08x,&08x)...\n"` |
| `func_800755F0` | `PutDispEnv` | `"PutDispEnv(%08x)...\n"` |

## libETC (display)

| Function | SDK name | Evidence |
| --- | --- | --- |
| `func_80073A44` | `VSync` | Calls the timeout path that cross-references `"VSync: timeout\n"` |

## libCD (segment `6ACD0.s`, approximately `0x8007B010–0x8007BDDC`)

| Function | SDK name | Evidence |
| --- | --- | --- |
| `func_8007B010` | `CD_sync` | `"CD_sync"` |
| `func_8007B290` | `CD_ready` | `"CD_ready"` |
| `func_8007B558` | `CD_cw` | `"CD_cw"` |
| `func_8007BBFC` | `CD_init` | `"CD_init:"` |
| `func_8007BDDC` | `CD_datasync` | `"CD_datasync"` |

## DS filesystem (segment `71A68.s`)

| Function | SDK name | Evidence |
| --- | --- | --- |
| `func_80081414` | `DsSearchFile` | `"DsSearchFile: disc error\n"` and related search diagnostics |
| `func_80081714` | `DS_newmedia` | `"DS_newmedia: Read error in ds_read(PVD)\n"` and related media diagnostics |
| `func_80081A7C` | `DS_cachefile` | `"DS_cachefile: dir not found\n"` and related cache diagnostics |

## libSPU voice-register helpers (hardware-signature identified)

The `func_8008770C`–`func_800877BC` cluster writes the SPU voice/control MMIO
block at `0x1F801C00`–`0x1F801D9A`. The paired helpers at `8770C`, `87728`,
`87744`, `87760`, and `8777C` write 32-bit values as two 16-bit stores to the
SPU control registers; nearby indexed helpers address per-voice registers at
`0x1F801C00 + voice*0x10`. This identifies the cluster as libSPU voice-control
support (including `SpuSetKey`-adjacent operations) by hardware layout. Exact
PsyQ routine names are not yet string- or symbol-proven.

## Still to map

These families are redirectable but have not yet been identified completely by
string cross-reference or signature matching:

- libGTE (`RotTransPers`, matrix operations, and related geometry helpers)
  - Screened for port-scope SKIP: the handwritten COP2 register/helper
    cluster `func_80078E04` through `func_80079024` in `asm/disc1/68478.s`.
    See `docs/evidence/volume-campaign-20260824/COP2_SDK_SCREEN.md`.
  - Screened separately: handwritten COP2 operation helper
    `func_800792D4` (`lwc2`/`mvmva`/`swc2`/`cfc2`), with six direct callers.
    Its exact PsyQ routine name remains unproven.
- remaining libSPU names outside the hardware-signature cluster above
- MDEC / libpress
- pad and SIO
- interrupt and root-counter support

## Port policy

These libraries are **statically linked** into PE1. This build carries their
debug strings, which provide the identity evidence recorded above.

For the port, redirect these functions and address ranges to PsyCross; do not
decompile them as game code. For a later 100% matching decompilation they may
still be matched, but they remain SDK code rather than PE1 game logic.
