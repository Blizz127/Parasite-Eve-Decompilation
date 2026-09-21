# Porting matched decompiled C into `pc_port/`

This document is the contract for **deriving** the native `pc_port/` implementation
from the verified matching decompilation in `src/`. It is the "how a new matched
leaf gets ported" recipe referenced by the generated TUs' provenance banners and
by `tools/analysis/gen_decomp_ports.py`.

Read `CLAUDE.md` first. The hard rules there still hold: never invent decompiled C,
every claim needs evidence, `src/` is the era-exactness authority and is **never
written to** by this mechanism.

---

## 1. The mechanism: generated derived TUs

Three candidate mechanisms were considered:

| Option | Verdict | Why |
| --- | --- | --- |
| **Shared source** — compile `src/*.c` natively with a forced-include shim | rejected | `src/` carries era-only artifacts (`register X asm("$6")` pins, `asm volatile` fences, Psy-Q prototypes). A macro layer cannot redefine `asm` pins or `register` keywords portably, and the shim would have to be kept in exact lockstep with 700+ leaves; any drift silently changes host codegen with no link-time signal. |
| **Derived port TUs** — generate `pc_port/game/decomp/func_XXXX_port.c` from the leaf | **chosen** | `src/` stays untouched and authoritative; the host adaptation is explicit, reviewable, and regenerable; it matches the existing `*_port.c` convention already used throughout `pc_port/`; and provenance is stamped into every file. |
| **Hybrid** — mechanical for pure-logic, hand for hardware | chosen *inside* the derived mechanism | The generator is conservative: pure-logic leaves are derived verbatim; hardware/pointer/asm-touching leaves are *reported*, not guessed, and need a hand-written adapter. |

`tools/analysis/gen_decomp_ports.py` is the mechanism. It:

1. reads the matched `c` span list from `configs/USA/disc1.yaml` (read-only — never edited);
2. reads each `src/func_XXXXXXXX.c`;
3. applies an explicit **eligibility contract** (below);
4. copies the leaf's body **verbatim** into `pc_port/game/decomp/func_XXXXXXXX_port.c`,
   wrapping it in only a provenance banner, a `#include "pe_guest_decomp.h"`,
   generated data-symbol macros, and generated loud-boundary shims;
5. never modifies `src/`, the manifest, or any sibling-owned file.

Run it:

```bash
python3 tools/analysis/gen_decomp_ports.py            # (re)generate
python3 tools/analysis/gen_decomp_ports.py --check    # totals + skip histogram
python3 tools/analysis/gen_decomp_ports.py --check --verbose  # every skip + reason
python3 tools/analysis/gen_decomp_ports.py --verify   # exit 1 if a TU drifted
python3 tools/analysis/gen_decomp_ports.py --only func_80123456   # one leaf
```

It is **idempotent**: re-running writes only files whose content changed, so
`--verify` is a clean drift gate after the matching decomp advances. The build
globs `pc_port/game/decomp/*_port.c` (`CMakeLists.txt`, `file(GLOB CONFIGURE_DEPENDS)`),
so a newly generated TU joins the library with no build-file edit.

---

## 2. Host adaptation: what the shim does

`pc_port/include/pe_guest_decomp.h` is the only header a generated TU needs. Every
platform difference the leaf relies on is absorbed by one of these macros:

### Data symbols (`D_800XXXXX`)

A matched leaf addresses fixed absolute data. The generated TU declares one macro
per declared symbol, chosen by the declaration's shape:

| Leaf declaration | Generated macro | Host semantics |
| --- | --- | --- |
| `extern int D_8009CF0C;` | `PE_DECOMP_SCALAR(0x8009CF0Cu, int)` | a `volatile` lvalue in the one 2 MiB guest-RAM authority (`PE_Translate`), so repeated reads are not CSE'd away exactly as retail reloads them |
| `extern short D_8009589C[];` | `PE_DECOMP_ARRAY(0x8009589Cu, short)` | a base pointer into guest RAM; the leaf indexes it |
| `extern unsigned char *D_8009D048;` | `PE_DECOMP_PTRGLOBAL(0x8009D048u, unsigned char)` | loads the 32-bit guest address stored in the slot and translates it — retail's `lui`/`lw` base |

