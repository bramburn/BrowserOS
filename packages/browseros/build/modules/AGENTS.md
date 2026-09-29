# `build/modules/` — the 14 build-step areas

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

Every step the build pipeline can run, grouped into 14 areas. A step is
a class extending `CommandModule` (`../common/module.py`) with
`produces`, `requires`, `description`, `validate(ctx)` and
`execute(ctx)`. `../cli/build.py` imports these classes and registers
them in `AVAILABLE_MODULES` under a snake_case name; a
`build/config/*.yaml` recipe names them in its ordered `modules:` list.

This directory has **no `__init__.py`** — it is a PEP 420 namespace
package. Five of the 14 areas (`setup/`, `patches/`, `resources/`,
`sign/`, `package/`) also have no `__init__.py`; the other nine do. Both
forms work, so the inconsistency is cosmetic, but it is deliberate
enough to be worth knowing before you add a file.

## Contents

```
modules/
├── setup/       clean, git_setup, sparkle_setup, configure
├── patches/     patches (chromium_patches/), series_patches (GNU quilt)
├── apply/       apply_all, apply_feature, apply_patch, apply_changed, apply_force
├── extract/     extract_commit, extract_range, extract_patch
├── feature/     features.yaml manipulation + validation
├── annotate/    commits-per-feature from features.yaml
├── resources/   resources (copy), chromium_replace, string_replaces
├── extensions/  bundled_extensions (CDN CRX manifest)
├── compile/     compile (autoninja), universal_build (macOS universal)
├── sign/        sign_macos, sign_windows, sign_linux, sparkle_sign
├── package/     package_macos, package_windows, package_linux, merge
├── ota/         server_ota (appcast + upload; driven by `browseros ota`)
├── release/     list, appcast, github, publish, download
└── storage/     upload (R2), download_resources (R2)
```

Each area has its own `AGENTS.md`. Start there.

## Rules

**MOD1 — A step is a `CommandModule` subclass, registered in
`../cli/build.py`.** A free function in this tree is a helper, not a
pipeline step. The registry key is the name used in every YAML
`modules:` list.

**MOD2 — `validate()` raises `ValidationError`; `execute()` raises on
failure.** Never swallow an exception in `execute()` to let the pipeline
continue with a half-built state. Upload failures inside
`compile/universal.py` are the one deliberate exception and they are
logged as warnings with an explicit comment.

**MOD3 — Declare `produces` and `requires` even when you are unsure.**
`universal.py` and `windows.py` sign modules list them accurately
(`produces = ["signed_installer"]`, `requires = ["built_app"]`); most
other areas use `[]` because nothing resolves them yet. Fill them in
when you add a genuinely new artifact.

**MOD4 — All state comes from the `ctx: Context` first argument.**
No `os.environ`, no CWD-relative paths, no module-level mutable state.
`universal.py` builds *new* `Context` objects per architecture rather
than mutating a shared one — copy that pattern.

**MOD5 — Import the shared git plumbing, do not reimplement it.**
`apply/utils.py` and `extract/utils.py` each carry a
`run_git_command()` / `parse_diff_output()` pair. `annotate/annotate.py`
imports `run_git_command` from `../apply/utils.py` rather than
re-inventing it; follow that.

**MOD6 — Platform modules are imported unconditionally and gated in
`validate()`.** `../cli/build.py` imports all three sign and all three
package modules on every OS. Use `IS_MACOS()` / `IS_WINDOWS()` /
`IS_LINUX()` from `../common/utils.py`, never `sys.platform`, and never
a conditional import.

**MOD7 — Tests live next to the code they test**, named `<file>_test.py`
(`package/linux_test.py`, `storage/upload_test.py`,
`ota/bundle_test.py`, `extract/extract_base_default_test.py`). Add new
ones in the same directory.

## Workflows

**Adding a new step to an existing area**
1. Create `modules/<area>/<step>.py` with the `CommandModule` shape.
2. Import the class and add an `AVAILABLE_MODULES` entry in
   `../cli/build.py`.
3. Add the CLI name to the `modules:` list of each
   `../config/release.*.yaml` that should run it.
4. Add `<step>_test.py` beside it if there is logic worth isolating.

**Adding a new area**
1. `mkdir modules/<area>/` — no `__init__.py` needed, but add one if
   you want a docstring, matching the nine areas that have one.
2. Write the `CommandModule` classes.
3. Register them in `../cli/build.py::AVAILABLE_MODULES`.
4. Add `modules/<area>/AGENTS.md` following the same structure as its
   siblings.

**Tracing why a step ran (or did not)**
1. The name in the YAML must match an `AVAILABLE_MODULES` key, or
   `../common/pipeline.py::validate_pipeline()` exits with the list of
   valid names.
2. `requires` / `produces` are descriptive, not enforced by a scheduler
   — reordering a YAML list is the only thing that changes execution
   order.

**Resolving a patch conflict**
1. `browseros dev apply force` (`apply/apply_force.py`) applies every
   patch and writes `.rej` files instead of stopping.
2. Fix the underlying `.patch` in `../../chromium_patches/`, then
   re-run `browseros dev apply all`.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-system overview.
- [`../common/AGENTS.md`](../common/AGENTS.md) — `Context` and `CommandModule` contracts.
- [`../cli/AGENTS.md`](../cli/AGENTS.md) — `AVAILABLE_MODULES`, the registry every area must appear in.
- [`../config/AGENTS.md`](../config/AGENTS.md) — the YAML recipes that name these steps.
- [`../../../AGENTS-build.md`](../../../../AGENTS-build.md) — the pipeline narrative these areas implement.
