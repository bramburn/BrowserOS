# `build/modules/extract/` — turn Chromium git history into patch files

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

The write side of the patch workflow. Given a commit, a range of
commits, or a single file, it produces the corresponding files under
`chromium_patches/` using `git show --format= --patch`, and offers to
apply each patch back into the working tree. `common.py` resolves the
base commit and writes the patch set; `utils.py` is the git/diff
plumbing.

`utils.py` is a near-duplicate of `../apply/utils.py` — same functions,
same names, with a few extract-only additions. They are separate files
on purpose (extract writes, apply reads), but they drift.

## Contents

| File | What it does |
|---|---|
| `utils.py` | `FileOperation`, `FilePatch`, `GitError`, `run_git_command()`, `validate_git_repository()`, `validate_commit_exists()`, `get_commit_changed_files_with_status()`, `get_commit_changed_files()`, `parse_diff_output()`, `write_patch_file()`, `create_deletion_marker()`, `create_binary_marker()`, `apply_single_patch()`, `handle_patch_conflict()`, `create_git_commit()`, `get_commit_info()`, `prompt_yes_no()`, `log_extraction_summary()`, `log_apply_summary()`. |
| `common.py` | `resolve_base_commit()`, `check_overwrite()`, `write_patches()`, `extract_with_base()`. |
| `extract_commit.py` | `extract_single_commit()` + `class ExtractCommitModule`. |
| `extract_range.py` | `get_range_changed_files_with_status()`, `extract_commit_range()`, `extract_commits_individually()` + `class ExtractRangeModule`. |
| `extract_patch.py` | `extract_single_file_patch()` — helper, not a registered module. |
| `extract_base_default_test.py` | Unit tests for the base-commit defaulting logic (uses a `SimpleNamespace` fake context). |
| `__init__.py` | Re-exports the two module classes and the single-file helper. |

All registered modules use `produces = []` and `requires = []`.

## Rules

**EX1 — Patch output paths mirror Chromium paths, and that is the
whole contract.** `write_patch_file(ctx, file_path, ...)` writes to
`ctx.get_patch_path_for_file(file_path)`, i.e.
`chromium_patches/<chromium path>`. Any regrouping breaks
`../apply/`.

**EX2 — Base-commit defaulting lives in `common.py::resolve_base_commit()`.**
When no `--base` is passed it derives one (from the repo's pinned
`BASE_COMMIT`, or the parent of the first commit). Do not re-derive it
in an extract_* module; `extract_base_default_test.py` covers the
behaviour.

**EX3 — `check_overwrite()` guards existing patch files.** Extraction
refuses to clobber a patch that is already on disk unless the user
confirms. Keep that gate — silently overwriting hand-edited patches is
how patches get lost.

**EX4 — Deletions and binaries get marker files, not empty patches.**
`create_deletion_marker()` and `create_binary_marker()` write the
sentinel the Chromium tree expects. `git show` emits no diff body for
either case.

**EX5 — `extract_commits_individually()` and `extract_commit_range()`
are different outputs.** The former produces one patch set per commit;
the latter produces a single squashed set. Do not merge them.

**EX6 — Keep `utils.py` and `../apply/utils.py` behaviourally in sync
when you change shared functions** (`parse_diff_output`,
`apply_single_patch`, `handle_patch_conflict`, marker writers). They
are duplicated, not shared, so nothing enforces this for you.

## Workflows

**Extracting a commit's changes into patches**
1. `browseros dev extract commit <sha>`.
2. Confirm the overwrite prompt if patches already exist.
3. Add the new paths to a feature block in `build/features.yaml` —
   extraction does not do this for you (pass `--feature` to be
   prompted for it).
4. Apply with `browseros dev apply feature <name>`.

**Extracting a range**
1. `browseros dev extract range <start> <end>` produces one patch set per
   commit by default; add `--squash` for a single squashed set.
2. Review the summary printed by `log_extraction_summary()`.
3. Resolve any conflicts raised by `handle_patch_conflict()`.

**Extracting one file's patch**
1. `browseros dev extract patch <chromium-path>` (or call
   `extract_single_file_patch()`).
2. Useful when a commit touched unrelated churn you do not want.

**After extraction**
1. `browseros dev apply all` to verify the patches actually apply.
2. `browseros dev annotate` to create the per-feature commits.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../apply/AGENTS.md`](../apply/AGENTS.md) — the mirror area; shares function names with `utils.py`.
- [`../feature/AGENTS.md`](../feature/AGENTS.md) — writes the `features.yaml` entries extraction implies.
- [`../../cli/dev.py`](../../cli/dev.py) — the `browseros dev extract` commands.
- [`../../../chromium_patches/AGENTS.md`](../../../chromium_patches/AGENTS.md) — where the output lands.
