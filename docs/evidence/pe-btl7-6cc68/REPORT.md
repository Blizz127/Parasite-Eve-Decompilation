# PE-BTL7 — func_8006CC68 is the 3F074 poll-exit

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

79 words `0x8006CC68..0x8006CDA4`, SHA-256 `262dcfc6…a74d`.
Zero `jalr`. Every arm returns 0 (`addu $v0,$zero` at
`0x8006CD88`, or the `+0x98` skip delay).

## Live gates

```text
lw overlay[0]; and 0x000C0000; bnez → v0=0
lw D254; beqz → v0=0
lw actor+0x98; and 0x20000040; bnez → v0=0
if D1A0 bit1 clear and D2E8 bit1 set → v0=0
if D_800B0D10==0:
    sw D254+0x1B4 → D_800B0D10   # overlay+0x38
    sh 3 → D_800B0D14            # overlay+0x3C
    sh 0x12 → D_800B0D16         # overlay+0x3E
jal 661A4                      # OFX/OFY from D_800BCF94/96
jal 3A088(overlay+0x14)
jal 3AC90(overlay+0x14, D_800B89F8)
if D1A0 bit1 or actor+0x252: jal 3AF14
jal 661CC                      # SetGeomOffset(0xA0, 0x70)
v0=0
```

`661A4` is 10 words (ctc2 `$24/$25`). `661CC` is 8 words.
`3AC90` is 161 words. `3AF14` is 140 words. Those two are
not this cut. `3A088` uses the existing mode-0 empty cut
until dest+0 is proven live.

## Why 0x55 stayed parked

`6C4C4` re-ORs `+0xE` bit1 while `D1A0` bit1 is set. A
one-shot `6C5BC` per 0x55 tick (35558 shape) therefore
re-dirties the wait every parked tick. Retail `3F074`
does one `6C4C4` then polls `6C5BC` until v0=0, so `0x3B`
can observe the `6CC2C` clear.

`0x3B` v0=1 completes `0x55` into the next field opcode
(`0x89` = `D_8009D28C=6`). That is not battle-over.

## After the poll (not this cut)

```text
jal 0x8001A918   # 56w
jal 0x800371B0   # 169w  a0 = overlay bit 0x40000000 ? B162C : B1628
jal 0x800125E0   # 35w
jal 0x800E0060   # loaded; do not fake
jal 0x80074DC0(0)
jal 0x80074D28(1)
D2E8 &= ~0xC
overlay[0] &= ~0x00000402
```

## Verify

```text
python3 pc_port/tools/pe_btl7_6cc68_oracle.py
python3 pc_port/tools/pe_btl7_3f074_poll_oracle.py
PE_TEST_FILTER=BTL7 ./pc_port/build/pe-native-tests
```
