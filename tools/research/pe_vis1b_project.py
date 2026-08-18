#!/usr/bin/env python3
"""PE-VIS1-B evidence-only retail world→screen derivation.

Reads a registered USA Disc 1 (or extracted SLUS + PE.IMG). Does not
implement native projection and does not edit production runtime.
"""

from __future__ import annotations

import argparse
import hashlib
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.research import pe_pst0_scan as pst0

LOAD = 0x80010000
EXE_HDR = 0x800
FIELD_TABLE = 0x83B78

M0002I_GEOM = 0x48DA0
M0003I_GEOM = 0x686AC
M0002I_COLL = 0x48874
M0003I_COLL = 0x684D8
M0002I_SCRIPT = 0x4794C
M0003I_SCRIPT = 0x67568

# VIS-C posed idle world samples (do not reopen model math).
VISC_M0003I = (
    ("crown", 8, -284, -451, -189),
    ("pelvis", 0, -272, -372, -184),
    ("r_foot", 438, -248, -222, -169),
)
VISC_M0002I_LOCAL = (
    # posed local from VIS-C m0003i idle; yaw-compose + actor origin applied below
    ("crown", 12, -447, 13),
    ("pelvis", 0, -368, 8),
    ("r_foot", -24, -218, -7),
)

# VIS-C full-mesh SY span (panned display), frozen cad4598.
VISC_SPAN = {
    "m0002i": (102.09, 128.18, 26.09),
    "m0003i": (154.20, 183.90, 29.71),
}


def s16(v: int) -> int:
    v &= 0xFFFF
    return v - 0x10000 if v >= 0x8000 else v


def s32(v: int) -> int:
    v &= 0xFFFFFFFF
    return v - 0x100000000 if v >= 0x80000000 else v


def ru16(b: bytes, o: int) -> int:
    return struct.unpack_from("<H", b, o)[0]


def rs16(b: bytes, o: int) -> int:
    return struct.unpack_from("<h", b, o)[0]


def ru32(b: bytes, o: int) -> int:
    return struct.unpack_from("<I", b, o)[0]


def rs32(b: bytes, o: int) -> int:
    return struct.unpack_from("<i", b, o)[0]


def va2off(va: int) -> int:
    return va - LOAD + EXE_HDR


def word(exe: bytes, va: int) -> int:
    return ru32(exe, va2off(va))


def disasm_window(exe: bytes, va: int, n: int) -> list[str]:
    rows = []
    for i in range(n):
        w = word(exe, va + i * 4)
        rows.append(f"0x{va + i * 4:08X} {w:08X}")
    return rows


def decode_view(package: bytes, geom: int, index: int = 0) -> dict:
    view_base = ru32(package, geom + 0x1C)
    rec = geom + view_base + index * 52
    rot = [rs16(package, rec + 2 + i * 2) for i in range(9)]
    clamp = [rs16(package, rec + 0x2C + i * 2) for i in range(4)]
    mid_x = ((clamp[0] & 0xFFFF) + (clamp[1] & 0xFFFF)) >> 1
    mid_y = ((clamp[2] & 0xFFFF) + (clamp[3] & 0xFFFF)) >> 1
    return {
        "offset": rec,
        "H": ru16(package, rec),
        "R": rot,
        "TR": [rs32(package, rec + 0x14), rs32(package, rec + 0x18), rs32(package, rec + 0x1C)],
        "unk20": rs32(package, rec + 0x20),
        "unk24": rs32(package, rec + 0x24),
        "viewport": [ru16(package, rec + 0x28), ru16(package, rec + 0x2A)],
        "clamp": clamp,
        "pan": (160 - mid_x, 112 - mid_y),
        "header_pan_authored": (rs16(package, geom + 0x38), rs16(package, geom + 0x3A)),
        "view_base": view_base,
        "aux_off": ru32(package, geom + 0x10),
        "layer_off": ru32(package, geom + 0x14),
        "aux_count": ru16(package, geom + 0x04),
        "layer_count": ru16(package, geom + 0x06),
    }


def decode_group_height(package: bytes, coll: int) -> dict:
    blob = package[coll:]
    group_count = ru16(blob, 2)
    header20 = ru32(blob, 0x20)
    group_at = ru32(blob, 0x28)
    height = rs16(blob, group_at) if group_count else 0
    return {
        "group_count": group_count,
        "header20": header20,
        "group_at": group_at,
        "height": height,
        "classic": header20 == 0,
    }


