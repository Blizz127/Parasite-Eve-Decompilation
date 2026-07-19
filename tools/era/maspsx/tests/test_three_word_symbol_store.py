import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestThreeWordSymbolStore(unittest.TestCase):
    STORE = "sh\t$0,D_8009448C($4)"
    LEGACY_STORE = [
        ".set\tnoat",
        "lui\t$at,%hi(D_8009448C)",
        "addiu\t$at,$at,%lo(D_8009448C)",
        "addu\t$at,$at,$4",
        "sh\t$0,0x0($at)",
        ".set\tat",
    ]

    @staticmethod
    def process(lines, *, constructor_enabled=False, environment_enabled=False):
        value = "1" if environment_enabled else "0"
        with patch.dict(os.environ, {"MASPSX_THREE_WORD_SYMBOL_STORE": value}):
            return strip_comments(
                MaspsxProcessor(
                    lines,
                    addiu_at=True,
                    three_word_symbol_store=constructor_enabled,
                ).process_lines()
            )

    def test_disabled_keeps_legacy_four_word_expansion(self):
        self.assertEqual(
            self.LEGACY_STORE,
            self.process([self.STORE]),
        )

    def test_environment_flag_selects_three_word_assembler_path(self):
        # This is the ASPSX 2.30 path: GNU as expands the retained pseudo to
        # lui %hi / addu index / sh %lo, as covered by the linked probe.
        self.assertEqual(
            [self.STORE],
            self.process([self.STORE], environment_enabled=True),
        )

    def test_constructor_flag_selects_three_word_assembler_path(self):
        self.assertEqual(
            [self.STORE],
            self.process([self.STORE], constructor_enabled=True),
        )

    def test_compound_line_keeps_exact_legacy_expansion(self):
        compound = f"{self.STORE};nop"

        self.assertEqual(self.LEGACY_STORE, self.process([compound]))
        self.assertEqual(
            self.LEGACY_STORE,
            self.process([compound], environment_enabled=True),
        )

    def test_indexed_symbolic_load_is_unchanged(self):
        load = "lh\t$2,D_8009448C($4)"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(D_8009448C)",
            "addiu\t$at,$at,%lo(D_8009448C)",
            "addu\t$at,$at,$4",
            "lh\t$2,0x0($at)",
            ".set\tat",
        ]

        self.assertEqual(expected, self.process([load]))
        self.assertEqual(
            expected,
            self.process([load], environment_enabled=True),
        )


if __name__ == "__main__":
    unittest.main()