The macro name is the **data symbol**, so the leaf's body text is byte-for-byte
unchanged. `PE_DECOMP_SCALAR` includes `sizeof(type)` in the bounds check;
`PE_DECOMP_ARRAY` probes one byte because an `extern T name[];` has no known
extent (tests pin the retail extents). A pointer global holding a null or
out-of-range value aborts loudly in `PE_Translate` — the intended
"unported subsystem" signal, never a silent zero.

If the pc_port shim headers already declare a macro at that address with a
different host type (`#define D_8009D048 PE_GUEST_U32(...)`), the generator emits
`#undef D_8009D048` first so the **matched leaf's own declaration wins**.

### Callees pc_port has not implemented: loud boundaries

A derived leaf may call a retail function pc_port has not ported yet. That callee
cannot be a real `extern` (there is no host prototype) and must not be quietly
zeroed. The generator emits, per unresolved callee, a macro that redirects the
call site into the argument-forwarding boundary shim:

```c
/* boundary: func_8004E704 has no pc_port implementation yet */
#define func_8004E704(...) \
    PE_D_COMP_BOUNDARY1("func_8004E704", 0x8004E704u, __VA_ARGS__)
```

`PE_Decomp_Boundary()` records the symbol in the boundary registry **and** pushes
the guest argument vector (arity 0–4) into the established
`Bootstrap_ReturnInt4Indirect` log, so a test can assert both *which* edge was
unresolved and *what arguments the matched leaf actually passed*. Existing
hand-written ports use that same boundary log; derived ports join it rather than
inventing a parallel one. The generated arity comes from the balanced-paren
argument text of the call site, so a short call yields zero, never a syntax error.

Crucially, a boundary callee keeps the leaf's own declaration **verbatim**
(removing it would let the generated boundary macro rewrite the prototype into
nonsense). This is asserted by the generated-TU build plus
`test_DECOMP_boundary_argument_vector`.

### Declarations the generator rewrites

`neutralize_declarations()` replaces, with a `/* shimmed by pe_guest_decomp.h: X */`
comment:

- `extern` declarations containing a `D_` data symbol (the generated macro owns it);
- prototypes of callees that are loud boundaries (the boundary macro owns them);
- prototypes of callees with a **canonical `pe_port_compat.h`/`pe_sdk.h` prototype**,
  where the leaf's redeclaration could clash on signedness or pointer-ness with the
  canonical host type. The matched leaf stays authoritative for the **call site**;
  the host prototype comes from the canonical header.

Everything else — `typedef`s, `struct`s, prototypes of other decomp-derived
leaves — stays byte-for-byte.

### Constructs deliberately *not* auto-ported

There is no host model for these, so the generator reports the leaf instead of
guessing (the eligibility contract, §3):

- memory-mapped hardware and BIOS space (`0x1F8xxxxx`, `0xA0xxxxxx`, `0xB0xxxxxx`);
- GTE/COP2;
- BIOS trampolines (`jr $t2`) and `syscall`;
- `register X asm("$N")` pins and `asm volatile` fences;
- guest pointers held in host pointers where the pointer is **not** a direct
  address-valued parameter (pointer returns, callee results dereferenced,
  pointer casts of non-`D_` operands, function-address values, pointer-to-pointer
  parameters, and a pointer parameter *stored into guest RAM* — see E7b).

Everything else with a plain address-valued pointer parameter is adapted by the
**pointer-parameter host adapter** (E7b below), so those leaves are now derived
rather than reported.

The listed categories still need a **hand-written adapter** in `pc_port/` (as they
always have), and the coverage tool keeps counting them as `absent` /
`hand-translated` until that adapter lands.

---

## 3. Eligibility contract (what the generator will derive)

A leaf is derived only if **all** of these hold. Everything else is skipped with
a printed reason (`--check --verbose`) — never silently mis-ported.

