#!/usr/bin/env python3
"""Tests for the unified-diff hunk-header validator.

The validator is only useful if it is right in both directions: it must catch
a header that disagrees with its body, and it must stay silent on all 299 real
patches. Both are covered here, because an off-by-one that reports every file
as broken is worse than no validator at all.
"""

import tempfile
import unittest
from pathlib import Path

from check_patch_headers import check, main

REPO_PATCHES = Path(__file__).resolve().parents[1] / "chromium_patches"


def write_patch(body: str) -> Path:
    d = Path(tempfile.mkdtemp())
    p = d / "sample.cc"
    p.write_text(body, encoding="utf-8")
    return p


class HunkHeaderTest(unittest.TestCase):
    def test_new_file_hunk_with_correct_count_is_clean(self):
        p = write_patch(
            "diff --git a/x.cc b/x.cc\n"
            "new file mode 100644\n"
            "--- /dev/null\n"
            "+++ b/x.cc\n"
            "@@ -0,0 +1,3 @@\n"
            "+one\n"
            "+two\n"
            "+three\n"
        )
        self.assertEqual(check(p), [])

    def test_count_too_low_is_reported(self):
        p = write_patch(
            "--- /dev/null\n"
            "+++ b/x.cc\n"
            "@@ -0,0 +1,2 @@\n"
            "+one\n"
            "+two\n"
            "+three\n"
        )
        problems = check(p)
        self.assertEqual(len(problems), 1)
        self.assertIn("declares -0 +2", problems[0])
        self.assertIn("body has -0 +3", problems[0])

    def test_trailing_newline_is_not_a_context_line(self):
        # A file ending in "\n" splits into a trailing "" that must not be
        # counted. This was a real bug: it flagged every patch in the repo.
        p = write_patch(
            "--- /dev/null\n+++ b/x.cc\n@@ -0,0 +1,2 @@\n+one\n+two\n"
        )
        self.assertEqual(check(p), [])

    def test_file_without_trailing_newline_is_clean(self):
        p = write_patch(
            "--- /dev/null\n+++ b/x.cc\n@@ -0,0 +1,2 @@\n+one\n+two"
        )
        self.assertEqual(check(p), [])

    def test_omitted_count_means_one_line(self):
        p = write_patch("--- a/x\n+++ b/x\n@@ -5 +5 @@\n-old\n+new\n")
        self.assertEqual(check(p), [])

    def test_implicit_one_against_two_lines_is_reported(self):
        p = write_patch("--- a/x\n+++ b/x\n@@ -5 +5 @@\n-old\n+new\n+extra\n")
        self.assertEqual(len(check(p)), 1)

    def test_context_lines_count_on_both_sides(self):
        p = write_patch(
            "--- a/x\n"
            "+++ b/x\n"
            "@@ -5,3 +5,4 @@\n"
            " context\n"
            "-removed\n"
            "+added\n"
            "+also\n"
            " tail\n"
        )
        self.assertEqual(check(p), [])

    def test_removed_line_missing_from_count_is_reported(self):
        # Body " context" / "-removed" / " tail" -> old=2, new=2,
        # but the header claims old=1.
        p = write_patch(
            "--- a/x\n+++ b/x\n@@ -5,1 +5,2 @@\n context\n-removed\n tail\n"
        )
        self.assertEqual(len(check(p)), 1)

    def test_blank_context_line_counts_both_sides(self):
        p = write_patch(
            "--- a/x\n+++ b/x\n@@ -5,2 +5,2 @@\n first\n\n"
        )
        self.assertEqual(check(p), [])

    def test_no_newline_marker_is_skipped(self):
        p = write_patch(
            "--- a/x\n+++ b/x\n@@ -5,1 +5,1 @@\n-old\n+new\n"
            "\\ No newline at end of file\n"
        )
        self.assertEqual(check(p), [])

    def test_multiple_hunks_each_checked(self):
        # Hunk 1 body " a" / "+b"  -> old=1, new=2
        # Hunk 2 body "-c" / "+d"  -> old=1, new=1
        p = write_patch(
            "--- a/x\n"
            "+++ b/x\n"
            "@@ -1,1 +1,2 @@\n"
            " a\n"
            "+b\n"
            "@@ -10,1 +10,1 @@\n"
            "-c\n"
            "+d\n"
        )
        self.assertEqual(check(p), [])

    def test_second_hunk_bad_count_is_reported(self):
        p = write_patch(
            "--- a/x\n"
            "+++ b/x\n"
            "@@ -1,1 +1,2 @@\n"
            " a\n"
            "+b\n"
            "@@ -10,1 +10,2 @@\n"
            "-c\n"
            "+d\n"
        )
        problems = check(p)
        self.assertEqual(len(problems), 1)
        # Only the second hunk is bad: -c/+d is 1 old and 1 new, not 2 new.
        self.assertIn("declares -1 +2 but body has -1 +1", problems[0])


class RealRepoTest(unittest.TestCase):
    def test_every_shipped_patch_is_self_consistent(self):
        """The regression guard for the false-positive bug.

        If this fails, either a patch really did drift or the validator
        regressed. Do not "fix" it by loosening the arithmetic.
        """
        diffs = [
            p
            for p in REPO_PATCHES.rglob("*")
            if p.is_file() and p.suffix in {".h", ".cc", ".gn", ".grd", ".mm"}
        ]
        self.assertGreater(len(diffs), 250, "patch discovery broke")
        problems = [p for d in diffs for p in check(d)]
        self.assertEqual(problems, [], "\n".join(problems))

    def test_main_exits_zero_on_a_clean_tree(self):
        self.assertEqual(main_with(str(REPO_PATCHES)), 0)


def main_with(root: str) -> int:
    import sys

    saved = sys.argv
    sys.argv = ["check_patch_headers.py", root]
    try:
        return main()
    finally:
        sys.argv = saved


if __name__ == "__main__":
    unittest.main()
