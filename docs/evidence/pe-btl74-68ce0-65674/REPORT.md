# PE-BTL74 — 68CE0 sequencer and 65674 D1A0 out

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`68CE0` — 18 words `0x80068CE0..0x80068D28`,
SHA-256 `55c7dbcc…8e5b`. `3F3C4` @ `3F560`.
`v0=0`. Jals `66CE8`, `65674`, `67E1C`, `67A78`,
`67B74`, `67D18`.

`65674` — 166 words `0x80065674..0x8006590C`,
SHA-256 `9b325f33…3c81`. Zero jal. First load is
`D1A0`; `bne` skips the `B1624` walk. Live `3E974`
leaves `D1A0|=0x4000`, so the body is not this
cut.

`67E1C` is the next live tail: it runs when
`(D1A0&0x104)==0` and walks `B1624`. Not this cut.

## Type-6 scratch[0]&4 (producer census)

`0x09` subop 3 then 7: `cond[1] = !(scratch[0]&4)`.
`0x05` skips to the `0x12` fork only when the bit
is **set**. Clear bit waits. BTL61 wait-while-set
is REJECTED.

EXE `lui/addiu` sites for `B6A80` are only `1266C`
zero, `17018` kind-4 decode, and `34F10` zero.
m0005i script setter of bit 2 is type-6 `+0x1850`
`0x2A[0,2]`, reached after unported `0xAE`.
M0367I type-1 has no kind-4 write. Do not force
the bit.

Type-3 `0x85` dests are `0xA8000248` → M0004I and
`0xA80004C8` → M0009I (exits), not the NYPD
`0x55(2)` at type-6 `+0xFAC`.

## Verify

```text
python3 pc_port/tools/pe_btl74_68ce0_oracle.py
PE_TEST_FILTER=BTL74 ./pc_port/build/pe-native-tests
```
