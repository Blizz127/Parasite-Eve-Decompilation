# PE-BTL50 — type-2 0x6B / 187C0 through 6F6D4

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_800187C0` — 22 words `0x800187C0..0x80018818`,
SHA-256 `1e733223…da40`. `D_800910A0[0x6B]`.
`jal 6F6D4(*arg0, 0, *arg1, *arg2, *arg3, *arg4)`;
v0=1. Live `(local[7], 1, 0, 0, 0)`.

`func_8006F6D4` — 83 words `0x8006F6D4..0x8006F820`,
SHA-256 `c38426e2…1389`. Index `<0x16` else -10.
Index `<0xB` uses `*D_800942E4` stride `0xA0C`.
Remaps slot+1 `>=0x55` to table `0x55`.
`table[0x55]+8 = 0x800D4698`.

`func_800D4698` — 27 words `0x800D4698..0x800D4704`,
SHA-256 `c33ca3b9…6991`. Mode 0 writes slot →
`0x800F32D0` and slot+0x0C → `0x800E2368`, then
jalrs `*(slot+0x8C)+0x30`. Live CE49C left
`+0x8C=0`; kuseg `0x30` is KSEG0 `0x80000030`
(zero). Jalr skipped. Return 0.

Unknown jalr targets are not invented.
Do not force scratch / hit / pad.

## Verify

```text
python3 pc_port/tools/pe_btl50_187c0_oracle.py
PE_TEST_FILTER=BTL50 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
