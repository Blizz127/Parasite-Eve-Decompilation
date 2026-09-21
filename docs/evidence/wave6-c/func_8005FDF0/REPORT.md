# func_8005FDF0

- VRAM `0x8005FDF0`, file `0x505F0-0x50728`, size `0x138` (312 B).
- Profile `era_o2_g8_expand_div` (`-O2 -G8` + `MASPSX_EXPAND_DIV=1`).
- Commit `1603a5fc`, landed 884 -> 890.

Four-digit signed decimal emitter: identical body to `func_8005FA3C` with
`digits = 4` and the negative prefix reducing the field to 3 digits. Generated
from the same template and matched on the first `try_leaf` attempt. See
`../func_8005FA3C/REPORT.md` for the ternary / expand-div / self-store levers.
