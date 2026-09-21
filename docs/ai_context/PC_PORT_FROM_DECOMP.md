# PC port from decomp — generated host TUs

_Read `docs/ai_context/PORT_GOAL_AND_PLAN.md` first. This file documents the
mechanism that turns the verified matching C leaves under `src/` into host TUs
under `pc_port/game/decomp/`, and how to regenerate and verify them._

## What this is

The matching leaves in `src/` are written for Psy-Q `cc1` targeting the PS1:
fixed absolute data addresses (`D_800XXXXX`), pointer globals, and bare calls
to retail functions. `pc_port/game/decomp/` holds one **generated** host TU per
leaf: the leaf body is copied verbatim and only the platform differences are
adapted by macros from `pc_port/include/pe_guest_decomp.h`.

The `src/` tree is never modified. The matching leaf remains the authority; if
a generated TU and its leaf disagree, the generator is wrong (or the leaf
moved), never the other way round.

- Generator: `tools/analysis/gen_decomp_ports.py`
- Shim header: `pc_port/include/pe_guest_decomp.h`
- Boundary registry: `pc_port/platform/pe_guest_decomp.c`
- Regression test: `pc_port/tests/test_decomp_ports_manifest.c`
- Orphan allowlist: `tools/analysis/decomp_port_orphans.txt`

## Two templates

The generator picks a template from the leaf signature.

### 1. Leaf template (no pointer parameters)

The whole `src/<name>.c` file is reproduced, with `extern` data declarations
and callee prototypes replaced by marker comments, after a generated header:

```c
#include "pe_guest_decomp.h"

#define D_8009D28C PE_DECOMP_SCALAR(0x8009D28Cu, int)

/* ── verbatim matching leaf (src/func_80017FDC.c) ─────────────── */

/* shimmed by pe_guest_decomp.h: D_8009D28C */

int func_80017FDC(void) {
    D_8009D28C = 5;
    return 1;
}
```

### 2. Pointer-parameter template

When a parameter is a pointer, the host signature takes a `pe_addr_t` guest
address and the body runs against translated host pointers:

```c
void func_80083C20(pe_addr_t pe_a0)
{
    unsigned char *host_a0 = (unsigned char *)PE_Translate(pe_a0, 1);
    /* verbatim body (src/func_80083C20.c) */

    unsigned int value = *(unsigned int *)(host_a0 + 0x20);
    host_a0[0x36] = 0x4D;
    ...
}
```

A pointer parameter used as a *bare call argument* is forwarded as its guest
address (`pe_name`); any other use gets a translated `host_name` local. Struct
typedefs are hoisted verbatim under `/* verbatim leaf types */`.

## Symbol expanders

`pe_guest_decomp.h` provides three checked guest-RAM expanders:

| declaration in `src/` | generated define |
|---|---|
| `extern T D_XXXXXXXX;` | `#define D_XXXXXXXX PE_DECOMP_SCALAR(0xXXXXXXXXu, T)` |
| `extern T D_XXXXXXXX[];` | `#define D_XXXXXXXX PE_DECOMP_ARRAY(0xXXXXXXXXu, T)` |
| `extern T *D_XXXXXXXX;` | `#define D_XXXXXXXX PE_DECOMP_PTRGLOBAL(0xXXXXXXXXu, T)` |

