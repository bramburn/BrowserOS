# `build/modules/apply/` — apply patches into the Chromium tree

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

Everything that takes patch files out of `chromium_patches/` and
applies them to `<chromium_src>/`. Five entry points, five CLI names
under `browseros dev apply`: `all`, `feature`, `patch`, `force`,
`changed`. `utils.py` is the shared git plumbing (`run_git_command`,
`parse_diff_output`, `apply_single_patch`, `handle_patch_conflict`,
marker-file writers) and `common.py` is a second, thinner layer used
by the "apply everything" path.

Two ways to reach a patch: `all` / `force` walk
`chromium_patches/` on disk, while `feature` / `changed` select by
feature name or by commit range.

## Contents

| File | What it does |
|---|---|
| `utils.py` | `FileOperation` enum, `FilePatch` dataclass, `GitError`, `run_git_command()`, `validate_git_repository()`, `validate_commit_exists()`, `file_exists_in_commit()`, `reset_file_to_commit()`, `parse_diff_output()`, `write_patch_file()`, `create_deletion_marker()`, `create_binary_marker()`, `apply_single_patch()`, `handle_patch_conflict()`, `create_git_commit()`, `get_commit_info()`, `prompt_yes_no()`, `log_extraction_summary()`, `log_apply_summary()`. |
| `common.py` | `find_patch_files()`, `apply_single_patch()`, `create_patch_commit()`, `process_patch_list()` — the older helper layer still used by `apply_all`. |
| `apply_all.py` | `apply_all_patches()` + `class ApplyAllModule` (`description = "Apply all patches from chromium_patches/"`). |
| `apply_force.py` | `apply_patch_with_reject()`, `apply_all_force()` + `class ApplyForceModule` — non-interactive; writes `.rej` files instead of stopping. |
| `apply_feature.py` | `apply_feature_patches()` + `class ApplyFeatureModule` — applies only the files listed for one feature in `features.yaml`. |
| `apply_patch.py` | `apply_single_file_patch()` — one file; a helper, not a registered module. |
| `apply_changed.py` | `ChangeType` enum, `PatchChange`, `get_git_root()`, `get_changed_files_in_commit()`, `get_changed_files_in_range()`, `filter_patch_changes()`, `format_confirmation_prompt()`, `apply_changed_patches()` + `class ApplyChangedModule`. |
| `__init__.py` | Re-exports the five module classes plus the single-file helper. |

All four registered modules use `produces = []` and `requires = []`.

## Rules

**AP1 — Patch paths mirror Chromium paths exactly.** A patch at
`chromium_patches/chrome/browser/foo/bar.cc` targets
`<chromium_src>/chrome/browser/foo/bar.cc`. `write_patch_file()` and
`find_patch_files()` both depend on this; renaming or nesting a patch
to group it differently breaks application.

**AP2 — `--force` is the conflict strategy, not `--force-overwrite`.**
`apply_force.py` applies every patch and records failures in `.rej`
files beside the target. Never make it silently overwrite.

**AP3 — `apply_all` and `apply_force` are different code paths on
purpose.** `apply_all` goes through `common.py::process_patch_list()`;
`apply_force` uses its own `apply_patch_with_reject()`. If you fix a
conflict-handling bug, check whether `../extract/utils.py` has the same
function and fix both — `apply/utils.py` and `extract/utils.py` are
near-duplicates by design (extract is the write side, apply is the
read/apply side).

**AP4 — Deletions and binaries need marker files, not empty patches.**
`create_deletion_marker()` and `create_binary_marker()` write the
sentinel the Chromium tree expects. Do not replace them with a zero
byte `.patch`.

**AP5 — `apply_changed` prompts before touching anything.**
`format_confirmation_prompt()` is not decorative; it is the review
gate for a commit range. Keep the prompt on the default path.

**AP6 — These modules never write outside `<chromium_src>` and
`chromium_patches/`.** They are safe to re-run, and the docs say so;
keep `apply_single_patch()` idempotent.

## Workflows

**Applying everything before a build**
1. `browseros dev apply all` (or `browseros build -m patches`, which
   runs `PatchesModule` from `../patches/patches.py`).
2. On a conflict, the pipeline stops; re-run with
   `browseros dev apply force` to get all `.rej` files at once.
3. Fix the offending `.patch` under `../../../chromium_patches/`.

**Applying one feature's patches**
1. Confirm the feature exists in `build/features.yaml`.
2. `browseros dev apply feature <name>` (the name is a positional
   argument, not `--feature`).
3. Verify with `git -C <chromium_src> status`.

**Applying patches touched by a specific commit**
1. `browseros dev apply changed --reset-to <base> --commit <sha>`, or
   `--range-start <sha> --range-end <sha>` for a range. `--reset-to` is
   required.
2. Review the confirmation prompt; answer `n` to abort.
3. Re-run after fixing any rejected hunks.

**Adding a new apply strategy**
1. Add `<strategy>.py` implementing `CommandModule`.
2. Export it from `__init__.py`.
3. Wire a `@apply_app.command(name=...)` in `../../cli/dev.py`.
4. Add `<strategy>_test.py` beside it if there is logic to isolate.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../extract/AGENTS.md`](../extract/AGENTS.md) — the write-side mirror of this area.
- [`../patches/AGENTS.md`](../patches/AGENTS.md) — the `patches` / `series_patches` pipeline modules.
- [`../feature/AGENTS.md`](../feature/AGENTS.md) — where `features.yaml` is read/written.
- [`../../../chromium_patches/AGENTS.md`](../../../chromium_patches/AGENTS.md) — the patch tree.
