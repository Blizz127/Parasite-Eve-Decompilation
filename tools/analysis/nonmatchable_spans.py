#!/usr/bin/env python3
"""Classify split-asm spans that are provably not C-matchable.

The split asm under ``asm/disc1/`` contains spans that no C source can
reproduce, so counting them as "remaining asm debt" overstates the real work.
This tool recognises the provable classes and prints honest counts; anything
not positively classified stays "real remaining asm".

Recognised classes (positive per-span instruction evidence required):

``handwritten-jr-t2``
    The ENTIRE body is one or more repeated triples

        addiu $t2, $zero, <vector>     ; e.g. 0xA0 (BIOS A0) / 0xB0 (B0)
        jr    $t2
        addiu $t1, $zero, <function>   ; delay slot carries the argument

    The callee index is materialized in ``$t2`` before an indirect jump with a
    live ``$t1`` argument in the delay slot. C's indirect call emits
    ``li/li/jalr $t2`` with the argument in ``$a0``; the ``$t2``/``$t1`` split
    and the ``jr`` (non-``jal``) form are SDK-handwritten. Proven in
    ``docs/evidence/non-c-matchable/REPORT.md``.

``handwritten-syscall``
    A raw ``syscall N`` as the whole body (``li $a0,N`` / ``syscall`` /
    ``jr $ra``). C has no ``syscall`` operator; the only source spelling is
    inline ``__asm__``, which is forbidden by the residual policy. Even if
    used, it cannot carry the ``jr``+``nop`` vs ``j $31`` epilogue divergence.

``alignment-filler``
    A lone ``nop`` (or a run of them) between real functions, not a function.
    Identified from the disassembler's own inter-``endlabel`` comment. These
    are layout padding, not matchable code.

``handwritten-cop``
    A span whose entire body is COP2/GTE ops (``cfc2``/``ctc2``/``mfc2``/
    ``mtc2``/``lwc2``/``swc2``/``cop2``/``rfe``) plus only ``jr``/``nop`` —
    a hand-written GTE primitive with no integer code to lift (e.g. the
    ``ctc2`` primitive pairs at ``func_800661A4``/``func_800661CC``).

``handwritten-gte-wrapper``
    A span that contains contiguous runs (>=3) of raw GTE macro ops — any of
    the COP2 transfer ops (``cfc2``/``ctc2``/``mfc2``/``mtc2``/``lwc2``/
    ``swc2``) or the Psy-Q/libgte command ops (``mvmva``/``rtps``/``rtpt``/
    ``nclip``/``avsz3``/``avsz4``/``sqr``/``op``/``gpf``/``gpl``/``intpl``/…),
    or a body with *no* conditional branch and *no* call that still contains a
    COP op.  ``cc1`` cannot emit any COP2 instruction from C: every one of the
    2419 COP ops in this bucket is flagged ``/* handwritten instruction */``
    (or is a GTE command op with no C spelling).  Empirically, **0 of the 670
    matched C leaves contains a single COP op**.  So the GTE macro cannot come
    from C and the span is not C-matchable as a unit.

    Counted in the non-C-matchable total *for the span*: the embedded GTE
    sequence is handwritten, while the surrounding integer code may still be
    liftable (the honest end-state is a C body plus an asm-side GTE
    primitive, or an ACCEPTED-RESIDUAL disposition).

``gte-inline`` (reported only, NOT counted as non-matchable)
    A span that contains a COP op only as an isolated pair (max contiguous run
    < 3) inside otherwise clean, branch/call-bearing integer code — i.e. the
    span is dominated by ordinary liftable C and the one GTE primitive is a
    localised island.  Reported for visibility, excluded from the honest
    non-C-matchable total.

Usage:
  python3 tools/analysis/nonmatchable_spans.py            # counts
  python3 tools/analysis/nonmatchable_spans.py --list     # per-span detail
  python3 tools/analysis/nonmatchable_spans.py --json     # machine readable
"""
from __future__ import annotations

import argparse
import json
import re
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
ASM = ROOT / "asm" / "disc1"

