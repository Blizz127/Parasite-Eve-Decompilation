# Pump ordering

Retail `func_8003F3C4` (field tick, caller `0x800123D8`):

```text
D_8009D250++
jal func_80065400      # drain previous 0x1C   @ 0x8003F4E8
jal func_80035558      # actor / script tick   @ 0x8003F4F0
```

A send in this frame is visible to `0x1F` on the **next** field
frame, after the next drain creates the poll task.

## Native site

```text
runtime_tick_input  (every field frame, all phases)
  mailbox_drain(&sim.mailbox, &sim, &vm)     # func_80065400
  field_tick(...)                            # actor motion
  vm_tick(...)                               # scripts = 35558 stand-in
  maybe enter_m0003i                         # hop does not clear the queue
```

`field_pump_func_8003F3C4` is the same order for isolated tests.

```text
pump_site=func_8003F3C4
pump_order=before_func_80035558
drain_per_tick=1
```

## Proof the pump is before this frame's scripts

Unit `test_mailbox_pump_order`:

1. Live task issues `0x1C (1,0,7)` then yields.
2. After one pump: `count==1`, no poll task, `local[4]==0`.
3. After the next pump: drain delivers, `0x1F` writes `7`.

If drain ran after scripts, step 2 would already have delivered.

## Oracle path

m0002i→m0003i has no `0x1C`. The pump is a no-op there
(`count==0` → store 0 and return). A moved trace would mean the
drain ran at the wrong point or mutated other state. The fixture
was not rewritten.

## Hop

Same-tick `0x1C` then `0x31` hops with `count>0` because drain
already ran. Next scene's first `65400` offers those records to
whatever actors now match. `enter_m0003i` rebuilds the actor
(extras cleared) and does **not** call `6536C`.
