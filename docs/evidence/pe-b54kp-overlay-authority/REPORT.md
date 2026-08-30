# PE-B54K-P — adopt retail overlay-load authority

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

B54K-O proved that native 6E834 was using fixture values where a real-disc
run needs executable-backed rodata. This rung adds a bounded authority
handoff immediately after the already-authenticated PS-X EXE is copied into
guest RAM.

## Contract

`PE_Globals_AdoptRetailImage()` reads, validates, then publishes:

```text
D_80011614                       0x8018EFF0
D_80093164[0..3]                 03D2 0457 04FC 0516
first PE.IMG range               [03D2,0457)
first range bytes                0x42800
destination range                [0x8018EFF0,0x801D17F0)
```

It rejects descending sector ranges and any destination/size outside the
2 MiB guest-RAM authority before changing host state. `port_main` calls it
only after `PE_GuestImage_LoadExe` has validated and loaded the retail boot
executable. A malformed authority aborts startup loudly.

Bootstrap fixtures deliberately retain their prior host defaults
(`D_80011614=0x8010BD00`, zero range table), so isolated tests do not pretend
to contain PE.IMG. This is an explicit real-image adoption boundary rather
than a global constant change or planted overlay state.

## Verification

The independent oracle authenticates the exact executable, all four table
values, destination, range arithmetic, and 6E834's loads of both authorities.
Focused tests prove valid adoption and mutation-free rejection. Full normal
and fresh sanitizer suites pass:

```text
B54K-P overlay authority oracle: PASS.
Results: 956 run, 956 passed, 0 failed, 0 skipped
fresh ASan/UBSan: 956 run, 956 passed, 0 failed, 0 skipped
SANITIZER_DIAGNOSTICS=0
```

A real Disc 1 strict smoke authenticated the image, adopted the values, and
still stopped at the unchanged current frontier:

```text
[DISC] boot executable loaded into guest RAM
FATAL: strict-stubs — first unresolved BOOTSTRAP_RET provider:
       func_8006AD40_D_80093126_archive_cut
       called from: func_8006AD40
exit=1
```

The retail executable remains SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`. This rung does not execute the
overlay or bypass B54K-M; it makes the future load source and destination
honest.

```text
PRODUCTION_REACHABILITY=blocked_at_func_8006AD40_D_80093126_archive_cut
OVERLAY_LOAD_AUTHORITY=RETAIL_EXE_ADOPTED_AFTER_AUTHENTICATED_LOAD
FUNC_801909B4_BYTES=STATICALLY_RECOVERED_NOT_IMPLEMENTED
SCHEDULER_PROVENANCE=NEEDS_ARTIFACT
```
