#!/usr/bin/env python3
"""Unit tests for tools/build/disc1_build.py's dispatch-table tolerance.

The compiled `.rodata` of a foldable `switch` must be provably identical to the
pool table splat put in `asm/disc1/data/*.rodata.s` before the build zeroes it.
cc1 pads the section to 8 bytes, so an odd-word table arrives four bytes longer
than the pool entry. These tests pin the exact accepted shapes.

Run: python3 tools/build/test_disc1_build.py
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from disc1_build import dispatch_rodata_pad_ok  # noqa: E402

ZERO4 = b"\x00\x00\x00\x00"


class DispatchRodataPadOkTests(unittest.TestCase):
    def test_exact_even_table_accepted(self) -> None:
        self.assertTrue(dispatch_rodata_pad_ok(0x60, 0x60, b""))

    def test_exact_odd_table_without_padding_accepted(self) -> None:
        # A table that happens to land 8-byte aligned needs no padding.
        self.assertTrue(dispatch_rodata_pad_ok(0x3C, 0x3C, b""))

    def test_odd_table_with_four_zero_pad_bytes_accepted(self) -> None:
        # 23 words = 0x5C, aligned to 0x60 by `.align 3`.
        self.assertTrue(dispatch_rodata_pad_ok(0x60, 0x5C, ZERO4))

    def test_padding_rejected_when_table_already_aligned(self) -> None:
        # 0x60 is already 8-byte sized, so `.align 3` adds nothing; a longer
        # section is then a genuinely different table, not padding.
        self.assertFalse(dispatch_rodata_pad_ok(0x64, 0x60, ZERO4))

    def test_nonzero_padding_rejected(self) -> None:
        self.assertFalse(dispatch_rodata_pad_ok(0x60, 0x5C, b"\x01\x00\x00\x00"))

    def test_padding_larger_than_four_rejected(self) -> None:
        self.assertFalse(dispatch_rodata_pad_ok(0x64, 0x5C, ZERO4 * 2))

    def test_section_shorter_than_pool_rejected(self) -> None:
        self.assertFalse(dispatch_rodata_pad_ok(0x58, 0x5C, b""))

    def test_section_longer_than_pool_plus_pad_rejected(self) -> None:
        self.assertFalse(dispatch_rodata_pad_ok(0x64, 0x60, ZERO4))

    def test_four_pad_bytes_required_to_be_present(self) -> None:
        self.assertFalse(dispatch_rodata_pad_ok(0x60, 0x5C, b"\x00\x00\x00"))


if __name__ == "__main__":
    unittest.main(verbosity=2)
