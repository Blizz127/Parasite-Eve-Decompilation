#!/usr/bin/env python3
"""merge_agent_branch.py — merge a decompilation agent branch safely.

Every agent branch in this project touches the same handful of files, so the
same conflicts recur on every merge. Resolving them by hand is where mistakes
creep in (this session left `parked_blockers.json` structurally invalid twice
and committed leftover conflict markers into a generated doc). This script
encodes the resolution for each file, then *validates* the result before it
commits anything.

What each conflict actually means:

* `configs/USA/disc1.yaml` — both sides carved `c` leaves out of overlapping
  `asm` runs. Resolution: merge the two sides' entry groups in ascending file
  offset. When both sides have an entry at the same offset and one is a `c`
  leaf while the other is the `asm` run it was carved from, the `c` entry wins.
* `configs/USA/disc1_build_profiles.json` — both sides added profile
  assignments and/or new profiles. Resolution: union the `profiles` map and the
  `assignments` lists, then enforce a single owner per symbol (HEAD wins).
* `docs/ai_context/parked_blockers.json` — both sides appended blocker objects.
  Resolution: union by `id`.
* `docs/ai_context/ACTIVE_HANDOFF.md` — both sides prepended a section.
  Resolution: concatenate.
* `docs/generated/DISC1_MATCHING_STATUS.md` — generated. Regenerated from the
  plan afterwards, never resolved by hand.
* `src/<name>.c` added by both sides (two agents matched the same function) —
  keep HEAD's, since HEAD's version is already merged and verified.
* `tools/**` modified by both sides — keep HEAD's (parent tooling is newer).

Usage:
    python3 tools/analysis/merge_agent_branch.py BRANCH [-m MESSAGE] [--dry-run]

Exit status is non-zero if validation fails, in which case the merge is left
staged but uncommitted for inspection.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
YAML = "configs/USA/disc1.yaml"
PROFILES = "configs/USA/disc1_build_profiles.json"
BLOCKERS = "docs/ai_context/parked_blockers.json"
HANDOFF = "docs/ai_context/ACTIVE_HANDOFF.md"
STATUS = "docs/generated/DISC1_MATCHING_STATUS.md"

MARKERS = re.compile(r"^(<{7}|={7}|>{7})", re.M)
ENTRY_RE = re.compile(r"^(\s*)- \[(0x[0-9A-Fa-f]+), (\w+)")
CONFLICT_RE = re.compile(
    r"<<<<<<< [^\n]*\n(.*?)=======\n(.*?)>>>>>>> [^\n]*\n", re.S
)


def git(*args: str, check: bool = True) -> subprocess.CompletedProcess:
    return subprocess.run(
        ["git", *args], cwd=ROOT, capture_output=True, text=True, check=check
    )


def unmerged_paths() -> list[str]:
    out = git("diff", "--name-only", "--diff-filter=U").stdout
    return [line for line in out.splitlines() if line.strip()]


def read_blob(rev: str, path: str) -> str:
    return git("show", f"{rev}:{path}").stdout


def has_markers(text: str) -> bool:
    return bool(MARKERS.search(text))


# ── disc1.yaml ────────────────────────────────────────────────────────────────

def entry_groups(text: str) -> list[list[str]]:
    """Split a yaml fragment into comment+entry groups keyed by their offset."""
    groups: list[list[str]] = []
    pending: list[str] = []
    for line in text.splitlines(keepends=True):
        if ENTRY_RE.match(line):
            groups.append(pending + [line])
            pending = []
        elif line.strip():
            pending.append(line)
    if pending:
        groups.append(pending)
    return groups


def group_offset(group: list[str]) -> int | None:
    for line in group:
        m = ENTRY_RE.match(line)
        if m:
            return int(m.group(2), 16)
    return None


def group_kind(group: list[str]) -> str | None:
    for line in group:
        m = ENTRY_RE.match(line)
        if m:
            return m.group(3)
    return None


def resolve_yaml_side(a: str, b: str) -> str:
    ga, gb = entry_groups(a), entry_groups(b)
    has_a = any(group_offset(g) is not None for g in ga)
    has_b = any(group_offset(g) is not None for g in gb)
    if not has_a and not has_b:
        # Comment-only disagreement over the same carve; concatenating would
        # emit two contradictory descriptions of one C span. Keep ours.
        return a
    if not has_a:
        return b
    if not has_b:
        return a

    keyed = [g for g in ga + gb if group_offset(g) is not None]
    loose = [g for g in ga + gb if group_offset(g) is None]
    keyed.sort(key=lambda g: group_offset(g))

    merged: list[list[str]] = []
    for group in keyed:
        off = group_offset(group)
        if merged and group_offset(merged[-1]) == off:
            # Same offset from both sides. A `c` leaf supersedes the `asm`
            # run it was carved from; identical kinds keep ours.
            if group_kind(merged[-1]) == "c" or group_kind(group) != "c":
                continue
            merged.pop()
        merged.append(group)

    # comment-only fragments (no entry) attach at the front
    return "".join("".join(g) for g in loose + merged)


def resolve_yaml(text: str) -> str:
    seen_offsets: dict[int, str] = {}
    out = []
    pos = 0
    for m in CONFLICT_RE.finditer(text):
        out.append(text[pos : m.start()])
        out.append(resolve_yaml_side(m.group(1), m.group(2)))
        pos = m.end()
    out.append(text[pos:])
    resolved = "".join(out)

    # reject a genuinely contradictory duplicate (same offset, two c leaves)
    for m in re.finditer(r"^\s*- \[(0x[0-9A-Fa-f]+), c, (\w+)\]", resolved, re.M):
        off, name = int(m.group(1), 16), m.group(2)
        if off in seen_offsets and seen_offsets[off] != name:
            raise SystemExit(
                f"{YAML}: offset 0x{off:X} claimed by both "
                f"{seen_offsets[off]} and {name}; resolve by hand"
            )
        seen_offsets[off] = name
    return resolved


# ── JSON files ────────────────────────────────────────────────────────────────

def resolve_profiles() -> str:
    head = json.loads(read_blob("HEAD", PROFILES))
    theirs = json.loads(read_blob("MERGE_HEAD", PROFILES))

    profiles = dict(theirs.get("profiles", {}))
    profiles.update(head.get("profiles", {}))  # HEAD wins on a name clash

    assign = dict(theirs.get("assignments", {}))
    for prof, syms in head.get("assignments", {}).items():
        assign[prof] = list(dict.fromkeys(assign.get(prof, []) + syms))

    owner: dict[str, str] = {}
    for prof in list(assign):
        for sym in assign[prof]:
            owner.setdefault(sym, prof)
    clean = {
        prof: [s for s in syms if owner[s] == prof] for prof, syms in assign.items()
    }
    clean = {prof: syms for prof, syms in clean.items() if syms}

    result = dict(head)
    result["profiles"] = profiles
    result["assignments"] = clean
    return json.dumps(result, indent=2) + "\n"


def resolve_blockers() -> str:
    head = json.loads(read_blob("HEAD", BLOCKERS))
    theirs = json.loads(read_blob("MERGE_HEAD", BLOCKERS))
    key = "blockers" if "blockers" in head else next(iter(head))
    known = {b.get("id") for b in head[key]}
    head[key].extend(b for b in theirs[key] if b.get("id") not in known)
    return json.dumps(head, indent=1, ensure_ascii=False)


def resolve_additive(text: str) -> str:
    return CONFLICT_RE.sub(lambda m: m.group(1) + m.group(2), text)


# ── driver ────────────────────────────────────────────────────────────────────

def resolve_file(path: str) -> str:
    if path == YAML:
        return resolve_yaml(Path(ROOT / path).read_text())
    if path == PROFILES:
        return resolve_profiles()
    if path == BLOCKERS:
        return resolve_blockers()
    if path == HANDOFF:
        return resolve_additive(Path(ROOT / path).read_text())
    if path == STATUS:
        return Path(ROOT / path).read_text()  # regenerated below
    # src/*.c added by both sides: HEAD's version is already merged+verified.
    # tools/**: parent tooling is newer. Both fall back to ours.
    return read_blob("HEAD", path)


def validate() -> list[str]:
    problems: list[str] = []

    yaml_text = (ROOT / YAML).read_text()
    if has_markers(yaml_text):
        problems.append(f"{YAML}: conflict markers remain")
    offs = [
        int(m.group(1), 16)
        for m in re.finditer(r"^\s*- \[(0x[0-9A-Fa-f]+), \w+", yaml_text, re.M)
    ]
    bad = [
        (hex(a), hex(b))
        for a, b in zip(offs, offs[1:])
        if b <= a
    ]
    if bad:
        problems.append(f"{YAML}: offsets not increasing: {bad[:5]}")

    for path in (PROFILES, BLOCKERS):
        try:
            json.loads((ROOT / path).read_text())
        except json.JSONDecodeError as exc:
            problems.append(f"{path}: invalid JSON: {exc}")

    for path in (HANDOFF,):
        if has_markers((ROOT / path).read_text()):
            problems.append(f"{path}: conflict markers remain")

    plan = subprocess.run(
        [sys.executable, "tools/build/disc1_plan.py", "--check"],
        cwd=ROOT,
        capture_output=True,
        text=True,
    )
    if plan.returncode != 0:
        problems.append("disc1_plan --check failed: " + plan.stdout.strip()[-300:])
    return problems


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("branch")
    ap.add_argument("-m", "--message", default=None)
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    merge = git(
        "merge", "--no-ff", "-m", args.message or f"Merge {args.branch}",
        args.branch, check=False,
    )
    if merge.returncode == 0:
        print(merge.stdout.strip() or "already up to date")
        return 0

    paths = unmerged_paths()
    if not paths:
        print(merge.stdout, merge.stderr, file=sys.stderr)
        print("merge failed without conflicts; stopping", file=sys.stderr)
        return 1

    print(f"resolving {len(paths)} conflicted path(s):")
    for path in paths:
        text = resolve_file(path)
        (ROOT / path).write_text(text)
        git("add", path)
        print(f"  {path}")

    subprocess.run(
        [sys.executable, "tools/build/disc1_plan.py", "--write-status"],
        cwd=ROOT,
        check=False,
    )
    git("add", STATUS)

    problems = validate()
    if problems:
        print("\nVALIDATION FAILED (merge left staged, not committed):")
        for p in problems:
            print(f"  - {p}")
        return 1

    counts = subprocess.run(
        [sys.executable, "tools/build/disc1_plan.py", "--check"],
        cwd=ROOT, capture_output=True, text=True,
    ).stdout.strip()
    print(f"\nvalidated: {counts}")

    if args.dry_run:
        print("dry run: not committing")
        return 0

    msg = args.message or f"Merge {args.branch}"
    commit = git("commit", "-m", msg, check=False)
    if commit.returncode != 0:
        print(commit.stdout, commit.stderr, file=sys.stderr)
        return 1
    print(git("log", "--oneline", "-1").stdout.strip())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
