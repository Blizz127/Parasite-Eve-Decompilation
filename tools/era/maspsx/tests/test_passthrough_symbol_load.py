import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestPassthroughSymbolLoad(unittest.TestCase):
    """MASPSX_PASSTHROUGH_SYMBOL_LOAD legacy/ROM shapes.

    Some ROM leaves keep cc1's `op $r,SYM($b)` line, which GNU as expands with
    the DESTINATION register as the address temp:
        lui $r,%hi(SYM) / addu $r,$r,$b / op $r,%lo(SYM)($r)
    The default maspsx path instead forces $at.  The gate passes the line
    through unchanged for those leaves (ROM-proven: func_80076B44,
    func_8007FBF0).
    """

    LOAD = "lbu\t$2,D_800A3348($4)"

    @staticmethod
    def process(lines, *, constructor_enabled=False, environment_enabled=False,
                addiu_at=True):
        value = "1" if environment_enabled else "0"
        with patch.dict(os.environ, {"MASPSX_PASSTHROUGH_SYMBOL_LOAD": value}):
            return strip_comments(
                MaspsxProcessor(
                    lines,
                    addiu_at=addiu_at,
                    passthrough_symbol_load=constructor_enabled,
                ).process_lines()
            )

    def test_disabled_keeps_four_word_at_expansion(self):
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(D_800A3348)",
            "addiu\t$at,$at,%lo(D_800A3348)",
            "addu\t$at,$at,$4",
            "lbu\t$2,0x0($at)",
            ".set\tat",
        ]
        self.assertEqual(expected, self.process([self.LOAD]))

    def test_environment_flag_passes_line_through(self):
        self.assertEqual([self.LOAD], self.process([self.LOAD],
                                                   environment_enabled=True))

    def test_constructor_flag_passes_line_through(self):
        self.assertEqual([self.LOAD], self.process([self.LOAD],
                                                   constructor_enabled=True))

    def test_disabled_three_word_at_form_without_addiu_at(self):
        # addiu_at=False is the ASPSX 2.30 three-word load path; the new gate
        # must not change it when OFF.
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(D_800A3348)",
            "addu\t$at,$at,$4",
            "lbu\t$2,%lo(D_800A3348)($at)",
            ".set\tat",
        ]
        self.assertEqual(expected, self.process([self.LOAD], addiu_at=False))

    def test_compound_line_keeps_legacy_expansion(self):
        # A compound macro line is never passed through, flag ON or OFF.
        compound = f"{self.LOAD};nop"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(D_800A3348)",
            "addiu\t$at,$at,%lo(D_800A3348)",
            "addu\t$at,$at,$4",
            "lbu\t$2,0x0($at)",
            ".set\tat",
        ]
        self.assertEqual(expected, self.process([compound]))
        self.assertEqual(expected, self.process([compound],
                                                environment_enabled=True))

    def test_gate_does_not_touch_absolute_load(self):
        # No index register: unchanged by the gate.
        load = "lbu\t$2,D_800A3348"
        off = self.process([load])
        on = self.process([load], environment_enabled=True)
        self.assertEqual(off, on)


if __name__ == "__main__":
    unittest.main()
