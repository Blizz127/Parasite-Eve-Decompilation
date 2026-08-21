# Gates

Matching EXE SHA-1 is unchanged: this rung edits `pc_port/` and
docs only.

```text
native=582/582 (580 baseline + 2 focused B54K-A)
asan_ubsan=582/582 in toolbox jk2026-dev, zero sanitizer diagnostics
focused_B54KA=2/2
retained_B54G=2/2
retained_B54I=1/1
retained_PEGPU1=1/1
b54ka_oracle=8/8
b54j_oracle=19 groups PASS (retained)
b54i_oracle=30 checks PASS (retained)
pe_gpu1_oracle=18 checks PASS (retained)
exe_oracles=56/56 (every exe-argument oracle in pc_port/tools;
            b21_bzero_oracle.py uses DST LEN and is not in that set)
matching_exe=unchanged by construction (no src/, configs/, asm/ edits;
             SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b)
strict_real_disc=exit 1 at func_80030894_L2L3_cut from func_80030894
bootstrap_disc=exit 1 at func_8007F72C from func_800698D4 (unchanged)
```

## Reproduce

```sh
cmake --build pc_port/build -j8 --target pe-native-tests &&
  ./pc_port/build/pe-native-tests            # 582/582
toolbox run -c jk2026-dev bash -lc '
  cmake --build pc_port/build-san -j8 --target pe-native-tests &&
  ./pc_port/build-san/pe-native-tests'       # 582/582 ASan+UBSan
python3 pc_port/tools/b54ka_30894_l2l3_oracle.py   # 8/8
python3 pc_port/tools/b54j_30894_audit_oracle.py   # 19 groups
python3 pc_port/tools/b54i_gpu_leaves_oracle.py    # 30 checks
python3 pc_port/tools/pe_gpu1_header_leaves_oracle.py  # 18 checks

DISC=$(cat local/pe_disc1.path)
./pc_port/build/parasite-eve-port --headless \
  --disc-image "$DISC" --strict-stubs
# FATAL: func_80030894_L2L3_cut from func_80030894; exit 1
```
