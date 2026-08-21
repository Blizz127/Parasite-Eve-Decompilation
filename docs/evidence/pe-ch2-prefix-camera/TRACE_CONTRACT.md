# PE-CH2 prefix camera trace contract

This is a static, census-backed native leaf trace. It is not a field-VM,
projection, framing, or playable-Carnegie claim.

## Columns

```text
field_scene,field_script_pc,opcode,arg0,arg1,
slot0_flags,slot1_flags,slot0_param,slot1_param,
view_index,view_h,matrix_r11,matrix_trx
```

## Required sequence

```text
m0003i mod0+0x4D0 0x7B (0,0x4000)
m0003i mod0+0x4E0 0x7B (1,0x4000)
m0003i mod0+0x4F0 0x75 (0,1)
m0003i mod0+0x500 0x75 (1,1)
m0372i mod3+0xAE8 0x82 (1)
m0004i mod0+0x0F4 0x82 (1)
m0005i mod0+0x6BC 0x31 (0xA80002C8)
```

`0x75` and `0x7B` do not occur in m0004i. The route therefore spans
m0003i setup and the m0372i/m0004i view-1 applications.

## State policy

- `slot*_flags` records the one-byte `0x75` store.
- `slot*_param` records the `0x7B` halfword store after `value >> 8`.
- `view_*` and `matrix_*` record selected `0x82` outputs from a clearly
  synthetic 52-byte record-1 fixture.
- The synthetic values prove copy widths/addressing only. They are not
  claimed as the retail Carnegie record-1 data or visual framing.
