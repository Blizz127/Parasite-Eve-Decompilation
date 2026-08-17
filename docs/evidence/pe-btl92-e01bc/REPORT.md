# PE-BTL92 — 3F3C4 jals E01BC

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`E01BC` — 44 words `0x800E01BC..0x800E026C`,
SHA-256 `87f953d4…7cd8`. `3F3C4` @ `3F578`
between `661A4` and `661CC`.

`lh E21A4`, `lw E2800`, `blez` return.
Live count is 0, so `E026C` / `E03A0` are
not this cut. Stores of `E21A4` / `E2800`
are only the `E00xx` cluster.

## Type-6 scratch[0]&4 (producer census)

Disc-wide `lui 0x800B` + `addiu 0x6A80` is
only the EXE sites `1266C` / `17018` /
`34F10`. No `0x800B6A80` data literal.
m0004i and m0367i have no `0x2A`.
m0005i setter of bit 2 is still type-6
`+0x1850` after `0x55`. Do not force the bit.

## Verify

```text
python3 pc_port/tools/pe_btl92_e01bc_oracle.py
PE_TEST_FILTER=BTL92 ./pc_port/build/pe-native-tests
```
