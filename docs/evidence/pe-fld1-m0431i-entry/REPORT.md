# PE-FLD1 — M0431I field entry: three-chunk PE.IMG load

The New-Game map enters through the already-translated
`func_8006B4F8_dest_load_cut`: token `0xA80830C8` → `M0431I` → index
431 → slot 430 (`rel 0x182CF`, `packed 0x01200911`) → `B0DD8 + rel`
base LBA → 17 + 9 + 18 sectors into overlay `+0x194` / `+0x168` /
`+0x18C`, then the `6E1C0` loops, `72714/726C4/72724`, and the 12574
publish. No new port code: this rung proves the retail-data path with
a disc-gated test mirroring BTL63 (`TEST_RETAIL_DISC1`, skips cleanly
without `PE_DISC1_BIN` or `local/pe_disc1.path`).

Expected chunk heads were derived from the user-supplied Disc 1 image
(`rom/image/`, read-only input) at 2352-byte sectors + 24 data offset —
base `1013 + 0x182CF`: chunk0 `0x8064`, chunk1 `0x4050`, chunk2
`0x8A1C`/`0x89B4`. The extraction method was validated first by
reproducing all three BTL63 `m0367i` heads exactly (`0x1008C`,
`0x54908`, `0x26BD8`) before reading `m0431i`.

## Verify

```text
PE_DISC1_BIN="rom/image/Parasite Eve (USA) (Disc 1)/Parasite Eve (USA) (Disc 1).bin" \
  PE_TEST_FILTER=FLD1 ./pc_port/build/pe-native-tests
```

With the image present the FULL suite is green: 1022 run / 1022
passed / 0 failed / 0 skipped (env var only — no repo or `local/`
changes). Without it the standard gate holds: 1022 run / 1004 passed /
1 pre-existing environmental failure (`B54KY` missing disc path) /
17 skipped.
