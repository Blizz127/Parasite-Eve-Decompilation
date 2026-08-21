#!/usr/bin/env python3
"""Replica of func_8001C614 D1D8==0 arm for fixture search."""


def c614(pairs, rec_idx, a1, a2):
    # rec_idx[0] unused here; +2,+4,+6 are indices
    t1 = pairs[rec_idx[6 // 2]][1]  # wait
    # initial from +6
    i0 = rec_idx[3]  # +6 / 2
    t1, a3 = pairs[i0][1], pairs[i0][0]
    t0 = 0
    t4, t2 = a2, a1
    t3_off = 0
    for _ in range(3):
        a2_saved, a0_saved = a3, t1
        idx = rec_idx[(t3_off + 2) // 2]
        t1, a3 = pairs[idx][1], pairs[idx][0]
        y2, y1 = t1, a0_saved
        x2, x1 = a3, a2_saved
        y_lt = t4 < y2
        if y_lt:
            if t4 < y1:
                t3_off += 2
                continue
        elif not (t4 < y1):
            t3_off += 2
            continue
        if t2 < x2:
            if t2 < x1:
                t0 = int(t0 < 1)
                t3_off += 2
                continue
            x_cross = True
        elif t2 < x1:
            x_cross = True
        else:
            t3_off += 2
            continue
        if x_cross:
            dy = y1 - y2
            dx = x1 - x2
            lhs = (dx * (t4 - y2)) & 0xFFFFFFFF
            rhs = (dy * (t2 - x2)) & 0xFFFFFFFF
            if lhs >= 0x80000000:
                lhs -= 0x100000000
            if rhs >= 0x80000000:
                rhs -= 0x100000000
            if dy < 0:
                pass_ = lhs < rhs
            else:
                pass_ = rhs < lhs
            if pass_:
                t0 = int(t0 < 1)
        t3_off += 2
    return t0


pairs = [(0, 0), (30, 0), (15, 30)]
# +2=1, +4=2, +6=0
rec = {1: 1, 2: 2, 3: 0}
print("triangle (15,10)", c614(pairs, rec, 15, 10))
print("triangle (15,5)", c614(pairs, rec, 15, 5))
print("triangle (40,10)", c614(pairs, rec, 40, 10))
print("triangle (15,-5)", c614(pairs, rec, 15, -5))

# search
for x in range(-5, 40, 5):
    for y in range(-5, 40, 5):
        r = c614(pairs, rec, x, y)
        if r:
            print("hit", x, y)
