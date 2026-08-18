# FIRST_EVENTS — completed retail capture

Source: `/var/home/blizz/Applications/pcsx-redux/captures/pe-btl83`
IDENTITY PASS. Disc 1 dest combat token `0xA8001248` (M0036I).

```
retail_first_4D4_nonzero_writer=0x80033A34 ra=0x80019D34 fn=func_80033A2C
retail_mode7_writer=0x8002CF24 (li 7 / sw gp+0x51C) inside 2CEE0
retail_192bc=func_800192B8 opcode 0x95 sw $zero D28C (mode 0, not 7)
retail_2cee0_entry ra=0x8002BCAC s1=1 (2BC90 delay addiu s1,1)
retail_mode6_dispatch=2A7F8 arm jal 0x8002BC90 @ 0x8002A9EC
retail_first_1D340_entry ra=0x8002A504 a0=1 dest=0xA8001248 hp=40 4D4=1
retail_first_hp_subtractive_writer=0x8001F704 ra=0x8001F5F0 40→39 then 39→34
```

Do not poke `4D4`, mode 7, `1D340`, or HP. Opcode `0xCF` (`19D24`) is
the live jal to `33A2C`. `6914C(0)==0` plus `s1!=0` is the live gate
into `2CF24`. Opcode `0x95` (`192B8`) stores mode 0. Native PE-BTL98
jals `1D340` at `2A4FC` and takes `1F704` 40→39.
