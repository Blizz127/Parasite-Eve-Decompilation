# PE-MV1b — 7A88C/7B964 live; 91B64 BB0 fix; BD4C verified then parked on delivery

## What landed

- `func_8007A88C` (8 words, `0x8007A88C..0x8007A8AC`, from the
  splitter gap between `6ACD0.s` and `6B130.s`): call 7B964,
  return 1. The 870F0 caller ignores the result.
- `func_8007B964` (34 words, `0x8007B964..0x8007B9EC`): pushes
  the four 870F0 latch bytes plus the 2/3/0x20 tags through the
  CD pointer tables, shadow-routed by CD0. Sole caller is 7A88C
  (no other `jal 0x8007B964` in any dump).
- `func_80191B64` BB0 inversion fix: retail is `beqz v0 → BD0`
  with `s0--` in the delay (zero proceeds, nonzero retries to
  the 2000-countdown `return 0`). The MV1a port had it backwards.
  Proven by the `1BB0..1BCC` bytes of the overlay dump.
- B54KAD/AE pin the live 7B964 through the shadow
  (`[1F801800..03] == 3,0,0,0x20`, mailbox `0x1325`).

## BD4C: transcribed, verified on the real stream, reverted

`func_8010BD4C` (overlay `0x8010BD4C..0x8010BE2C`, disassembled
from `/tmp/ov_10BD4C.bin` with capstone) is a terminator-driven
RLE decoder: datum < 0xF0 literal/back-copy runs of datum+1
bytes (both `bltz` dead: `a1 = datum & 0xFF >= 0`; both copy
loops advance the pointer in the `bgez` delay slot, even on the
exiting byte), datum == 0xF0 clears v1, datum > 0xF0 loads the
extension byte with `v1 = ((datum << 8) | ext) + 0xFFFF0F01`
(mod 2^32), exit when `v1 == 0xF00` (the FF FF terminator:
0xFFFF + 0xFFFF0F01 wraps to 0xF00). The count argument is dead
(retail clobbers a1 with the first datum). Exit runs a
34812-halfword XOR-delta pass from dst+8 (a1 = 4..0x8800).

Live-trace on the B54KY real disc: `BD4C enter
dst=80162100 d0=07 d1=02`, `exit iters=2239 a2=80173100` — a
complete 69632-byte frame decode, then the XOR pass. Correct.

Reverted to the named stop anyway: the E0 poll behind BD4C
needs the 7C564 delivery pump. E0 exhausts (no state-2 slot
without delivery) into the give-up path, whose
`while (7F72C() != -1)` can never terminate in-port — nothing
in the translated tree writes -1 to the lane word (only 7ED58
→ 1 and 7FB44 → 2 exist; the -1 producer lives in the
untranslated completion path). Live BD4C hung B54KY in
give-up and would hang strict production the same way. The
wiring is reverted; the transcription below relands with
delivery. Strict frontier stays `func_8010BD4C`.

## Delivery firewall (next rung)

- `func_8007C214` is translated and publishes state 2, but
  nothing invokes it: `func_8010C0D8` registers the identity
  without delivering, and `func_80081314` installs 7C214 via
  DMA slot 3 without pumping completion.
- The pump (per-sector 7C214 + lane -1 on completion) is the
  583-word 7C564 machine and its interrupt path.
- Unvalidated plant design for the reland (never executed —
  the suite hung in B54KY first): state-2 record at
  `0x801FFF30`, rec steering 91B64 past its 870F0 re-entry,
  70 KB buffer at `0x801B0000`, FF FF at `0x8010CBFC`.

## Reland transcription (verified, unwired)

```c
void func_8010BD4C(pe_addr_t dst, uint32_t count)
{
    uint32_t v1 = 0u;
    uint32_t a3 = 0x8010CBFCu;
    uint32_t a2 = dst;
    const uint32_t t2 = 0xF0u;
    const uint32_t t0 = 0xFFFF0F01u;
    const uint32_t t1 = 0xF00u;
    uint32_t a1;
    (void)count;
    for (;;) {
        uint32_t v0 = PE_LoadU8(a3);
        a1 = v0 & 0xFFu;
        if (a1 >= 0xF0u) {
            a3++;
            if (a1 == t2) {
                v1 = 0u;
            } else {
                v1 = PE_LoadU8(a3);
                a3++;
                v1 = ((a1 << 8) | v1) + t0;
            }
        } else {
            a3++;
            if (v1 != 0u) {
                if ((int32_t)a1 >= 0) {
                    for (;;) {
                        v0 = PE_LoadU8(a2 - v1);
                        a1--;
                        PE_StoreU8(a2, (uint8_t)v0);
                        a2++;
                        if ((int32_t)a1 < 0)
                            break;
                    }
                }
            } else {
                if ((int32_t)a1 >= 0) {
                    for (;;) {
                        v0 = PE_LoadU8(a3);
                        a3++;
                        a1--;
                        PE_StoreU8(a2, (uint8_t)v0);
                        a2++;
                        if ((int32_t)a1 < 0)
                            break;
                    }
                }
            }
        }
        a1 = 4u;
        if (v1 != t1)
            continue;
        break;
    }
    {
        uint32_t a1x = 4u;
        uint32_t a0x = dst + 8u;
        for (;;) {
            uint32_t x0 = PE_LoadU16(a0x);
            uint32_t x1 = PE_LoadU16(a0x - 8u);
            a1x++;
            PE_StoreU16(a0x, (uint16_t)(x0 ^ x1));
            a0x += 2u;
            if (0x87FFu < a1x)
                break;
        }
    }
}
```

## Verify

```
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
Results: 1064 run, 1064 passed, 0 failed, 0 skipped
```

Both with `PE_DISC1_BIN`. Gateless: `1046 passed, 1 failed
(B54KY env), 17 skipped`. Normal CTest with disc: `100% tests
passed, 0 tests failed out of 2`. Fresh ASan/UBSan CTest with
disc: `100% tests passed, 0 tests failed out of 2`, zero
sanitizer diagnostics. MV1a oracle still green (870F0
untouched). Strict disc run exits at `func_8010BD4C` from
`func_801924F8` (no hang). Leaves 560; no src/YAML changes.
