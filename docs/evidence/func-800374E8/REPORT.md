# func_800374E8 — LINK_EXACT

24 words (0x60), VRAM `0x800374E8`, file `0x27CE8.s`, era `-O2 -G0`,
profile `era_o2_g0_three_word` (maspsx patch 2,
`MASPSX_THREE_WORD_SYMBOL_STORE=1`).

## Semantics

Clears four 56-byte records (`D_800BCEA8 + 56*i`): zeroes the record's byte-0
flag and masks the word at `+0x0C` with `0xFDFFFFFF` (clears bit 25).

```c
typedef struct {
    unsigned char flag;         /* +0x00 */
    unsigned char pad[0xB];
    unsigned int word;          /* +0x0C */
    unsigned char pad2[0x28];
} Rec;
extern Rec D_800BCEA8[4];
void func_800374E8(void) {
    unsigned char i = 0;
    do {
        D_800BCEA8[i].flag = 0;
        D_800BCEA8[i].word &= 0xFDFFFFFF;
        i++;
    } while (i < 4);
}
```

## Levers

1. **Real aggregate element type with stride 56.** `i * 56` is emitted as
   `sll $v0,$v1,3` / `subu $v0,$v0,$v1` / `sll $v0,$v0,3`, matching retail.
2. **Patch 2 is load-bearing and both statements need it.** Retail re-derives
   the `lui $at,0x800C` base for **every** access — both the `sb $0,flag(idx)`
   and the `lw`/`sw` word pair (`0x...: 3c01800c / addu at,at,v0 / ...(%lo)(at)`).
   Plain maspsx collapses each to `la $6,SYM` + `addu` (a saved-temp load),
   which is 16 mismatches; patch 2's three-word `lui`/indexed-`addu`/`%lo` form
   is byte-exact. This is the same knob the `0x27D48` twin `func_80037548`
   already uses.
3. The accumulator is a **narrow `unsigned char`** (`andi $v0,$a0,0xFF` after
   `addiu $a0,$a0,1`) and the loop bound is an `sltiu`-shaped unsigned compare
   (< 4), so the counter must be `unsigned char` with an `unsigned` compare.