| # | Rule |
| --- | --- |
| E1 | It is a matched `c` span in `configs/USA/disc1.yaml`. |
| E2 | No `asm`/`__asm__` (register pins, scheduler fences). |
| E3 | No hardware/BIOS-space 32-bit literal (`0x1F8xxxxx`/`0xA0xxxxxx`/`0xB0xxxxxx`). Ordinary masks stay verbatim. |
| E4 | The file defines exactly one `func_*` with a body, and that name is the leaf. (A bare prototype is not a definition — forward declarations of callees must not disqualify a leaf.) |
| E5 | No indirect call through a local pointer. |
| E6 | No `&func_*` and no bare `func_*` used as a **value** — 32-bit code pointers are not host pointers. |
| E7 | No pointer return type. |
| E7b | A pointer-typed parameter is adapted by the guest-address host adapter **only** if it is address-valued: no pointer-to-pointer parameter, no `&param`, no *storing* the parameter into guest RAM (`dst->field = p` would write a 64-bit host pointer into a 32-bit guest slot), no `(T *)p` store-through-dereference cast, no recursive self-call, and no non-primitive pointee type. Every call site then forwards the guest address (`pe_p`), while memory access goes through the translated `host_p` pointer. |
| E8 | No callee result indexed/dereferenced (would be a 32-bit guest pointer). |
| E9 | No pointer cast whose operand does not derive from a `D_` data symbol or an `extern D_` local. |
| E10 | No pointer-pointer store into a `D_` symbol. |
| E11 | Every called `func_*` either has a known pc_port definition (linkable) or becomes a loud boundary; an existing definition with a **canonical prototype must accept the same arity** as the matched call site (otherwise the host would reinterpret arguments → reported). |
| E12 | No assignment to a pointer-global slot (the translate-and-dereference macro cannot represent storing a 32-bit guest address). |

### One deterministic rewrite: guest addresses as arguments

When a bare `D_XXXXXXXX` appears as a call argument, the generated macro would
expand to a **host** pointer, but the host callee expects `pe_addr_t`. The
generator rewrites only that position:

```c
func_800C2414(arg0, D_800E0824);   /*  = func_800C2414(arg0, (pe_addr_t)0x800E0824u);  */
```

The argument *semantics* (the retail guest address) are preserved exactly; the
rewrite is positional and mechanical, and it is the only textual change the
generator makes inside a leaf body.

### One generated host adapter: address-valued pointer parameters (E7b)

Roughly a third of the remaining ineligible leaves take a plain address-valued
pointer parameter — the shape that dominated the first eligibility pass. Those
are now derived with a **host adapter** instead of being reported, because the
guest address *is* the whole interface and the body can stay verbatim.

The retail leaf:

```c
typedef struct { short *first; short *second; unsigned short *third; } Arguments;
int func_80017C8C(Arguments *arg0) {
    func_800661EC(*arg0->first, *arg0->second, *arg0->third, 8);
    return 1;
}
```

becomes (provenance banner elided):

```c
int func_80017C8C(pe_addr_t pe_arg0)
{
    Arguments *host_arg0 = (Arguments *)PE_Translate(pe_arg0, 1);
    /* verbatim body (src/func_80017C8C.c) */
    func_800661EC(*host_arg0->first, *host_arg0->second, *host_arg0->third, 8);
    return 1;
}
```

The rules that keep it honest:

- **Only the convention adapts.** The signature takes `pe_addr_t`, a `host_<p>`
  local is `PE_Translate`d from it, and the body text is otherwise the matched C.
  Control flow, arithmetic, bit constants and signedness are untouched.
- **Memory access** goes through `host_<p>`; a call site hands the callee the
  *guest address* (`pe_<p>`), because a canonical pc_port prototype or a boundary
  records a guest argument, not a host pointer.
- **`D_` symbols and struct access** are textually rebound to their `host_`
  accessors, with a `(?<![.>])` guard so a struct *member* of the same name is
  never mistaken for the parameter.
- **The adapter refuses shapes where the guest/host distinction leaks.** A
  pointer-to-pointer parameter, `&param`, storing the parameter into guest RAM,
  a store through a `(T *)param` cast, a recursive self-call, or a
  non-primitive pointee type all fall back to "report, do not guess".
- Verified by `pc_port/tests/test_decomp_ptr_params.h`, which pins both the
  memory effect of the dereferenced body and the *guest address* forwarded to a
  boundary (`test_DECOMPPTR_boundary_guest_addresses`).

---

## 4. Per-leaf recipe (adding a port)

