# PE-BTL27 — type-0/2 0x9B / 15240 empty dest

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80015240` — 239 words `0x80015240..0x800155FC`,
SHA-256 `fb261fcb…c9f8`. `D_800910A0[0x9B]`.

Live type-0 imms `0x55/0xA0/0x28`. Type-2 `0x5A/0x96/0x3C`.
Always jals `794C4`, `6698C`, `39B74`, `3A088`, `3A6A8`,
`3B97C`, `3BCE0`. `+0x98` bit `0x10000000` (35038) takes
the no-yield `v0=1` arm.

`func_80039B74` — 108 words `0x80039B74..0x80039D24`,
SHA-256 `8c66e398…84e7`. `beq a1,0` at `0x80039B8C` is
authentic. Live `+0x1B0` is 0 (35038 zeros it; `1A680`
skipped on empty `+0x1AC`). Do not force `+0x1B0`.
Nonzero clip (`39D24`/`39ED4`/`79754`) is not this cut.

`func_800362B8` — 79 words `0x800362B8..0x800363F4`,
SHA-256 `c1d94293…b78d`. Zero jal. Size-class bank
allocator. Consumed by 35038 only when `+0x1AC!=0`.

`func_8003A6A8` dest+0==0 jals `3E188` then returns.
`3E188` 187 words `0x8003E188..0x8003E474`. Live empty
dest has dest+0x24==0, so retail loads KUSEG `0x84`
(same physical as `0x80000084`). Host maps through
KSEG0 like 12574/1A918. Not a NULL skip. Not a fake
parent object. Full 187-word COP2 body is a named cut.

Authentic type-0 after `0x9B` is already-ported through
`0x02` yield at script `+0x2E8`. Second visit reaches
`0xAA` (new). Type-3 second miss `0x05` rel `0xF2` is
`base+(rel<<1)=+0x1E4`; persist `<40` skip is `+0x3E4`
`0x02` then `0x00` goto `+0xC`.

Do not publish `B0E70[0]` / 6B804 until `3D050` tail
is real. Do not force `D2E8` / `3999C`.

## Verify

```text
python3 pc_port/tools/pe_btl27_15240_oracle.py
PE_TEST_FILTER=BTL27 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
