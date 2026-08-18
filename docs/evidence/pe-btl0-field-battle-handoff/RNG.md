# RNG

## Script opcode `0x1A`

```text
handler   0x800176FC
if arg1 == arg2:  jal func_80070D6C()
else:             jal func_80070DD0(arg1, arg2)
sw result, *dest
return 1
```

`func_80070D10` / `70D6C` / `70DD0` are the already-translated
lagged-Fibonacci RNG (native port / oracle). `0x1A` is a **generic
field RNG**, used on many maps that never fight.

## First Day 1 consumption

m0005i module 2 `+0x1DCC`:

```text
0x1A  dest=local[24]  lo=0  hi=100     ; 70DD0 range
0x09  slt local[24] < 19
      < 19  -> local[24] = 49
      else  -> local[24] = 50
0x6F  always
```

Width: 32-bit result from `70DD0`. One roll. It selects a **variant
id** (49 vs 50), not whether battle happens.

## Battle cluster

Scan `0x80029800–0x80031000` for `jal` to `70D10` / `70D6C` / `70DD0`:
**zero hits**.

Entry after the script roll does not consume further retail RNG in
that window. Full in-battle hit/miss RNG is out of scope.

```text
rng_handoff_status=SCRIPT_0x1A_BEFORE_SETUP
battle_init_jal_rng=no
```
