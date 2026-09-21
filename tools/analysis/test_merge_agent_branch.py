#!/usr/bin/env python3
"""Unit tests for tools/analysis/merge_agent_branch.py's conflict resolvers.

These cover the three shapes that actually recur when merging decompilation
agent branches:

1. both sides carve different leaves out of the same `asm` run (concatenate in
   offset order);
2. one side refines the other's `asm` boundary into a `c` leaf (the `c` wins);
3. comment-only disagreement over the same carve (keep ours).

Run: python3 tools/analysis/test_merge_agent_branch.py
"""

from __future__ import annotations

import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from merge_agent_branch import (  # noqa: E402
    entry_groups,
    resolve_additive,
    resolve_yaml_side,
)


class EntryGroupsTests(unittest.TestCase):
    def test_comments_attach_to_following_entry(self) -> None:
        text = "# about A\n- [0x10, c, a]\n# about B\n- [0x20, asm]\n"
        groups = entry_groups(text)
        self.assertEqual(len(groups), 2)
        self.assertIn("# about A", "".join(groups[0]))
        self.assertIn("# about B", "".join(groups[1]))


class ResolveYamlSideTests(unittest.TestCase):
    def test_disjoint_carves_merge_in_offset_order(self) -> None:
        ours = "# leaf A\n- [0x177D0, c, func_A]\n- [0x177F8, asm]\n"
        theirs = "# leaf B\n- [0x1254C, c, func_B]\n- [0x125E0, asm]\n"
        out = resolve_yaml_side(ours, theirs)
        self.assertLess(out.index("0x1254C"), out.index("0x177D0"))
        self.assertIn("func_A", out)
        self.assertIn("func_B", out)

    def test_refinement_replaces_asm_boundary(self) -> None:
        # Ours ends a leaf then resumes asm at 0x5AAE8; theirs carves a new
        # leaf starting exactly at 0x5AAE8.
        ours = (
            "# leaf A\n- [0x5AA5C, c, func_A]\n- [0x5AAE8, asm]\n"
        )
        theirs = (
            "# leaf B\n- [0x5AAE8, c, func_B]\n- [0x5AB18, asm]\n"
        )
        out = resolve_yaml_side(ours, theirs)
        self.assertNotIn("0x5AAE8, asm", out)
        self.assertIn("- [0x5AAE8, c, func_B]", out)
        self.assertIn("func_A", out)

    def test_comment_only_disagreement_keeps_ours(self) -> None:
        ours = "# ours: era -O2 -G0\n"
        theirs = "# theirs: era -O2 -G8\n"
        self.assertEqual(resolve_yaml_side(ours, theirs), ours)

    def test_single_side_with_entries_is_taken(self) -> None:
        ours = "# comment only\n"
        theirs = "# leaf\n- [0x10, c, func_X]\n- [0x20, asm]\n"
        out = resolve_yaml_side(ours, theirs)
        # The entries come from the side that has them; a stray comment from
        # the other side is harmless and may be retained.
        self.assertIn("- [0x10, c, func_X]", out)
        self.assertIn("- [0x20, asm]", out)
        self.assertEqual(out.count("c, func_X"), 1)


class ResolveAdditiveTests(unittest.TestCase):
    def test_concatenates_both_sides(self) -> None:
        text = "top\n<<<<<<< HEAD\nours\n=======\ntheirs\n>>>>>>> b\ntail\n"
        out = resolve_additive(text)
        self.assertEqual(out, "top\nours\ntheirs\ntail\n")
        self.assertNotIn("<<<<<<<", out)


if __name__ == "__main__":
    unittest.main(verbosity=2)
