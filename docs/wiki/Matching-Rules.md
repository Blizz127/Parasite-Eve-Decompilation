# Matching Rules

Hard rules for this project (see also `CLAUDE.md`):

1. **Never invent decompiled C.** Only add C verified by the rebuild oracle.
2. **Never claim matching** without `scripts/build_us.sh` exit 0 and recorded SHA-1.
3. **Never commit game data** (ISO/BIN/CUE, extracted files, proprietary SDK).
4. **Every claim needs evidence** (command, checksum, disassembly excerpt, doc note).
5. **Phases must be reproducible** via `scripts/` and `configs/`.

## Workflow discipline

- Small PRs: **one C function** at a time for leaf conversions.
- No stacking Phase N+1 on an unmerged Phase N branch.
- Baseline exact match on `main` before the next edit.
- Temporary types only — no speculative struct/field names.
- No PC-port work mixed into matching PRs.
- No hand-written replacement asm labeled as C.

## Oracle

```bash
scripts/build_us.sh   # exit 0 ⇔ exact SHA-1
```
