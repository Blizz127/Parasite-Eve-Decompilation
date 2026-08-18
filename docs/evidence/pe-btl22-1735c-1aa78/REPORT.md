# PE-BTL22 — type-1 0x08 / 1735C / 1AA78 / 1C614

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C.

## func_8001735C

38 words `0x8001735C..0x800173F4`, SHA-256 `a367c31f…9af8`.
`D_800910A0[0x08]`. Zero TEXT jal sites. Always v0=1.

```text
desc[0] = (u8)*arg0
desc[1] = (u8)*arg1
actor = 35038(&desc, *D_8009D2F0, 1)
actor+0x28 = *arg2
actor+0x2C = *arg3
actor+0x30 = *arg4          # delay of jal 1AA78
1AA78(actor)
return 1
```

Live type-1 second visit, three `0x0000A008` (argc 5, kinds 0):

| order | type | idB | +0x28 | +0x2C | +0x30 |
|---|---:|---:|---|---|---|
| 1 | 3 | 0 | `0x20000` | `0xFBC80000` | `0x2710000` |
| 2 | 0 | 0 | 0 | 0 | 0 |
| 3 | 5 | 0 | 0 | 0 | 0 |

Parent is the current type-1 actor. Inserts as siblings after
it. 17018 already advanced CE00; v0=1 re-fetches the next 0x08
in the same visit. Type 0 publishes `D_8009D254`. `D2E8` bit0
stays set (0x3F is not on this prefix).

## func_8001AA78

154 words `0x8001AA78..0x8001ACE0`, SHA-256 `eaf36cc4…a935`.
Callers `12C9C`, `1735C@173D4`. Void. Jals `1C614` and `3708C`.

`+0x98 & 0x80` returns immediately. 35038 ORs `0xE0` when
`+0x1AC==0`, so empty-`B0E70[type]` actors no-op here.

Live 6B4F8 hdr+0x0C writes `D_800B0E70[idB]` for idB 2 and 5
only. Type 3 stays 0. Type 0's `[0]` is the later 6C118 package
bind (not this cut). Host BSS therefore keeps type 3 / type 0
on the 0x80 no-op unless those publishers are run. Type 5 is
nonempty after the 6B804 loop.

Live 1A918 obj: `lhu(+2)=1`, `+0x20=0x760` → `D1D8 != 0` arm
(stride 28, three `3708C` into `+0x2C`).

## func_8001C614

114 words `0x8001C614..0x8001C7DC`, SHA-256 `53432d0c…942d`.
Zero jal. Three-edge crossing test. `v0 = t0`.

## Actor ledger

| off | note | confidence |
|---|---|---|
| +0x28 | 1735C `*arg2`; 1AA78 also `lhu +0x2A` as sx | PROVEN |
| +0x2C | 1735C `*arg3`; 1AA78 may overwrite on hit | PROVEN |
| +0x30 | 1735C `*arg4`; 1AA78 `lhu +0x32` as sz | PROVEN |
| +0x1A4 / +0x1A8 | 1AA78 hit record | PROVEN dest |

Type 3 first script word is already-ported `0x02` (yield).
Type 0 first is `0x9B`. Type 5 first is `0x14` then `0x0B`.
Do not force `3999C`.

## Verify

```text
python3 pc_port/tools/pe_btl22_1735c_1aa78_oracle.py
PE_TEST_FILTER=BTL22 ./pc_port/build/pe-native-tests
```
