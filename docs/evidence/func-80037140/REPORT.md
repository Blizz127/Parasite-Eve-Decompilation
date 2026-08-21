# func_80037140 — packet setup/submit twin

```text
MATCHING_C — era -O2 -G0, 25/25 words
vram        0x80037140..0x800371A4 exclusive
file        0x27940 size 0x64
```

This is the `func_800370DC` wrapper twin, using `func_80077C44` for its
embedded packet configuration. The direct C form reproduces its retail frame,
calls, slots, conditional error report, and epilogue exactly.
