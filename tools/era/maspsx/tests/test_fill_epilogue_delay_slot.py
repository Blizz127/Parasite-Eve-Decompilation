import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor


class TestFillEpilogueDelaySlotGuards(unittest.TestCase):
    @staticmethod
    def process(lines, enabled):
        # Make the constructor flag authoritative even if the caller's shell
        # has the production opt-in environment variable set.
        with patch.dict(os.environ, {"MASPSX_FILL_EPILOGUE_DELAY_SLOT": "0"}):
            return MaspsxProcessor(
                lines, fill_epilogue_delay_slot=enabled
            ).process_lines()

    def test_stack_restore_before_return_jump_is_moved_into_slot(self):
        lines = [
            "lw\t$31,20($sp)",
            "addu\t$sp,$sp,24",
            "j\t$31",
        ]

        disabled = self.process(lines, enabled=False)
        enabled = self.process(lines, enabled=True)

        self.assertNotEqual(disabled, enabled)
        self.assertIn("# EPILOGUE_FILL_DELAY_SLOT START", enabled)
        # The jump must be emitted before the restore (the delay slot).
        j = enabled.index("j\t$31")
        addu = enabled.index("addu\t$sp,$sp,24")
        self.assertLess(j, addu)
        # The generated slot replaces the nop maspsx would append.
        self.assertNotIn("nop  # DEBUG: branch/jump", enabled)

    def test_addiu_stack_restore_also_moves(self):
        lines = [
            "lw\t$31,20($sp)",
            "addiu\t$sp,$sp,24",
            "j\t$31",
        ]
        enabled = self.process(lines, enabled=True)
        self.assertIn("# EPILOGUE_FILL_DELAY_SLOT START", enabled)

    def test_non_stack_addu_is_untouched(self):
        lines = [
            "addu\t$2,$2,$3",
            "j\t$31",
        ]
        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_label_blocks_fill(self):
        lines = [
            "addu\t$sp,$sp,24",
            "$L1:",
            "j\t$31",
        ]
        self.assertEqual(
            self.process(lines, enabled=False),
            self.process(lines, enabled=True),
        )

    def test_default_is_off(self):
        lines = [
            "addu\t$sp,$sp,24",
            "j\t$31",
        ]
        with patch.dict(os.environ, {"MASPSX_FILL_EPILOGUE_DELAY_SLOT": "0"}):
            default = MaspsxProcessor(lines).process_lines()
        self.assertEqual(self.process(lines, enabled=False), default)


if __name__ == "__main__":
    unittest.main()