1. **Confirm the leaf is matched.** `python3 tools/build/disc1_plan.py --check`
   must list `[<offset>, c, func_XXXXXXXX]` in `configs/USA/disc1.yaml`, and
   `docs/evidence/func-XXXXXXXX/REPORT.md` must exist.
2. **Regenerate.** `python3 tools/analysis/gen_decomp_ports.py`. If the leaf is
   eligible you get `pc_port/game/decomp/func_XXXXXXXX_port.c`; if not,
   `--check --verbose` prints the reason and you write a hand adapter instead.
3. **Build.** `cmake --build pc_port/build -j"$(nproc)"`. A compile error means
   an eligibility gap: fix the *generator rule*, not the generated file (generated
   files are overwritten on the next run and must never be hand-edited).
4. **Test.** Add or extend a case in `pc_port/tests/test_decomp_ports.h` (plain
   leaves) or `pc_port/tests/test_decomp_ptr_params.h` (E7b adapter leaves),
   pinning the retail behavior — especially quirks: exact bit constants
   (`0x20000000` vs `0x80000000`), signedness/narrowing, write/no-write guards,
   search-and-break order, double dereference, mirrored indexing. Call
   `ResetTestState()` first, seed guest RAM with `PE_StoreU32`/`PE_StoreU8`/
   `PE_StoreU16`, and read back with `PE_LoadU32`/`PE_LoadU8`/`PE_LoadU16`.
5. **Verify + document.**
   `python3 tools/analysis/gen_decomp_ports.py --verify` must report `0 drifted`.
   Refresh the coverage numbers with
   `python3 tools/analysis/pc_port_coverage.py`.
6. **Never edit `src/`.** If the leaf looks wrong, that is a decomp issue owned by
   the matching lane; report it, do not patch `src/`.

### Disagreements with an existing hand port

Where a pre-existing hand-written `*_port.c` and the matched C disagree, **the
matched C wins**, and the discrepancy is recorded here and in the test that pins
the matched behavior. The generator never *replaces* an existing definition on
its own — a leaf whose `func_*` is already defined elsewhere is reported as a
**conflict** and skipped, so overriding a hand port is a deliberate, reviewed act.

---

## 5. Coverage / gap tool

`tools/analysis/pc_port_coverage.py` maps every matched `c` span to its pc_port
counterpart and classifies it into exactly one bucket:

- **`ported-from-decomp`** — a `pc_port/game/decomp/*_port.c` with a
  `decomp-source:` provenance banner naming the matched leaf (structurally detected);
- **`hand-translated`** — a real pc_port definition written independently of `src/`;
- **`stub`** — only a bootstrap provider/stub (`BOOTSTRAP_RET`, `Bootstrap_Return*`);
- **`absent`** — no pc_port definition at all.

```bash
python3 tools/analysis/pc_port_coverage.py            # totals + one summary line
python3 tools/analysis/pc_port_coverage.py --json     # machine-readable
python3 tools/analysis/pc_port_coverage.py --list-bucket ported-from-decomp
python3 tools/analysis/pc_port_coverage.py --deps func_80030640   # dependency surface
```

Output is stable and diffable (sorted sections, deterministic one-line summary).
It re-reads the YAML and `src/` on every run, so it stays correct as matching
advances. Exit status is 0 for a successful report; low coverage is not an error.

---

## 6. Honest limits

- **Not every matched leaf is auto-portable.** The eligibility contract is
  conservative on purpose. Pointer-heavy, asm-bearing, and hardware-touching
  leaves need hand adapters; the generator reports them rather than guessing.
- **A derived port proves behavior, not cycle-exactness.** It reproduces the
  matched **C semantics**; it is not a matching leaf and does not add to the
  matched-C count (`pc_port/` is explicitly not a matching leaf per `CLAUDE.md`).
- **A boundary is a loud stub, not a port.** Where a derived leaf calls an
  unported callee, the call is recorded and returns 0. The edge is enumerable and
  testable, but the subsystem is still not implemented — the coverage tool keeps
  counting the callee as `absent` until its own adapter lands.
- **`--verify` is the drift gate.** If the matching lane edits a `src/` leaf, the
  generated TU is stale until the generator re-runs; `--verify` exits 1 and lists
  the stale files.
