# timer/latch family — eight matching C leaves (leaves 562–569)

- **Template**: `void f(void *unused, signed char *state, unsigned short *values)`
  with `values[2] -= D; values[3] += A; if ((short)values[2] < 20) { values[2] = 0; state[1] = 2; }`
- **Profile**: default `era_o2_g0` (`-O2 -G0`); no pins, no maspsx opt-in.
- **Members** (file offset / VRAM / D / A):

| leaf | file | VRAM | `-=` | `+=` |
|---|---|---|---|---|
| func_800C8C80 | 0xB9480 | 0x800C8C80 | 8 | 0x3C |
| func_800C8CBC | 0xB94BC | 0x800C8CBC | 8 | 0x78 |
| func_800C8CF8 | 0xB94F8 | 0x800C8CF8 | 8 | 0xA |
| func_800C9A34 | 0xBA234 | 0x800C9A34 | 8 | 0x28 |
| func_800CBBF0 | 0xBC3F0 | 0x800CBBF0 | 8 | 0x3C |
| func_800CBC2C | 0xBC42C | 0x800CBC2C | 8 | 0x78 |
| func_800CBC68 | 0xBC468 | 0x800CBC68 | 8 | 0xA |
| func_800CD5B0 | 0xBDDB0 | 0x800CD5B0 | 0xA | 0x3C |

Each member is 0x3C bytes (15 words).

## How it was found

Instruction-shape clustering over `asm/disc1/*.s` (`nonmatching func_*, size`
+ `glabel`/`endlabel`) grouped these eight under one relaxed shape. The
adjacent already-matched sibling `func_800CA540` showed the same idiom but with
the store-to-compare value forwarded (`sll`/`sra`); the family instead reloads
it as a **signed** halfword (`lh $v1,4($a2)`) because the intervening
`values[3] += A` update breaks the forward. The natural C phrasing above
reproduces retail exactly under `-O2 -G0`.

## Triage loop

`tools/analysis/try_leaf.py SRC OFFSET SIZE` compiles one leaf the way
`disc1_build.py` does and diffs `.text` against retail. All eight reported
`WORDS MATCH (+4 pad bytes, trimmed by the build)`.

## Proven by the harness

```sh
bash scripts/split_us.sh
podman run --rm --userns=keep-id -v /tmp/pe-agent-decomp:/workspace:Z \
  -w /workspace localhost/pe-mipsel-img:latest bash scripts/build_us.sh
```

```
Plan:      OK (857 YAML spans; no manual span lists)
Compile:   OK (569 generated C entries)
Compare:   EXACT SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
Matching claim: YES (569 registered C leaves)
BUILD EXIT: 0
```

Carves all split the head of an `asm` span into 0x3C `c` entries and resume the
span after the last family member.
