# PE-BTL90 — M0367I dest-ready runtime

Source of resource-lifecycle facts:
`docs/evidence/pe-battle-data-precovery/`.

Canonical dest-ready (no mode 7/9/10, no manufactured type-1 clip):

```
6B35C
AND 6B4F8 completed (CE2=hdr+1=10, hdr+0x0C, Writer A)
AND 6BECC == 0 (Writer B bank CE2=10)
AND 6C5BC == 0
AND 125E0 type-1 spawned
```

Type-1 first visit: `+0x1AC=0`, `+0x1B0=0`, first VM `0x40`,
first yield `+0x20 / 0x02`.

CE2=10 cmd `0x15` SHA-256
`e1cb9dfd14eafe873e4768722b39f7fd51e763ea5147279d6a09ad401c1ba40e`.
CE2=14 `+0x8` / 3D050 tail stay fail-closed.

## Commands

```
python3 pc_port/tools/pe_btl90_m0367i_dest_ready_oracle.py
# PASS: M0367I dest-ready is 6B35C/6B4F8/6BECC/6C5BC/125E0; not mode 7

python3 pc_port/tools/pe_battle_data_precovery_oracle.py
# PASS: battle-data precovery; … M0367I WA 36

PE_TEST_FILTER=BTL90_m0367i ./pc_port/build/pe-native-tests
# Results: 816 run, 1 passed, 0 failed, 815 skipped

./pc_port/build/pe-native-tests
# Results: 816 run, 816 passed, 0 failed, 0 skipped
```
