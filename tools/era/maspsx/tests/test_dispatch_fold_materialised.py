import os
import unittest
from unittest.mock import patch

from maspsx import MaspsxProcessor

from .util import strip_comments


class TestDispatchFoldMaterialised(unittest.TestCase):
    """Materialised switch dispatch retarget (MASPSX_DISPATCH_FOLD).

    When a `switch` sits inside a loop, cc1's LICM hoists the loop-invariant
    table base and emits `lui $r,%hi($L<n>)` / `addiu $r,$r,%lo($L<n>)`
    instead of the compound `lw $r,$L<n>($b)` form. The `$L<digits>` label
    never appears as an addend, so the compound-form fold cannot see it;
    under the SAME THREE_WORD + FOLD gates the local label inside %hi()/%lo()
    is substituted with the shared pool table. Without BOTH gates nothing
    changes, and only numeric compiler-local labels are eligible.

    The negatives are as load-bearing as the positive: a named symbol, a
    non-%hi/%lo use of $L (branch target), and a gate-off run must all pass
    through untouched.
    """

    MATERIALISED = ["lui\t$t6,%hi($L30)", "addiu\t$t6,$t6,%lo($L30)"]
    FOLDED = [
        "lui\t$t6,%hi(jtbl_800111F8)",
        "addiu\t$t6,$t6,%lo(jtbl_800111F8)",
    ]
    MATERIALISED_LA = ["la\t$14,$L19"]
    FOLDED_LA = ["la\t$14,jtbl_800111F8"]

    @staticmethod
    def process(
        lines,
        *,
        fold="jtbl_800111F8",
        three_word=True,
        env_fold=False,
    ):
        env = {
            "MASPSX_DISPATCH_FOLD": ("jtbl_800111F8" if env_fold else ""),
            "MASPSX_THREE_WORD_SYMBOL_STORE": "1" if three_word else "0",
        }
        with patch.dict(os.environ, env):
            return strip_comments(
                MaspsxProcessor(
                    lines,
                    addiu_at=True,
                    three_word_symbol_store=three_word,
                    dispatch_fold_symbol=fold,
                ).process_lines()
            )

    def test_positive_materialised_hi_lo_rewritten(self):
        self.assertEqual(self.FOLDED, self.process(self.MATERIALISED))

    def test_positive_materialised_la_rewritten(self):
        # cc1 emits the hoisted base as `la $r,$L<n>`; maspsx must retarget
        # the operand so GNU as expands it against the shared pool table.
        self.assertEqual(self.FOLDED_LA, self.process(self.MATERIALISED_LA))

    def test_negative_la_gate_off_leaves_local_label(self):
        for kwargs in ({"fold": None}, {"three_word": False}):
            with self.subTest(**kwargs):
                self.assertEqual(
                    self.MATERIALISED_LA,
                    self.process(self.MATERIALISED_LA, **kwargs),
                )

    def test_negative_la_named_symbol_is_never_substituted(self):
        named = ["la\t$2,D_800A1B48"]
        self.assertEqual(named, self.process(named))

    def test_negative_la_non_numeric_local_label_is_never_substituted(self):
        named = ["la\t$14,$Lmain"]
        self.assertEqual(named, self.process(named))

    def test_negative_la_branch_target_line_untouched(self):
        # A bare `$L` label definition and a branch to it are not `la`.
        lines = ["$L19:", "beq\t$2,$zero,$L19"]
        expected = ["$L19:", "beq\t$2,$zero,$L19", "nop"]
        self.assertEqual(expected, self.process(lines))

    def test_environment_flag_selects_materialised_fold(self):
        with patch.dict(
            os.environ, {"MASPSX_DISPATCH_FOLD": "jtbl_800111F8"}
        ):
            res = strip_comments(
                MaspsxProcessor(
                    self.MATERIALISED,
                    addiu_at=True,
                    three_word_symbol_store=True,
                ).process_lines()
            )
        self.assertEqual(self.FOLDED, res)

    def test_negative_fold_symbol_absent_leaves_materialised_untouched(self):
        for kwargs in ({"fold": None}, {"three_word": False}):
            with self.subTest(**kwargs):
                self.assertEqual(
                    self.MATERIALISED,
                    self.process(self.MATERIALISED, **kwargs),
                )

    def test_negative_named_symbol_is_never_substituted(self):
        named = [
            "lui\t$t6,%hi(D_800A1B48)",
            "addiu\t$t6,$t6,%lo(D_800A1B48)",
        ]
        self.assertEqual(named, self.process(named))

    def test_negative_non_numeric_local_label_is_never_substituted(self):
        named = [
            "lui\t$t6,%hi($Lmain)",
            "addiu\t$t6,$t6,%lo($Lmain)",
        ]
        self.assertEqual(named, self.process(named))

    def test_negative_plain_local_label_branch_target_untouched(self):
        branch = ["beq\t$2,$zero,$L30"]
        expected = ["beq\t$2,$zero,$L30", "nop"]
        self.assertEqual(expected, self.process(branch))

    def test_negative_compound_form_still_folds_unchanged(self):
        # Regression: the compound indexed form keeps its existing behavior
        # once the materialised-form extension is present.
        line = "lw\t$2,$L30($2)"
        expected = [
            ".set\tnoat",
            "lui\t$at,%hi(jtbl_800111F8)",
            "addu\t$at,$at,$2",
            "lw\t$2,%lo(jtbl_800111F8)($at)",
            ".set\tat",
        ]
        self.assertEqual(expected, self.process([line]))
