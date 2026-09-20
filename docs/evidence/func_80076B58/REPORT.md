# func_80076B58 — matching C leaf (708)

- VRAM 0x80076B58, file 0x67358, size 0x40 (16 words), unit `67358.s`.
- Authority: retail Disc1 EXE SHA-1
  `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.

## Behaviour

Store `0x04000000` to `*D_80095854`, then copy `a1` words from `a0` to
`*D_80095850` in a do/while whose countdown is in `a2`; return 0. The loop's
back-branch delay slot is the store `sw $v1,0($v0)`, and the `a1 == 0` guard
skips the loop while still doing the first store.

## Source

```c
extern int *D_80095854;
extern int *D_80095850;
int func_80076B58(int *a0, int a1) {
    int a2 = a1 - 1;
    *D_80095854 = 0x04000000;
    if (a1 != 0) {
        do {
            int v = *a0;
            a0++;
            *D_80095850 = v;
            a2--;
        } while (a2 != -1);
    }
    return 0;
}
```

## Verification

```text
$ python3 tools/analysis/try_leaf.py src/func_80076B58.c 0x67358 0x40
retail      64 bytes
candidate   64 bytes
WORDS MATCH (confirm with scripts/build_us.sh)

$ distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp9 && bash scripts/build_us.sh'
  RESULT: EXACT MATCH
  Matching claim: YES (708 registered C leaves)

$ distrobox enter pe-mipsel -- bash -lc 'cd /tmp/pe-agent-decomp9 && bash scripts/verify_us.sh'
  PASS all 708 packed C spans equal retail
  VERIFY_US=PASS
```

## Triage notes (other candidates this run)

`func_800C811C`, `func_8007E160`, `func_8008F430` were attempted and **not**
matched: the candidates are the right size but differ in instruction
scheduling / delay-slot fill (e.g. `func_8007E160`'s early-return delay slot is
`addu v0,zero,zero` in retail vs `nop` in every formulation tried;
`func_800C811C` schedules the `D_8009D254` pointer load before the first store
and uses `lh` for the `+0x2E` field). Drafts were removed, not committed.
