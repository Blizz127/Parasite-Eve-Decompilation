# PE-92CE8 post-E08 — media loop recovery

Status: **RE-LANDED INTO THE NATIVE PORT** (92CE8 complete CFG;
92934 body with Stage-1b-gated live C89C).

## Identity

```text
PE.IMG LBA                 1013 (raw 2352 Disc1)
overlay package            PE.IMG sector 0x03D2 -> RAM 0x8018EFF0
func_80192CE8              [0x80192CE8,0x80192F98)
complete size              0x2B0 / 172 words
complete SHA-256           ed89408bffde43742781c89f1d16b7a98c52dc165b67cafd62cfdcf9319e03e7
post-E08 span              [0x80192E08,0x80192F98)
post-E08 size              0x190 / 100 words
post-E08 SHA-256           77218c9c335f6b4a24416a39d21d09876d7bc9f74778fefb26d28d501dbfd01a
```

Carve artifacts: `func_80192CE8.bin`, `func_80192CE8_tail_E08.bin`,
`func_80192CE8.s.txt`.

## Post-E08 CFG (matched)

After `jal func_801924F8` @`0x80192E00`:

1. If `D_800B0DBA == 0` -> epilogue.
2. Loop while active:
   - If `(int16_t)D_800B0DBC <= 0` -> epilogue.
   - `func_8003EB04()` (pad).
   - `func_80192934()`; low-byte zero (`sll 24`) clears
     `801D0DE8/DEC/DFC/DF8/DF0/DF4` + `D_800B0DBA` and falls to frame tail.
   - Else if `D_8009D26C & 0x20000004`: abort path — decrement DBA,
     `870F0(0)`, `C0D8(0)`, `7A2A4()`, `80DC4(9,0,0)`, same clear,
     and if saved frame count < 1400: `VSync(0)`, `SetDispMask(0)`,
     `s3 = 1`.
   - `func_80070E54()`; continue while `D_800B0DBA != 0`.
3. Epilogue: `D_800B0CD8 &= ~0x200`; return `s3` (0 or 1).

## Port files

- `pc_port/game/boot/func_80192CE8_port.c` — full function including media loop
- `pc_port/game/boot/func_80192934_port.c` — body with Stage-1b-gated live C89C
