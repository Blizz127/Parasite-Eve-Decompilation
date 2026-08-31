# PE-B54K — remaining `func_80030894` tail partition

Date: 2026-08-30

Status: read-only audit; no production implementation in this evidence rung.

B54K-B6 leaves 190 of 788 words after the exact boundary at `0x800311EC`.
Retail instructions, call targets, and loop delay slots partition the tail
without cutting a packet setup or loop:

| Unit | Retail range | Words | SHA-256 |
| --- | --- | ---: | --- |
| L10 | `0x800311EC..0x80031320` | 77 | `b8eafebde2564d8c3315c39c6b7e37ae8fe04036c525f8bdd2d12a761d0e86e2` |
| L11 | `0x80031320..0x80031438` | 70 | `bdfec78193cfc60b0ed14829f5f3fb42ce74db2cbfe0431ad402f174fd665538` |
| outer close/final sprite | `0x80031438..0x800314B0` | 30 | `06f1268fddf6d3863439d142d53541c457dbe1809bac4e60cf81ebff965189f6` |
| epilogue | `0x800314B0..0x800314E4` | 13 | `70acab92b4119ef66a1612d7d93b7b2e4a75bf4cca277cc511da3dbecb27355f` |

Arithmetic closes: `77 + 70 + 30 + 13 = 190` words.

Call and branch ownership:

- L10 calls `func_800370DC` at `0x80031208`, `func_80077AA4` at
  `0x8003122C`, and `func_800370DC` at `0x80031268/0x800312E0`. Its sole
  branch is `0x80031318 -> 0x800312CC`, the literal-bound-two L10 loop.
- L11 calls `func_800370DC` at `0x8003133C`, then the literal-bound-thirteen
  loop calls `func_8005DADC`, `func_80077A64`, and `func_800370DC` at
  `0x80031394/0x800313AC/0x800313C8`. Its branch is
  `0x80031430 -> 0x80031394`.
- The outer close calls `func_80077A64` at `0x80031444` and
  `func_800370DC` at `0x80031468`, then owns the bank increment and outer
  `0x800314A8 -> 0x80030910` branch.
- The epilogue has no calls and only the canonical `jr ra` at `0x800314DC`
  with `nop` delay slot.

All ten remaining static call sites target helpers already native before this
audit. The next implementation unit is L10. This report does not move the
strict frontier, alter guest state, or claim tests beyond B54K-B6's 938/938.
