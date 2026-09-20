# func_800C22F8

- **VRAM**: 0x800C22F8
- **File offset**: 0xB2AF8 (size 0x11C)
- **Build profile**: era_o2_g0 (default `-O2 -G0`)
- **Status**: landed (wave-6 slice B, agent/wave6-b)

## Behaviour
Weapon-effect slot constructor. Clears the byte range `slot+0xC .. slot+0xA0C`
(pointer walk, do-while), stores the zero header bytes, publishes the four
sub-block pointers (`D_800E2248 = slot+0xC`, `D_800F34F4 = slot+0x80`,
`D_800F32A8 = slot`, `D_800F3330 = slot+0x200`), zeroes the three header
halfwords, re-zeroes 64 child records (`D_800F34F4[o]`, `[o+1]`,
`*(short*)(…+o+2)`, `*(short*)(…+o+4)`, `o += 6`), conditionally patches the
effect body when `func_800C6CE0(slot) == 3`, and returns `D_800E2248+0x6C`.

## Method
The `D_800F34F4` reloads between stores fall out of indexing the global pointer
directly. The indexed byte/halfword stores need the offset **first** in the
addition tree (`*(unsigned char *)(o + (int)D_800F34F4 + 1)`) to reproduce
retail's `addu $v0,$v1,$v0` operand order. The `slot+8` patch target is a
double dereference (`q = *(int **)(*(int **)(slot + 8));`).

## Evidence
try_leaf `WORDS MATCH (+4 pad bytes)`; fresh complete build EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b, `Matching claim: YES (889
registered C leaves)`, `VERIFY_US=PASS`.
