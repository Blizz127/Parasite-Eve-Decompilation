# Seven 3-word stubs — uncommitted batch review

> **SUPERSEDED_BY_CLASSIFICATION_AUDIT** — **NOT_MATCHING_C** —
> **DO_NOT_INTEGRATE**. This is a byte-representation diagnostic,
> not matching-C evidence. Retail audit classifies all seven spans as
> alignment padding (classification C), so the `.c` files below must not be
> integrated or counted. The authoritative audit is
> [func-80077-classification/REPORT.md](../func-80077-classification/REPORT.md).

## Leaves and exact bodies

All seven bodies are three retail words, each `00000000 / nop`. Each has no
callee, global reference, or relocation. The final source form is identical
apart from the symbol name:

```c
void func_80077B78(void);

__asm__(
    ".globl func_80077B78\n"
    ".ent func_80077B78\n"
    "func_80077B78:\n"
    "nop\n"
    "nop\n"
    "nop\n"
    ".end func_80077B78\n"
);
```

The same final C shape is present in `func_80077B98.c`, `func_80077BB8.c`,
`func_80077BD8.c`, `func_80077BF8.c`, `func_80077C18.c`, and
`func_80077C38.c`, with the corresponding symbol substituted. Ordinary
returning C cannot produce three zero words because it emits a return jump;
these source forms preserve the original zero-filled stub exactly.

| Function | File span | VA | Retail words | Built words | Boundaries | Evidence |
|---|---|---|---|---|---|---|
| `func_80077B78` | `68378..68383` | `80077B78` | `00000000 00000000 00000000` | same | `68374=a0820007`, `68384=24020006` | `func-80077b78/` |
| `func_80077B98` | `68398..683A3` | `80077B98` | `00000000 00000000 00000000` | same | `68394=a0820007`, `683A4=24020009` | `func-80077b98/` |
| `func_80077BB8` | `683B8..683C3` | `80077BB8` | `00000000 00000000 00000000` | same | `683B4=a0820007`, `683C4=24020008` | `func-80077bb8/` |
| `func_80077BD8` | `683D8..683E3` | `80077BD8` | `00000000 00000000 00000000` | same | `683D4=a0820007`, `683E4=2402000C` | `func-80077bd8/` |
| `func_80077BF8` | `683F8..68403` | `80077BF8` | `00000000 00000000 00000000` | same | `683F4=a0820007`, `68404=24020004` | `func-80077bf8/` |
| `func_80077C18` | `68418..68423` | `80077C18` | `00000000 00000000 00000000` | same | `68414=a0820007`, `68424=24020002` | `func-80077c18/` |
| `func_80077C38` | `68438..68443` | `80077C38` | `00000000 00000000 00000000` | same | `68434=a0820007`, `68444=24020003` | `func-80077c38/` |

## Historical byte gates (representation only)

- Docker build: exact SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
- Each seven C `.text` sections: `0x10` padded object trimmed to `0x0C`,
  with all three retained words exact.
- `scripts/verify_us.sh`: exit 0; split verification OK; exact candidate
  SHA match.
- `git diff --check`: passes.
- The former candidate count was **288** (`281 + 7`), but the seven are not
  matching-C leaves. The current project count is **281**.

## Proposed batch commit scope

Only the seven C sources, their seven per-stub evidence directories, the
contiguous YAML replacements, the corresponding `build_us.sh` object/source/
compile/trim/link wiring, and the verifier's current markers/artifacts. No
`pc_port` changes and no unrelated evidence directories.

This batch remains uncommitted and must not be committed as matching C.
