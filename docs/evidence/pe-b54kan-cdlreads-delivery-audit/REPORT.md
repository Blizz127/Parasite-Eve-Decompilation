# PE-B54K-AN — CdlReadS delivery readiness audit

Status: **STATIC RETAIL AUDIT COMPLETE; NO DELIVERY IMPLEMENTED**.

B54K-AM reached the real low-level issue boundary. This audit maps what must
happen after that issue and rejects the tempting but false shortcut of
copying sectors directly into the movie ring and returning success.

## Authenticated callback chain

```text
func_800813E8 wrapper [0x800813E8,0x80081408) /   8 words
SHA-256              0a54e2df91311e97e5ae88a9dc0b0ca485386c20ae1bb2a109b83ae193f7edd0
func_8007C564 CD cb   [0x8007C564,0x8007CE80) / 583 words
SHA-256              7c4ca8096a8921313e5b9a5f036f2b9341d656633afed2132b62af8d40aa8c74
func_8007C214 DMA cb  [0x8007C214,0x8007C2A0) /  35 words
SHA-256              bfe0812b6e5d671c9b90c55f63306291ed8a733953e57a1378930757146bc775
```

`func_800813E8` is only a canonical wrapper around `func_8007C564`.
`func_8007C564` is not a completion bit setter. It is a 583-word state
machine over the 64-record pool and stream globals. Static control flow
proves that it:

- gates re-entry with `D_800B89F4`;
- selects the producer record using `D_800BE998`, pool base `D_800C0DC8`,
  and 32-byte record stride;
- examines stream/header state through the `D_8009B32C..D_8009B374`
  family and mode globals established by `func_8007C304`;
- copies variable word runs with `func_8007CE80`;
- issues channel DMA through `func_8007CEAC`, which polls channel CHCR at
  `0x1F801088 + channel*0x10` and programs DMA registers;
- advances or clears producer/consumer and partial-record state across
  several branches;
- writes record status `3` before advancing the producer; and
- conditionally calls `func_8007C214` at its tail when `D_800B89F4` is
  nonzero.

The 35-word DMA callback is a distinct phase. It marks the selected
record status `2`, copies its tail metadata to `D_800A3490/94`, publishes
the consumer index, invokes the optional stream callback at `D_800B0CC8`,
then clears `D_800B89F4`. Retail registers this function on DMA channel 3,
but the CD handler also has the proven conditional direct call at its tail.
Static evidence alone does not yet distinguish every direct-call versus DMA
dispatcher case. It does prove that parsing/status `3` precedes any such
status-`2` publication and consumer notification.

## Decision

A host shortcut that reads the STR file, advances `D_800BE998`, and invokes
the callback in one call would erase retail's partial-sector branches,
record statuses, ring-full behavior, DMA-in-flight state, and callback
ordering. B54K-AN adds no such shortcut.

The next implementation rung must first define and test a generic event
contract with at least:

1. four-command queue acceptance and ready/queue transitions;
2. one authenticated raw/user-sector input from the current CdlLOC;
3. CD callback parsing into the exact 32-byte producer record;
4. DMA3 issue plus the exact `D_800B89F4` rule selecting direct-tail versus
   dispatcher completion;
5. status `3 -> 2`, producer/consumer, and callback ordering;
6. ring-full, malformed-sector, absent-disc, and stale-completion controls.

The real Disc 1 STR is already available, so this is artifact-free work, but
it is a subsystem rung rather than a safe one-function collapse.

```text
CdlReadS_DELIVERY=NOT_IMPLEMENTED
DIRECT_SECTOR_TO_RING_SHORTCUT=REJECTED
CALLBACK_ORDER=PARSE_STATUS3_BEFORE_STATUS2_AND_NOTIFY
PRODUCTION_REACHABILITY=blocked_at_func_80081314_func_8007F0C8_cut
NEXT_RUNG=generic_CdlReadS_event_contract_and_first_sector_oracle
```
