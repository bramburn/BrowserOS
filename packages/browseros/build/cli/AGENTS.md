# `build/cli/` — Typer sub-command implementations

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros/build`.

## What's here

One module per top-level Typer sub-command, all registered by
`../browseros.py`. `build.py` is the important one: it owns the module
registry (`AVAILABLE_MODULES`), the default execution order, and the
run loop that drives `validate()` / `execute()` across the pipeline.
`dev.py` is the patch-management CLI (`extract`, `apply`, `feature`,
`annotate`), `release.py` drives post-build release automation,
`ota.py` drives Server OTA publishing, and `storage.py` is the
`browseros upload` command for pushing third-party binaries (Lima, Bun)
to R2. `__init__.py` is a one-line docstring only.

## Contents

| File | What it does |
|---|---|
| `build.py` | `browseros build`. Owns `AVAILABLE_MODULES`, `EXECUTION_ORDER`, `NOTIFY_MODULES`, `execute_pipeline()`, `main()`. Imports every module class. |
| `dev.py` | `browseros dev` — sub-apps `extract`, `apply`, `feature`; commands `status` and `annotate` on the app itself. |
| `release.py` | `browseros release` — `list`, `appcast`, `github`, `publish`, `download` via `modules/release/AVAILABLE_MODULES`. |
| `ota.py` | `browseros ota` — `server release`, `server release-appcast`, `server list-platforms`, `test-signing`. |
| `storage.py` | `browseros upload` — `lima` and `bun` sub-commands: download a GitHub release, checksum, upload to R2, write a manifest, roll back on failure. |
| `storage_test.py` | Unit tests for the tarball/zip fixtures `storage.py` extracts from. |

## Rules

**CLI1 — `AVAILABLE_MODULES` is the single registry.** A module that is
not a key in `build.py::AVAILABLE_MODULES` cannot be named in a YAML
`modules:` list or a `--modules` flag. Add the class there and the
`common/pipeline.py::validate_pipeline` check will accept it.

**CLI2 — Typer option defaults must be `None` when the value is
overridable.** `common/resolver.py` treats a non-`None` default as
"the user typed this", which silently beats YAML. Let the resolver
supply the default instead.

**CLI3 — Do not call module `execute()` directly from a CLI command
without going through `validate()`.** `release.py` and `ota.py` each have an
`execute_module(ctx, module)` helper that runs `validate()` then `execute()`;
`build.py`'s `execute_pipeline()` does the same inline for every pipeline
entry. Copy one of those shapes.

**CLI4 — Platform modules are imported unconditionally.** The import
block at the top of `build.py` imports macOS, Windows and Linux sign /
package modules on every platform; gating happens in
`module.validate()` via `IS_MACOS()` / `IS_WINDOWS()` / `IS_LINUX()`.
Keep it that way so `--list` works everywhere.

**CLI5 — Never build a `Context` by hand outside `cli/`.** Use
`create_build_context()` (`dev.py`), `create_release_context()`
(`release.py`), `create_ota_context()` (`ota.py`), or
`common/resolver.py::resolve_config()`. They each apply the
`chromium_src` / `CHROMIUM_SRC` resolution rules.

**CLI6 — Notify through `common/notify.py`, never a raw webhook POST.**
Modules emit `notify_module_start` / `notify_module_completion`;
`build.py` decides which entries in `NOTIFY_MODULES` actually fire, and
`notifications.slack: false` in a YAML config suppresses them.

## Workflows

**Adding a sub-command to `browseros dev`**
1. Add the command function to `dev.py` (use the existing `@app.command()`
   or `@<sub>_app.command(name=...)` decorators).
2. Get a `Context` from `dev.py::create_build_context()`.
3. Call the function in `modules/<area>/` — do not reimplement logic here.
4. Document it in `../AGENTS.md`'s sub-command list.

**Registering a new build module**
1. Write the `CommandModule` subclass in `modules/<area>/`.
2. Import it in `build.py` and add a `AVAILABLE_MODULES` entry with the
   CLI name (snake_case, no platform suffix unless it is platform-specific).
3. Add the CLI name to the `modules:` list in the relevant
   `../config/release.*.yaml`.

**Adding a new third-party binary upload**
1. Follow `storage.py::upload_lima` — it is the shape: download →
   verify checksums → extract per-arch → upload → write `manifest.json`.
2. Use `_rollback()` if any upload fails; a half-uploaded prefix is
   worse than none.
3. Add a `browseros upload <name>` sub-command for it.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-system overview.
- [`../common/AGENTS.md`](../common/AGENTS.md) — `Context`, `CommandModule`, resolver.
- [`../modules/AGENTS.md`](../modules/AGENTS.md) — the step areas these commands call into.
- [`../../../../AGENTS-build.md`](../../../../AGENTS-build.md) — build pipeline narrative.
