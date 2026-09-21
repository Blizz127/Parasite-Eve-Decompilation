# TOOLCHAIN_REBUILD — rootless mipsel toolchain and the exact packed SHA-1

Status (2026-09-11): **the exact packed rebuild runs in this host environment
without docker, and produces the retail SHA-1.**

```
orig SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
cand SHA-1: 452fb033f2eaa4b18aa20a5bca60b8125af3a37b
RESULT: EXACT MATCH
...
VERIFY_US=PASS
matching-C count: 570 (from YAML)
```

The exact run was made against a **frozen snapshot** of the working tree
(`configs` + `src` copied to a scratch dir) because sibling workers were
editing `configs/USA/disc1.yaml` and `src/` during every split. The snapshot
needed three leaf corrections that are **sibling-owned matching edits** (no
`src/`/`disc1.yaml` file was changed in this repo) — see "Leaf findings".
Apart from those three leaves, all 570 C spans and 288 asm spans round-trip
byte-exact through this toolchain.

This document owns the build-environment facts. Matching progress and the
handoff remain in `docs/ai_context/ACTIVE_HANDOFF.md`.

## Environment / why rootless

- Ubuntu 26.04 (resolute), `uid=1000`, `sudo` requires interactive auth.
- No `docker`, `podman`, or `distrobox` → the documented `pe-mipsel-img`
  container cannot be entered.
- Ubuntu 26.04 apt has **no** `gcc-mipsel-linux-gnu` (`Candidate: (none)`);
  only `binutils-mipsel-linux-gnu` exists. So a native distro install is not
  possible non-interactively.
- `dev/mipsel/Dockerfile` pins `debian:trixie` + `gcc-mipsel-linux-gnu`
  (= **GCC 14.2.0-13cross1**, binutils 2.44), which is the compiler the
  `modern` build profile was matched against.

The fix is to unpack the **same Debian trixie packages** the Dockerfile
installs into a git-ignored tree-local prefix, with no root and no container.

## Install (idempotent, pinned)

```
scripts/setup_mipsel_host.sh
```

- Location: `tools/mipsel-host/` (now git-ignored via `.gitignore`
  `tools/mipsel-host/`). Debs cached in `tools/mipsel-host/.debs/`.
- Entry points: `tools/mipsel-host/bin/mipsel-linux-gnu-{gcc,as,ld,objcopy,readelf}`
  (shims that set `LD_LIBRARY_PATH` and re-export `PATH`, then exec the real
  binary under `usr/bin/`). GCC 14 finds its own `cc1` because Debian's
  driver is relocatable relative to `<prefix>/usr/bin`.
- Installed versions (verified):

```
mipsel-linux-gnu-gcc-14 (Debian 14.2.0-13) 14.2.0
GNU assembler (GNU Binutils for Debian) 2.44
GNU ld (GNU Binutils for Debian) 2.44
```

Packages, URLs, and SHA-256 (Debian trixie, host `amd64`; base
`http://deb.debian.org/debian/`):

| package | pool path | SHA-256 |
|---|---|---|
| `gcc-14-mipsel-linux-gnu` | `pool/main/g/gcc-14-cross-mipsen/gcc-14-mipsel-linux-gnu_14.2.0-13cross1_amd64.deb` | `04a58165ab8f9a1c692d5244b5608251e9b18649e8832a8226cd103de2fa878a` |
| `gcc-14-mipsel-linux-gnu-base` | `pool/main/g/gcc-14-cross-mipsen/gcc-14-mipsel-linux-gnu-base_14.2.0-13cross1_amd64.deb` | `3983042af2d6b7b365a588b394d538559a59a3d0d2cc2a0efac2bb3a23c99501` |
| `gcc-14-cross-base-mipsen` | `pool/main/g/gcc-14-cross-mipsen/gcc-14-cross-base-mipsen_14.2.0-13cross1_all.deb` | `5823a47dba0c833ce0381cd8e5cf21c9120df92a54a603685dedad8dbf6ea74a` |
| `cpp-14-mipsel-linux-gnu` | `pool/main/g/gcc-14-cross-mipsen/cpp-14-mipsel-linux-gnu_14.2.0-13cross1_amd64.deb` | `40fec73bd218c5766da2981103b8e8f66f7bc60de748d0673f1f10e657d8be43` |
| `libgcc-14-dev-mipsel-cross` | `pool/main/g/gcc-14-cross-mipsen/libgcc-14-dev-mipsel-cross_14.2.0-13cross1_all.deb` | `4b5fa7f019ebf939eeeeead8551b99cf2f777479b84af1d920193849fe969174` |
| `binutils-mipsel-linux-gnu` | `pool/main/b/binutils-mipsen/binutils-mipsel-linux-gnu_2.44-3cross1+nmu1+b1_amd64.deb` | `2514dd910f344c923531f959ea4ffd542e51640aac0caa1bc51ee293851788ab` |
| `binutils-common` | `pool/main/b/binutils/binutils-common_2.44-3_amd64.deb` | `002da5d23f8757dee97a2c0a40e0e1d4d85a43da094488ee2ee7068d4d3691f9` |
| `libsframe1` | `pool/main/b/binutils/libsframe1_2.44-3_amd64.deb` | `38f625dfdc582717029ac3a3e97c51d994ec2e7a0e9b230c6b44e40d1276311f` |
| `libmpc3` | `pool/main/m/mpclib3/libmpc3_1.3.1-1+b3_amd64.deb` | `2af0a5c128e03694a41c0b011bd8a958b7297436cdb3a15ddad7866dae8c300b` |
| `libisl23` | `pool/main/i/isl/libisl23_0.27-1_amd64.deb` | `ac8518042e81c00de1effb72bba7e88ac4ecd488f7ea8b9e3ebc63159cb53b35` |

