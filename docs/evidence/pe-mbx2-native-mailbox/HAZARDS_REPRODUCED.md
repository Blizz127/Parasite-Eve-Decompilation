# Hazards reproduced (retail-faithful-and-unsafe)

These are EXE behavior. A clamp, a wipe, or a hop-clear is a
failed rung. They are recorded as DEBT1
`RETAIL_FAITHFUL_UNSAFE` instead of being "fixed."

## DEBT-FID1-034 — no clamp at 28

`func_800653B8` indexes `D_800A3180 + count*12` with the raw `u8`
count. A 29th append in one tick writes at `0x800A32D0`.

Native backing covers the full `u8` index space so that write is
observable instead of C++ UB. Index 28 is a real store. There is
no `if (count >= 28) return`.

`test_mailbox_hazard_no_clamp`: 28 appends leave a `0xCD` canary
at slot 28; the 29th overwrites it (`type=9 id=8 payload=0x2A`);
`count==29`.

`test_mailbox_boot_clear_spares_overflow`: `6536C` zeros 28 rows
only; slot 28 `0xAB` survives.

## DEBT-FID1-035 — drain zeros the count only

`func_80065400` ends with `sb $zero, 0x44($gp)`. It does not
store into the records.

`test_mailbox_hazard_stale_after_drain`: two appends, drain,
`count==0`, slot 0 payload `0x1B` and slot 1 payload `0x1C` still
readable.

## DEBT-FID1-036 — undelivered records persist across a hop

`6536C` is boot-only. Scene load / `enter_m0003i` does not zero
the table.

`test_mailbox_hazard_hop_persist`: append `0xFF`, rebuild the
actor, `count==1` and the record bytes remain.

`test_mailbox_runtime_hop_stale`: append `0x3C` after
`runtime_boot_m0002i`, run the live hop; after
`PlayingM0003i` the count is 0 (drain ran) and the stale bytes
are still `payload=0x3C` / `sender=0x99`.

## What was not done

```text
clamp_added=no
records_wiped_on_drain=no
undelivered_persists_across_hop=yes
python_shim_rehosted=no
```

DEBT-FID1-004 is no longer "UE0 fail-closes 0x1C/0x1F." The
native lane now has the queue. The Python payload-byte machine
and the matching-tree empty table after `6536C` remain other
lanes.