def world_to_camera(view: dict, wx: int, wy: int, wz: int) -> tuple[int, int, int]:
    world = (wx, wy, wz)
    out = []
    for row in range(3):
        mac = view["TR"][row] << 12
        mac += view["R"][row * 3 + 0] * world[0]
        mac += view["R"][row * 3 + 1] * world[1]
        mac += view["R"][row * 3 + 2] * world[2]
        out.append(mac >> 12)
    return out[0], out[1], out[2]


def project_rational(h: int, ir1: int, ir2: int, sz: int, ofx: int = 160, ofy: int = 112):
    if sz == 0:
        raise ZeroDivisionError("SZ")
    sx_num = ofx * sz + h * ir1
    sy_num = ofy * sz + h * ir2
    return sx_num, sy_num, sz, h


def yaw_xz(x: int, z: int, yaw: int, sincos: list[tuple[int, int]]) -> tuple[int, int]:
    s, c = sincos[yaw & 0x0FFF]
    wx = (c * x + s * z) >> 12
    wz = (-s * x + c * z) >> 12
    return wx, wz


def load_sincos(exe: bytes) -> list[tuple[int, int]]:
    off = va2off(0x800966EC)
    out = []
    for i in range(0x1000):
        w = ru32(exe, off + i * 4)
        out.append((s16(w & 0xFFFF), s16(w >> 16)))
    return out


def load_disc(path: Path) -> tuple[bytes, bytes]:
    with pst0.RawMode2Image(path) as image:
        entries = pst0.iso_entries(image)
        exe_ent = pst0.find_entry(entries, pst0.DISC1_EXE)
        img_ent = pst0.find_entry(entries, pst0.PE_IMG)
        exe = image.read_form1_extent(exe_ent[1], exe_ent[2])
        peimg = image.read_form1_extent(img_ent[1], img_ent[2])
    return exe, peimg


def load_extracted(exe_path: Path, peimg_path: Path) -> tuple[bytes, bytes]:
    exe = exe_path.read_bytes()
    peimg = peimg_path.read_bytes()
    return exe, peimg


def package_from_table(exe: bytes, peimg: bytes, index: int) -> bytes:
    start = ru32(exe, FIELD_TABLE + index * 8)
    end = ru32(exe, FIELD_TABLE + (index + 1) * 8)
    return peimg[start * 2048 : end * 2048]


