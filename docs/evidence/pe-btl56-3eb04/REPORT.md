# PE-BTL56 — func_8003EB04 digital pad edge

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
3EB04       348w 0x8003EB04..0x8003F074  sha c6a27aa9…660b
jal         3F3C4 @ 0x8003F40C
```

Matching `src/` C was not added. 825C0 / analog are not this cut.

## Edge

```text
previous = D26C
rebuild D26C from lhu(0x800BE9A2) through A76F0[0..31]
D1F4 = (held ^ previous) & held
D1E4 = (held ^ previous) & previous
```

3E974 maps raw Cross (`~raw & 0x4000`, swapped to `0x2000`) onto
processed bit 8 (`0x100`). Held-from-before does not set D1F4.

Do not invent type-5 pad-hit D1F4. This edge is the message-close
producer for type-0 `0x22`.
