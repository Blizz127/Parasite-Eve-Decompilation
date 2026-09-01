# PE-B54K-AM — CdlReadS registration prefix

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates the production prefix of `func_80081314` through its
mode state and two callback registrations, then stops before the low-level
four-command `CdlReadS` queue issue. Registration is not delivery.

## Retail identity

```text
func_80081314 complete [0x80081314,0x800813E8) / 53 words
SHA-256               fe43d63bd26dd2279dbdc1c8858998cace3f0da9d7cd16172aa61b8d32a6ce42
translated prefix     [0x80081314,0x8008138C) / 30 words
SHA-256               ec6afdca38a2662a48bf9f6fa9f97113a48c2ed75c9e8b814c69d69a0d77da35
func_8007F0C8 complete [0x8007F0C8,0x8007F418) / 212 words
SHA-256               5c9c7e3533b61fe6c52eacfa4cac2b7934a7cfbba68c55089109db8c8604ad80
```

Production passes `(D_801D0DC4, 0x1E0)`. Both `0x100` and `0x20` are set,
so retail stores zero to `D_800A8020`, registers guest callback
`0x8007C214` through the complete channel-3 DMA setter, and exchanges
`D_800B8AB4` for guest callback `0x800813E8`. The exact helpers are:

```text
func_800824C8 [0x800824C8,0x800824DC) / 5 words
func_800824F0 [0x800824F0,0x80082514) / 9 words
```

The next call is `func_8007F0C8(0xE0, location, 0x1B, 0, -1)`. The retail
command-name table identifies `0x1B` as `CdlReadS`; its jump-table entry
selects `0x8007F2A4`. Static reversal proves that path constructs four queue
records in order: CdlPause (`9`), CdlSetmode (`0x0E`, parameter `0xE0`),
CdlSetloc (`2`, four-byte location), and CdlReadS (`0x1B`). It requires four
free queue slots, allocates command descriptors, and starts hardware service
when the CD lane is ready.

That final issue is deliberately not collapsed. Honest continuation needs a
generic queued CdlReadS producer, sector/DMA delivery into the stream ring,
and the registered callback lifecycle. Returning success after registration
would falsely wake movie decoding without data.

## Controls

Focused tests prove both mode-bit branches, exact guest callback identities,
channel-3 DICR registration, full-width callback exchange, and zero callback
delivery. Strict Disc 1 reaches the internal boundary naturally after the
already-proven CdlSetloc.

```text
B54K-AM independent oracle: PASS
B54K-AM focused test:       1/1 PASS
native suite:               994/994
normal CTest:               2/2 PASS
fresh ASan/UBSan CTest:     2/2 PASS
strict real-disc exit:      1
strict frontier:            func_80081314_func_8007F0C8_cut
```

No sector, DMA completion, callback invocation, stream record, decoded
frame, scene, story, persistence, or destination state is fabricated.

```text
FUNC_80081314=AUTHENTICATED_PREFIX_30_OF_53_WORDS
CALLBACK_REGISTRATION=COMPLETE_NOT_DELIVERY
CdlReadS_QUEUE_PATH=PROVEN_NOT_IMPLEMENTED
PRODUCTION_REACHABILITY=blocked_at_func_80081314_func_8007F0C8_cut
NEXT_RUNG=generic_CdlReadS_queue_and_sector_delivery_contract
```
