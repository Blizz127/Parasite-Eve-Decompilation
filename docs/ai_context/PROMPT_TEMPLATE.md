# PROMPT_TEMPLATE — handing leaf work to an agent

Every agent failure this project has logged came from a missing line in a
prompt: an unverified baseline, an open-ended scope, a stale artifact
hashed as evidence, a `git add -A` that swept in junk, or an agent
grinding one stubborn leaf all night. This template encodes the shape
that worked (the 2026-08-21 four-leaf run: 270 → 275, zero false claims).
Copy it, fill the brackets, delete nothing.

```
PARASITE EVE — <one-line task name>

Worktree: <absolute path>   Branch: <branch>
Baseline: <N> leaves, SHA-1 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
```

## 1. Baseline gate

State the starting leaf count and the retail SHA-1 in the prompt. The
agent's first build must reproduce the baseline EXACT MATCH before it
changes anything. If the baseline does not build, the task is "fix the
environment and report", not the original task. (This is what caught the
stale docker image: the build failed loudly instead of a leaf silently
inheriting a broken environment.)

## 2. Bounded scope

Name every function to be touched, in order, with file offset and size,
and say what one commit contains. End with an explicit stop:

```
Port exactly these, in order, one commit each:
  func_XXXXXXXX  (0xOFFSET, N words)
  ...
Stop after all <k>. This is not an open-ended grind.
```

"Keep going until morning" is how drift and false-claim commits happen.
An agent that finishes early and stops is a success.

## 3. Fresh-build-only SHA

The only acceptable match evidence is, in the same session, in order:

1. `scripts/split_us.sh` prints the **incremented** `c: N split`.
2. The docker build runs to completion, **exits 0**, and itself prints
   `Compare: EXACT SHA-1 MATCH`:
   `docker run --rm -v "$PWD:/workspace" -w /workspace
    --user "$(id -u):$(id -g)" pe-mipsel-img:latest bash scripts/build_us.sh`
3. `sha1sum build/disc1.candidate.exe` equals
   `452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with a fresh timestamp.

Never hash a standing artifact. `build_us.sh` deletes the candidate at
start precisely so a failed run leaves nothing to hash — do not weaken
that guard. A SHA printed by anything other than this run's completed
build is not evidence.

## 4. Named-paths-only git

`git add <each file by name>`. Never `git add -A`, `git add .`, or
`git commit -a`. The commit must contain every file the match depends on
(.c, yaml carve, build_us.sh wiring, verify_us.sh markers, evidence
REPORT.md) and nothing else. Push after every leaf commit — unbacked
local-only progress is one `mv` away from gone.

## 5. Two-attempt park

If a leaf does not match after two serious attempts: `git checkout` the
touched files back, write what was tried and where the bytes diverged
into `docs/ai_context/parked_blockers.json` (or the evidence dir), and
move to the next item. Do not force it, do not invent variant C, do not
loosen the gate. A parked leaf with a good divergence note is progress;
a third attempt at 4am is where hard-rule violations come from.

## Standing hard rules (restate in every prompt)

- Do not touch yaml entries outside the named functions.
- Do not modify any existing matched leaf.
- Read other worktrees only as source; never commit or build there
  unless the task says so (see MACHINE TOPOLOGY in ACTIVE_HANDOFF.md).
- Update ACTIVE_HANDOFF.md and write the evidence REPORT.md per leaf.
