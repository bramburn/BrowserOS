"""Validate the hunk headers of every BrowserOS patch diff.

A unified diff's `@@ -a,b +c,d @@` header declares how many lines the hunk
spans. If the real body has a different number of +, - or space-prefixed lines,
`git apply` fails with "corrupt patch at line N" -- usually far from the real
mistake. This checks the arithmetic before the tree is even needed.

Usage:  python scripts/check_patch_headers.py [root]
"""
import re
import sys
from pathlib import Path

HUNK = re.compile(r"^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@")


def check(path: Path) -> list[str]:
    problems = []
    lines = path.read_text(encoding="utf-8").split("\n")

    old_seen = new_seen = 0
    expect_old = expect_new = None
    start = 0

    for i, line in enumerate(lines, 1):
        m = HUNK.match(line)
        if m:
            if expect_old is not None:
                if old_seen != expect_old or new_seen != expect_new:
                    problems.append(
                        f"{path}:{start}: hunk declares "
                        f"-{expect_old} +{expect_new} but body has "
                        f"-{old_seen} +{new_seen}"
                    )
            start = i
            expect_old = int(m.group(2) or 1)
            expect_new = int(m.group(4) or 1)
            old_seen = new_seen = 0
            continue

        if expect_old is None:
            continue
        if line.startswith("\\"):  # "\ No newline at end of file"
            continue
        # A file ending in a newline yields a trailing "" from split("\n").
        # That is not a diff line, so it must not be counted as context.
        if line == "" and i == len(lines):
            continue

        tag = line[:1]
        if tag == "+":
            new_seen += 1
        elif tag == "-":
            old_seen += 1
        elif tag == " " or line == "":
            # An empty line is a context line whose leading space was trimmed.
            old_seen += 1
            new_seen += 1
        else:
            # Header/footer or next diff: close out this hunk.
            if old_seen != expect_old or new_seen != expect_new:
                problems.append(
                    f"{path}:{start}: hunk declares "
                    f"-{expect_old} +{expect_new} but body has "
                    f"-{old_seen} +{new_seen}"
                )
            expect_old = None

    if expect_old is not None and (
        old_seen != expect_old or new_seen != expect_new
    ):
        problems.append(
            f"{path}:{start}: hunk declares -{expect_old} +{expect_new} "
            f"but body has -{old_seen} +{new_seen}"
        )
    return problems


def main() -> int:
    root = Path(sys.argv[1] if len(sys.argv) > 1 else "chromium_patches")
    if not root.is_dir():
        print(f"no such directory: {root}", file=sys.stderr)
        return 2

    diffs = sorted(
        p
        for p in root.rglob("*")
        if p.is_file() and p.suffix in {".h", ".cc", ".gn", ".grd", ".mm"}
    )
    all_problems = []
    for d in diffs:
        all_problems.extend(check(d))

    print(f"checked {len(diffs)} patch files under {root}")
    for p in all_problems:
        print(f"  BAD  {p}")
    print("all hunk headers consistent" if not all_problems
          else f"{len(all_problems)} problem(s)")
    return 1 if all_problems else 0


if __name__ == "__main__":
    raise SystemExit(main())
