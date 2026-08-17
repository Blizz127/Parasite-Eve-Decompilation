# PE-BTL4 — Writer A; type at 1A680 is ctor desc[0]==0

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C was added. No idle / freeze / idA=2 pointer was
invented.

## Actor type at `func_8001A680` (this rung)

The 0x55 → `29810` → `1A680` actor is `*D_8009D254`. Opcode `0x08`
(`D_800910A0[8]=0x8001735C`) jals ctor `func_80035038`. That ctor:

```text
0x80035100  lbu desc[0]
0x80035148  bne desc[0], $zero  → skip D254 publish
0x80035158  sw actor, 0x4E4($gp)   # gp=0x8009CD70 → D_8009D254
0x8003517C  sb desc[0], actor+0x0C
0x80035188  sb desc[1], actor+0x0D
```

Type-0 Aya is the only publisher of `D_8009D254`. Text `sw 0x4E4($gp)`
sites are `0x80034FB0`, `0x8003502C`, `0x80035158`, `0x80036124` (three
zeros + the type-0 publish). `1A680` `lbu actor+0x0C` therefore reads
ctor `desc[0]`.

The HP object is a different pointer (`D_8009D278`). `293F4` `sh +0x0C`
is current HP on that record, not actor type. `209F0` copies rodata
`0x800106A4` onto `$sp+0x10` (the `lw/sw/sb +0x0C` there are memcpy
lanes) then writes the record, never `D_8009D254`. `144FC`, `29810`
through the `1A680` jal, `30640`, and `339A0` do not jal the ctor,
`0x5A` wrapper `0x80018164`, or `2FF78`, and they do not store actor
`+0x0C`.

So on this NYPD path the table row is **type 0 / command 4**. A later
row is not the slot.

## Writer A vs room / CE2=10 directories

`func_8001A680` only **reads** `D_800B0E98[type*192 + idB*4]` at
`0x8001A6C8` and stores that word to actor `+0x1B0`.

Writer A clip-directory loop inside `func_8006B4F8`:

```text
0x8006B548  lui/addiu $s6, D_800B0CD8
0x8006B84C  lbu idA, record+0xB
            lbu idB, record+0x7
            lw  ptr,  record+4
            slot = D_800B0CD8 + 0x1C0 + idA*192 + idB*4
            sw  ($s4 + (ptr & 0x00FFFFFF)), 0(slot)
```

`$s4` is the chunk-2 dest (`D_800B0CD8+0x18C`). A scan for
`*192` then `+0x1C0` finds only `0x8006B868`. Writer B
(`0x8006C140`) stores `overlay+0x1C0+idB*4` without `idA*192`;
`144FC` / `29810` do not jal `6C1CC` / `6BECC`.

m0005i chunk2 type-0 idBs are `0x18, 0x1D, 0x1E, 0x1F, 0x20`
(no command 4). The only m0005i idB=4 is **idA=2** (27/28-bone
room actor). Do not bind that to Aya. CE2=10 also has no type-0
idB=4.

## Verify

```text
python3 pc_port/tools/pe_btl4_command_table_oracle.py
python3 pc_port/tools/pe_btl3_29810_tail_oracle.py
```

Observed this rung: both PASS, including ctor `sb desc[0]` /
`D_8009D254` publish / `1A680` type load. NEXT: prove who fills
type-0 command 4 *before* that load, or STOP without a ROM-word
payload. Do not invent overlay returns, ATB, mode 7, or `0x55`
completion.
