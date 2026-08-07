#!/usr/bin/env python3
"""Production Phase 6E-B49 real-disc host-loop acceptance harness."""

from __future__ import annotations

import argparse
import hashlib
import pathlib
import re
import subprocess
import tempfile


CANONICAL_FRAMEBUFFER = (
    "fb28dc21dd1e41eb72b8fe22dd3295bb8ed0c040aa88f7885a68dedc2629dfdb"
)
FB_STATE = re.compile(r"\[FB\].*presents=(\d+).*main_iters=(\d+)")


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run(
    executable: pathlib.Path,
    disc: pathlib.Path | None,
    work: pathlib.Path,
    name: str,
    *options: str,
    timeout: int,
) -> tuple[subprocess.CompletedProcess[str], pathlib.Path, pathlib.Path]:
    screenshot = work / f"{name}.ppm"
    trace = work / f"{name}.trace"
    command = [str(executable), "--headless", *options]
    if disc is not None:
        command += ["--disc-image", str(disc)]
    command += ["--screenshot", str(screenshot), "--trace", str(trace)]
    try:
        result = subprocess.run(
            command,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            timeout=timeout,
            check=False,
        )
    except subprocess.TimeoutExpired as exc:
        raise SystemExit(f"FAIL: {name} exceeded the {timeout}s bound") from exc
    return result, screenshot, trace


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"FAIL: {message}")


def state(result: subprocess.CompletedProcess[str], name: str) -> tuple[int, int]:
    match = FB_STATE.search(result.stderr)
    require(match is not None, f"{name}: missing framebuffer state")
    assert match is not None
    return int(match.group(1)), int(match.group(2))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("executable", type=pathlib.Path)
    parser.add_argument("disc1", type=pathlib.Path)
    parser.add_argument("--timeout", type=int, default=60)
    args = parser.parse_args()

    executable = args.executable.resolve()
    disc = args.disc1.resolve()
    require(executable.is_file(), f"missing executable: {executable}")
    require(disc.is_file(), f"missing Disc 1 image: {disc}")

    with tempfile.TemporaryDirectory(prefix="pe-b49-") as temp:
        work = pathlib.Path(temp)

        one_a, one_a_ppm, one_a_trace = run(
            executable, disc, work, "one-a",
            "--strict-stubs", "--max-frames", "1", timeout=args.timeout,
        )
        require(one_a.returncode == 0, "one-frame strict run did not exit 0")
        require(state(one_a, "one-a") == (1, 1),
                "one-frame run did not stop at exactly frame 1 / iteration 1")
        require("[HOST] stop_reason=frame-limit" in one_a.stderr,
                "one-frame run did not report its explicit budget")
        require(sha256(one_a_ppm) == CANONICAL_FRAMEBUFFER,
                "one-frame framebuffer changed")

        one_b, one_b_ppm, one_b_trace = run(
            executable, disc, work, "one-b",
            "--strict-stubs", "--max-frames", "1", timeout=args.timeout,
        )
        require(one_b.returncode == 0, "repeated one-frame run did not exit 0")
        require(state(one_b, "one-b") == (1, 1),
                "repeated one-frame state changed")
        require(sha256(one_b_ppm) == sha256(one_a_ppm),
                "repeated one-frame screenshot changed")
        require(one_b_trace.read_bytes() == one_a_trace.read_bytes(),
                "repeated one-frame trace changed")

        two, two_ppm, _ = run(
            executable, disc, work, "two",
            "--max-frames", "2", timeout=args.timeout,
        )
        require(two.returncode == 0, "two-frame run did not exit 0")
        require(state(two, "two") == (2, 1),
                "two-frame run did not stop at exactly frame 2 / iteration 1")
        require("[STUB:BOOTSTRAP_RET] func_801909B4" in two.stderr,
                "two-frame run did not execute beyond the obsolete stop")
        two_hash = sha256(two_ppm)

        strict, _, _ = run(
            executable, disc, work, "strict-two",
            "--strict-stubs", "--max-frames", "2", timeout=args.timeout,
        )
        require(strict.returncode == 1, "continuing strict run did not exit 1")
        require(
            "first unresolved BOOTSTRAP_RET provider: func_8006AD40" in strict.stderr
            and "called from: func_8001220C" in strict.stderr,
            "continuing strict run did not expose func_8006AD40 from func_8001220C",
        )

        bootstrap, _, _ = run(
            executable, None, work, "bootstrap-strict",
            "--bootstrap-disc", "--strict-stubs", "--max-frames", "2",
            timeout=args.timeout,
        )
        require(bootstrap.returncode == 1,
                "bootstrap strict run did not exit 1")
        require(
            "first unresolved BOOTSTRAP_RET provider: func_8007F72C" in bootstrap.stderr
            and "called from: func_800698D4" in bootstrap.stderr,
            "bootstrap strict frontier changed",
        )

        print("B49 host-loop harness: PASS")
        print(f"one-frame sha256={CANONICAL_FRAMEBUFFER} frames=1 iterations=1")
        print(f"two-frame sha256={two_hash} frames=2 iterations=1")
        print("strict frontier=func_8006AD40 caller=func_8001220C")
        print("bootstrap strict frontier=func_8007F72C caller=func_800698D4")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
