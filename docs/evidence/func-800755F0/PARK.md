# func_800755F0 — PARKED (structural/scale residual)

VRAM `0x800755F0`, size `0x4F8` (318 words), era `-O2 -G0` (+ patch 3).
File `0x65DF0` in `asm/disc1/655C0.s`.

## What it is

The `654C8` display cluster's big gate/state machine. It

1. gates on `D_8009574E >= 2` and logs through `D_80095748(&D_80011970, s1)`;
2. pushes a `0x5000000` draw-mode word through the `D_80095744->f(0x10)`
   slot (`D_80095744 + 4`), built from `s1[0]`/`s1[2]` low 10 bits;
3. compares the `D_8009574E+0x7A` and `+0x6A..0x70` shadow against the
   live `s1+0x10`/`s1+0..6`; on match jumps to the second half;
4. otherwise calls `func_80074A28()`, rebuilds a flag word
   (`0x08000008` base | bit 8/0x10/0x20/0x80 | range-selected 1/2/3/0x40 |
   0x24), pushes it through the same `f(0x10)` slot, and sets `s1[0x12]=8`;
5. compares the `D_800957C0` shadow window against `s1+8..0xE`;
6. on mismatch rebuilds a scaled/clamped (x,y,x2,y2) quad from the
   `D_80095848[]` scale table and the `D_80095820[]` min/max pairs, then
   pushes `0x6000000` and `0x7000000` words out `f(0x10)`;
7. resets a `0x14`-byte record from the `D_800957B8` template via
   `func_80071A34`.

## Residual

Written out in full (script plus structured transcription, both checked in
the session transcript); the object diverges pervasively — 298/318 words,
with the first divergence in the prologue. Two independent problems:

1. **Prologue/scheduling**: retail materializes the `D_8009574E` pointer
   into `$s2` *before* the `lbu`, and computes `lui $s0,0x800` (the
   `0x08000000` flag base) in the `bnez` delay slot; cc1 here issues the
   `lbu` first and defers the flag base. Register set matches
   (`s0`=flags, `s1`=arg, `s2`=gate ptr), so this is block scheduling, not
   typing.
2. **Tail clamp/dispatch**: the `s1[0x12] == 0` split at `0x800758BC` feeds
   two long clamp ladders (`0x21C/0xC94/0xCBD/0xCBC/0x13/0x130/0x12F/0x131`
   vs `0x1F4/0xCB2/0xCDA/0xCDB/0x10/0x102/0x101/0x102`) that read
   `D_80095848[a6]` twice and `D_80095820[s1[0x12]*5+a6]` twice (`lhu`+`sll`
   /`sra` sign extension). The retail control flow re-converges through
   `0x800759A8`/`0x80075A38` and shared `0x80075A50` tails; a linear C
   transcription does not reproduce the shared-tail block layout.

Both need a structural lever (shared-tail source shape and/or a scheduling
knob) not yet in hand. Not invented; left as `asm`.

Note the flag base is `0x08000000`, not `0x80000000` — the `lui $s0,0x800`
in the `bnez` delay slot is `0x08000000>>16`.
