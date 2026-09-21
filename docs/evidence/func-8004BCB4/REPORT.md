# func_8004BCB4 — MATCHED (13 words, LINK_EXACT)

VRAM `0x8004BCB4`, file `0x3C4B4`, span `0x34`. The span begins exactly at the
function and ends at `0x3C4E8` where the next function starts.

```c
void func_8004BCB4(void) {
    func_8005E8A4(0, 4);
    func_8005F594(func_8005DC4C(0x19));
}
```

The inner `func_8005DC4C(0x19)` result is forwarded in `$a0` (the `addu $a0,$v0`
in the `jal` delay slot). era `-O2 -G0`. Gate: `check_leaf.sh` →
`LINK_EXACT`, deep span exact.
