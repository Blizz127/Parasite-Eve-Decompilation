# Selector pair, table-lookup quartet, flag trio — 9 matching C leaves (611–619)

Continuation of the CD-wrapper vein and adjacent spans on `agent/decompile-100`.

## 611–612 — `func_80085F14`, `func_80085F44` (asm 765E8.s)

- `func_80085F44` file `0x76744` (0x24): change-guarded setter,
  `v0 = D_8009B434; if (a0 != v0) D_8009B434 = a0; return v0;` — default
  `era_o2_g0`.
- `func_80085F14` file `0x76714` (0x30): a `switch (a0)` selecting 0/1/0
  with `D_8009B38C = a0; D_8009B418 = v0; return v0;`. The **store must land
  in the `jr` delay slot**, so the leaf uses the existing
  `era_o2_g0_fill_store_delay_slot` profile (`MASPSX_FILL_STORE_DELAY_SLOT=1`).
  Added `func_80085F14` to that profile's assignment list.

## 613–616 — indexed table lookup quartet (asm 4E44C.s)

`func_8005DC4C` / `func_8005DC9C` / `func_8005DCEC` / `func_8005DD3C`, file
`0x4E44C`/`0x4E49C`/`0x4E4EC`/`0x4E53C`, 0x50 each. Same body; the record
offset field varies +4/+8/+0xC/+0x10:

```c
unsigned char *f(unsigned int a0) {
    unsigned char *v0 = (unsigned char *)(D_800A802C + (int)D_800A8028);
    unsigned char *v1 = v0 + *(int *)(v0 + FIELD);
    if (!(a0 < *(unsigned short *)v1)) return 0;
    return v1 + *(short *)(v1 + a0 * 2 + 2);
}
```

Two phrasing levers: invert the guard (`if (!(...)) return 0;`) to get
retail's `beqz`, and keep `v0`/`v1` as **pointers** so the base+offset add
emits `addu v1,v0,v1` instead of `addu v1,v1,v0`.

## 617–619 — flag-clear/status-set trio (asm 804BC.s)

`func_8008FED8` / `func_80090054` / `func_80090178`, file
`0x806D8`/`0x80854`/`0x80978`, 0x24 each. `v0 &= -N; v1 |= M;` plus a
halfword zero at +0xE8/+0xEA/+0xEC. Register pins
`register int v0 asm("$2"); register int v1 asm("$3");` are required — without
them GCC put the mask constant in `$2`/`$5` instead of `$3`.

## Proven by the harness

```sh
bash scripts/split_us.sh
podman run --rm --userns=keep-id -v /tmp/pe-agent-decomp:/workspace:Z \
  -w /workspace localhost/pe-mipsel-img:latest bash scripts/build_us.sh
```

```
Plan:      OK (914 YAML spans; no manual span lists)
Compile:   OK (619 generated C entries)
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (619 registered C leaves)
BUILD EXIT: 0
```
