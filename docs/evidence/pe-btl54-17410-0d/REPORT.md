# PE-BTL54 — opcode 0x0D / 0x22 message open and poll

```text
exe_sha1    452fb033f2eaa4b18aa20a5bca60b8125af3a37b
17410       13w  0x80017410..0x80017444  sha f84d1f42…518e
177C8       22w  0x800177C8..0x80017820  sha 0da54c55…2432
37548       27w  0x80037548..0x800375B4  sha 0fae0d80…8fc3
375E0       161w 0x800375E0..0x80037864  sha d7f8b96c…d58a
```

Matching `src/` C was not added. `func_80037548` already matches
in the decomp tree; this rung is the host translation only.

## Live type-0 FF arm

After `+0x8D0` yield:

```text
+8D8  0xEA
+8F8  0x09 AND persist[0] with 4 → cond[0]
+910  0x05 skip-if-true to base+(0x4EA<<1)=+0x9D4
+920  0x0D id 7
+92C  0x22 id 7
```

Live persist[0] is 0 after 34F10 / fade `&=~2`, so the skip is
**not** taken. `0x0D` then `0x22` execute. The skip target is
another `0x0D`/`0x22` pair (id `0x26`), not a bypass of the
message system.

## 0x0D / 17410

`lh *arg0`, `$a1=0`, stack s16 `-1`, `jal 375E0`, `v0=1`.
Non-blocking. First free `D_800BCEA8` record (byte0==0, stride
56) becomes state 1, `+0x10=id`, `+0x08=0`, flags clear
`0x00100000` and `0x00200000`.

## 0x22 / 177C8

`jal 37548(lh *arg0)`. Signed byte0==0 → `v0=1`. Else
`CE00-=0xC`, `D300+0x10=1`, `v0=0`.

## Next authentic site

`3F3C4` jals `37870` at `0x8003F568` when `B0CD8&0x200==0`.
`0xAA` sets `0x2000`, not `0x200`. The current 3F3C4 named cut
does not yet issue that jal. 37870 is 1064 words; state-1 bind
and the F9/FF close rules are documented in
`docs/evidence/pe-txt0-retail-text/TEXT_LIFECYCLE.md`.

Do not invent pad `D1F4`, persist==39, scratch[0]&4, or a
type-3 hit.
