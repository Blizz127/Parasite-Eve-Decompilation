# agent/decompile-ba3 — new maspsx delay-slot lever; small-leaf pool exhausted

Branch `agent/decompile-ba3` from `10b589e4` (main, 708 leaves). No new matching
leaf landed this run; one new **tooling lever** was added and the remaining
small-function candidates were triaged with first-mismatch data.

## New lever: `MASPSX_FILL_REGISTER_STORE_DELAY_SLOT=1`

`tools/era/maspsx/maspsx/__init__.py` gains a per-leaf gate (default OFF,
env-selected) for a **plain register-offset store** immediately before a bare
`j $31`:

```
sh $6,2($4)
j $31
```

cc1 leaves the store pre-`jr`; retail ASPSX scheduled it into the return delay
slot. The gate emits `j $31` then the store (a pure reorder, no address
synthesis) and consumes the original jump line. This is distinct from the two
existing gates: `MASPSX_FILL_STORE_DELAY_SLOT` handles absolute `sw $r,SYM`, and
`MASPSX_FILL_INDEXED_STORE_DELAY_SLOT` handles symbolic `op $r,SYM($base)`.

Unit tests added to the tracked `tools/era/maspsx/tests/test_fill_store_delay_slot.py`
(`TestFillRegisterStoreDelaySlotGuards`: fills a register store, never fills a
load, a label blocks the fill). All 9 tracked gate tests pass:

```
cd tools/era/maspsx && PYTHONPATH=. python3 tests/test_fill_store_delay_slot.py
Ran 9 tests in 0.005s
OK
```

The gate default OFF leaves the exact rebuild untouched:

```
bash scripts/build_us.sh  -> RESULT: EXACT MATCH, SHA-1 452fb033… (708 leaves)
bash scripts/verify_us.sh -> VERIFY_US=PASS, all 708 packed C spans equal retail
```

## Near-misses (with the gate enabled where noted)

All are register-allocation/scheduling misses against the retail ccpsx output.
First-mismatch words (`retail` vs `candidate`), file offset 0x0 of the leaf:

| leaf | span | bytes | first mismatch | note |
|---|---|---|---|---|
| `func_80087798` | 0x77F98 / 0x24 | 36 vs 48 | 0x000C `00822021` vs `00821021` | cc1 puts the MMIO pointer in `v0`; retail reuses `a0` (`addu a0,a0,v0` vs `addu v0,a0,v0`). cc1 emits `li $2,0x1f800000 / ori / sll $4 / addu $2,$4,$2` for every source shape tried (`a`, `a2`–`a5`, pointer/temp/reassign variants). |
| `func_8003E0A4` | 0x2E8A4 / 0x2C | 44 vs 48 | 0x0014 `24020001` vs `24070001` | selector lands in `a3` instead of `v0`; the gate fixed the trailing `sh $a2,0x2A($a0)` delay slot (0x0024 now matches). Register binding `register int v0 asm("$2")` made it worse. |
| `func_80056C14` | 0x47414 / 0x2C | 44 vs 48 | 0x0010 `00220821` vs `00410821` | only the `addu` operand order differs (`addu v0,at,v0` vs `addu v0,v0,at`) plus one extra trailing word; every expression order tried (`base+idx`, `idx+base`, `<<5`, pointer index, temp) is byte-identical. |

Also confirmed as pre-existing non-matches (not re-attempted): `func_800370BC`/`CC`
(reg-reg `add` vs `addu`), `func_8001A374` (load-delay nop), `func_8005DB8C`
(load hoisted above shift), `func_80078C94`, `func_80075C44`, `func_80083578`,
`func_80082ADC`, `func_80084F8C`, `func_8006599C`/`659C8`, `func_80018E84`,
`func_800199CC`.

## Reproduction

```
git worktree add /tmp/pe-agent-decomp-ba3 -b agent/decompile-ba3 10b589e4
# copy tools/era build/extracted asm include local/pe_disc1.path rom/image
bash scripts/split_us.sh                       # host (needs splat)
distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp-ba3 && bash scripts/build_us.sh'
```
