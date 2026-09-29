# `build/modules/resources/` — copy files into the Chromium tree

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

The three phase-2 modules that mutate `<chromium_src>` before
compilation, each driven by a different mechanism:
`resources.py` copies files declared in
`../../config/copy_resources.yaml`; `chromium_replace.py` overwrites
Chromium files wholesale from `chromium_files/`; `string_replaces.py`
applies branding regex substitutions to two `.grd` files.

The area has no `__init__.py` — PEP 420 namespace package.

## Contents

| File | What it does |
|---|---|
| `resources.py` | `class ResourcesModule` — `description = "Copy resources (icons, extensions) to Chromium"`. `copy_resources_impl(ctx, commit_each=False)`, `commit_resource_copy()`. Reads `ctx.get_copy_resources_config()`. |
| `chromium_replace.py` | `class ChromiumReplaceModule` — `description = "Replace Chromium source files with custom versions"`. `replace_chromium_files_impl(ctx, replacements=None)`, `add_file_to_replacements()`. Reads `ctx.get_chromium_replace_files_dir()`. |
| `string_replaces.py` | `class StringReplacesModule` — `description = "Apply branding string replacements in Chromium"`. `apply_string_replacements_impl(ctx)`. Module-level `branding_replacements` list and `target_files` list. |

All three use `produces = []` and `requires = []`.

## Rules

**RES1 — `resources.py` has no directory scan; it iterates
`copy_operations` only.** A file in `../../resources/` that no entry in
`../../config/copy_resources.yaml` names is never copied. It will not
error — it will simply not ship.

**RES2 — `copy_operations` filters are `build_type`, `os`, `arch`.**
`os` values are case-sensitive and come from `get_platform()`
(`windows` / `macos` / `linux`); `arch` is matched against
`ctx.architecture` (`x64` / `arm64`). A mismatch logs a skip line and
continues — it is not an error.

**RES3 — A missing source is a warning, not a failure.** All three
`type:` branches log `log_warning` when the source is absent. Do not
"fix" this to raise: a build for macOS should not fail because a
Linux-only icon directory is absent.

**RES4 — `chromium_replace.py` DOES raise on a missing destination.**
`dst_file` not existing in `chromium_src` is a `FileNotFoundError`,
because it means the Chromium version drifted and the replacement no
longer has a target. That asymmetry with rule RES3 is deliberate.

**RES5 — `.debug` / `.release` suffixes in `chromium_files/` select by
build type.** `foo.cc.release` overwrites `foo.cc` in a release build
and is skipped in debug; if a variant exists for the current build
type, the unsuffixed file is skipped in its favour.

**RES6 — `string_replaces.py` touches exactly two files.**
`chrome/app/chromium_strings.grd` and
`chrome/app/settings_chromium_strings.grdp`. Adding a third target
means adding it to `target_files`; adding a fourth regex means adding
it to `branding_replacements` in order — the list is applied
sequentially, so `(Google)(?! Play)` must run before the bare `Google`
rule or Play would be rewritten.

**RES7 — `commit_each=True` is for the dev annotate flow, not the
build.** `ResourcesModule.execute()` calls `copy_resources_impl(ctx,
commit_each=False)`. Keep the flag off in the pipeline; per-resource
`git add -A` commits in a release build are wrong.

**RES8 — `add_file_to_replacements()` is broken and unused.** It does
`from context import BuildContext` — there is no top-level `context`
module and no `BuildContext` class. Nothing calls it. Fix or delete
rather than invoking it.

## Workflows

**Adding an asset to the shipped product**
1. Put it under `../../../../resources/<area>/`.
2. Add a `copy_operations:` entry in
   `../../config/copy_resources.yaml` with the right `type:` and
   `os:` / `arch:` / `build_type:` filters.
3. Run `browseros build -m resources` and confirm the file landed under
   `<chromium_src>/`.

**Replacing a whole Chromium file**
1. Put the replacement at
   `../../../../chromium_files/<chromium path>` (optionally
   `.debug` / `.release` suffixed).
2. `browseros build -m chromium_replace`.
3. A `FileNotFoundError` here means the Chromium file moved — update
   `chromium_patches/` in the same change.

**Changing the branding strings**
1. Edit `branding_replacements` in `string_replaces.py`, keeping the
   ordering rules in RES6.
2. Both `.grd` targets are processed in one pass; there is no
   per-file configuration.
3. Verify with `git -C <chromium_src> diff chrome/app/chromium_strings.grd`.

**Debugging "resource not copied"**
1. Check the config entry's `os` / `arch` / `build_type` against the
   current build.
2. Check the `source:` path resolves from `ctx.root_dir`, not from the
   config file's directory.
3. A `type: files` entry uses `glob` — verify the pattern actually
   matches (`resources/icons/*.png`, not `resources/icons/`).

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../../config/AGENTS.md`](../../config/AGENTS.md) — `copy_resources.yaml`, this area's input.
- [`../../../../resources/AGENTS.md`](../../../resources/AGENTS.md) — the source tree being copied.
- [`../../../../chromium_files/AGENTS.md`](../../../chromium_files/AGENTS.md) — the replacement tree.
- [`../extensions/AGENTS.md`](../extensions/AGENTS.md) — the adjacent, manifest-driven extension bundling.
