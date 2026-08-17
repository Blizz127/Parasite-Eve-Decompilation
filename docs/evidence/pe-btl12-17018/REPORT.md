# PE-BTL12 — func_80017018 task VM

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## Leaf

159 words `0x80017018..0x80017294`, SHA-256 `0b2a2f69…fcd8`.
Sole TEXT jal `361F4@0x80036224`. Zero `jal`; one `jalr` of
`D_800910A0[word & 0x1FFF]`.

```text
task = D_8009D300
if (task+8) & 0x50: walk +0x24
if actor+0x98 & 0x1000 and !(task+8 & 0x80): walk
if D1A0 & 0x100 and actor != D254 and !(task+8 & 0x80): walk
if task+0x10 == 0: walk
task+0x10--; if != 0: walk
fetch; argc=(word>>13)&0xF; op=word&0x1FFF
decode kinds (word>>17, 3 bits; word+4 after 5 args)
jalr table[op](sp+16)
v0!=0 re-fetch from gp+0x90 (ROM bne @17248→170F0);
v0==0 store that PC, walk +0x24
```

`12700` sets `+0x10=1`, so the first field tick runs one
opcode. Native arg frame is `0x80120F80` (APPROXIMATION: no
guest `$sp`).

## Handlers this cut

| op | VA | Effect |
|---|---|---|
| 0 | `0x80017294` | PC = `*(actor+0x9C) + imm<<1`; v0=1 |
| 1 | `0x800172BC` | `actor+0x98 \|= 0x10`; v0=0 |
| 2 | `0x800172E0` | `task+0x10 = lhu(*arg0)`; v0=0 |
| 0x20 | `0x800172FC` | `task+8 \|= 0x10`; v0=0 |
| 0x1C | `0x80017764` | already-ported mailbox send |
| 0x1F | `0x800177AC` | already-ported mailbox poll |

Other table slots return 0 (advance). Not M2.

## Verify

```text
python3 pc_port/tools/pe_btl12_17018_oracle.py
PE_TEST_FILTER=BTL12 ./pc_port/build/pe-native-tests
```