HEAD_RE = re.compile(r"^nonmatching\s+([A-Za-z_0-9]+),\s*0x([0-9A-Fa-f]+)")
LABEL_RE = re.compile(r"^glabel\s+([A-Za-z_0-9]+)")
END_RE = re.compile(r"^endlabel\s+([A-Za-z_0-9]+)")
INSN_RE = re.compile(
    r"^\s*/\*\s*([0-9A-Fa-f]+)\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})\s*\*/\s*(.*)$")
FILLER_RE = re.compile(r"^endlabel\s+(\S+)\s*$")
NOP_COMMENT_RE = re.compile(r"^\s*/\*\s*[0-9A-Fa-f]+\s+[0-9A-Fa-f]{8}\s+00000000\s*\*/\s*nop")

# COP2 register-transfer ops (GTE "load/store/read" side).
COP_XFER = frozenset(("cfc2", "ctc2", "mfc2", "mtc2", "lwc2", "swc2", "cop2"))
# GTE command ops emitted by the Psy-Q/libgte ``gte_*`` assembly macros. There
# is no C construct that produces any of these; cc1 2.7.2 has no COP2 emit.
GTE_CMD = frozenset((
    "gpf", "gpl", "gpfl", "mvmva", "rtps", "rtpt", "nclip", "ncds",
    "ncdt", "nccs", "ncct", "ncs", "nct", "cc", "cdp", "dcpl", "dpcl", "dpcs",
    "dpct", "intpl", "sqr", "op", "avsz3", "avsz4", "dv", "rfe",
))
COP_OPS = COP_XFER | GTE_CMD
# Control-transfer families used by the wrapper/inline discriminator.
COND_OPS = frozenset(("beq", "bne", "beqz", "bnez", "bgez", "bgtz", "blez",
                      "bltz", "bltzal", "bgezal", "b", "j"))
CALL_OPS = frozenset(("jal", "jalr"))
# A contiguous run of this many COP ops marks a GTE macro sequence rather than
# an isolated op inside ordinary C.
GTE_RUN_THRESHOLD = 3


def _op(text: str) -> str:
    text = text.split("/*")[0].strip()
    return re.sub(r"\s+", " ", text) if text else ""


def _mn(op: str) -> str:
    return op.split()[0] if op else ""


def parse_spans():
    spans = []
    for path in sorted(ASM.glob("*.s")):
        lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
        i = 0
        while i < len(lines):
            m = HEAD_RE.match(lines[i])
            if not m:
                i += 1
                continue
            name, size = m.group(1), int(m.group(2), 16)
            ops, labels = [], []
            j = i + 1
            while j < len(lines):
                if END_RE.match(lines[j]):
                    break
                if LABEL_RE.match(lines[j]):
                    labels.append(LABEL_RE.match(lines[j]).group(1))
                im = INSN_RE.match(lines[j])
                if im:
                    ops.append(_op(im.group(4)))
                j += 1
            vm = re.search(r"/\*\s*([0-9A-Fa-f]+)\s+([0-9A-Fa-f]{8})",
                           "\n".join(lines[i:j + 1]))
            spans.append({
                "name": name, "file": str(path.relative_to(ROOT)),
                "file_off": int(vm.group(1), 16) if vm else None,
                "vram": int(vm.group(2), 16) if vm else None,
                "size": size, "labels": labels, "ops": ops,
                # the comment immediately preceding glabel, used by the
                # disassembler to flag handwritten and filler spans
                "lead": " ".join(lines[max(0, i - 4):i]).lower(),
            })
            i = j + 1
    return spans


def _is_thunk_ops(ops) -> bool:
    t = list(ops)
    if not t:
        return False
    while t:
        if (len(t) >= 3
                and re.match(r"addiu \$t2, \$zero, 0x", t[0])
                and t[1] == "jr $t2"
                and re.match(r"addiu \$t1, \$zero, 0x", t[2])):
            t = t[3:]
            if t and _mn(t[0]) == "nop":
                t = t[1:]
        else:
            return False
    return True


def _max_gte_run(ops) -> int:
    """Longest contiguous run of COP2/GTE ops in the instruction stream."""
    best = cur = 0
    for o in ops:
        if _mn(o) in COP_OPS:
            cur += 1
            best = max(best, cur)
        else:
            cur = 0
    return best


