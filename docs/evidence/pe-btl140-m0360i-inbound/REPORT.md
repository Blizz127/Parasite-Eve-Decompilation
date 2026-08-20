# PE-BTL140 REPORT — m0360i has zero 0x31 inbound hops

```text
PE-BTL140 PROVEN — no field-script 0x31 dest token names m0360i
scripts_found=414 scripts_missing=0
0x31_tokens=1011 all immediate (mode 0)
packed m0360i = 0xA8066048 (absent from EXE and from every script)
```

Scanner: `tools/research/pe_btl139_m0360i_inbound.py` (kept name;
this rung is BTL140). Every field-table script was decoded. Every
`0x31` dest is immediate. None decode to `m0360i`. None have
table-index formula 359. The packed token is not in SLUS_006.62.

m0354i (27 inbound hops, a real hub) goes to itself, `m0353i`,
`m0355i`, and death dest `0xA9400048` — not to m0360i.

m0360i is the unique persist[0] bit-4 writer (BTL130). Reaching it
is not a script `0x31` hop. Next leads: EXE dest-change after the
Day-1 cinema, world-map overlay, or a non-0x31 opcode. Do not poke
persist or plant a dest token.

## Files

```text
docs/evidence/pe-btl140-m0360i-inbound/REPORT.md
tools/research/pe_btl139_m0360i_inbound.py
```
