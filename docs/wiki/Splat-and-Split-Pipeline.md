# Splat and Split Pipeline

## Config

- Disc 1: `configs/USA/disc1.yaml`
- Disc 2: pointer/alias (EXE is byte-identical)
- Pinned splat: `splat64[mips]==0.41.0`

## Commands

```bash
scripts/setup_env.sh          # once: .venv + splat
scripts/extract_us.sh 1       # local disc image → build/extracted/
scripts/split_us.sh --check   # dry-run gates
scripts/split_us.sh           # real split (git-ignored outputs)
```

## Outputs (local only)

| Path | Role |
| --- | --- |
| `asm/disc1/` | Generated assembly units |
| `linkers/disc1.ld` | Splat linker script (C layout; production uses custom ROM-order script) |
| `src/*.c` | **Tracked** production C (hand-authored, matched) |
| `undefined_*_auto.txt` | Auto symbols |

## Policy

- Split outputs are **study artifacts**, never committed.
- Subsegment cuts need evidence (prologue, epilogue, pointer tables, strings).
- Nested splat jtbl suggestions are deferred unless they fix a misclassification.