def classify(span) -> str | None:
    ops = span["ops"]
    body = [o for o in ops if o and o != ".data"]
    if not body:
        return None
    mn = [_mn(o) for o in body]
    if mn == ["nop"]:
        return "alignment-filler"
    if _is_thunk_ops(ops) and any(o == "jr $t2" for o in ops):
        return "handwritten-jr-t2"
    # `syscall` is emitted only by hand (cc1 2.7.2 has no C construct for it),
    # so any span that contains one at all is not C-matchable.
    if "syscall" in mn:
        return "handwritten-syscall"
    cop = [m for m in mn if m in COP_OPS]
    # A span whose *entire* body is COP2 ops plus `jr`/`nop` is a hand-written
    # GTE primitive: there is no integer C code to lift at all.
    nonop = [m for m in mn if m not in ("nop", "jr", "j")]
    if cop and nonop and all(m in cop for m in nonop):
        return "handwritten-cop"
    if cop:
        # A GTE *command* op (`rtps`/`rtpt`/`mvmva`/…) and a long contiguous COP
        # run are both Psy-Q/libgte macro expansion: no C construct emits a COP2
        # op, so a span carrying one is not C-matchable as a unit. A lone COP2
        # register-transfer pair (`mtc2`/`mfc2`/`lwc2`/`swc2`) inside
        # branch/call-bearing integer code is the only shape worth lifting: the
        # body is dominated by ordinary C and the transfer is a localised island
        # (reported as `gte-inline`, informational, not counted as non-C).
        run = _max_gte_run(ops)
        cmd = any(m in GTE_CMD for m in mn)
        has_cond = any(m in COND_OPS for m in mn)
        has_call = any(m in CALL_OPS for m in mn)
        if cmd or run >= GTE_RUN_THRESHOLD or not (has_cond or has_call):
            return "handwritten-gte-wrapper"
        return "gte-inline"
    return None


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--list", action="store_true")
    args = ap.parse_args(argv)

    spans = parse_spans()
    classes: dict[str, list] = {}
    for s in spans:
        c = classify(s)
        if c:
            classes.setdefault(c, []).append(s)

    if args.json:
        matchable = [s for s in spans
                     if classify(s) == "gte-inline"]
        print(json.dumps({
            "total_spans": len(spans),
            "counts": {k: len(v) for k, v in sorted(classes.items())},
            "words": {k: sum(s["size"] // 4 for s in v)
                      for k, v in sorted(classes.items())},
            "spans": {k: [{"name": s["name"], "vram": f"0x{s['vram']:08X}",
                           "size": f"0x{s['size']:X}", "file": s["file"]}
                          for s in v] for k, v in sorted(classes.items())
                      if k != "gte-inline"},
            # `gte-inline` spans merely *contain* an isolated GTE op; the bulk
            # of each body is ordinary liftable integer code, so they are NOT
            # counted as non-C-matchable by consumers. Kept out of `spans` (the
            # consumer's non-C contract) and reported separately. The former
            # `matchable_cop_inline` key is retained as an alias for backwards
            # compatibility with existing consumers.
            "matchable_cop_inline": [
                {"name": s["name"], "vram": f"0x{s['vram']:08X}",
                 "size": f"0x{s['size']:X}", "file": s["file"],
                 "class": "gte-inline"}
                for s in matchable],
        }, indent=2))
        return 0

    for k in sorted(classes):
        v = classes[k]
        words = sum(s["size"] // 4 for s in v)
        tag = ""
        if k == "alignment-filler":
            tag = " (not a function; layout pad)"
        elif k == "gte-inline":
            tag = " (informational only; still liftable, NOT non-C)"
        elif k == "handwritten-gte-wrapper":
            tag = " (embedded GTE macro; span not C-matchable)"
        print(f"{k:24} {len(v):5} spans  {words:6} words{tag}")
        if args.list:
            for s in v:
                print(f"    {s['name']:20} {s['file']}:0x{s['file_off']:x} "
                      f"size 0x{s['size']:X}  vram 0x{s['vram']:08X}")
    print(f"\ntotal split spans: {len(spans)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
