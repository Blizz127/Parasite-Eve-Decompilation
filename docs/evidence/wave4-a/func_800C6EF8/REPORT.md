# func_800C6EF8

- **VRAM**: 0x800C6EF8
- **File offset**: 0xB76F8 (size 0x54)
- **Unit**: B76F8
- **Build profile**: era_o2_g0
- **Tests**: try_leaf WORDS MATCH; full build_us.sh + verify_us.sh
- **Status**: landed (wave4-a, agent/wave4-a)

## Behaviour
Copies `*(u16 *)(mesh + 0xA)` 32-bit words from `mesh + *(u16 *)(mesh + 8)`
into the palette scratch `D_800E2370`. No return value (retail leaves the
last `slt` result in `$v0`).

## Method
Read `glabel`..`endlabel` in `asm/disc1/B76F8.s` and the sibling matched
`src/func_800C6EC0.c`/`func_800C6EE8.c`. Two cc1 levers were required:

- **phantom frame**: retail reserves 8 stack bytes (`addiu $sp,$sp,-8`)
  with no stack access anywhere. No `-O*`/`-G*` combination produces a frame
  here, but an address-taken dead aggregate (here `int tmp[2]`, kept with
  `(void)tmp;`) makes cc1 count 8 bytes of `vars` and emit the prologue/
  epilogue pair with zero accesses. Other dead 8-byte locals (struct/array)
  behave the same; this is cc1's documented phantom frame in this project.
- **counter register pin**: with natural colouring cc1 puts the counter in
  `$a2` and the source pointer in `$a1`; retail has counter `$a1`, source
  `$a2`. The `do { i++; ... } while (i < reloaded count)` body is otherwise
  exact. `register int var_a1 asm("$5");` closed the residual.

## Evidence
Fresh complete retail build reported `EXACT SHA-1
452fb033f2eaa4b18aa20a5bca60b8125af3a37b` with the registered C leaf count
incremented (864 -> 867 across this wave). `VERIFY_US=PASS`.

## Divergences
None in the landed source. The `tmp[2]` local is a frame-artifact reproducer,
not a claim about the original source.
