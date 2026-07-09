# PR and Branch Workflow

## Linear main discipline

```
merge PR N to main
→ git switch main && git pull
→ split / verify / build baseline exact
→ git switch -c phase…-next-c-leaf
→ one function only
→ push + PR
→ do not start N+1 until merge
```

## Recent C leaf PRs

| PR | Function | Status |
| --- | --- | --- |
| #9 | `func_80090C38` | Merged |
| #10 | `func_80090C4C` | Merged |
| #12 | `func_80090F54` | Merged |
| #13 | `func_80090C60` | Open |

## Why no stacking

- Keeps `main` always buildable and exact-match.
- Avoids rebase/cleanup noise between leaf PRs.
- Matches “one tiny leaf, merge, repeat.”

## Docs / wiki PRs

- Prefer **separate** docs-only PRs (do not mix with C leaf PRs).
- Update wiki only after durable milestones (merged PR, phase complete, process change).
