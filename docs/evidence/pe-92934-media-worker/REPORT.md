# PE-92934 — media worker body from 0x80192960

Status: **AUTHENTICATED CARVE + MATCHED PORT DRAFT** (C89C named cut).

## Identity

```text
func_80192934              [0x80192934,0x80192C48)
size                       0x314 / 197 words
SHA-256                    9a88d5065fe2f4dc9fa860771a755eec3c53dfef824fc1bbec0df7a8b3991780
through siblings           [0x80192934,0x80192CE8)
size                       0x3B4 / 237 words
SHA-256                    bd9d3e37b7bb80c471b2c3592035615e59f84924d5e549b7beaafaa624b24b3e
siblings                   [0x80192C48,0x80192CE8) 0xA0 / 40 words
siblings SHA-256           cfb22cf0545457344f784b0c7756ee2074d666876e3692b4e85dfbe8873e4133
```

Carve: PE.IMG overlay sector `0x03D2` @ RAM `0x8018EFF0` from Disc1 BIN
LBA 1013 (raw 2352). Artifacts: `func_80192934.bin`,
`func_80192934_through_siblings.bin`, `func_80192C48_siblings.bin`,
`func_80192934.s.txt`.

## CFG from 0x80192960

1. If `[801D0DC0] == 2`: `918F8((s8)(ACDDC^1), (s8)DBB)`; clear DC0.
2. Copy `801D0DC4 -> 801D0DDC` (lwl/lwr).
3. `BFA0(*[1464+[146C]*4], [801D0DBE])`.
4. `C01C(*[146C+[1478]*4+4], (1490*1492)/2 signed)`.
5. Poll `91B64(1464)` up to 2000 times.
6. **Got-frame**: bump DBC, flip 146C, then retail `C89C` + `7C394`.
   Port stops at `func_80192934_8010C89C_cut` (Stage-1b; no live C89C).
7. **Poll fail (-1)**: CD reissue `7C2A0(DDC)` / wait `7F72C==1 &&
   7F778==0` / `80D5C(2,DDC,sp+10)` / `81314(DDC,0x1E0)`; retry poll.
8. **Success path** (post-C89C): wait/force `[1494]`, if `[DBD]==1` set
   s3 and run abort teardown `DBA--`, `C0D8(0)`, `7A2A4()`, `80DC4(9,0,0)`;
   else return 1.

## Siblings

- `func_80192C48`: `DBA--`, `870F0(0)`, `C0D8(0)`, `7A2A4()`, `80DC4(9,0,0)`.
- `func_80192C9C(flag)`: store `DC0=1` when `(flag<<24!=0) XOR (DBB!=0)`.

## Stubs / gates

| Symbol   | Port status                                      |
|----------|--------------------------------------------------|
| 918F8    | ported                                           |
| 0BFA0    | ported                                           |
| C01C     | ported                                           |
| 91B64    | ported                                           |
| C89C     | **named cut** (do not wire live; Stage-1b)       |
| 7C394    | ported (behind C89C cut)                         |
| CD chain | 7C2A0 / 7F72C / 7F778 / 80D5C / 81314 ported     |
| abort    | C0D8 / 7A2A4 / 80DC4 ported; 92C48 adds 870F0    |

Match confidence: **high** on carve SHA + CFG; port draft matched with
explicit C89C cut (not byte-claiming the cut path).
