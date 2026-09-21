# func_80044E14 — landed (770 → 771)

**Slice:** pb-multiunit (agent/pb-multiunit)
**VRAM:** 0x80044E14 · **file:** 0x35614 · **size:** 0x84 (33 words)
**Unit:** 35614.s (head), resume at 0x35698
**Profile:** `era_o2_g0` (default)

## Result

| gate | value |
| --- | --- |
| `try_leaf.py src/func_80044E14.c 0x35614 0x84 --flags "-O2 -G0"` | `WORDS MATCH (+12 pad bytes, trimmed by the build)` |
| `bash scripts/build_us.sh` | `EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, `Matching claim: YES (771 registered C leaves)` |
| `bash scripts/verify_us.sh` | `VERIFY_US=PASS` |

## Semantics

`idx = *(int *)(arg0 + 0x24) - 0x29`, then:

1. `func_8005E8A4(0, 0xA)`
2. `func_8005EB58(0)`
3. `func_8005F594(D_800A1980 + idx * 64)`
4. `r = D_8009CFA0[idx]`; when `r != 0`: `func_8005E8A4(0, 0xE)` then
   `r = func_80062A7C(D_8009CFA0[idx])` (table entry reloaded at the call).
5. returns `r`.

Frame -0x18, `$ra`/`$s0` saved; `$a0` for the `beqz` delay slot is
zero, matching retail.

## Levers

1. **Reload through the array expression, not a live pointer local.**
   The matching form is `r = func_80062A7C(D_8009CFA0[idx]);`. Declaring
   `int *p = &D_8009CFA0[idx];` and passing `*p` keeps the pointer in
   `$s0` and the value in `$v0`, and sinks the table load into `$s0`
   (variant A = 1 diff, variant D = 10 diffs). The array form makes cc1
   materialize `&D_8009CFA0` into `$v0`, `addu $s0,$s0,$v0`, `lw $v0,0($s0)`
   then reload `lw $a0,0($s0)` — exactly retail.
2. `if (r != 0) { ... r = ...; } return r;` (assign, don't early-return)
   avoids a synthetic `bnez` + `j` epilogue (11 diffs).

## Divergences / negatives

* `-O1 -G0` = 20 diffs; `-O2 -G8` = 11.
* m2c draft had the flow but used synthetic `temp_s0_2` pointer typing.
