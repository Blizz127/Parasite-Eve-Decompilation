#!/usr/bin/env python3
"""Verify Disc 1 authority, artifacts, and exact matching status."""

from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import Any

from disc1_plan import (
    DEFAULT_STATUS,
    PlanError,
    build_plan,
    matching_status,
    verification_manifest,
    write_generated,
)


ROOT = Path(__file__).resolve().parents[2]
EXE = Path("build/extracted/disc1/SLUS_006.62")
CANDIDATE = Path("build/disc1.candidate.exe")


class VerifyError(RuntimeError):
    """A verification gate failed."""


def gate(number: int, total: int, label: str) -> None:
    print(f"[{number}/{total}] {label}")


def pass_line(message: str) -> None:
    print(f"  PASS {message}")


def sha1(path: Path) -> str:
    return hashlib.sha1(path.read_bytes()).hexdigest()


def check_status(plan: dict[str, Any]) -> None:
    path = ROOT / DEFAULT_STATUS
    if not path.is_file():
        raise VerifyError(
            f"missing generated status {path.relative_to(ROOT)}; "
            "run python3 tools/build/disc1_plan.py --write-status"
        )
    expected = matching_status(plan)
    if path.read_text(encoding="utf-8") != expected:
        raise VerifyError(
            f"stale generated status {path.relative_to(ROOT)}; "
            "run python3 tools/build/disc1_plan.py --write-status"
        )


