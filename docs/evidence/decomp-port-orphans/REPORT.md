# Decomp-port orphans — TUs with no in-tree C leaf

**Status:** open (tracked). **Decision:** keep, allowlisted.

## Summary

- `pc_port/game/decomp/` contains **272** generated host TUs.
- **190** have a matching `src/<name>.c` leaf and regenerate byte-for-byte from it (see `docs/ai_context/PC_PORT_FROM_DECOMP.md`).
- **82** have **no** `src/<name>.c` in this checkout and cannot be regenerated from authority. They are the subject of this report.
- Every one of the 82 addresses lies inside an anonymous `asm` subsegment of `configs/USA/disc1.yaml`, so each is real retail code whose matching C leaf exists on the generator's branch/worktree but not here.
- Every one is referenced as a callee by at least one src-backed generated TU, so the set is load-bearing: dropping the files without also re-deriving the callers would leave unresolved calls.
- No orphan has an equivalent leaf under another name in this checkout (`src/` is flat `src/func_XXXXXXXX.c`; none match).

## Method

```sh
python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans
python3 tools/analysis/gen_decomp_ports.py --check      # lists the 82
```

Cross-reference produced by matching each orphan name against all of `src/`, `configs/`, the other generated TUs, and the hand-written `pc_port/` sources; the address comparison uses the subsegment table in `configs/USA/disc1.yaml`. Raw table: `orphans.tsv`.

## Recommendation

**Keep all 82 files.** They are compiled, referenced host TUs whose behavior is exercised by the existing suite; removing them would break the link and lose working ports. Mark them as lacking in-tree authority through the machine-checkable allowlist `tools/analysis/decomp_port_orphans.txt` (checked by `pe-decomp-port-tests`) rather than by editing the generated files, which carry a `do not edit by hand` banner.

The correct long-term fix is **not** to delete: restore the matching `src/<name>.c` leaves from the branch that generated them, add the corresponding `c` subsegment to `configs/USA/disc1.yaml`, then re-run the generator. At that point the TU regenerates from authority and the allowlist entry must be removed (the check fails on a stale allowlist entry, which forces this).

## The 82 orphans

