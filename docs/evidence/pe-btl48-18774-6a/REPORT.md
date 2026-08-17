# PE-BTL48 — type-2 0x6A / 18774 through 6F39C

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80018774` — 19 words `0x80018774..0x800187C0`,
SHA-256 `e32a2874…d822`. `D_800910A0[0x6A]`.
`jal 6F39C(*arg0, D2F0)`; `*arg1=v0`; v0=1.
Live `(0x75) → local[7]`.

`func_8006F39C` — 206 words `0x8006F39C..0x8006F6D4`,
SHA-256 `ee236f21…8337`. `6914C(0)` then remap
`code>=0x55` to table index `0x55`.
`D_800942E0` EXE word is `0x80094188`;
`table[0x55]=0x800E13D4`; `+4=0x800D4620`.
Free slot from `*D_800942E4` (6A8D4 `B0E60`,
stride `0xA0C`).

`func_800CE49C` — 23 words. Live extra `0x20`
reads `D_800E1044[0x20]=0` and returns -1
(printf `71A74` not this cut). `6F39C`
ignores that and jalrs `D4620`.

`func_800D4620` — 30 words, zero jal. Inits
the slot (`+2=1`, `+0x10=slot+0x90`, eight
`0xFFFF` records). Return index 0 on the
first free slot.

Codes `0x6C..0x72` CD/XA prelude is not this
cut. Unknown jalr targets are not invented.
Do not force scratch / hit / pad.

## Verify

```text
python3 pc_port/tools/pe_btl48_18774_oracle.py
PE_TEST_FILTER=BTL48 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
