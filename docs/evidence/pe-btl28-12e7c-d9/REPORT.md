# PE-BTL28 — type-5 0x0C / 12E7C and 0xD9 / 1A15C

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

`func_80012E7C` — 142 words `0x80012E7C..0x800130B4`,
SHA-256 `76d82640…3dc8`. `D_800910A0[0x0C]`. Zero jal.
Always v0=1. Jump table `0x80010080`. Read twin of
`0x0B`/`12C20`: copies `D_8009D2F0` pose groups into
`*arg1`/`*arg2`/`*arg3`.

| code | fields |
|---|---|
| 0 | `+0x28`/`+0x2C`/`+0x30` |
| 1 | `+0x40`/`+0x44`/`+0x48` |
| 2 | `+0x68`/`+0x6C`/`+0x70` |
| 3 | `+0x78`/`+0x7C`/`+0x80` |
| 4 | `+0x88`/`+0x8C`/`+0x90` |
| 5 | `lh +0x38`/`+0x3A`/`+0x3C` (sign-extend `sw`) |
| 6 | `+0x58`/`+0x5C`/`+0x60` |
| ≥7 | v0=1, no store |

Live type-5 scratch-miss `+0x14C`: code 0 →
`local[0x0D]`/`[0x0F]`/`[0x0E]`.

`func_8001A15C` — 19 words `0x8001A15C..0x8001A1A8`,
SHA-256 `f53fb5e6…9760`. `D_800910A0[0xD9]`.
`*arg2 = func_80079FB4(*arg0, *arg1)`; v0=1.
Live word `00B660D9` kinds k3,k3,k1:
`cond[0]`, `cond[1]` → `local[1]`.

`func_80079FB4` — 93 words `0x80079FB4..0x8007A128`,
SHA-256 `e5b0edc7…f820`. Zero jal. Signed ratan2,
4096-circle. Table `D_8009A6EC` (`lh` at
`0x8007A0F8`). Both-zero returns 0 at `0x80079FE0`.
`|x|<|y|` uses the table directly; else `1024 - table`.
Then if y was neg: `2048 - angle`; if x was neg:
`-angle`. Host skips retail `break` (spec 0d) on
div0 / overflow. After `PE_RamReset` the table is
zeros unless planted (same as sincos `966EC`).
Live zero poses → `ratan2(0,0)=0`, no table needed.

After `0xD9` the stream is already-ported ALU /
`0x5E` / `0x09` / `0x0A` / `0x05`. `0x05` rel
`0x162` skips to `+0x2C4` when `cond[2]==0`;
else `0x20` then `0x24` (new). Scratch is 0 on
host, so the live arm is the miss path. Do not
invent a scratch bit or a pose hit.

## Verify

```text
python3 pc_port/tools/pe_btl28_12e7c_d9_oracle.py
PE_TEST_FILTER=BTL28 ./pc_port/build/pe-native-tests
./pc_port/build/pe-native-tests
```
