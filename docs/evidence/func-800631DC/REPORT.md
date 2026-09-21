# func_800631DC — MATCHED (20 words, LINK_EXACT)

VRAM `0x800631DC`, file `0x539DC`, span `0x50`. Carve: splits the former
`[0x539DC, asm]` span into C `0x50` + resume `0x1FC` (`func_8006322C` follows
at file `0x53A2C`).

```c
typedef struct Node {
    struct Node *next;          /* +0x00 */
    unsigned char pad4[0x1C];   /* +0x04..+0x1F */
    int type;                   /* +0x20 */
    unsigned char pad24[0x20];  /* +0x24..+0x43 */
    unsigned int flag;          /* +0x44 */
} Node;

extern Node *D_8009D154;

Node *func_800631DC(void) {
    Node *result = 0;
    Node *node = D_8009D154;

    while (node) {
        if (node->type == 1 && node->flag != 0) {
            result = node;
        }
        node = node->next;
    }
    return result;
}
```

**Semantics.** Walks the gp-relative free-list head `D_8009D154`
(`gp+0x3E4`, `_gp = 0x8009CD70`) and returns the *last* node whose `+0x20`
word equals 1 **and** whose `+0x44` word is nonzero; returns 0 for an empty
list or no match. Because the match assigns the running accumulator rather
than returning, a later match wins — the `move $a0,$zero` seed sits in the
`beqz` delay slot and the `move $v0,$a0` result moves into the `jr` delay
slot at exit. The shared `Node` layout is the one derived in
`src/func_80062A34.c` (keys at `+0x20`/`+0x24`), extended here with `+0x44`.

**Levers.**
1. **The `type == 1` literal hoists to `$a1`** (`li $a1,1` before the loop)
   only when the comparison is written as `node->type == 1` against an `int`
   field — no separate local needed.
2. **The accumulator is seeded before the loop** (`Node *result = 0;` then
   `node = D_8009D154;`), which puts `move $a0,$zero` in the guard delay slot.
3. **Short-circuit `&&` keeps the two-field compare in order** (`+0x20`
   `bne` first, then `+0x44` `beqz`).
4. **`unsigned int flag != 0`** yields the plain `beqz` (no sign trickery).

**Build profile.** `era_o2_g8` (the head load is gp-relative; the compare
literal and the table fields use absolute addressing).

**Gate.**

```
tools/analysis/check_leaf.sh func_800631DC 0x800631DC 0x50 -O2 -G8
  → linked .text 80 bytes, target 0x50, word mismatches=0, nonzero_pad=0
  → LINK_EXACT
  → disc1_preflight: PASS (deep, 787 c / 347 asm / 2 rodata)
```
