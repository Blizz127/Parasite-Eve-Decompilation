# Boot to Black — Phase 6A milestone

## What was achieved

A native Linux executable (`parasite-eve-port`) that compiles and executes
translated Parasite Eve boot functions natively, reaching the first display
clear operation through the retail `func_8006E9A0` call chain.

No PS1 emulator is involved at runtime. PCSX-Redux is only the retail oracle.

## Boot trace (actual runtime)

```
0001 native_executable_start
0002 host_init_begin
0003 host_init_end
0004 bootstrap_disc_mode
0005 call_func_8006E834
0006 call_func_8006E9A0        ← translated PE C code
  → VSync(0)                   ← IMPLEMENTED
  → SetDispMask(0)             ← IMPLEMENTED
  → PutDispEnv                 ← HOST_ADAPTED
  → ClearImage(rect, 0, 0, 1) ← IMPLEMENTED — THE BLACK FRAME
  → DrawSync(0)                ← IMPLEMENTED
  → func_8005E588              ← BOOTSTRAP_RET
  → func_80066B60(2)           ← BOOTSTRAP_RET
  → ClearOTagR (x1)            ← BOOTSTRAP_RET
  → func_80068E24              ← BOOTSTRAP_RET
  → func_80070E54              ← BOOTSTRAP_RET
  → func_80038D1C              ← BOOTSTRAP_RET
0007 func_8006E9A0_returned
0008 first_frame_presented
0009 boot_complete
0010 shutdown_begin
0011 screenshot_written
0012 shutdown_end
```

## ClearImage call

The critical display operation comes from translated PE code:

```c
rect.x = 0;
rect.y = 0;
rect.w = 0x140;   // 320
rect.h = 0x1C0;   // 448 (clamped to 240 by host framebuffer)
func_80074F44(&rect, 0, 0, 1);  // ClearImage(r=0, g=0, b=1)
```

## Framebuffer proof

| Property | Value |
|----------|-------|
| Dimensions | 320×240 |
| Format | RGB 8:8:8 |
| Requested clear | RGB(0, 0, 1) |
| Actual pixels | All RGB(0, 0, 1) — exact match |
| SHA-256 (3 runs) | `fb28dc21...` — identical all 3 runs |
| Deterministic | ✅ Byte-identical across runs |

## What this milestone does NOT prove

- Full boot chain correctness (main not yet invoked)
- Disc I/O (bootstrap disc mode only)
- MDEC/logo rendering
- Any playable game state
- Audio, input, or save/load
