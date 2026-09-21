# Deref-forward-store family — 3 matching C leaves (626–628)

- `func_800194B0` file `0x9CB0` and `func_800194F8` file `0x9CF8` (asm 9CB0.s),
  plus `func_80018C10` file `0x9410` (asm 9410.s); 0x48 bytes each.
- Body (default `era_o2_g0`):

```c
int f(int *a0) {
    int v0 = a0[0];
    int r = CALLEE(*(int *)v0);
    *(int *)(a0[1]) = r;
    return 1;
}
```

Callees: `func_80053D2C`, `func_80053E6C`, `func_8006599C` respectively.

## Proven by the harness

```sh
bash scripts/split_us.sh
podman run --rm --userns=keep-id -v /tmp/pe-agent-decomp:/workspace:Z \
  -w /workspace localhost/pe-mipsel-img:latest bash scripts/build_us.sh
```

```
Plan:      OK (923 YAML spans; no manual span lists)
Compile:   OK (628 generated C entries)
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (628 registered C leaves)
BUILD EXIT: 0
```