- `PE_DECOMP_SCALAR` is a volatile lvalue in the one 2 MiB guest-RAM
  authority (so repeated reads are not CSE'd away, matching retail reloads).
- `PE_DECOMP_ARRAY` is the translated base of an array/struct with unknown
  extent; tests pin the real extents.
- `PE_DECOMP_PTRGLOBAL` loads the pointer *value* from the guest slot and
  translates it; a null/out-of-range value aborts loudly.

`psx_compat.h` already macro-defines some of these slots. When a leaf declares
such a symbol the generator emits `#undef D_XXXXXXXX` immediately before the
generated `#define` to avoid a redefinition error.

A data symbol passed where the callee's parameter is a guest address is
forwarded as `(pe_addr_t)0xXXXXXXXXu`, not as the shim lvalue. This applies to
a symbol that opens an argument list (a textual rule that also rewrites the
leaf's ROM comments, reproduced intentionally) and to a symbol at a pointer
position of the callee's prototype.

## Loud-boundary contract

Any callee a leaf calls that pc_port has **not** implemented becomes a loud
boundary instead of a silent zero:

```c
/* boundary: func_8004E704 has no pc_port implementation yet */
#define func_8004E704(...) PE_D_COMP_BOUNDARY1("func_8004E704", 0x8004E704u, __VA_ARGS__)
```

`PE_D_COMP_BOUNDARY0..4` select the guest argument vector actually passed and
call `PE_Decomp_Boundary()`, which records the symbol in a registry
(`PE_Decomp_BoundaryCount()` / `PE_Decomp_BoundaryName()`) and forwards the
arguments to the established `Bootstrap_ReturnInt4Indirect` log. A boundary is
an explicit "this subsystem is not ported yet" marker; nothing is invented.

A callee is "implemented" when a real function definition for it exists
somewhere under `pc_port/` (a `#define ... PE_D_COMP_BOUNDARY` does not count).
Prototype handling follows from that:

- **boundary** callee → marker comment **and** a boundary define;
- **implemented** callee already declared by a pc_port runtime header
  (`pc_port/**/*.h`, `tests/` excluded) → marker comment only, so the host
  declaration is used and the leaf's register-typed prototype does not
  conflict with it;
- **implemented** callee with no host declaration → the prototype is kept
  verbatim.

## Metadata

The generated header's `matched VMA … file … span … words` line comes from
`configs/USA/disc1.yaml`, whose subsegment list is authoritative:

- `file` is the subsegment offset;
- `span` is the distance to the **next subsegment of any kind** (`c`, `asm`,
  `rodata`, `data`, `bss`, …) — a leaf is often followed by `rodata` or `asm`,
  not another `c`;
- `VMA = 0x80010000 + file - 0x800`, checked against the function name.

## Which leaves are ported

The set of generated TUs is the set of existing
`pc_port/game/decomp/*_port.c` files. Which leaves to port is a project
decision, not a property of the leaf, so the generator reproduces that set; it
does not invent a new one. `--only NAME` regenerates a single TU.

## Regeneration

From the repository root:

```sh
# regenerate every src-backed TU in place
python3 tools/analysis/gen_decomp_ports.py

# verify, writing nothing (fails on any mismatch or unlisted orphan)
python3 tools/analysis/gen_decomp_ports.py --check --allow-orphans

# prove the files are byte-identical to their committed state
python3 tools/analysis/gen_decomp_ports.py && git diff --exit-code -- pc_port/game/decomp

# refresh the orphan allowlist after deliberately adding/removing an orphan
python3 tools/analysis/gen_decomp_ports.py --update-orphans
```

`--check --allow-orphans` returns non-zero if any src-backed TU would change,
if a new TU has no `src/` leaf, or if an allowlisted orphan has gained a leaf
(stale allowlist). The regression test `pe-decomp-port-tests` runs the same
check together with a self-contained allowlist comparison:

```sh
cmake --build pc_port/build --target pe-decomp-port-tests
./pc_port/build/pe-decomp-port-tests
# or, from the build directory:
ctest -R decomp-port-reproducibility --output-on-failure
```

## Orphans

A TU with no matching `src/<name>.c` cannot be regenerated from authority.
There are 82 such TUs; every one is listed in
`tools/analysis/decomp_port_orphans.txt` and sits inside an `asm` subsegment of
`configs/USA/disc1.yaml`, i.e. it is real retail code whose matching C leaf is
not in this checkout. See
`docs/evidence/decomp-port-orphans/REPORT.md` for the per-name cross-reference
and the keep/remove recommendation. They are **kept**; the allowlist is the
machine-checkable "no in-tree authority" marker.
