#!/usr/bin/env python3
"""Build the USA Disc 1 executable from the YAML-derived plan."""

from __future__ import annotations

import argparse
import hashlib
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Sequence

from disc1_plan import PlanError, build_plan, write_generated


ROOT = Path(__file__).resolve().parents[2]
GENERATED = Path("build/generated")
EXE = Path("build/extracted/disc1/SLUS_006.62")
ERA_CPP = Path("tools/era/gcc-2.7.2-psx/cpp")
ERA_CC1 = Path("tools/era/gcc-2.7.2-psx/cc1")
MASPSX = Path("tools/era/maspsx/maspsx.py")
TRIM = Path("tools/trim_elf_section_pad.py")
AS_FLAGS = ["-EL", "-mips1", "-mabi=32"]
MODERN_C_FLAGS = [
    "-EL",
    "-mips1",
    "-mfp32",
    "-mabi=32",
    "-G0",
    "-fno-pic",
    "-mno-abicalls",
    "-ffreestanding",
    "-fno-builtin",
    "-O1",
]


class BuildError(RuntimeError):
    """The generated build could not be completed or did not match retail."""


@dataclass(frozen=True)
class Toolchain:
    runner: tuple[str, ...]
    assembler: str
    linker: str
    objcopy: str
    readelf: str
    compiler: str
    note: str

    def command(self, tool: str, *arguments: str) -> list[str]:
        return [*self.runner, tool, *arguments]


def step(label: str) -> None:
    print(f"\n=== {label} ===", flush=True)


def info(message: str) -> None:
    print(f"  {message}", flush=True)


def run(
    command: Sequence[str],
    *,
    cwd: Path = ROOT,
    env: dict[str, str] | None = None,
    stdin: Any = None,
    stdout: Any = None,
    stderr: Any = None,
    check: bool = True,
) -> subprocess.CompletedProcess[Any]:
    try:
        return subprocess.run(
            list(command),
            cwd=cwd,
            env=env,
            stdin=stdin,
            stdout=stdout,
            stderr=stderr,
            check=check,
        )
    except FileNotFoundError as exc:
        raise BuildError(f"command not found: {command[0]}") from exc
    except subprocess.CalledProcessError as exc:
        raise BuildError(
            f"command failed ({exc.returncode}): {' '.join(command)}"
        ) from exc


def first_version_line(command: Sequence[str]) -> str:
    result = run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        check=False,
    )
    text = result.stdout.decode(errors="replace") if result.stdout else ""
    return text.splitlines()[0] if text.splitlines() else "unknown"


