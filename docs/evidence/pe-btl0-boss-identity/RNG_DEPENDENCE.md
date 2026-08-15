# RNG_DEPENDENCE — 49/50 vs the 0x89 store

## Verdict

```text
formation_rng_dependent = yes          ; the 49/50 write
identity_rng_dependent  = no           ; 1332/1333/1334 and opened names
rng_consumed_at_0x89    = no
```

BTL0's `rng_handoff_status=SCRIPT_0x1A_BEFORE_SETUP` stands. `0x1A` is
still generic field RNG (`func_800176FC` → `func_80070DD0` when bounds
differ). The battle cluster `0x80029800–0x80031000` still has **zero**
`jal` to `70D10` / `70D6C` / `70DD0`.

## Exact roll

Site: m0005i module 2 `+0x1DCC`, **before** `0x6F` and **before** the
first `0x89` at module 6 `+0x350C`.

```text
handler   0x800176FC
lo=0  hi=100  ->  jal func_80070DD0(0, 100)
```

`func_80070DD0` (13 words, `0x80070DD0`):

```text
r      = func_80070D6C() & 0xFFFF
result = 0 + ((int64)(int)r * 100 >> 16)     ; 32-bit wrap on the add
```

Range is `0..99`. Then `slt < 19` writes `local[24] = 49` else `50`.

`func_80070D6C` returns the 32-bit sum of two lagged-Fibonacci table
words and stores that sum back. Table / indices (already translated):

```text
D_80070E0C   17-word table
D_80070E04   index1  (byte offset, seed 0x40)
D_80070E08   index2  (byte offset, seed 0x10)
```

The word that must be reproducible is that **`func_80070D6C` 32-bit
result** (low 16 bits scale the 0..99 draw). Pinning the 14-word
image `0x80070E04..0x80070E4C` immediately **before** the `0x1A` is
equivalent and matches BTL0 `TRACE_CONTRACT` column `rng_state`.

## Not at the 0x89 store

`0x89` / `func_80017FF0` is only `D_8009D28C = 6`. It does not call
the RNG trio. By the time the script reaches `+0x350C`, `local[24]`
already holds 49 or 50. Pinning RNG **at** the `0x89` store is too
late for the variant and unnecessary for enemy identity.

```text
rng_word_to_pin = func_80070D6C_u32_low16
rng_pin_site    = m0005i_mod2_+0x1DCC
rng_pin_at_0x89 = no
```

A later `0x1A` exists at module 6 `+0x4668` (`local[31] = rng(0,11)`).
That is after all three `0x89` and is not the formation variant.

## BTL1

Identity (Actress / Eve strings, slot 1332/1333/1334) is constant
across the 49/50 arms. A first-play trace is deterministic for
**who is fought** without a pin. Record the `0x1A` result anyway so
`local[24]` and the TRACE_CONTRACT `formation_id` cell (`49;1332,1333,1334`
or `50;1332,1333,1334`) match a given first-play seed.

```text
btl1_determinism_ready = YES
```
