import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestFillStoreDelaySlotGuards(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_FILL_STORE_DELAY_SLOT": "0"}):
            return MaspsxProcessor(
                lines, fill_store_delay_slot=enabled
            ).process_lines()

    def test_immediate_return_jump_is_filled(self):
        lines = [
            "sw\t$4,D_800A36A0",
            "j\t$31",
            "#nop",
        ]

        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)

        self.assertNotEqual(disabled, enabled)
        self.assertIn("# FILL_STORE_DELAY_SLOT START", enabled)

    def test_label_blocks_fill(self):
        lines = [
            "sw\t$4,D_800A36A0",
            "$L1:",
            "j\t$31",
            "#nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_set_directive_blocks_fill(self):
        lines = [
            "sw\t$4,D_800A36A0",
            ".set\tnoreorder",
            "j\t$31",
            "sw\t$4,D_800A36A0",
            ".set\treorder",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )


class TestFillIndexedStoreDelaySlotGuards(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_FILL_INDEXED_STORE_DELAY_SLOT": "0"}):
            return MaspsxProcessor(
                lines, fill_indexed_store_delay_slot=enabled
            ).process_lines()

    def test_indexed_store_fills_return_jump(self):
        lines = [
            "sb\t$4,D_800A3348($2)",
            "j\t$31",
            "#nop",
        ]

        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)

        self.assertNotEqual(disabled, enabled)
        self.assertIn("# INDEXED_FILL_STORE_DELAY_SLOT START", enabled)
        # ASPSX order: the jump must precede the store (store in its slot).
        idx_jump = enabled.index("j\t$31")
        idx_store = next(
            i for i, line in enumerate(enabled) if line.startswith("sb\t")
        )
        self.assertLess(idx_jump, idx_store)

    def test_load_is_never_filled(self):
        lines = [
            "lw\t$4,D_800A3348($2)",
            "j\t$31",
            "#nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_label_blocks_fill(self):
        lines = [
            "sb\t$4,D_800A3348($2)",
            "$L1:",
            "j\t$31",
            "#nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )


if __name__ == "__main__":
    unittest.main()
