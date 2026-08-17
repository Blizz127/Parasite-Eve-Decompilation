# PE-BTL3 — 29810 tail through bound first actor command

Authority is the Disc 1 EXE SHA-1
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b`.
No matching `src/` C was added.

## Proven windows

```text
0x80029918..0x800299CC  45 words  29810 post-293F4 tail
0x8001A680..0x8001A704  33 words  1A680 command-store prefix
0x8001A6A0..0x8001A6CC  11 words  type*192 + command*4 load
0x8006C140..0x8006C174  13 words  Writer B type-0 clip bind
0x8006C068..0x8006C090  10 words  6BECC state-4 CE2+8 CD index
0x8006C23C..0x8006C250   5 words  6C1CC a0=1 sets CE2=14
0x80014630..0x80014658  10 words  0x55 state-0x3B overlay gate
0x8002CEE0..0x8002CF2C  19 words  mode-7 wait/gate/store
```

After `func_800293F4(0)` stores byte `4` at Aya record `+0x12`,
`func_80029810`:

1. calls `func_800209F0`;
2. floors record `+0x08` to `0x00010000` when non-positive, otherwise
   caps it at record `+0x28` and writes `240` to `+0x34` on the cap arm;
3. calls `func_80030640`;
4. writes callback `0x8002D268` to actor `+0x194`;
5. loads `*(actor+0x238)+0x18`, subtracts 100, and `sh` stores
   `D_8009D27C`;
6. calls `func_800339A0(encounter & 0xFF)`;
7. `lbu` loads record `+0x12` and calls `func_8001A680`.

`func_8001A680` then stores that exact byte at actor `+0x0E`, zeroes
actor `+0x14/+0x18`, stores a command-table resource pointer at
actor `+0x1B0`, clears flag `0x200` at actor `+0x98`, and stores
resource byte `+2 minus 1` at actor `+0x0F`.

Index math at `0x8001A6A0..0x8001A6C8`:

```text
type    = lbu actor+0x0C
table   = D_800B0E98 + type*192 + (command & 0xFFFF)*4
resource = lw table[0]
```

`type*192` is `sll 1; addu; sll 6`. Slot 4 of the type-0 row is
`D_800B0E98 + 16`.

PE-BTL4 now proves that type: ctor `func_80035038` `sb desc[0]` at
actor `+0x0C` and, when that byte is 0, `sw` the actor to
`D_8009D254`. `29810` passes exactly that pointer to `1A680`.
The HP `sh +0x0C` is `D_8009D278`, a different object. So the row
on this path is type 0 / command 4, not a mutated type. See
`docs/evidence/pe-btl4-command-table/REPORT.md`.

TRACE `first_command` therefore means actor animation/clip command `4`.
It is not a battle-menu choice, ATB action, damage command, PE action,
AI decision, enemy identity, or inferred command word.
`command_bound` means actor `+0x1B0` holds the Writer B pointer for
that slot.

## Producer (Writer B)

`func_8006BECC` state 6 bind loop `0x8006C140..0x8006C174`:

```text
idB = rec[+7]
ptr = rec[+4] & 0x00FFFFFF
sw  (package + ptr), 0x1C0($s4 + idB*4)
rec += 12
count = *(dir+0x10) >> 22
```

`$s4 = D_800B0CD8`, so the store is `D_800B0E98[idB]` (type-0 row only).
Directory: `package + *(package+4)`, records at `packed & 0x3FFFFF`.
Record size is 12 bytes. Writer A at `0x8006B84C` uses the same record
shape plus `idA*192` for room packages; m0005i slot-3 type-0 idBs are
24, 29–32 only (no command 4).

`6BECC` state 4 CD-reads `D_800930D8[CE2+8] .. D_800930D8[CE2+9]`:

```text
CE2=10 → PE.IMG [288,316)  no type-0 idB=4
CE2=14 → PE.IMG [396,428)  25 clips, idA=0 idB=0..19 including
         idB=4 ptr=0x6C14 size=1700 encoding=1 bones=31 frames=18
         clip SHA-256 6207fbca…1885e4
         bank SHA-256 db785a5e…b1b2
```

`func_8006C1CC(a0=1)` is the only EXE `sb 14, D_800B0CE2`. Callers are
`func_80024A3C` states 0/1/2 (`jal 0x8006C1CC` at `0x80024A78` with
`a0=1` in the delay slot). `0x800144FC` and `func_80029810` do not jal
`6C1CC` or `6BECC`. Native therefore treats Writer B as a global table
producer that `1A680` consumes; it does not claim `29810` calls it.

## Overlay and mode 7

The `0x55` state `0x3B` gate reads `lbu D_800B0CD8+0xE`, masks with
`3`, and parks while nonzero. On zero it clears state `+0xF4`, clears
actor flag `0x00800000`, and returns success.

The later mode-7 path calls `func_8006914C(0)`. A nonzero return clears
the live `$s1` readiness byte; mode 7 is reached only when readiness
survives. Immediately before `D_8009D28C=7`, record bytes `+0x48/+0x49`
are zeroed. Still unported; do not auto-issue mode 7 or complete `0x55`.

## Native scope

Matching-tree native cuts cover the tail stores, command prefix, Writer B
type-0 clip bind, and state-0x3B gate. Tests plant a 12-byte directory
package and run the writer; they do not poke `D_800B0E98[4]` directly.
Playable vis3 loads the CE2=14 bank, applies the same stores, and lets
`first_command` consume slot 4. Overlay return, ATB, menus, damage, PE,
AI, enemy names, and `0x55` completion remain out of scope.

## Verify

```text
python3 pc_port/tools/pe_btl3_29810_tail_oracle.py
# PASS: 29810 tail 45 + 1A680 command 33 + 0x55 wait 10 + mode7 gate 19
python3 pc_port/tools/pe_btl3_command_table_oracle.py
# PASS: Writer B 13 + 1A680 index 11 + CE2=14 map + PE.IMG clip4
#       idB=4 size=1700 frames=18
PE_TEST_FILTER=BTL3 ./pc_port/build/pe-native-tests
# 661 run, 5 passed, 0 failed, 656 skipped
./pc_port/build/pe-native-tests
# 661 run, 661 passed, 0 failed, 0 skipped
python3 pc_port/tools/pe_btl2_hp_trace_oracle.py
# PASS: BTL3 TRACE encounter_55 → hp_copied → first_command →
#       command_bound; mode 0
```

Playable vis3 `--battle-init-smoke` PASS `resource=6c14 frames-1=17`
at `345,-1111,633`, HP 40/45, mode 0, inhibit 1, screenshot SHA
`2c38a90aa78eba6cbf1f1f1daac576bbe2eb8a6d42b48206f988112e5f8ce87d`.
No matching `src/` C or split config changed. Overlay `+0xE & 3` /
`func_8006914C` / `0x55` completion remain unported.
