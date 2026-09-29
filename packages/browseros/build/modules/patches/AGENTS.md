# `build/modules/patches/` — pipeline patch application

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

The two `CommandModule` classes the build pipeline itself uses to
apply patches. They are deliberately thin wrappers: `patches.py` walks
`chromium_patches/` and calls into `../apply/`, and `series_patches.py`
applies the GNU-Quilt `series_patches/` directory in series order. The
interactive, feature-scoped and commit-scoped apply flows live in
`../apply/` and are only reachable through `browseros dev apply`.

## Contents

| File | What it does |
|---|---|
| `patches.py` | `class PatchesModule(CommandModule)` — `produces = []`, `requires = []`, `description = "Apply BrowserOS patches to Chromium"`; plus `apply_patches_impl(ctx, interactive=False)`, the standalone callable form. |
| `series_patches.py` | `class SeriesPatchesModule(CommandModule)` — `description = "Apply series-based patches (GNU Quilt format)"`; plus `parse_series()`, `get_series_files()`, `apply_single_patch()`, `apply_series_patches_impl()`. |

The area has no `__init__.py` — PEP 420 namespace package.

## Rules

**PAT1 — These are build-pipeline modules, not dev tools.** Only
`PatchesModule` and `SeriesPatchesModule` are registered in
`../../cli/build.py::AVAILABLE_MODULES`. The five richer apply flows
in `../apply/` are `browseros dev` commands and must not appear in a
`config/*.yaml` `modules:` list.

**PAT2 — `series_patches/` is ordered and last-resort.** `parse_series()`
honours the `series` file order; that ordering is load-bearing for
patches that depend on earlier ones. When a quilt patch fails, the
whole series stops — that is intentional, unlike `../apply/apply_force.py`
which writes `.rej` files and continues.

**PAT3 — Both modules are non-interactive by default.**
`apply_patches_impl(ctx, interactive=False)` is how the pipeline calls
it. Interactive conflict resolution belongs to `browseros dev apply`,
never to a CI build.

**PAT4 — Both are idempotent re-runs only against a clean-ish tree.**
They do `git apply` against `<chromium_src>`; a tree with uncommitted
edits from a previous run will conflict. `../setup/clean.py` exists for
this.

**PAT5 — `features.yaml` is not consulted here.** Neither module reads
the feature manifest — they apply everything on disk. Feature-scoped
application is `../apply/apply_feature.py`. Do not add feature
filtering to the pipeline path.

**PAT6 — `patches` and `series_patches` are separate pipeline entries.**
`config/release.linux.yaml` and the macOS configs list `series_patches`
immediately before `patches`. Preserve that order; the quilt series
must land before the bulk patch set.

## Workflows

**Applying patches as part of a build**
1. The config lists `series_patches` then `patches`.
2. Both run non-interactively; a failure aborts the pipeline with a
   `ValidationError` or an exception from `git apply`.
3. Follow with `chromium_replace` and `string_replaces` for the
   wholesale file and branding changes.

**Debugging a series-patch failure**
1. `parse_series()` lists the patches in the order they will be
   applied; the failing one is the one whose path is named in the error.
2. Check whether an earlier patch in the series already applied and
   changed the context.
3. Fall back to `browseros dev apply force` to get all `.rej` files and
   resolve them together.

**Adding a patch to the quilt series**
1. Drop the `.patch` into `packages/browseros/series_patches/`.
2. Append its path to the `series` file in that directory.
3. Do not add a bare filename with no `./` prefix — `parse_series()`
   resolves paths relative to the series directory.

**Switching a pipeline off quilt patches**
1. Remove `series_patches` from the `modules:` list in
   `../../config/release.*.yaml`.
2. Confirm the remaining patches still apply in order without it.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../apply/AGENTS.md`](../apply/AGENTS.md) — the interactive apply flows and shared plumbing.
- [`../setup/AGENTS.md`](../setup/AGENTS.md) — `clean`, the module that resets the tree.
- [`../../../series_patches/`](../../../series_patches/) — the quilt patch directory.
- [`../../config/AGENTS.md`](../../config/AGENTS.md) — the recipes that list these two modules.
