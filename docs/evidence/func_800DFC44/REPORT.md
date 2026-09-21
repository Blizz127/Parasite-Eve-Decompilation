# func_800DFC44 — exact byte-twin of func_8005186C

- **Span**: file `0xD0444`–`0xD0480` (0x3C bytes / 15 words), VRAM
  `0x800DFC44`, inside the previously monolithic `0xC5060` asm span.
- **Claim**: matching C leaf **561**.

## Evidence

The function bytes in the retail image are byte-for-byte equal to the
already-matched loop-as-volume leaf `func_8005186C`
(`src/func_8005186C.c`, file `0x4206C`, size `0x3C`). A byte-identical body
therefore matches under the same default era profile (`era_o2_g0`, `-O2 -G0`).

Twin search: read `build/extracted/disc1/SLUS_006.62` at every matched leaf's
manifest span and every `asm/disc1/*.s` `nonmatching func_*` span; exactly one
remaining asm function has a byte sequence equal to a matched leaf, this one.

## Carve

`configs/USA/disc1.yaml` tail (was a single `[0xC5060, asm]`):

```yaml
      - [0xC5060, asm]
      - [0xD0444, c, func_800DFC44]
      - [0xD0480, asm]
```

## Commands and result

```sh
bash scripts/split_us.sh          # 849 spans (561 c, 286 asm, 2 rodata)
podman run --rm --userns=keep-id -v /tmp/pe-agent-decomp:/workspace:Z \
  -w /workspace localhost/pe-mipsel-img:latest bash scripts/build_us.sh
```

```
Plan:      OK (849 YAML spans; no manual span lists)
Compile:   OK (561 generated C entries)
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (561 registered C leaves)
BUILD EXIT: 0
```

Verifier (`distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp && bash scripts/verify_us.sh'`):
`VERIFY_US=PASS`, 561 packed C spans equal retail.
