import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestFillJalDelaySlotGuards(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_FILL_JAL_DELAY_SLOT": "0"}):
            return MaspsxProcessor(
                lines, fill_jal_delay_slot=enabled
            ).process_lines()

    def test_absolute_halfword_store_fills_call_slot(self):
        # ROM func_80073F00 @0x648A8: sh $zero,D_800945E6 / jal func_80074384.
        lines = [
            "sh\t$zero,D_800945E6",
            "jal\tfunc_80074384",
            "lw\t$31,16($sp)",
        ]

        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)

        self.assertNotEqual(disabled, enabled)
        self.assertIn("# FILL_JAL_DELAY_SLOT START", enabled)
        # ASPSX order: lui $at,%hi / jal / store %lo($at) in the slot.
        idx_lui = enabled.index("lui\t$at,%hi(D_800945E6)")
        idx_jump = enabled.index("jal\tfunc_80074384")
        idx_store = enabled.index("sh\t$zero,%lo(D_800945E6)($at)")
        self.assertLess(idx_lui, idx_jump)
        self.assertLess(idx_jump, idx_store)
        # Consuming the jal line suppresses maspsx's appended nop.
        self.assertNotIn("nop  # DEBUG: branch/jump", enabled)

    def test_absolute_word_store_fills_call_slot(self):
        # ROM func_80076C34 @0x6769C: sw $v0,D_80095874 / jal func_80073E10.
        lines = [
            "sw\t$v0,D_80095874",
            "jal\tfunc_80073E10",
            "nop",
        ]

        enabled = self.process(lines, enabled=True)
        self.assertIn("# FILL_JAL_DELAY_SLOT START", enabled)
        self.assertIn("jal\tfunc_80073E10", enabled)
        self.assertIn("sw\t$v0,%lo(D_80095874)($at)", enabled)

    def test_absolute_byte_store_fills_call_slot(self):
        lines = [
            "sb\t$4,D_800A36A0",
            "jal\tfunc_80012345",
            "nop",
        ]

        enabled = self.process(lines, enabled=True)
        self.assertIn("# FILL_JAL_DELAY_SLOT START", enabled)
        self.assertIn("sb\t$4,%lo(D_800A36A0)($at)", enabled)

    def test_load_is_never_filled(self):
        lines = [
            "lw\t$4,D_800A36A0",
            "jal\tfunc_80012345",
            "nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_register_offset_store_is_never_filled(self):
        # cc1's reorg already schedules register-offset stores; this gate only
        # rewrites the absolute symbol-store macro.
        lines = [
            "sh\t$6,2($4)",
            "jal\tfunc_80012345",
            "nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_label_blocks_fill(self):
        lines = [
            "sh\t$zero,D_800945E6",
            "$L1:",
            "jal\tfunc_80074384",
            "nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_set_directive_blocks_fill(self):
        lines = [
            "sh\t$zero,D_800945E6",
            ".set\tnoreorder",
            "jal\tfunc_80074384",
            "nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_return_jump_is_left_to_the_store_gate(self):
        # The jr gate is a separate, sw-only opt-in; enabling the jal gate must
        # not make a `j $31` return slot fill (and vice versa).
        lines = [
            "sh\t$zero,D_800945E6",
            "j\t$31",
            "#nop",
        ]

        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_default_is_off(self):
        lines = [
            "sh\t$zero,D_800945E6",
            "jal\tfunc_80074384",
            "nop",
        ]

        with patch.dict(os.environ, {"MASPSX_FILL_JAL_DELAY_SLOT": "0"}):
            default = MaspsxProcessor(lines).process_lines()
        self.assertEqual(self.process(lines, enabled=False), default)


if __name__ == "__main__":
    unittest.main()