def tracked_sources(plan: dict[str, Any]) -> None:
    result = subprocess.run(
        ["git", "ls-files", "-z"],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        check=True,
    )
    tracked = set(result.stdout.decode().split("\0"))
    missing = [
        unit["source"]
        for unit in plan["units"]
        if unit["kind"] == "c" and unit["source"] not in tracked
    ]
    if missing:
        raise VerifyError("YAML C sources are not tracked: " + ", ".join(missing))

    yaml_sources = {
        unit["source"] for unit in plan["units"] if unit["kind"] == "c"
    }
    tracked_function_sources = {
        path
        for path in tracked
        if re.fullmatch(r"src/func_[0-9A-Fa-f]{8}\.c", path)
    }
    residual_policy_path = ROOT / "docs/acceptance/MATCHING_RESIDUAL_POLICY.md"
    residual_policy = residual_policy_path.read_text(encoding="utf-8")
    residual_names = set(re.findall(r"func_[0-9A-Fa-f]{8}", residual_policy))
    count_match = re.search(
        r"There are also \*\*(\d+) `ACCEPTED-RESIDUAL` leaves\*\*",
        residual_policy,
    )
    if not count_match or int(count_match.group(1)) != len(residual_names):
        raise VerifyError(
            "accepted-residual published count does not equal its function inventory"
        )
    parked = (ROOT / "docs/ai_context/parked_blockers.json").read_text(
        encoding="utf-8"
    )
    missing_park_records = sorted(name for name in residual_names if name not in parked)
    if missing_park_records:
        raise VerifyError(
            "accepted residuals missing parked records: " + ", ".join(missing_park_records)
        )
    disposition_path = ROOT / "configs/USA/disc1_nonmatching_sources.json"
    try:
        dispositions = json.loads(disposition_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise VerifyError(f"cannot load nonmatching source dispositions: {exc}") from exc
    if dispositions.get("schema_version") != 1 or not isinstance(
        dispositions.get("sources"), dict
    ):
        raise VerifyError("nonmatching source dispositions have invalid schema")
    declared = dispositions["sources"]
    extra_sources = tracked_function_sources - yaml_sources
    if set(declared) != extra_sources:
        missing_declarations = sorted(extra_sources - set(declared))
        stale_declarations = sorted(set(declared) - extra_sources)
        raise VerifyError(
            "nonmatching source disposition bijection failed: "
            f"undeclared={missing_declarations}, stale={stale_declarations}"
        )
    allowed_dispositions = {
        "accepted-residual",
        "nonmatching-native-cut",
        "rejected-tail-draft",
        "in-progress-checkpoint",
    }
    for source, record in declared.items():
        if not isinstance(record, dict) or record.get("disposition") not in allowed_dispositions:
            raise VerifyError(f"invalid nonmatching disposition for {source}")
        evidence = record.get("evidence")
        if not isinstance(evidence, str) or not (ROOT / evidence).is_file():
            raise VerifyError(f"missing nonmatching evidence for {source}")
        is_residual = Path(source).stem in residual_names
        if is_residual != (record["disposition"] == "accepted-residual"):
            raise VerifyError(f"residual policy/disposition disagreement for {source}")
    file_scope_assembly = []
    for source in sorted(tracked_function_sources):
        text = (ROOT / source).read_text(encoding="utf-8")
        if re.search(r"(?m)^\s*__asm__\s*\(", text):
            file_scope_assembly.append(source)
    if file_scope_assembly:
        raise VerifyError(
            "file-scope assembly cannot be matching/residual C: "
            + ", ".join(file_scope_assembly)
        )


def public_checks(plan: dict[str, Any]) -> None:
    manifest = verification_manifest(plan)
    spans = manifest["spans"]
    names = [span["name"] for span in spans]
    sources = [span["source"] for span in spans]
    objects = [span["object"] for span in spans]
    if not (len(names) == len(set(names)) == plan["counts"]["c"]):
        raise VerifyError("C symbol mapping is not bijective")
    if len(sources) != len(set(sources)) or len(objects) != len(set(objects)):
        raise VerifyError("C source/object mapping is not bijective")
    if any(source.replace("src/", "build/src/") + ".o" != obj for source, obj in zip(sources, objects)):
        raise VerifyError("C source/object derivation invariant failed")

    build_wrapper = (ROOT / "scripts/build_us.sh").read_text(encoding="utf-8")
    verify_wrapper = (ROOT / "scripts/verify_us.sh").read_text(encoding="utf-8")
    if "func_" in build_wrapper or "func_" in verify_wrapper:
        raise VerifyError("per-leaf symbol leaked back into a build/verify wrapper")


def local_generated_checks(plan: dict[str, Any]) -> None:
    expected_sources = [plan["header"]["source"], *(u["source"] for u in plan["units"])]
    missing = [source for source in expected_sources if not (ROOT / source).is_file()]
    if missing:
        raise VerifyError(
            "missing split source(s): "
            + ", ".join(missing[:12])
            + (f" (+{len(missing)-12} more)" if len(missing) > 12 else "")
        )
    check_ignore = subprocess.run(
        ["git", "check-ignore", "--stdin"],
        cwd=ROOT,
        input=("\n".join(expected_sources) + "\n").encode(),
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    ignored = set(check_ignore.stdout.decode().splitlines())
    generated = [source for source in expected_sources if not source.startswith("src/")]
    not_ignored = [source for source in generated if source not in ignored]
    if not_ignored:
        raise VerifyError("split artifacts are not git-ignored: " + ", ".join(not_ignored))


def exact_checks(plan: dict[str, Any]) -> None:
    exe = ROOT / EXE
    candidate = ROOT / CANDIDATE
    if not exe.is_file():
        raise VerifyError(f"missing retail executable {EXE}")
    if not candidate.is_file():
        raise VerifyError(
            f"missing candidate {CANDIDATE}; run scripts/build_us.sh first"
        )
    original_hash = sha1(exe)
    candidate_hash = sha1(candidate)
    if original_hash != plan["expected_sha1"]:
        raise VerifyError(
            f"retail SHA-1 {original_hash} != config {plan['expected_sha1']}"
        )
    if candidate_hash != plan["expected_sha1"]:
        raise VerifyError(
            f"candidate SHA-1 {candidate_hash} != retail {plan['expected_sha1']}"
        )
    original = exe.read_bytes()
    rebuilt = candidate.read_bytes()
    if rebuilt != original:
        raise VerifyError("candidate hash matched but bytes differ (impossible collision gate)")
    for unit in plan["units"]:
        if unit["kind"] != "c":
            continue
        if rebuilt[unit["start"] : unit["end"]] != original[unit["start"] : unit["end"]]:
            raise VerifyError(f"packed C span differs: {unit['name']}")
    pass_line(f"retail SHA-1 {original_hash}")
    pass_line(f"candidate SHA-1 {candidate_hash}")
    pass_line(f"all {plan['counts']['c']} packed C spans equal retail")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--public",
        action="store_true",
        help="artifact-independent authority gate (no retail image or split required)",
    )
    args = parser.parse_args(argv)
    total = 4 if args.public else 7
    try:
        gate(1, total, "YAML span geometry and build-profile model")
        plan = build_plan(root=ROOT, require_generated=not args.public)
        pass_line(
            f"{plan['counts']['units']} spans = {plan['counts']['c']} c + "
            f"{plan['counts']['asm']} asm + {plan['counts']['rodata']} rodata"
        )
        pass_line(f"load geometry closes at 0x{plan['file']['load_size']:X}")

        gate(2, total, "Generated build/verification bijection")
        public_checks(plan)
        pass_line(f"{plan['counts']['c']} YAML C spans -> source -> object -> verify span")

        gate(3, total, "Tracked C sources and generated published status")
        tracked_sources(plan)
        check_status(plan)
        pass_line("every YAML C source is tracked")
        pass_line("every extra tracked function C has a nonmatching disposition")
        pass_line("published matching count equals YAML")

        gate(4, total, "Deterministic generated plans")
        # Verification is read-only with respect to the build tree. Emit into
        # a private temporary directory so a Docker-created build owned by a
        # different UID cannot make this gate fail (or tempt a chmod/chown).
        with tempfile.TemporaryDirectory(prefix="pe-disc1-verify-") as temporary:
            output = Path(temporary)
            write_generated(plan, output)
            generated_manifest = json.loads(
                (output / "disc1_verify_manifest.json").read_text(encoding="utf-8")
            )
        if generated_manifest != verification_manifest(plan):
            raise VerifyError("generated verifier manifest is not deterministic")
        pass_line(f"plan SHA-256 {plan['plan_sha256']}")
        if args.public:
            print("\nPUBLIC_VERIFY=PASS")
            print(f"matching-C count: {plan['counts']['c']} (from YAML)")
            return 0

        gate(5, total, "Split-generated source coverage")
        local_generated_checks(plan)
        pass_line("all YAML-derived split sources exist and are ignored")

        gate(6, total, "Split prerequisite gate")
        split = subprocess.run(
            [str(ROOT / "scripts/split_us.sh"), "--check"],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            check=False,
        )
        if split.returncode:
            raise VerifyError(
                "scripts/split_us.sh --check failed:\n"
                + split.stdout.decode(errors="replace")
            )
        pass_line("scripts/split_us.sh --check")

        gate(7, total, "Exact candidate and packed-span verification")
        exact_checks(plan)
    except (PlanError, VerifyError, subprocess.CalledProcessError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    print("\nVERIFY_US=PASS")
    print(f"matching-C count: {plan['counts']['c']} (from YAML)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
