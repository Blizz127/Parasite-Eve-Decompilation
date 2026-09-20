# func_8007BAC0

- **VRAM**: 0x8007BAC0
- **File offset**: 0x6C2C0 (size 0xF0)
- **Unit**: 6C1EC
- **Build profile**: era_o2_g0_fill_epilogue_delay_slot (`-O2 -G0` + `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`)
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave5-c, agent/wave5-c)

## Behaviour
Sequenced SPU/voice register init. Reads two conditional status halfwords:
if `*(u16 *)(D_8009B290 + 0x1B8) == 0` and `*(u16 *)(D_8009B290 + 0x1BA) == 0`,
writes 0x3FFF to +0x180/+0x182. Then unconditionally writes 0x3FFF to
+0x1B0/+0x1B2 and 0xC001 to +0x1AA. Finally seven byte writes through the
pointer globals D_8009B27C/B280/B284/B288, fed by a 4-byte stack scratch
array holding {0x80, 0, 0x80, 0}; returns 0.

## Method
Straight translation of `asm/disc1/6C1EC.s` with a `unsigned char sp[4]` local
(the real source scratch), which reproduces the `addiu $sp,$sp,-8` frame with
only 4 bytes accessed. Repeated `D_8009B290` dereferences reproduce the
pointer reload after the +0x180/+0x182 stores (CSE invalidation by the
stores). The single residual was the epilogue: cc1 emits
`addu $sp,$sp,8; j $31` in the reordered region and maspsx moves the restore
before the jump; the per-leaf gate `MASPSX_FILL_EPILOGUE_DELAY_SLOT=1`
(profile `era_o2_g0_fill_epilogue_delay_slot`) moves it into the `jr $ra`
delay slot, matching retail byte-for-byte.

## Evidence
Fresh complete retail build: `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (874
registered C leaves)`, `VERIFY_US=PASS`.

## Divergences
None.
