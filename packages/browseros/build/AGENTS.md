# `build/` — the BrowserOS build CLI (Python, Typer)

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

The whole BrowserOS build system: a Typer CLI (`python -m build`, or
`browseros` once installed) that turns a vanilla Chromium 148 checkout
into a BrowserOS binary. `browseros.py` wires the five sub-commands
(`build`, `dev`, `release`, `ota`, `upload`); `cli/` implements them;
`common/` holds the shared runtime (`Context`, `CommandModule`, config
resolution, logging); `modules/` holds the 14 build-step areas;
`config/` holds the YAML/GN build recipes; `scripts/` holds CI helpers.
`features.yaml` — the manifest that decides which patches ship — lives
here, not at the package root.

Nothing in this directory imports from `packages/browseros-agent`; the
dependency direction is strictly downward (`cli` → `modules` → `common`).

## Contents

```
build/
├── browseros.py                 ← Typer app; registers build/dev/release/ota/upload
├── __main__.py                  ← `python -m build` shim
├── features.yaml                ← PATCH MANIFEST (version 1.0; one feature = one commit)
├── build_annotate.py            ← legacy standalone click script (DEAD: broken import)
├── cli/                         ← one module per sub-command
├── common/                      ← Context, CommandModule, resolver, logging, env
├── modules/                     ← 14 step areas (namespace pkg, no __init__.py)
├── config/                      ← YAML build recipes + GN flag files + appcast seeds
│   ├── gn/                      ← per-platform per-build-type GN args
│   └── appcast/                 ← Sparkle appcast XML seeds for Server OTA
├── docs/                        ← nightly-macos-ci.md (macOS self-hosted runner)
└── scripts/                     ← bump_version.py + icon_generation/
```

`build_annotate.py` is superseded by `modules/annotate/annotate.py`
(the `AnnotateModule`). It does `from utils import log_info, ...` and
there is no `build/utils.py`, so the module cannot import. Do not use
it as a reference; the live behaviour is in `modules/annotate/`.

## Rules

**B1 — `features.yaml` is the only place that decides what ships.**
Every patch in `chromium_patches/` must be reachable from a feature
block. `modules/annotate/annotate.py` iterates this file to build one
git commit per feature, and `modules/feature/*` rewrites it when
extracting. Editing a patch file without adding its path here produces
an uncommitted orphan.

**B2 — All build state flows through `Context`.** `common/context.py`
is constructed once by `common/resolver.py` and passed as the first
argument to every `CommandModule.validate()` / `.execute()`. Never add
a module-level mutable global and never `os.environ[...]` directly —
declare the variable in `common/env.py` and read it via `ctx.env`.

**B3 — Modules are classes, not functions, in the build pipeline.**
Every step registered in `cli/build.py`'s `AVAILABLE_MODULES` must be a
`CommandModule` subclass with `produces`, `requires`, `description`,
`validate()`, `execute()`. Free functions are fine as helpers inside
the area, but the registry entry must be the class.

**B4 — `modules/` is a namespace package (no `__init__.py`).** Do not
add one. `modules/setup/`, `modules/sign/`, `modules/package/`,
`modules/patches/` and `modules/resources/` deliberately have no
`__init__.py`; the other areas do. Both forms work under PEP 420, so
adding or removing one is cosmetic — but keep a new area consistent
with the file that sits next to it in `cli/build.py`'s import block.

**B5 — Config lives in `config/`, not in Python.** Module order, GN
args, required env vars and Slack toggles are declared in
`config/release.*.yaml` / `config/sign.*.yaml` / `config/package.*.yaml`
/ `config/debug.yaml`. A new release variant is a new YAML file, not a
new `if` in `cli/build.py`.

**B6 — Never hardcode a Chromium path.** `root_dir` always comes from
`common/paths.py::get_package_root()` (walks up for
`pyproject.toml` with `name = "browseros"`), and `chromium_src` is
resolved in `common/resolver.py` from CLI → env (`CHROMIUM_SRC`) →
default. CWD is not a valid source of truth.

## Workflows

**Adding a new build step**
1. Put the module class in the matching `modules/<area>/<step>.py` as a
   `CommandModule` with `produces` / `requires` / `description`.
2. Register it under a CLI name in `cli/build.py::AVAILABLE_MODULES`.
3. Add that CLI name to the `modules:` list of any `config/*.yaml`
   pipeline that should run it.
4. If it needs new inputs, extend `Context` (in `common/context.py`) —
   do not read a file path out of thin air.

**Adding a release variant (e.g. a new platform)**
1. Copy the closest `config/release.<platform>.yaml`.
2. Point `gn_flags.file` at a new `config/gn/flags.<platform>.<type>.gn`.
3. Edit the `modules:` list only — the pipeline order is explicit here,
   `produces`/`requires` is the safety net, not the scheduler.
4. Add a `modules/package/<platform>.py` and a
   `modules/sign/<platform>.py` if the platform is new.

**Reproducing a CI release locally**
1. `uv run browseros build --config build/config/release.macos.arm64.yaml --chromium-src <src>`
2. Export the `required_envs` that config lists.
3. Read `../logs/build_<timestamp>.log` — see [`../logs/AGENTS.md`](../logs/AGENTS.md)
   for how that file is produced.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — package-level view (Chromium fork).
- [`../../../AGENTS-build.md`](../../../AGENTS-build.md) — full build pipeline narrative.
- [`../../../AGENTS-architecture.md`](../../../AGENTS-architecture.md) — cross-repo map.
- [`features.yaml`](features.yaml) — the patch manifest.
- [`../chromium_patches/AGENTS.md`](../chromium_patches/AGENTS.md) — the patch tree this manifest indexes.
