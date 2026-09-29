# `build/modules/annotate/` — commits-per-feature from `features.yaml`

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

Two files, one module. `annotate.py` reads `build/features.yaml`, asks
git which of each feature's declared files are actually modified or
untracked in the working tree, and creates one git commit per feature
using the feature's `description` as the commit message. `__init__.py`
re-exports the three public names.

The module is exposed as `browseros dev annotate` and is also
reachable as `AnnotateModule` for programmatic use. It writes into the
**Chromium** checkout (`ctx.chromium_src`), not into this repo.

## Contents

| File | What it does |
|---|---|
| `annotate.py` | `load_features()`, `get_modified_files()`, `git_add_and_commit()`, `annotate_features(ctx, feature_filter=None)`, `annotate_single_feature(ctx, name)`, and `class AnnotateModule(CommandModule)`. |
| `__init__.py` | Re-exports `annotate_features`, `annotate_single_feature`, `AnnotateModule`. |

`AnnotateModule.produces = []`, `.requires = []`,
`.description = "Create git commits organized by features"`.

## Rules

**ANN1 — The commit message is the feature `description` alone.** Not
`"<name>: <description>"`. `annotate_features()` does
`commit_message = description`. Do not "improve" this by prefixing the
feature name — downstream tooling and the human review flow expect the
bare description.

**ANN2 — Only files listed in the feature block are considered.**
`get_modified_files()` iterates the feature's `files:` list and runs
`git status --porcelain <path>` on each. A modified file absent from
`features.yaml` is invisible here and will never get a commit.

**ANN3 — A feature with an empty `files:` list is skipped, not
errored.** Same for a feature whose files are all unmodified. Both
increment `features_skipped` rather than raising, so a single stale
feature does not abort a whole annotate pass.

**ANN4 — `annotate_features()` returns `(commits_created,
features_skipped)`, not a bool.** Use `annotate_single_feature()` if
you want a boolean for a named feature.

**ANN5 — This module reuses `run_git_command` from
`../apply/utils.py`,** it does not shell out itself. If you extend the
git interaction, extend `../apply/utils.py`.

**ANN6 — `build_annotate.py` at `build/` root is the dead predecessor.**
It imports `from utils import ...`, which cannot resolve. Do not
synchronise changes between the two; the live implementation is
`annotate.py`.

## Workflows

**Turning in-tree Chromium edits into feature commits**
1. Make the edits under `<chromium_src>/`.
2. Confirm the paths appear under a feature in `build/features.yaml`.
3. `browseros dev annotate` (or `browseros dev annotate <name>` for one
   feature — the name is a positional argument, not `--feature`).
4. Check `git log` in `<chromium_src>/` — one commit per feature with
   changes, skipped features reported in the summary.

**Adding a new feature to the manifest**
1. Add a block to `build/features.yaml` with `description:` (this
   becomes the commit message) and `files:`.
2. If the description must start with a conventional-commit prefix,
   `../feature/validation.py::VALID_PREFIXES` defines the accepted set.

**Using it from a script**
1. Build a `Context` (`../common/resolver.py::resolve_config()` or one
   of the `create_*_context()` helpers in `../cli/`).
2. Call `annotate_features(ctx)` or `annotate_single_feature(ctx, name)`.
3. Do not call `AnnotateModule().execute()` without `validate()` first.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../feature/AGENTS.md`](../feature/AGENTS.md) — the module that writes `features.yaml`.
- [`../../features.yaml`](../../features.yaml) — the manifest this reads.
- [`../apply/AGENTS.md`](../apply/AGENTS.md) — home of `run_git_command()`.
- [`../../cli/dev.py`](../../cli/dev.py) — the `browseros dev annotate` command.