No libc headers / target libc are needed: every `src/*.c` is freestanding
(zero `#include <...>`), the build compiles with `-c`, and links with
`-nostdlib`. `libmpc3` / `libisl23` are the only host runtime libraries Ubuntu
26.04 lacks for `cc1`; the shims provide them.

## Container-vs-native decision path

`tools/build/disc1_build.py::find_toolchain()` now resolves, in order:

1. the five `mipsel-linux-gnu-*` tools already on `PATH` (`note="host PATH"`);
2. the repo-local shim dir `tools/mipsel-host/bin` (prepended to `PATH`,
   `note="repo-local tools/mipsel-host/bin"`);
3. `distrobox enter pe-mipsel` if distrobox is installed;
4. else a hard error that now points at `scripts/setup_mipsel_host.sh`.

So the documented docker flow is unchanged (step 1 wins inside the image), and
the native path is usable on a host with no container runtime. The error text
was extended accordingly.

## Exact run (commands)

In the frozen snapshot (or in the repo once the three leaf fixes below land):

```
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
scripts/split_us.sh              # splat; ~52 s
scripts/build_us.sh              # ~125 s; ends RESULT: EXACT MATCH
scripts/verify_us.sh             # 7/7 gates; ends VERIFY_US=PASS
```

Snapshot used for the recorded proof:

- `configs/USA/disc1.yaml` SHA-256 `62bbb0c86390d57a5cd3af119e7e569092b7d1fab3c5eac547413164c4692f5f`,
  570 C spans / 288 asm / 2 rodata.
- plan SHA-256 `b3f26413a65ddc886afdfda54fd2e464ce19a51e3303a05f99427cb97cf785a2`.

`scripts/split_us.sh` can still exit 1 in the live repo with
`ERROR: the split created files git does not ignore: src/func_*.c` — this is a
**race with concurrent sibling edits** (a `src/func_*.c` appearing between the
script's before/after `git status` snapshots), not a real ignore violation;
splat itself completed and the artifacts are valid.

## Leaf findings (sibling-owned; NOT edited in this repo)

Running the full link for the first time exposed three integrated C leaves
whose object-level match does not survive relocation/scheduling resolution.
All three were fixed in the scratch snapshot only.

1. `func_80086FF8` / `func_80087024` (`src/func_80086FF8.c`,
   `src/func_80087024.c`). Source declares `extern int D_800CCD80;`, which the
   build resolves from the symbol name to `0x800CCD80`. Retail's address is
   `0x800BCD80`: `lui at,0x800C` + `sw v0,-0x3280(at)` (`0x800C0000 - 0x3280`).
   Fix: name the global by its real address, `extern int D_800BCD80;`, in both
   files (the repo convention is address-named symbols and the linker's
   name-derived fallback then yields the right value). Rename-only was
   verified to rebuild exact.
2. `func_8006F2C4` (`src/func_8006F2C4.c`). The `unsigned int *flagsPtr =
   &D_800B0CD8;` is declared **before** the `D_800E10A0` clear loop, so cc1
   hoists the address materialization and allocates it to a caller-saved
   register; retail materializes `&D_800B0CD8` in `$v1` **after** the loop.
   Fix: declare `unsigned int *flagsPtr;` (or nothing) before the loop and
   assign `flagsPtr = &D_800B0CD8;` after it — cc1 then emits the retail
   `li $4,-131072` / `la $3,D_800B0CD8` / `lw $2,0($3)` / `ori $4,$4,0xffff` /
   `and` / `sw` sequence.

With those three files corrected (and nothing else), `build_us.sh` and
`verify_us.sh` both succeed and the packed SHA-1 equals retail.

## Reproduce from scratch

```
scripts/setup_era.sh            # era gcc-2.7.2-psx + vendored maspsx (unchanged)
scripts/setup_mipsel_host.sh    # modern GCC 14.2.0 + binutils 2.44 (this doc)
export PATH="$PWD/tools/mipsel-host/bin:$PATH"
scripts/split_us.sh && scripts/build_us.sh && scripts/verify_us.sh
```
