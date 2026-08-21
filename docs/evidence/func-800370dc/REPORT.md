# func_800370DC — packet setup/submit wrapper

```text
MATCHING_C — era -O2 -G0, 25/25 words
vram        0x800370DC..0x80037140 exclusive
file        0x278DC size 0x64
```

The wrapper initializes the head packet, configures its embedded `+8` packet,
submits it, and reports `-1` when submission fails. The direct C form
reproduces the retail 0x20 frame, save order, all call slots, and branch tail.
The asm remainder resumes at `27940.s`.
