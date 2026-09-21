# CRC tail is the LOAD path — it cannot close the save menu

Branch scope: port `func_80042264` and reconcile `func_80040F80`'s flag, as
requested by the post-save close-path handoff.  Authority: retail Disc1 EXE
SHA-1 `452fb033f2eaa4b18aa20a5bca60b8125af3a37b`, rebuilt ELF `build/disc1.elf`.

## Result (short)

`func_80042264` is real and I reconstructed it, but it is called **only from
state 7, the LOAD read path**.  The save path never enters state 7, so porting
it cannot close the save menu.  Wiring it also breaks three recorded card
oracles because those oracles transcribe the port's **wrong** global addresses.
Details and evidence below; the port tree is left unchanged (tests 1392/1392
green).

## 1. The real entry point

The ELF symbol names in this repo are offset by +0x30 from the code they label
(splat artefact).  `objdump` labels `0x80042294` as `func_80042264` and the
call at `0x80041BE0` targets `80042294`:

```
80041be0: 0c0108a5  jal  80042294 <func_80042264>
```

So the function is `[0x80042294,0x80042490)` (0x1FC bytes).  The earlier report's
`0x80042400..0x80042430` listing was the same code shifted by 0x30.

The only two `jal`s in the whole EXE that reach this region are:

```
80040fd8: jal 80042258   ; func_80040F80 -> func_80042228 (menu close helper)
80041be0: jal 80042294   ; state 7 tail -> func_80042264
```

## 2. Only the LOAD path reaches it

State dispatch table at VA `0x80010F6C` (absolute handler addresses):

```
state 7  = 0x80041B18   <- contains the 0x80041BE0 jal 80042294
state 8  = 0x80041A58
state 9  = 0x80041C0C   <- save write
state 10 = 0x80041D04
```

The only `li v0,7` in the card state machine is at `0x80041958`, inside
**state 5** (`0x800418B4`, the load-open handler).  The save flow is:

```
state 4 (0x800417C0) -> state 6 (0x80041980) -> state 9 write (0x80041C3C)
  -> state 3 (0x800415B0) -> state 8 read -> state 10 -> state 3/12
```

State 9's completion (`0x80041CB0..0x80041CE4`) sets state 3, `[0x800A1A0C]=0`,
`func_8004D9D8()`, notice `0x53`, and returns to the dispatcher.  It never
touches state 7.  **Route proof:** the port's state-7 boundary
(`operation_boundary("func_80041108", 0x80042264u, ...)`) never fires across the
full 42000-frame `--route-pad` run; the run reaches `stop_reason=frame-limit`
with no boundary stop.

## 3. Retail globals (why the port's differ)

```
0x80040fb4  lw v0,6676(v0)     ; func_80040F80 reads [0x800A1A14]
0x80042440  sw v0,6676(at)     ; func_80042264: [0x800A1A14]=1 on CRC success
0x800425f0  sw zero,6676(at)   ; func_800425DC clears [0x800A1A14]
0x80041bd0  sw zero,6668(at)   ; state 7 tail: [0x800A1A0C]=0
0x80041bd8  sw zero,6640(at)   ; state 7 tail: [0x800A19F0]=0
0x80041cc4  sw zero,6668(at)   ; state 9 tail: [0x800A1A0C]=0
```

`[0x800A1A14]` is written **only** by `func_80042264`.  The port instead uses
`0x800A1854` / `0x800A185C` in these four places
(`func_80041108_port.c` state-7/9 tails and `func_80040F80`), which are the
crude-carve labels the card work inherited.

## 4. Why the fix cannot land yet

I implemented `func_80042264` (copy of 0x12E4 bytes, the 0x800-byte
`func_8003FBD8` helper, the 8192-iteration CRC-16/CCITT fold, the match/mismatch
tails), reconstructed the retail globals, and wired the state-7 tail.  Result:

```
DAY1_card_driver          FAIL: card driver state differs from original
DAY1_card_cleanup         FAIL: card cleanup state differs from original
DAY1_card_operation_frame FAIL: card post-format boundary differs from original
```

Cause: the card oracles **encode the port's wrong addresses** —
`pc_port/tools/pe_card_operation_oracle.py:44` does `sw(0xA185C,0)` and the
recorded models use `0x800A1854`.  The oracle replays "original" code that reads
the port's addresses, not retail's.  Fixing the port to retail's addresses makes
it disagree with the (mistranscribed) oracle.  The oracle models must be
regenerated with `0x800A1A0C` / `0x800A1A14` first.

## 5. What actually closes the save menu

The save-complete notice is created by state 9 with `func_8004CC50(0x53,0)`;
the port's `func_8004CC50` installs input callback `func_8004D030` but calls
`func_8004D024(0)`, so the dismiss has no close callback.  After the notice is
dismissed the game returns to the slot list, and the recorded `--route-pad`
sequence goes idle over the save-menu segment (only the Cross auto-pulse fires),
so the menu is never exited.  The remaining step is therefore one of:

1. the retail action that leaves the save/load menu after a save (probably a
   CANCEL/END input, or a callback armed by `func_8004D9D8`), and then adding
   that input to the route data with evidence; and
2. regenerating the card oracles with the retail global addresses so
   `func_80042264` can be wired for the LOAD path.

## Commands run

```
cmake -S pc_port -B pc_port/build && cmake --build pc_port/build -j
./pc_port/build/pe-native-tests                       -> 1392/1392 (after revert)
distrobox enter pe-mipsel -- bash -lc 'mipsel-linux-gnu-objdump -d build/disc1.elf'
./pc_port/build/parasite-eve-port --disc-image "$(cat local/pe_disc1.path)" \
    --headless --route-pad --max-frames 42000 --boundary-report
```

## Non-claims

No matching-leaf change; no save-menu exit; the tree is unchanged.  This report
records that the requested `func_80042264` port is a LOAD-path fix and not the
save-menu unlock, with the disassembly evidence for that conclusion.
