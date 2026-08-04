#!/usr/bin/env python3
"""Independent write-order oracle for retail func_8005DE88."""
from __future__ import annotations

BASE = 0x800A2090
END = 0x800A2180
STATE = 0x800A2174
GP = 0x8009CD70


def expected_writes():
    writes = [(node, node + 0xC) for node in range(BASE, END, 0xC)]
    writes.append((STATE, 0))
    writes.extend(((GP + 0x36C, BASE), (GP + 0x374, 0),
                   (GP + 0x370, 0), (GP + 0x378, 0),
                   (GP + 0x37C, 0), (GP + 0x380, 0)))
    return writes


def final_words():
    result = {address: value for address, value in expected_writes()}
    return result


def validate(observed) -> None:
    assert list(observed) == expected_writes(), "5DE88 write order mismatch"


if __name__ == "__main__":
    writes = expected_writes()
    print("writes", len(writes), "first", writes[0], "last", writes[-1])
