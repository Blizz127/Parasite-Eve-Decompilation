# PE-PRS1 — VRAM → host framebuffer display presentation

Authority: retail Disc 1 executable `SLUS_006.62`, SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. No retail words are
translated by this rung: the 318-word `func_800755F0` (PutDispEnv) GP1
body stays untranslated, so there is deliberately NO byte oracle. The
host display is the hardware authority: this cut reads the DISPENV disp
RECT from guest RAM and copies that VRAM window into the host
framebuffer through the GPU VRAM model.

## What changed

- `pc_port/platform/host_framebuffer.[ch]`: new
  `HostFB_PresentDispEnv(pe_addr_t env)`. Reads x/y/w/h halfwords at
  env+0/2/4/6 via `PE_LoadU16`; when the display mask is on, copies
  min(h,240) rows × min(w,320) cols from `(dx,dy)` through
  `PE_GPU_ReadVRAM` + `HostVRAM_DecodePixel` into `fb` (out-of-VRAM
  reads decode as black, so the copy is total and deterministic); when
  the mask is off, presents black (retail blanks). Keeps the existing
  present counter and `--max-frames` budget, then invokes the present
  hook. Zero guest writes. Legacy `HostFB_Present()` (counter-only)
  now also invokes the hook so windowed runs refresh on every present.
- `pc_port/platform/pe_libgpu.c`: `func_800755F0(pe_addr_t env)` is now
  a real implementation delegating to `HostFB_PresentDispEnv`.
  `pc_port/include/psx_compat.h:106` drops the `void*` counter stub for
  the real prototype. All six call sites pass guest addresses:
  `func_80070E54_port.c` (`0x800BCE80 + 20*CDDC`),
  `func_8003E754_port.c` / `func_8006E9A0_port.c` (`0x800BCE80`),
  `func_8006AD40_port.c` (`0x800BCE80 + 20*[0x800ACDDC]`, DISP_ENV
  stride 20), `func_80190660_port.c` (`environment + 0x5C`),
  `func_8006E834_port.c` (`PE_6E834_ENV_ADDR`).
- `pc_port/bootstrap/game_port.[ch]`: `PE_Port_SetPresentHook` /
  `PE_Port_InvokePresentHook` (`PEPortPresentHook`, host-data-only
  contract); cleared by `PE_Port_RunControlReset` like the quit poll.
- `pc_port/src/port_main.c`: windowed runs register
  `PresentHook_BlitWindow` (`HostWindow_Blit` + `HostWindow_Poll`);
  cleared with the quit poll before `HostWindow_Close()`. The shutdown
  final blit is unchanged. Headless builds are untouched (hook stays
  NULL).

## G3 (no new HOST_ADAPTED)

Platform code only reads guest RAM (`PE_LoadU16`) and the VRAM model
(`PE_GPU_ReadVRAM`); it writes the host `fb[]` only. The window hook
reads host pixels. No guest mutation anywhere on this path.

## Tests (5, all passing)

- `PRS1_dispenv_window_copy`: 64×32 FILL at VRAM (10,20), full-screen
  env, mask on → decoded `{0,0,255}` inside the block, black outside,
  present counted, no stop.
- `PRS1_dispenv_offset_window`: 4×4 env at (12,22) → fb origin shows
  VRAM(12,22); pins the x/y mapping, not just sizes; no width
  overflow.
- `PRS1_mask_off_presents_black`: white-poisoned fb + mask off →
  all black, present still counted.
- `PRS1_frame_budget_stops_present`: `--max-frames`-style limit of 1 →
  second present blocked, `frame-limit` stop reason.
- `PRS1_present_hook_fires_and_resets`: hook fires on both present
  forms; `RunControlReset` clears it.

## Verify

```
Results: 1048 run, 1030 passed, 1 failed, 17 skipped
Results: 1048 run, 1048 passed, 0 failed, 0 skipped
```

First line: plain suite (the 1 failure is the pre-existing B54KY
environmental case, `missing local/pe_disc1.path`, identical on base).
Second line: `PE_DISC1_BIN` set (real Disc 1 image). ASan/UBSan CTest:
`100% tests passed, 0 tests failed out of 2`, zero sanitizer
diagnostics. Determinism:
`parasite-eve-port --headless --bootstrap-disc --max-frames 1
--screenshot` run twice produces byte-identical PPMs (`cmp` clean,
exit 0 both runs, `[FB] presents=1`, `stop_reason=frame-limit`, zero
`HOST_ADAPTED` lines).