| # | TU | header VMA | header file | header span | enclosing asm |
|---|---|---|---|---|---|
| 1 | `func_80019260` | `0x80019260` | `0x9A60` | `0x38` (14 w) | `0x9970` |
| 2 | `func_80019CEC` | `0x80019CEC` | `0xA4EC` | `0x38` (14 w) | `0xA44C` |
| 3 | `func_80038910` | `0x80038910` | `0x29110` | `0x30` (12 w) | `0x28070` |
| 4 | `func_80042964` | `0x80042964` | `0x33164` | `0x28` (10 w) | `0x33128` |
| 5 | `func_8004BCB4` | `0x8004BCB4` | `0x3C4B4` | `0x34` (13 w) | `0x3BD84` |
| 6 | `func_800504BC` | `0x800504BC` | `0x40CBC` | `0x34` (13 w) | `0x40A80` |
| 7 | `func_800528C4` | `0x800528C4` | `0x430C4` | `0x2C` (11 w) | `0x43094` |
| 8 | `func_8005E4E4` | `0x8005E4E4` | `0x4ECE4` | `0x34` (13 w) | `0x4E92C` |
| 9 | `func_8005E518` | `0x8005E518` | `0x4ED18` | `0x34` (13 w) | `0x4E92C` |
| 10 | `func_8006599C` | `0x8006599C` | `0x5619C` | `0x2C` (11 w) | `0x55C00` |
| 11 | `func_80065B70` | `0x80065B70` | `0x56370` | `0xC8` (50 w) | `0x55C00` |
| 12 | `func_8006F224` | `0x8006F224` | `0x5FA24` | `0xA0` (40 w) | `0x5F484` |
| 13 | `func_8006F2C4` | `0x8006F2C4` | `0x5FAC4` | `0xD8` (54 w) | `0x5F484` |
| 14 | `func_80075C04` | `0x80075C04` | `0x66404` | `0x40` (16 w) | `0x654C8` |
| 15 | `func_80075C6C` | `0x80075C6C` | `0x6646C` | `0x28` (10 w) | `0x654C8` |
| 16 | `func_80075C94` | `0x80075C94` | `0x66494` | `0x54` (21 w) | `0x654C8` |
| 17 | `func_80076B20` | `0x80076B20` | `0x67320` | `0x24` (9 w) | `0x66B54` |
| 18 | `func_80076B44` | `0x80076B44` | `0x67344` | `0x14` (5 w) | `0x66B54` |
| 19 | `func_80076B58` | `0x80076B58` | `0x67358` | `0x40` (16 w) | `0x66B54` |
| 20 | `func_80076BE0` | `0x80076BE0` | `0x673E0` | `0x30` (12 w) | `0x66B54` |
| 21 | `func_80077D30` | `0x80077D30` | `0x68530` | `0x90` (36 w) | `0x684EC` |
| 22 | `func_8007A400` | `0x8007A400` | `0x6AC00` | `0x34` (13 w) | `0x6AC00` |
| 23 | `func_8007A434` | `0x8007A434` | `0x6AC34` | `0x34` (13 w) | `0x6AC00` |
| 24 | `func_8007DBC8` | `0x8007DBC8` | `0x6E3C8` | `0x3C` (15 w) | `0x6D874` |
| 25 | `func_8007DC5C` | `0x8007DC5C` | `0x6E45C` | `0x28` (10 w) | `0x6D874` |
| 26 | `func_8007DC84` | `0x8007DC84` | `0x6E484` | `0x28` (10 w) | `0x6D874` |
| 27 | `func_8007DD74` | `0x8007DD74` | `0x6E574` | `0x34` (13 w) | `0x6E538` |
| 28 | `func_8007DE40` | `0x8007DE40` | `0x6E640` | `0x38` (14 w) | `0x6E538` |
| 29 | `func_8007DFE0` | `0x8007DFE0` | `0x6E7E0` | `0x30` (12 w) | `0x6E6C0` |
| 30 | `func_8007E0C0` | `0x8007E0C0` | `0x6E8C0` | `0x38` (14 w) | `0x6E6C0` |
| 31 | `func_8007E594` | `0x8007E594` | `0x6ED94` | `0x30` (12 w) | `0x6E6C0` |
| 32 | `func_800858E8` | `0x800858E8` | `0x760E8` | `0x30` (12 w) | `0x75F44` |
| 33 | `func_80085918` | `0x80085918` | `0x76118` | `0x34` (13 w) | `0x75F44` |
| 34 | `func_80085F44` | `0x80085F44` | `0x76744` | `0x24` (9 w) | `0x765E8` |
| 35 | `func_800864CC` | `0x800864CC` | `0x76CCC` | `0x2C` (11 w) | `0x765E8` |
| 36 | `func_80086568` | `0x80086568` | `0x76D68` | `0x3C` (15 w) | `0x765E8` |
| 37 | `func_800865A4` | `0x800865A4` | `0x76DA4` | `0x64` (25 w) | `0x765E8` |
| 38 | `func_800866F0` | `0x800866F0` | `0x76EF0` | `0x38` (14 w) | `0x765E8` |
| 39 | `func_800867B0` | `0x800867B0` | `0x76FB0` | `0x34` (13 w) | `0x765E8` |
| 40 | `func_800867E4` | `0x800867E4` | `0x76FE4` | `0x48` (18 w) | `0x765E8` |
| 41 | `func_80086874` | `0x80086874` | `0x77074` | `0x38` (14 w) | `0x765E8` |
| 42 | `func_80086948` | `0x80086948` | `0x77148` | `0x64` (25 w) | `0x765E8` |
| 43 | `func_800869AC` | `0x800869AC` | `0x771AC` | `0x38` (14 w) | `0x765E8` |
| 44 | `func_800869E4` | `0x800869E4` | `0x771E4` | `0x44` (17 w) | `0x765E8` |
| 45 | `func_80086A80` | `0x80086A80` | `0x77280` | `0x64` (25 w) | `0x765E8` |
| 46 | `func_80086AE4` | `0x80086AE4` | `0x772E4` | `0x38` (14 w) | `0x765E8` |
| 47 | `func_80086B1C` | `0x80086B1C` | `0x7731C` | `0x44` (17 w) | `0x765E8` |
| 48 | `func_80086B60` | `0x80086B60` | `0x77360` | `0x58` (22 w) | `0x765E8` |
| 49 | `func_80086BB8` | `0x80086BB8` | `0x773B8` | `0x64` (25 w) | `0x765E8` |
| 50 | `func_80086CF8` | `0x80086CF8` | `0x774F8` | `0x34` (13 w) | `0x765E8` |
| 51 | `func_80086D2C` | `0x80086D2C` | `0x7752C` | `0x3C` (15 w) | `0x765E8` |
| 52 | `func_80086D68` | `0x80086D68` | `0x77568` | `0x44` (17 w) | `0x765E8` |
| 53 | `func_80086DAC` | `0x80086DAC` | `0x775AC` | `0x38` (14 w) | `0x765E8` |
| 54 | `func_80086DE4` | `0x80086DE4` | `0x775E4` | `0x40` (16 w) | `0x765E8` |
| 55 | `func_80086E24` | `0x80086E24` | `0x77624` | `0x4C` (19 w) | `0x765E8` |
| 56 | `func_80086E70` | `0x80086E70` | `0x77670` | `0x38` (14 w) | `0x765E8` |
| 57 | `func_80086EA8` | `0x80086EA8` | `0x776A8` | `0x40` (16 w) | `0x765E8` |
| 58 | `func_80086EE8` | `0x80086EE8` | `0x776E8` | `0x4C` (19 w) | `0x765E8` |
| 59 | `func_80086F34` | `0x80086F34` | `0x77734` | `0x38` (14 w) | `0x765E8` |
| 60 | `func_80086F6C` | `0x80086F6C` | `0x7776C` | `0x40` (16 w) | `0x765E8` |
| 61 | `func_80086FAC` | `0x80086FAC` | `0x777AC` | `0x4C` (19 w) | `0x765E8` |
| 62 | `func_80087050` | `0x80087050` | `0x77850` | `0x40` (16 w) | `0x765E8` |
| 63 | `func_80089F08` | `0x80089F08` | `0x7A708` | `0x1C` (7 w) | `0x7A510` |
| 64 | `func_8008A02C` | `0x8008A02C` | `0x7A82C` | `0x3C` (15 w) | `0x7A510` |
| 65 | `func_8008C6D0` | `0x8008C6D0` | `0x7CED0` | `0x3C` (15 w) | `0x7CA90` |
| 66 | `func_8008E7F4` | `0x8008E7F4` | `0x7EFF4` | `0x4C` (19 w) | `0x7E044` |
| 67 | `func_8008F178` | `0x8008F178` | `0x7F978` | `0x38` (14 w) | `0x7E044` |
| 68 | `func_8008FED8` | `0x8008FED8` | `0x806D8` | `0x24` (9 w) | `0x804BC` |
| 69 | `func_80090054` | `0x80090054` | `0x80854` | `0x24` (9 w) | `0x804BC` |
| 70 | `func_80090178` | `0x80090178` | `0x80978` | `0x24` (9 w) | `0x804BC` |
| 71 | `func_800906E4` | `0x800906E4` | `0x80EE4` | `0x38` (14 w) | `0x80EE4` |
| 72 | `func_800C8C80` | `0x800C8C80` | `0xB9480` | `0x3C` (15 w) | `0xB9480` |
| 73 | `func_800C8CBC` | `0x800C8CBC` | `0xB94BC` | `0x3C` (15 w) | `0xB9480` |
| 74 | `func_800C8CF8` | `0x800C8CF8` | `0xB94F8` | `0x3C` (15 w) | `0xB9480` |
| 75 | `func_800C9A34` | `0x800C9A34` | `0xBA234` | `0x3C` (15 w) | `0xBA234` |
| 76 | `func_800CBBF0` | `0x800CBBF0` | `0xBC3F0` | `0x3C` (15 w) | `0xBC3F0` |
| 77 | `func_800CBC2C` | `0x800CBC2C` | `0xBC42C` | `0x3C` (15 w) | `0xBC3F0` |
| 78 | `func_800CBC68` | `0x800CBC68` | `0xBC468` | `0x3C` (15 w) | `0xBC3F0` |
| 79 | `func_800CCA40` | `0x800CCA40` | `0xBD240` | `0x38` (14 w) | `0xBC7C4` |
| 80 | `func_800CCA78` | `0x800CCA78` | `0xBD278` | `0x38` (14 w) | `0xBC7C4` |
| 81 | `func_800CCB6C` | `0x800CCB6C` | `0xBD36C` | `0x3C` (15 w) | `0xBC7C4` |
| 82 | `func_800CD5B0` | `0x800CD5B0` | `0xBDDB0` | `0x3C` (15 w) | `0xBDDB0` |

## Verification

```
python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans
# targets: 272
# orphans (no src/ leaf, cannot regenerate): 82
# stale boundaries (callee is now implemented): 0
# mismatched (regeneration differs): 0
# check: OK
```
