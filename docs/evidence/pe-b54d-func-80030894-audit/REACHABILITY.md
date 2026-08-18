# PE-B54D — reachability of `func_80030894`

## Sole caller

Executable-wide scan of the SHA-1-exact SLUS
`452fb033f2eaa4b18aa20a5bca60b8125af3a37b` (main text
`0x8001220C..0x80091080` and tail `0x800C22F8..0x800E0600`):

```text
jal 0x80030894          1 site   0x8006B0AC   in func_8006AD40
la   0x80030894          0 sites
static word 0x80030894   0 sites
```

There is no second entry. Reachability is exactly the `0x8006B0AC`
arm of `func_8006AD40`.

`func_8006AD40` itself is a direct `jal` from boot `func_8001220C`
at `0x800122C4` (nop delay). That site is unconditional: the
`beq` at `0x800122A0` skips optional `func_80069B08` *to*
`0x800122C4`, and the fall-through does `69B08` then the same
`jal`. No other `jal` / `la` / static word holds `0x8006AD40`.

`func_8006AD40` has one return: `jr $ra` / `nop` at
`0x8006B354` / `0x8006B358`. No `jalr`, no `j` out of the body,
no other `jr`. The proven first-play field prefix
(`m0002i → m0003i → m0372i → m0004i → m0378i`) runs only after
`func_8001220C` continues past that return. So `func_80030894`
is on the canonical first-play prefix if and only if
`0x8006B0AC` is taken on this Disc 1 boot.

## How `0x8006B0AC` is reached

Literal suffix from the live cut, delay slots indented. Words
re-read from the same EXE (`taddr = 0x80010000`).

```text
8006AF44  addiu  s0, zero, 1
8006AF48  addiu  v0, zero, -1
8006AF4C  beq    s2, v0, 0x8006ADD8     ; reissue channel 2 on timeout
8006AF50   nop
8006AF54  jal    func_8006E7E8          ; LIVE CUT — do not assign result
8006AF58   nop
8006AF5C  addu   s2, v0, zero
8006AF60  bne    s2, zero, 0x8006AE08   ; busy/timeout -> wait/reissue
8006AF64   nop
.L8006AF68:                             ; poll == 0 fallthrough
8006AF68  addu   s0, zero, zero
          ; issue D_800930EE / buf+0x180 via translated func_8006E6A8
8006AFA8  jal    func_800718D0          ; translated TIM walker
          ; pack records 0 and 1 (0x8006AFF8 / 0x8006B02C)
8006B04C  jal    func_8006E7E8          ; second live poll
8006B058  bne    s2, zero, 0x8006AF9C   ; same wait shape
          ; issue D_800930F0 / buf+0x14C via func_8006E6A8
8006B0A4  jal    func_800718D0
8006B0A8   addiu s0, zero, 1            ; delay
8006B0AC  jal    func_80030894
8006B0B0   nop
```

At the current cut, canonical locals are `s0 = 1`, `s2 = 1`
(B54D material prefix). The `bne` at `0x8006AE08` is therefore
`s0 != 0 → 0x8006AF4C`, which is the wait head, not the texture
loop.

Taken together, `0x8006AF54` is a **wait/reissue loop** around
already-translated `func_8006E7E8`:

| `func_8006E7E8` | next |
| --- | --- |
| `1` (busy) | `AE08` → `AF4C` → poll again |
| `-1` (timeout) | `AE08` → `AF4C` → reissue channel 2 at `ADD8` → poll again |
| `0` (complete) | fall through `AF68` toward `718D0` / `30894` |

`0x8006B04C` is the same shape for the next CD range. `30894`
needs both polls to report `0` and `s0 == 0` at `0x8006B098`.
The first visit to `B098` always has `s0 == 0`: second-poll
success zeros it at `B060` before the next `6E6A8` issue. The
`bne s0, B0B4` skip is the *later* reissue wait, after
`0x8006B0A8` has set `s0 = 1` in the `718D0` delay slot.

Therefore every path that leaves `AF54` and returns from
`func_8006AD40` calls `func_80030894` exactly once.

## Is the poll at `0x8006AF54` ever 0?

Yes. That is a different claim from "the first sample is 0" and
it does not assign `poll=0`.

### Control-flow (retail, Disc 1 boot)

`func_8006AD40` cannot return from the live cut unless this wait
observes 0:

- Canonical locals at `AF54` are `s0 = 1`, `s2 = 1` (B54D
  material prefix). `AF4C` (`s2 == -1` reissue) is not taken
  on entry. `AE08` with `s0 != 0` is the wait head, not the
  texture loop.
- Non-zero `func_8006E7E8` returns to `AE08` → `AF4C` → poll
  again. `-1` reissues channel 2 at `ADD8` and polls again.
- The only fall-through is `s2 == 0` at `AF68`. The function's
  sole `jr $ra` is at `0x8006B354`, after `B0AC`.
- `func_8001220C` always calls `func_8006AD40` (`0x800122C4`)
  and first-play field starts only after that call returns.

So a completed Disc 1 boot is itself the observation that some
sample at `AF54` was 0. A hang in this wait never reaches field.

### Host (observed contract, not a production constant)

Channel 2 has already issued on the implemented prefix:

```text
s1 = &D_800930EC
a0 = s6 + lhu(D_800930EC)     ; lba_base + 180
a1 = lw(D_800B0CD8 + 0x174)   ; dest 0x8012F1A0 on Disc 1
a2 = lhu(D_800930EE) - 180    ; 197 - 180 = 17 sectors
jal func_8006E6A8             ; 0x8006ADF4; issue status s2 = 1
```

`pe_libcd.c` writes `D_8009B6B4 = 0` before that issue returns
— the documented host-sync collapse. `func_8006E7E8` →
`func_800811E4` returns `D_8009B6B4` unless
`vsync > issue_vsync + 1200`. VBlank at the cut is 0, so the
timeout arm is not taken. The first host sample at `AF54` is
therefore 0.

This collapse is **not** retail timing and is **not** written
into `func_8006AD40`.

### Retail first sample

Retail CD is asynchronous. Seventeen sectors (`34816` bytes,
PE.IMG `[180,197)`, the B54C TIM atlas) are in flight when
`AF54` is first reached. The first retail sample may be `1`.
The wait still observes 0: that is the only success
continuation, and first-play boot returns through it.

```text
poll_at_AF54_ever_zero=yes
poll_assigned=no
first_retail_sample=UNKNOWN          # 0 immediately, or 1 then 0
host_first_sample=0                  # D_8009B6B4 collapse; not assigned
```

The same wait argument applies to `0x8006B04C` after the next
range is issued (`D_800930F0..F2` = `[200,203)`).

## Verdict

```text
reachable_from_canonical_prefix=yes
```

`func_80030894` sits on the live boot success path that first-play
requires. It is **not** reached from the current named cut without
consuming the live poll. B54C's open question was whether that
poll can be 0 at all; it can. The next rung still calls
`func_8006E7E8` instead of storing 0.

Current native execution stops before `0x8006AF54` and therefore
does not enter `30894` today. That is a frontier fact, not a
reachability fact.
