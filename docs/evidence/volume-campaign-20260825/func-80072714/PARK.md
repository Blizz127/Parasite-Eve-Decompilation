# func_80072714 — refreshed volume screen — handwritten syscall

Outcome: SKIP-SDK-LIBRARY-SYSCALL. No matching-C attempt or leaf claim.
Matching count remains 287.

## Function hood

- Span: file [0x62F14,0x62F24), VRAM [0x80072714,0x80072724), four words.
- Retail body: addiu a0,zero,1; syscall 0; jr ra; nop.
- The body ends in jr ra plus its delay-slot nop.
- Previous boundary word at 0x80072710 is the real nop delay slot ending func_80072704.
- Following boundary word at 0x80072724 is the real first instruction of adjacent func_80072724.
- Twenty-six unique direct jal callsites target the exact start:

- file 0x56D8 / VA 80014ED8
- file 0x5748 / VA 80014F48
- file 0x311DC / VA 800409DC
- file 0x5BF84 / VA 8006B784
- file 0x5F808 / VA 8006F008
- file 0x5F940 / VA 8006F140
- file 0x5F9F0 / VA 8006F1F0
- file 0x5FC58 / VA 8006F458
- file 0x64A3C / VA 8007423C
- file 0x6A93C / VA 8007A13C
- file 0x6AAAC / VA 8007A2AC
- file 0x6D978 / VA 8007D178
- file 0x6E5EC / VA 8007DDEC
- file 0x6E648 / VA 8007DE48
- file 0x6E6EC / VA 8007DEEC
- file 0x6E77C / VA 8007DF7C
- file 0x6E850 / VA 8007E050
- file 0x6E8C8 / VA 8007E0C8
- file 0x6EA34 / VA 8007E234
- file 0x6EAAC / VA 8007E2AC
- file 0x6EBE4 / VA 8007E3E4
- file 0x6EC78 / VA 8007E478
- file 0x6ED1C / VA 8007E51C
- file 0x73500 / VA 80082D00
- file 0x735C4 / VA 80082DC4
- file 0x761A0 / VA 800859A0

FUNCTION_HOOD=PROVEN. This is callable code, not padding or a mislabeled span.

## Retail words and static screens

asm/disc1/5F3E4.s records the exact retail bytes:

    80072714: 24040001  addiu  a0,zero,1
    80072718: 0000000C  syscall 0       # handwritten instruction
    8007271C: 03E00008  jr     ra
    80072720: 00000000  nop

| Screen | Result |
|---|---|
| Frame decomposition | no stack frame; no saves; no calls in the body |
| Callee buckets | none |
| Stage-0 written globals | none visible; the architectural syscall is the semantic sink |
| Coloring pressure | none |
| $v0 liveness | none in the visible body |
| Address retention | none |
| -O signal | none; no compiler-controlled loop or repeated materialization |
| Function family | adjacent func_80072724 has the same syscall wrapper with immediate 2 |

## C expressibility decision

The only ordinary-C boundary candidate is an empty void function, but it would emit jr ra; nop and cannot emit the required syscall 0 or the preceding addiu a0,zero,1. There is no sanctioned C intrinsic for this MIPS architectural operation in the repository/toolchain. R7 forbids file-scope assembly, inline assembly, padding nops, and mismatch-hiding macros; therefore no C phrasing or compiler-flag iteration is authorized.

The adjacent func_80072724 is the same proven handwritten syscall shape with immediate argument 2, so it is screened into the same skip family rather than burning a second attempt.

PARKED_CLASS=SKIP-SDK-LIBRARY-SYSCALL

## Disposition

No YAML, build, verifier, or source integration is made. The diagnostic empty C boundary is preserved in the labeled stash: park volume func_80072714 handwritten syscall unexpressible in C. The durable pool refresh removes both syscall wrappers from TIER 1 and adds them to SKIP; the next eligible Tier 1 candidate remains queued.
