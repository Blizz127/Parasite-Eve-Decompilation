# PE-B54K-AG — generic MDEC reset and table submission

Status: **VERIFIED AND INTEGRATED ON THE NATIVE GRIND LANE**.

This rung translates the complete internal reset reached by retail
`DecDCTReset(0)` and its table-submit helper. It provides only the generic
hardware-init state proven by retail; no frame decoding or movie completion is
claimed.

## Retail identity

```text
internal reset        [0x8010C0FC,0x8010C1EC)
reset size            0xF0 / 60 words
reset SHA-256         53670ac94ba335b065bac7d63504e80a75a3bffe01cc3dff07c1582da260808b
table-submit helper   [0x8010C1EC,0x8010C27C)
submit size           0x90 / 36 words
submit SHA-256        31f4ab22c790632c4409a877ecd6f1d93bdaf391fc5069fabe543fb6554e7f0e
caller return point   0x80192730
strict frontier       func_801924F8_80192730_cut
```

The module's literal pointer table proves the hardware classes:

```text
0x8010DB1C..0x8010DB30  DMA0/DMA1 MADR, BCR, CHCR
0x8010DB4C               0x1F801820 MDEC command/data
0x8010DB50               0x1F801824 MDEC control/status
0x8010DB54               0x1F8010F0 DPCR
```

## Reset contract

Both supported modes write the MDEC reset/control sequence, clear DMA0 and
DMA1 channel state, and leave the last control write as `0x60000000`.

Mode zero additionally calls the 36-word helper twice:

| Command block | Command | Payload | Payload SHA-256 |
| --- | --- | --- | --- |
| `0x8010DA0C` | `0x40000001` | 32 words / 128 bytes | `09cd1578bb59e1ed3968b2043278174f0eb74ecaab3b400ea048cbdab9a80328` |
| `0x8010DA90` | `0x60000000` | 32 words / 128 bytes | `b128878a4faf48a10681b904435430150bc932b420b87a5b4cf42ae2da5a0e63` |

Each submission performs the retail DPCR `| 0x88`, writes DMA0 MADR to
`command+4`, BCR to `0x00010020`, the MDEC command word, and CHCR to
`0x01000201`. The native model fingerprints the exact payload bytes with
standard FNV-1a64; this telemetry never becomes guest authority.

The first submission is marked completed only when the second helper call
performs its input-ready wait. The second remains active at reset return. This
models the observable issue order without inventing asynchronous completion.

Mode one performs no table submissions and does not modify shared DPCR. Modes
other than zero and one reach `func_8010C0FC_bad_mode` before any MDEC or DPCR
mutation, corresponding to retail's diagnostic arm.

## Controls

- Real Disc 1 loads the authenticated 38-sector module and proves both retail
  payload fingerprints, final DMA0 registers, one completed plus one pending
  submission, and shared DPCR `0x333333BB`.
- A synthetic test uses two distinct deterministic 128-byte payloads and
  compares the model fingerprints to an independent byte walk.
- Mode one proves reset-without-upload and preserved DPCR.
- Invalid mode 2 proves a named, mutation-free boundary.

## Verification

```text
B54K-AG independent oracle: PASS
B54K-AF regression oracle:  PASS
B54K-AE regression oracle:  PASS
B54K-AD regression oracle:  PASS
B54K-AC regression oracle:  PASS
normal CTest:                2/2 PASS
native suite:                989/989
fresh ASan/UBSan CTest:      2/2 PASS
strict real-disc exit:       1
strict frontier:             func_801924F8_80192730_cut
retail EXE SHA-1:            452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

No compressed frame is consumed, no MDEC output is produced, no XA audio is
played, and no scheduler, story, persistence, or destination state is planted.

```text
FUNC_8010C0FC=COMPLETE_MODE0_MODE1_RESET
FUNC_8010C1EC=COMPLETE_TABLE_SUBMIT_HELPER
MDEC_TABLE_DMA=SECOND_SUBMISSION_PENDING
PRODUCTION_REACHABILITY=blocked_at_func_801924F8_80192730_cut
NEXT_ARTIFACT_FREE_RUNG=DecDCToutCallback_and_following_movie_setup
```
