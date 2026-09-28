#!/usr/bin/env python3
"""Clean module for BrowserOS build system"""

import subprocess

from ...common.module import CommandModule, ValidationError
from ...common.context import Context
from ...common.utils import (
    run_command,
    log_info,
    log_success,
    log_warning,
    safe_rmtree,
)


class CleanModule(CommandModule):
    produces = []
    requires = []
    description = "Clean build artifacts and reset git state"

    def validate(self, ctx: Context) -> None:
        if not ctx.chromium_src.exists():
            raise ValidationError(f"Chromium source not found: {ctx.chromium_src}")

    def execute(self, ctx: Context) -> None:
        log_info("🧹 Cleaning build artifacts...")

        out_path = ctx.chromium_src / ctx.out_dir
        if out_path.exists():
            safe_rmtree(out_path)
            log_success("Cleaned build directory")

        if self._tree_is_pristine(ctx):
            log_success(
                "Tree is pristine (no out/, no patch residue) — "
                "skipping destructive git reset/clean"
            )
        else:
            log_info("\n🔀 Resetting git branch and removing tracked files...")
            self._git_reset(ctx)

        log_info("\n🧹 Cleaning Sparkle build artifacts...")
        self._clean_sparkle(ctx)

    def _clean_sparkle(self, ctx: Context) -> None:
        sparkle_dir = ctx.get_sparkle_dir()
        if sparkle_dir.exists():
            safe_rmtree(sparkle_dir)
        log_success("Cleaned Sparkle build directory")

    def _tree_is_pristine(self, ctx: Context) -> bool:
        """True when there is nothing for _git_reset to undo.

        `git clean -fdx chrome/ components/ third_party/` deletes the entire
        DEPS-managed third_party tree — the bulk of the ~30 GB checkout. On a
        freshly fetched tree there is no patch residue to remove, so running it
        just forces the following `gclient sync` to re-download all of it.
        EXECUTION_ORDER in cli/build.py puts `clean` before `git_setup`, so
        that cost otherwise lands on every first setup run.

        Pristine means: no previous build output, and no local modifications
        under the paths the BrowserOS patches touch.
        """
        if (ctx.chromium_src / ctx.out_dir).exists():
            return False

        result = subprocess.run(
            ["git", "status", "--porcelain",
             "chrome/", "components/", "third_party/"],
            cwd=ctx.chromium_src, text=True, capture_output=True,
        )
        if result.returncode != 0:
            # Can't prove it's clean, so fall back to the full reset.
            log_warning(
                f"⚠️  git status failed (exit {result.returncode}); "
                f"assuming the tree is dirty and running the full clean."
            )
            return False
        return not result.stdout.strip()

    def _git_reset(self, ctx: Context) -> None:
        run_command(["git", "reset", "--hard", "HEAD"], cwd=ctx.chromium_src)

        # Reset all dirty submodules so gclient sync doesn't choke
        log_info("🧹 Resetting dirty submodules...")
        run_command(
            ["git", "submodule", "foreach", "--recursive",
             "git checkout -- . && git clean -fd"],
            cwd=ctx.chromium_src,
        )

        log_info("🧹 Running git clean with exclusions...")
        run_command(
            [
                "git",
                "clean",
                "-fdx",
                "chrome/",
                "components/",
                "third_party/",
                "--exclude=build_tools/",
                "--exclude=uc_staging/",
                "--exclude=buildtools/",
                "--exclude=tools/",
                "--exclude=build/",
            ],
            cwd=ctx.chromium_src,
        )
        log_success("Git reset and clean complete")
