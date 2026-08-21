# func_8005184C — dynamic bit setter

```text
MATCHING_C — era -O2 -G0, 8/8 words
vram        0x8005184C..0x8005186C exclusive
file        0x4204C size 0x20
```

`*ptr |= mask << bit` produces retail's `lui/addiu/li/lw/sllv/or/jr/sw`
sequence when the global pointer and one-bit mask are pinned to `$v0` and
`$v1`, respectively. The following C leaf begins at `0x4206C`.

The full Docker rebuild produces the target SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