def find_toolchain() -> Toolchain:
    names = {
        "assembler": "mipsel-linux-gnu-as",
        "linker": "mipsel-linux-gnu-ld",
        "objcopy": "mipsel-linux-gnu-objcopy",
        "readelf": "mipsel-linux-gnu-readelf",
        "compiler": "mipsel-linux-gnu-gcc",
    }
    resolved = {key: shutil.which(value) for key, value in names.items()}
    if all(resolved.values()):
        return Toolchain(
            runner=(),
            note="host PATH",
            **{key: str(value) for key, value in resolved.items()},
        )

    distrobox = shutil.which("distrobox")
    if distrobox:
        listed = run(
            [distrobox, "list"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            check=False,
        )
        if b"pe-mipsel" in (listed.stdout or b""):
            return Toolchain(
                runner=(distrobox, "enter", "pe-mipsel", "--"),
                note="distrobox pe-mipsel",
                **names,
            )
    raise BuildError(
        "mipsel-linux-gnu toolchain not found; run inside pe-mipsel-img or "
        "install binutils-mipsel-linux-gnu and gcc-mipsel-linux-gnu"
    )


def require_inputs(plan: dict[str, Any]) -> None:
    if not (ROOT / "CLAUDE.md").is_file():
        raise BuildError(f"not a repository root: {ROOT}")
    exe = ROOT / EXE
    if not exe.is_file():
        raise BuildError(f"missing {EXE}; run scripts/extract_us.sh 1")
    actual = hashlib.sha1(exe.read_bytes()).hexdigest()
    if actual != plan["expected_sha1"]:
        raise BuildError(
            f"retail executable SHA-1 {actual} != {plan['expected_sha1']}"
        )
    for path in (ERA_CPP, ERA_CC1, MASPSX, TRIM):
        if not (ROOT / path).is_file():
            raise BuildError(f"missing required tool: {path}")
    if not os.access(ROOT / ERA_CPP, os.X_OK) or not os.access(ROOT / ERA_CC1, os.X_OK):
        raise BuildError("era cpp/cc1 are not executable; run scripts/setup_era.sh")
    info(f"OK original EXE SHA-1 {actual}")
    info(
        f"OK YAML plan: {plan['counts']['units']} spans, "
        f"{plan['counts']['c']} C leaves, geometry 0x{plan['file']['load_size']:X}"
    )


def absolutize_rodata(plan: dict[str, Any]) -> None:
    pattern = re.compile(r"(\.word\s+)\.L([0-9A-Fa-f]{8})(?=\s|$)")
    total = 0
    for unit in plan["units"]:
        if unit["kind"] != "rodata":
            continue
        path = ROOT / unit["source"]
        source = path.read_text(encoding="utf-8")

        def replace(match: re.Match[str]) -> str:
            nonlocal total
            address = int(match.group(2), 16)
            if not 0x80010000 <= address < 0x80200000:
                return match.group(0)
            total += 1
            return match.group(1) + "0x" + match.group(2)

        updated = pattern.sub(replace, source)
        if updated != source:
            path.write_text(updated, encoding="utf-8")
    info(f"rodata absolutize: {total} label refs -> literal")


def assemble_all(plan: dict[str, Any], tools: Toolchain) -> None:
    (ROOT / "build/asm/disc1/data").mkdir(parents=True, exist_ok=True)
    (ROOT / "build/src").mkdir(parents=True, exist_ok=True)
    header = plan["header"]
    run(
        tools.command(
            tools.assembler,
            *AS_FLAGS,
            "-I",
            str(ROOT / "include"),
            "-o",
            header["object"],
            header["source"],
        )
    )
    absolutize_rodata(plan)
    for unit in plan["units"]:
        if unit["kind"] == "c":
            continue
        Path(unit["object"]).parent.mkdir(parents=True, exist_ok=True)
        run(
            tools.command(
                tools.assembler,
                *AS_FLAGS,
                "-I",
                str(ROOT / "include"),
                "-o",
                unit["object"],
                unit["source"],
            )
        )
    info(f"assembled {plan['counts']['asm'] + plan['counts']['rodata'] + 1} units")


def force_absolute_symbols(assembly: Path, specification: str) -> None:
    text = assembly.read_text(encoding="utf-8")
    for symbol in specification.split(","):
        text = re.sub(
            rf"\t\.extern\t{re.escape(symbol)}, \d+\n",
            "",
            text,
        )
    assembly.write_text(text, encoding="utf-8")


def strip_dispatch_rodata(object_path: Path, symbol: str) -> None:
    if not re.fullmatch(r"jtbl_[0-9A-Fa-f]{8}", symbol):
        raise BuildError(
            f"MASPSX_DISPATCH_FOLD {symbol!r} is not jtbl_<vram-hex>"
        )
    pool_source = "".join(
        path.read_text(encoding="utf-8")
        for path in sorted((ROOT / "asm/disc1/data").glob("*.rodata.s"))
    )
    block = re.search(
        rf"dlabel {re.escape(symbol)}\n(.*?)enddlabel {re.escape(symbol)}",
        pool_source,
        re.DOTALL,
    )
    if not block:
        raise BuildError(f"pool source has no {symbol} block")
    literals = re.findall(r"\.word\s+(0x[0-9A-Fa-f]+)", block.group(1))
    if len(literals) < 2:
        raise BuildError(f"{symbol} pool block has no literal words")
    expected_size = len(literals) * 4

    data = bytearray(object_path.read_bytes())
    if data[:4] != b"\x7fELF":
        raise BuildError(f"{object_path}: not ELF")
    e_shoff = struct.unpack_from("<I", data, 32)[0]
    e_shentsize = struct.unpack_from("<H", data, 46)[0]
    e_shnum = struct.unpack_from("<H", data, 48)[0]
    e_shstrndx = struct.unpack_from("<H", data, 50)[0]
    shstr_off = struct.unpack_from(
        "<I", data, e_shoff + e_shstrndx * e_shentsize + 16
    )[0]

    def header(index: int) -> int:
        return e_shoff + index * e_shentsize

    def section_name(index: int) -> str:
        offset = struct.unpack_from("<I", data, header(index))[0]
        end = data.index(b"\x00", shstr_off + offset)
        return data[shstr_off + offset : end].decode()

    rodata_index = next(
        (index for index in range(e_shnum) if section_name(index) == ".rodata"),
        None,
    )
    if rodata_index is None:
        raise BuildError("compiled switch leaf has no .rodata")
    rodata_header = header(rodata_index)
    rodata_size = struct.unpack_from("<I", data, rodata_header + 20)[0]
    if rodata_size != expected_size:
        raise BuildError(
            f".rodata 0x{rodata_size:X} != pool table 0x{expected_size:X} "
            f"for {symbol}; refusing to strip"
        )

    text_index = next(
        (index for index in range(e_shnum) if section_name(index) == ".text"),
        None,
    )
    if text_index is None:
        raise BuildError("compiled switch leaf has no .text")
    text_size = struct.unpack_from("<I", data, header(text_index) + 20)[0]
    relocation_index = next(
        (
            index
            for index in range(e_shnum)
            if section_name(index) == ".rel.rodata"
        ),
        None,
    )
    if relocation_index is None:
        raise BuildError("no .rel.rodata; cannot prove dispatch table identity")
    relocation_header = header(relocation_index)
    relocation_offset = struct.unpack_from("<I", data, relocation_header + 16)[0]
    relocation_size = struct.unpack_from("<I", data, relocation_header + 20)[0]
    symbol_table_index = struct.unpack_from("<I", data, relocation_header + 24)[0]
    symbol_offset = struct.unpack_from(
        "<I", data, header(symbol_table_index) + 16
    )[0]
    words = relocation_size // 8
    if words != len(literals):
        raise BuildError(
            f"{words} dispatch relocs != {len(literals)} pool words for {symbol}"
        )
    for index in range(words):
        relocation_at, relocation_info = struct.unpack_from(
            "<II", data, relocation_offset + index * 8
        )
        if relocation_at != index * 4 or relocation_info & 0xFF != 2:
            raise BuildError(f"dispatch relocation {index} has unexpected shape")
        symbol_index = relocation_info >> 8
        symbol_value = struct.unpack_from(
            "<I", data, symbol_offset + symbol_index * 16 + 4
        )[0]
        symbol_section = struct.unpack_from(
            "<H", data, symbol_offset + symbol_index * 16 + 14
        )[0]
        if symbol_section != text_index or symbol_value >= text_size:
            raise BuildError(
                f"dispatch relocation {index} does not target local .text"
            )

    struct.pack_into("<I", data, rodata_header + 20, 0)
    struct.pack_into("<I", data, relocation_header + 20, 0)

    file_length = len(data)
    for index in range(e_shnum):
        section_header = header(index)
        offset, size = struct.unpack_from("<II", data, section_header + 16)
        section_type = struct.unpack_from("<I", data, section_header + 4)[0]
        if section_type != 8 and offset + size > file_length:
            raise BuildError(
                f"post-strip section {index} ({section_name(index)}) is out of bounds"
            )
        if section_type == 9:
            target = struct.unpack_from("<I", data, section_header + 28)[0]
            if target == rodata_index and size:
                raise BuildError("post-strip relocation still targets .rodata")

    symbol_table_header = header(symbol_table_index)
    symbol_offset_2 = struct.unpack_from("<I", data, symbol_table_header + 16)[0]
    symbol_size = struct.unpack_from("<I", data, symbol_table_header + 20)[0]
    if symbol_offset_2 + symbol_size > file_length:
        raise BuildError("post-strip symbol table is out of bounds")
    for index in range(symbol_size // 16):
        section_index = struct.unpack_from(
            "<H", data, symbol_offset_2 + index * 16 + 14
        )[0]
        if section_index >= e_shnum and section_index not in (0, 0xFFF1, 0xFFF2):
            raise BuildError(
                f"post-strip symbol {index} has bad shndx 0x{section_index:X}"
            )
    object_path.write_bytes(data)
    info(
        f"dispatch dedup: stripped {rodata_size}-byte duplicate table "
        f"from {object_path.relative_to(ROOT)}"
    )


def compile_era(unit: dict[str, Any], tools: Toolchain) -> None:
    source = ROOT / unit["source"]
    output = ROOT / unit["object"]
    leaf_environment = os.environ.copy()
    leaf_environment.update(unit["environment"])
    with tempfile.TemporaryDirectory(prefix="pe-era-") as temporary:
        temp = Path(temporary)
        preprocessed = temp / "x.i"
        assembly = temp / "x.s"
        expanded = temp / "xm.s"
        with preprocessed.open("wb") as stream:
            run(
                [str(ROOT / ERA_CPP), str(source)],
                env=leaf_environment,
                stdout=stream,
                stderr=subprocess.DEVNULL,
            )
        run(
            [
                str(ROOT / ERA_CC1),
                "-quiet",
                *unit["flags"],
                str(preprocessed),
                "-o",
                str(assembly),
            ],
            env=leaf_environment,
        )
        forced = unit["environment"].get("MASPSX_FORCE_ABSOLUTE_SYMBOLS")
        if forced:
            force_absolute_symbols(assembly, forced)
        aspsx_version = unit["environment"].get("ERA_ASPSX_VER", "2.21")
        maspsx_command = [
            sys.executable,
            str(ROOT / MASPSX),
            f"--aspsx-version={aspsx_version}",
            "--dont-expand-li",
        ]
        if unit["environment"].get("MASPSX_EXPAND_DIV") == "1":
            maspsx_command.append("--expand-div")
        maspsx_command.append(str(assembly))
        with expanded.open("wb") as stream:
            run(
                maspsx_command,
                env=leaf_environment,
                stdin=subprocess.DEVNULL,
                stdout=stream,
            )
        run(
            tools.command(
                tools.assembler,
                *AS_FLAGS,
                "-I",
                str(ROOT / "include"),
                "-o",
                str(output),
                str(expanded),
            ),
            env=leaf_environment,
        )
    dispatch = unit["environment"].get("MASPSX_DISPATCH_FOLD")
    if dispatch:
        strip_dispatch_rodata(output, dispatch)


def compile_all(plan: dict[str, Any], tools: Toolchain) -> None:
    counts: dict[str, int] = {"era": 0, "modern": 0}
    for unit in plan["units"]:
        if unit["kind"] != "c":
            continue
        Path(unit["object"]).parent.mkdir(parents=True, exist_ok=True)
        if unit["toolchain"] == "era":
            compile_era(unit, tools)
        elif unit["toolchain"] == "modern":
            run(
                tools.command(
                    tools.compiler,
                    *MODERN_C_FLAGS,
                    *unit["flags"],
                    "-c",
                    "-o",
                    unit["object"],
                    unit["source"],
                )
            )
        else:
            raise BuildError(f"unknown toolchain for {unit['name']}")
        counts[unit["toolchain"]] += 1
    info(
        f"compiled {sum(counts.values())} YAML C leaves "
        f"({counts['era']} era, {counts['modern']} modern)"
    )


def trim_all(plan: dict[str, Any]) -> None:
    for unit in plan["units"]:
        run(
            [
                sys.executable,
                str(ROOT / TRIM),
                unit["object"],
                unit["primary_section"],
                f"0x{unit['size']:X}",
            ]
        )
    header = plan["header"]
    result = run(
        [
            sys.executable,
            str(ROOT / TRIM),
            header["object"],
            header["section"],
            f"0x{header['size']:X}",
        ],
        check=False,
    )
    if result.returncode:
        info("header pad trim was not required")
    info(f"trimmed {len(plan['units'])} YAML-derived object spans")


def prepare_absolute_symbols() -> Path:
    output = ROOT / "build/abs_syms.ld"
    pieces: list[str] = []
    for name in ("undefined_syms_auto.txt", "undefined_funcs_auto.txt"):
        path = ROOT / name
        if path.is_file():
            pieces.append(path.read_text(encoding="utf-8"))
    pieces.extend([".L00000000_main = 0;\n", "_gp = 0x8009CD70;\n"])
    output.write_text("".join(pieces), encoding="utf-8")
    return output


def link(plan: dict[str, Any], tools: Toolchain) -> None:
    linker_script = ROOT / GENERATED / "disc1_romorder.ld"
    absolute_symbols = prepare_absolute_symbols()
    probe_error = ROOT / "build/link_probe.err"
    probe_command = tools.command(
        tools.linker,
        "-EL",
        "-m",
        "elf32ltsmip",
        "-nostdlib",
        "--no-check-sections",
        "-T",
        str(linker_script),
        "-T",
        str(absolute_symbols),
        "-o",
        "build/disc1_probe.elf",
    )
    with probe_error.open("wb") as stream:
        probe = run(probe_command, stderr=stream, check=False)
    if probe.returncode:
        errors = probe_error.read_text(encoding="utf-8", errors="replace")
        symbols = sorted(set(re.findall(r"undefined reference to `?(D_[0-9A-Fa-f]+)", errors)))
        if symbols:
            with absolute_symbols.open("a", encoding="utf-8") as stream:
                for symbol in symbols:
                    stream.write(f"{symbol} = 0x{symbol[2:]};\n")
            info(f"probe link added {len(symbols)} derived absolute symbols")

    link_error = ROOT / "build/link.err"
    link_command = tools.command(
        tools.linker,
        "-EL",
        "-m",
        "elf32ltsmip",
        "-nostdlib",
        "--no-check-sections",
        "-T",
        str(linker_script),
        "-T",
        str(absolute_symbols),
        "-Map",
        "build/disc1.map",
        "-o",
        "build/disc1.elf",
    )
    with link_error.open("wb") as stream:
        linked = run(link_command, stderr=stream, check=False)
    if linked.returncode:
        excerpt = "\n".join(
            link_error.read_text(encoding="utf-8", errors="replace").splitlines()[:40]
        )
        raise BuildError(f"link failed ({linked.returncode})\n{excerpt}")
    info("OK build/disc1.elf")


def pack_and_compare(plan: dict[str, Any], tools: Toolchain) -> None:
    run(
        tools.command(
            tools.objcopy,
            "-O",
            "binary",
            "-j",
            ".header",
            "build/disc1.elf",
            "build/disc1.header.bin",
        )
    )
    run(
        tools.command(
            tools.objcopy,
            "-O",
            "binary",
            "-j",
            ".main",
            "build/disc1.elf",
            "build/disc1.main.bin",
        )
    )
    header = (ROOT / "build/disc1.header.bin").read_bytes()[:0x800].ljust(
        0x800, b"\x00"
    )
    main_raw = (ROOT / "build/disc1.main.bin").read_bytes()
    body = main_raw[:0x1EE000].ljust(0x1EE000, b"\x00")
    candidate = header + body
    candidate_path = ROOT / "build/disc1.candidate.exe"
    candidate_path.write_bytes(candidate)
    original = (ROOT / EXE).read_bytes()
    original_sha1 = hashlib.sha1(original).hexdigest()
    candidate_sha1 = hashlib.sha1(candidate).hexdigest()
    info(f"header: {len(header)} bytes (match={header == original[:0x800]})")
    info(f"main raw: {len(main_raw)} (0x{len(main_raw):X}); packed body: 0x1EE000")
    info(f"candidate: {len(candidate)} (0x{len(candidate):X})")
    info(f"orig SHA-1: {original_sha1}")
    info(f"cand SHA-1: {candidate_sha1}")
    if candidate == original:
        info("RESULT: EXACT MATCH")
        return

    mismatch = next(
        (index for index, pair in enumerate(zip(candidate, original)) if pair[0] != pair[1]),
        min(len(candidate), len(original)),
    )
    owner = None
    for unit in plan["units"]:
        if unit["start"] <= mismatch < unit["end"]:
            owner = unit
            break
    detail = "header/outside YAML plan"
    if owner:
        detail = (
            f"{owner['kind']} {owner['name'] or owner['source']} "
            f"[0x{owner['start']:X},0x{owner['end']:X}) profile={owner['profile']}"
        )
    raise BuildError(
        f"NON-MATCH: first byte at 0x{mismatch:X}: "
        f"candidate=0x{candidate[mismatch]:02X} retail=0x{original[mismatch]:02X}; "
        f"owner={detail}; candidate SHA-1 {candidate_sha1}"
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--assemble-only", action="store_true")
    args = parser.parse_args(argv)
    candidate = ROOT / "build/disc1.candidate.exe"
    candidate.unlink(missing_ok=True)

    try:
        step("Generate and validate YAML build authority")
        plan = build_plan(root=ROOT, require_generated=True)
        write_generated(plan, ROOT / GENERATED)
        require_inputs(plan)

        step("Toolchain")
        tools = find_toolchain()
        info(f"using: {tools.note}")
        info(f"as: {first_version_line(tools.command(tools.assembler, '--version'))}")
        info(f"ld: {first_version_line(tools.command(tools.linker, '--version'))}")
        info(f"cc: {first_version_line(tools.command(tools.compiler, '--version'))}")

        step("Assemble YAML asm/rodata spans")
        assemble_all(plan, tools)
        step("Compile YAML C spans")
        compile_all(plan, tools)
        step("Trim objects to YAML span geometry")
        trim_all(plan)
        if args.assemble_only:
            print("\nAssemble/compile-only complete; no matching claim.")
            return 0

        step("Link generated ROM-order plan")
        link(plan, tools)
        step("Pack and compare retail executable")
        pack_and_compare(plan, tools)
    except (BuildError, PlanError) as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 1

    print("\n=== Summary ===")
    print(f"Plan:      OK ({plan['counts']['units']} YAML spans; no manual span lists)")
    print(f"Compile:   OK ({plan['counts']['c']} generated C entries)")
    print("Trim/link: OK (sizes/order generated from YAML edges)")
    print(f"Compare:   EXACT SHA-1 {plan['expected_sha1']}")
    print(f"Matching claim: YES ({plan['counts']['c']} registered C leaves)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
