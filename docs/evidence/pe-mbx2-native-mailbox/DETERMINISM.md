# Determinism

```text
oracle_trace_sha256=9cefa0bc9ad95afcf47a7bd0b790e7f741b3bfce42427e4bdaed94b7fcb87b27
trace_rebaselined=no
```

m0002i→m0003i has no `0x1C`. Drain on that path is
`count==0; return`. The accepted fixture is unchanged.

Command:

```text
PE_DISC1_BIN=<registered Disc 1 BIN> ./native/build/pe-ue0-tests
```

Three independent suite runs:

```text
run1  passed=1647 failed=0
run2  passed=1647 failed=0
run3  passed=1647 failed=0
```

`test_disc_route` already hashes three live traces against the
fixture and against each other, then compares 30 Hz and 60 Hz
`runtime_present` schedules to the same CSV.

`test_mailbox_determinism` repeats append → drain → `0x1F` three
times and requires identical `(count, local[4], task+0x14, flags)`.

```text
determinism=3/3 + 30Hz/60Hz present
row_for_row_parity=yes
```
