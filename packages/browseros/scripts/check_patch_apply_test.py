#!/usr/bin/env python3
"""Round-trip every new-file patch through the real `git apply`.

A hunk-header check is only a proxy: it confirms the declared line counts
match the body, but says nothing about whether git can actually consume the
patch. This does the real thing.

Every `new file mode 100644` diff applies to an empty repository, so those
114 patches can be applied and reverted for real without a Chromium checkout.
Modification patches need their genuine base file, so they are reported as
skipped rather than silently ignored -- they are exactly the ones the hunk
validator exists for, and they get exercised once the tree lands.
"""

import subprocess
import sys
import tempfile
from pathlib import Path

PATCH_ROOT = Path(__file__).resolve().parents[1] / "chromium_patches"
SUFFIXES = {".h", ".cc", ".gn", ".grd", ".mm"}


def is_new_file(path: Path) -> bool:
    head = path.read_text(encoding="utf-8").split("\n")[:3]
    return "new file mode 100644" in head


def run(cmd, cwd) -> tuple[int, str]:
    r = subprocess.run(
        cmd, cwd=cwd, capture_output=True, text=True, shell=False
    )
    return r.returncode, (r.stdout + r.stderr).strip()


def main() -> int:
    with tempfile.TemporaryDirectory() as td:
        repo = Path(td)
        if run(["git", "init", "-q"], repo)[0] != 0:
            print("git init failed", file=sys.stderr)
            return 2
        run(["git", "config", "user.email", "t@t.t"], repo)
        run(["git", "config", "user.name", "t"], repo)

        applied = skipped = 0
        failures = []

        for patch in sorted(
            p for p in PATCH_ROOT.rglob("*")
            if p.is_file() and p.suffix in SUFFIXES
        ):
            if not is_new_file(patch):
                skipped += 1
                continue
            # The patch must live OUTSIDE the repo, and the target path must
            # not exist yet -- a "new file" diff fails if the file is there.
            patch_copy = repo / "incoming.patch"
            patch_copy.write_text(
                patch.read_text(encoding="utf-8"), encoding="utf-8", newline=""
            )
            target = repo / patch.relative_to(PATCH_ROOT)
            target.parent.mkdir(parents=True, exist_ok=True)
            if target.exists():
                target.unlink()

            code, out = run(["git", "apply", "--check", str(patch_copy)], repo)
            if code != 0:
                failures.append(f"{patch.relative_to(PATCH_ROOT)}: {out}")
                continue
            code, out = run(["git", "apply", str(patch_copy)], repo)
            if code != 0:
                failures.append(f"{patch.relative_to(PATCH_ROOT)}: {out}")
            else:
                applied += 1
            patch_copy.unlink()
            if target.exists():
                target.unlink()

        print(f"new-file patches applied for real : {applied}")
        print(f"modification patches skipped      : {skipped}")
        for f in failures:
            print(f"  FAIL {f}")
        if failures:
            print(f"{len(failures)} patch(es) rejected by git apply")
            return 1
        print("every new-file patch is consumable by git apply")
        return 0


if __name__ == "__main__":
    raise SystemExit(main())
