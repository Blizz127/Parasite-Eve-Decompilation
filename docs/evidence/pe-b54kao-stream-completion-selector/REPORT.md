# PE-B54K-AO — stream completion selector

Status: **STATIC RETAIL SELECTOR CLOSED; NO DELIVERY IMPLEMENTED**.

B54K-AN left one ambiguity: the CD handler both registers
`func_8007C214` on DMA channel 3 and contains a conditional direct call to
it. This rung proves which branch the production movie path selects.

## Authority census

`D_800C0DB8` is the selector. A direct materialized-address store census over
the retail executable and all PE.IMG finds exactly one writer:

```text
8007C330 lui at,0x800C
8007C334 sw  zero,0x0DB8(at)
```

It is inside the complete `func_8007C304` initializer already executed by
the movie path. PE.IMG contains zero direct stores to this address. Thus the
authenticated production value is zero.

## Exact lifecycle

`D_800B89F4` has only these direct owners in the executable:

- record-pool initialization clears it;
- `func_8007C214` clears it after status-2 publication/notification;
- `func_8007C564` returns immediately when it enters with value `1`;
- one branch in `func_8007C564` sets it to `1` before the data-transfer path;
- the CD-handler tail first tests `D_800C0DB8`; only a nonzero selector can
  reach the conditional direct call to `func_8007C214`.

Because production `D_800C0DB8 == 0`, the tail branches directly to return.
It does **not** call the completion callback inline. `D_800B89F4` remains one
until the separately registered DMA3 callback runs and clears it. New CD
callback entries are suppressed while that DMA is in flight.

This closes the production rule:

```text
CD sector parse / DMA3 issue
-> D_800B89F4 = 1
-> CD handler returns without status-2 publication
-> later DMA3 dispatch calls func_8007C214
-> record status = 2; optional consumer notify; D_800B89F4 = 0
```

The alternate `D_800C0DB8 != 0` direct-tail mode exists in retail but is not
selected by this path. No host state is changed by this audit.

```text
PRODUCTION_STREAM_SELECTOR_D_800C0DB8=0
PRODUCTION_COMPLETION=SEPARATE_DMA3_CALLBACK
CD_REENTRY_WHILE_DMA=SUPPRESSED_BY_D_800B89F4
PRODUCTION_REACHABILITY=blocked_at_func_80081314_func_8007F0C8_cut
NEXT_RUNG=first_CdlReadS_sector_parse_and_deferred_DMA3_test_contract
```
