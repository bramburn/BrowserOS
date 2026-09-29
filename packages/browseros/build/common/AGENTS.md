# `build/common/` — shared build runtime

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros/build`.

## What's here

The runtime every other part of `build/` depends on. `context.py`
defines `Context` — the single object that carries paths, build type,
architecture, versions, artifacts and env access; `module.py` defines
the `CommandModule` base class and `ValidationError`; `resolver.py`
turns CLI args + YAML into `Context` objects; `config.py` loads the
YAML build recipes; `env.py` is the only place that reads
`os.environ`; `paths.py` locates the package root; `logger.py` and
`notify.py` handle console/file output and Slack.

Everything else in `build/` may import from `common/`; `common/` may
not import from `cli/`, `modules/` or `config/`.

## Contents

| File | What it does |
|---|---|
| `__init__.py` | Re-exports `Context`, `ArtifactRegistry`, `PathConfig`, `BuildConfig`, `CommandModule`, `ValidationError`, `EnvConfig`, `load_config`, `validate_required_envs`, `Notifier`, `get_notifier`. |
| `context.py` | `Context` (the god object, being decomposed into `ArtifactRegistry` / `PathConfig` / `BuildConfig`), plus every path helper: `get_patches_dir()`, `get_features_yaml_path()`, `get_gn_args_file()`, `get_dist_dir()`, `get_release_path()`, `get_artifact_name()`. |
| `module.py` | `CommandModule` base class (`produces` / `requires` / `description` / `validate()` / `execute()`) and `ValidationError`. ~105 lines, no dependencies. |
| `resolver.py` | `resolve_config()` — the only place a `Context` is built. CONFIG mode (YAML authoritative) vs DIRECT mode (CLI > env > defaults). Validates `VALID_ARCHITECTURES = {x64, arm64, universal}`. Also `resolve_pipeline()`. |
| `pipeline.py` | `validate_pipeline(pipeline, available_modules)` — rejects a `modules:` name that is not in `AVAILABLE_MODULES`; `show_available_modules()` backs `--list`. |
| `config.py` | `load_config()` (YAML load with an `!env` tag constructor) and `validate_required_envs()`. |
| `env.py` | `EnvConfig` — every env var the build needs, as properties. Auto-loads a dotenv file. `validate_required(*names)` raises on missing. |
| `paths.py` | `get_package_root()` — `@lru_cache` walk up for `pyproject.toml` containing `name = "browseros"`. **No local imports**, by design (see rule CM5). |
| `logger.py` | `log_info/warning/error/success/debug` — Typer echo + a timestamped file in `<package_root>/logs/`. `close_log_file()` at the end. |
| `notify.py` | `Notifier`, `get_notifier()`, colour constants, and the `notify_*` pipeline hooks used by `cli/build.py`. |
| `utils.py` | `run_command()`, `IS_WINDOWS()` / `IS_MACOS()` / `IS_LINUX()`, `get_platform()`, `get_platform_arch()`, `get_executable_extension()`, `join_paths()`, `safe_rmtree()`, `normalize_path()`. |
| `sparkle.py` | Sparkle Ed25519 signing helper (`sparkle_sign_file`) shared by `modules/sign/sparkle.py` and `modules/ota/`. |
| `server_binaries.py` | `SignSpec` dataclass + `MACOS_SERVER_BINARIES` / `WINDOWS_SERVER_BINARIES` maps, consumed by both `modules/sign/` and `modules/ota/`. |
| `server_binaries_test.py` | Unit tests for the binary maps. |

## Rules

**CM1 — `Context` is the only carrier of build state.** Add a new
build-wide value as a `Context` field or a `get_*()` method in
`context.py`. Do not pass it as an extra global or a module-level
singleton. `universal.py` and `merge.py` demonstrate the accepted
escape hatch: construct a *new* `Context` and set
`ctx._fixed_app_path` / `ctx.out_dir` on it.

**CM2 — `common/` never imports from `cli/`, `modules/`, or `config/`.**
The dependency arrow points one way only. If you need a constant that
currently lives in a module area, move or duplicate it *into* `common/`.

**CM3 — Environment variables are declared in `env.py`, not read at the
call site.** `EnvConfig` exposes typed properties (`env.r2_bucket`,
`env.macos_certificate_name`, `env.code_sign_tool_path`,
`env.has_sparkle_key()`). Modules access them via `ctx.env.<prop>`.
`os.environ` appears in exactly two places: `env.py` and the
deliberate `os.environ.copy()` in `modules/package/linux.py` to pass
`ARCH` to `appimagetool`.

**CM4 — New build steps extend `CommandModule`, they do not bypass it.**
`validate()` raises `ValidationError` for a missing precondition;
`execute()` raises on real failure. A module that silently returns on
error will let the pipeline continue with a broken state — see
`modules/sign/linux.py` for the one legitimate no-op case (Linux needs
no signing) and keep it explicit.

**CM5 — `paths.py` must stay import-free.** It is imported by both
`context.py` and `env.py` at module load time; a local import there
creates a cycle. `resolve_config()`'s `root_dir` is likewise always
`get_package_root()`, never CWD and never a config value.

**CM6 — Platform checks are the `IS_*()` helpers, never
`sys.platform`.** `utils.py` defines the three predicates; the rest of
the tree uses them. This keeps the Windows host (where many of these
modules are only *read*, never run) consistent with the code.

**CM7 — Every new artifact goes through `ArtifactRegistry`.**
`ctx.artifact_registry.add(name, path)` — with a distinct name per
variant (`dmg_arm64` vs `dmg_x64`) — because later modules resolve
inputs by that name. Never re-derive a path with string concatenation
when a registry entry already exists.

## Workflows

**Adding a new environment variable**
1. Add a property to `EnvConfig` in `env.py` (return `None` or a
   documented default; do not raise).
2. Add its name to the relevant `required_envs:` list in
   `../config/release.*.yaml` if the build cannot run without it.
3. Read it in the module via `ctx.env.<prop>` — never `os.environ`.

**Adding a new path helper**
1. Add a `get_*()` method to `Context` in `context.py`, composing from
   the existing `root_dir` / `chromium_src` / `out_dir`.
2. If it is a dev-CLI path (patches, features, replacements) put it with
   the other dev methods near the bottom of the class.
3. If it needs a well-known constant, add a `get_*` to `paths.py` only
   if that module already owns the concept.

**Adding a new module step type (not a step)**
1. Extend `module.py` only if the change is to the base contract;
   otherwise add a helper module here (e.g. `sparkle.py`).
2. Export it from `__init__.py`'s `__all__` so callers use one import path.
3. Add `*_test.py` next to it — `server_binaries_test.py` is the pattern.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-system overview.
- [`../cli/AGENTS.md`](../cli/AGENTS.md) — where `resolve_config()` and `validate_pipeline()` are called.
- [`../modules/AGENTS.md`](../modules/AGENTS.md) — the consumers of `Context` and `CommandModule`.
- [`../../../AGENTS-architecture.md`](../../../../AGENTS-architecture.md) — cross-repo map.