def find_0b_xyz(script: bytes) -> list[dict]:
    decoded = pst0.decode_script(script)
    rows = []
    for mod in decoded["modules"]:
        for cmd in mod["commands"]:
            if cmd["opcode"] == 0x0B and cmd["argc"] >= 4 and cmd["args"][0] == 0:
                rows.append(
                    {
                        "module": mod["index"],
                        "offset": cmd["offset"],
                        "x": s32(cmd["args"][1]),
                        "y": s32(cmd["args"][2]),
                        "z": s32(cmd["args"][3]),
                    }
                )
    return rows


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("disc", nargs="?", help="Disc 1 BIN")
    parser.add_argument("--exe", type=Path)
    parser.add_argument("--peimg", type=Path)
    args = parser.parse_args()

    if args.exe and args.peimg:
        exe, peimg = load_extracted(args.exe, args.peimg)
    elif args.disc:
        exe, peimg = load_disc(Path(args.disc))
    else:
        print("need disc or --exe/--peimg", file=sys.stderr)
        return 2

    exe_sha = hashlib.sha256(exe).hexdigest()
    print(f"exe_sha256={exe_sha}")
    print(f"exe_ok={exe_sha == pst0.DISC1_EXE_SHA256}")
    print(f"peimg_sha1={hashlib.sha1(peimg).hexdigest()}")
    print(f"peimg_ok={hashlib.sha1(peimg).hexdigest() == pst0.PE_IMG_SHA1}")

    sincos = load_sincos(exe)
    print("sincos0", sincos[0], "sincos400", sincos[0x400])

    # Key instruction fingerprints.
    print("=== fingerprints ===")
    print("SetGeomOffset", disasm_window(exe, 0x80079004, 6))
    print("SetGeomScreen", disasm_window(exe, 0x80079024, 4))
    print("func_80066800_head", disasm_window(exe, 0x80066800, 8))
    print("func_80068B94_head", disasm_window(exe, 0x80068B94, 8))
    print("func_8001AA78_head", disasm_window(exe, 0x8001AA78, 10))
    print("func_80079244_head", disasm_window(exe, 0x80079244, 6))
    print("RTPT_site", f"0x8003AEAC {word(exe, 0x8003AEAC):08X}")
    print("RTPS_68B94_scan")
    for va in range(0x80068B94, 0x80068CE0, 4):
        w = word(exe, va)
        if (w & 0xFE000000) == 0x4A000000 or w in (0x4A180001, 0x4A280030):
            print(f"  cop2 {va:08X} {w:08X}")

    rooms = {
        "m0002i": {
            "table": 1,
            "geom": M0002I_GEOM,
            "coll": M0002I_COLL,
            "script": M0002I_SCRIPT,
            "yaw": 0x0860,
            "entry_xyz": None,
        },
        "m0003i": {
            "table": 2,
            "geom": M0003I_GEOM,
            "coll": M0003I_COLL,
            "script": M0003I_SCRIPT,
            "yaw": 0x0800,
            "entry_off": 0x0144,
        },
    }

    for name, spec in rooms.items():
        pkg = package_from_table(exe, peimg, spec["table"])
        print(f"=== {name} package {len(pkg)} sha={hashlib.sha256(pkg).hexdigest()} ===")
        view = decode_view(pkg, spec["geom"])
        coll = decode_group_height(pkg, spec["coll"])
        script = pkg[spec["script"] :]
        poses = find_0b_xyz(script)
        print("view", {k: view[k] for k in ("offset", "H", "R", "TR", "viewport", "clamp", "pan", "unk24", "aux_off", "layer_off", "aux_count")})
        print("coll", coll)
        print("0x0B count", len(poses))
        for row in poses[:8]:
            print("  0x0B", row)

        if name == "m0002i":
            authored = next(p for p in poses if p["x"] == -127 << 16 or p["x"] == s32(0xFF810000))
        else:
            authored = next(p for p in poses if p["offset"] == 0x0144)
        authored_y = authored["y"]
        snapped_y = coll["height"] << 16
        wx, wy, wz = authored["x"] >> 16, snapped_y >> 16, authored["z"] >> 16
        print(
            f"authored=({authored['x']>>16},{authored_y>>16},{authored['z']>>16}) "
            f"snapped_y={wy} raw_auth_y={authored_y:#x} raw_snap_y={snapped_y:#x}"
        )
        cx, cy, cz = world_to_camera(view, wx, wy, wz)
        print(f"cam origin ({cx},{cy},{cz})")
        sx_n, sy_n, den, h = project_rational(view["H"], cx, cy, cz)
        print(f"rational sx={sx_n}/{den} sy={sy_n}/{den} H/SZ={h}/{den}")
        print(f"host sx={sx_n/den:.12f} sy={sy_n/den:.12f} scale={h/den:.12f}")
        print(f"trunc sx={sx_n//den} sy={sy_n//den}")
        panx, pany = view["pan"]
        print(f"panned sx={(sx_n/den)+panx:.12f} sy={(sy_n/den)+pany:.12f}")

        if name == "m0003i":
            print("VIS-C representative reproject")
            sys_list = []
            for label, _vid, vx, vy, vz in VISC_M0003I:
                cc = world_to_camera(view, vx, vy, vz)
                sn, yn, dn, _ = project_rational(view["H"], *cc)
                print(
                    f"  {label} world=({vx},{vy},{vz}) cam={cc} "
                    f"sx={sn/dn+panx:.4f} sy={yn/dn+pany:.4f} SZ={cc[2]}"
                )
                sys_list.append(yn / dn + pany)
        else:
            print("VIS-C local+yaw reproject")
            sys_list = []
            for label, lx, ly, lz in VISC_M0002I_LOCAL:
                rx, rz = yaw_xz(lx, lz, spec["yaw"], sincos)
                ww = (wx + rx, wy + ly, wz + rz)
                cc = world_to_camera(view, *ww)
                sn, yn, dn, _ = project_rational(view["H"], *cc)
                print(
                    f"  {label} world={ww} cam={cc} "
                    f"sx={sn/dn+panx:.4f} sy={yn/dn+pany:.4f} SZ={cc[2]}"
                )
                sys_list.append(yn / dn + pany)
        if sys_list:
            print(f"  sample_sy_span={max(sys_list)-min(sys_list):.4f}")
        print(f"  visc_full_span={VISC_SPAN[name]}")

    # Native float-path census (read-only).
    native = Path(
        "/var/home/blizz/dev/parasite-eve-port-black-worktrees/"
        "pe-ue0-native-field-bootstrap/native"
    )
    cam = (native / "src/pe_camera.cpp").read_text()
    rnd = (native / "src/pe_render.cpp").read_text()
    print("=== native float census ===")
    print("project_camera_double", "double* sx" in cam and "double(view.H)" in cam)
    print("nclip_double", "nclip_mac0(double" in cam)
    print("fill_tri_double", "void fill_tri(u8* rgb, double x0" in rnd)
    print("submit_player_double_sxy", "std::vector<double> sx" in rnd)
    return 0


if __name__ == "__main__":
    sys.exit(main())
