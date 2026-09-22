# `packages/browseros/` — Chromium fork view

> Sub-package AGENTS file. The C++ Chromium-fork side of the BrowserOS
> monorepo. For the cross-repo architecture see
> [`../../AGENTS-architecture.md`](../../AGENTS-architecture.md). For the
> full build pipeline see [`../../AGENTS-build.md`](../../AGENTS-build.md).
> For the dev workflow see
> [`../../AGENTS-ubuntu-dev.md`](../../AGENTS-ubuntu-dev.md) and
> [`../../AGENTS-toolchain.md`](../../AGENTS-toolchain.md).

## What's here

The Python CLI that turns a vanilla Chromium 148 checkout into a
BrowserOS binary. It is a Typer app: `python -m build` (or
`browseros` once installed) with five sub-commands: `build`, `dev`,
`release`, `ota`, `upload`. 342 patches + chromium_files +
series_patches + resources get layered on top of vanilla Chromium
according to a single source of truth: `build/features.yaml`.

```
packages/browseros/
├── CHROMIUM_VERSION                  ← pinned: 148.0.7778.97
├── BASE_COMMIT                       ← pinned: 6b3fa66a923a9442c8ab0bc71b4b41ff24528d3b
├── features.yaml                     ← PATCHES MANIFEST (one feature per commit)
├── README.md                         ← CLI reference (with stale `browseros setup` note)
├── pyproject.toml                    ← Python deps (Typer, etc.)
├── requirements.txt, uv.lock
├── build/                            ← Python source tree
│ ├── browseros.py                    ← Typer app entry, registers sub-commands
│ ├── build_annotate.py               ← runs patches as git commits
│ ├── __main__.py
│ ├── cli/
│ │ ├── build.py                      ← `browseros build` callback
│ │ ├── dev.py                        ← `browseros dev` (patch management)
│ │ ├── release.py                    ← `browseros release`
│ │ ├── ota.py                        ← `browseros ota`
│ │ └── storage.py                    ← `browseros upload` (R2)
│ ├── common/                         ← shared internals
│ │ ├── context.py                    ← Context class (paths, config, build state)
│ │ ├── config.py                     ← YAML config loader
│ │ ├── module.py                     ← CommandModule base class
│ │ ├── paths.py                      ← well-known paths
│ │ ├── pipeline.py                   ← pipeline runner
│ │ ├── resolver.py                   ← module ordering, direct vs config mode
│ │ ├── server_binaries.py            ← bundled MCP server binary map
│ │ ├── sparkle.py                    ← macOS update framework
│ │ ├── notify.py, logger.py, env.py
│ │ └── utils.py
│ ├── modules/                        ← 14 module areas (each is a Python package)
│ │ ├── setup/                        ← phase 1: clean, git_setup, sparkle_setup
│ │ ├── patches/                      ← patch file management
│ │ ├── apply/                        ← apply patches to chromium_src
│ │ ├── extract/                      ← extract patches from chromium_src
│ │ ├── feature/                      ← feature flag management
│ │ ├── resources/                    ← resources (icons, etc.)
│ │ ├── extensions/                   ← bundled extensions (CRX list)
│ │ ├── compile/                      ← autoninja (with universal_build for macOS)
│ │ ├── package/                      ← platform-specific packagers
│ │ ├── sign/                         ← code signing (mac/win/linux)
│ │ ├── ota/                          ← OTA manifest generation
│ │ ├── release/                      ← release pipeline helpers
│ │ ├── storage/                      ← R2 download/upload
│ │ └── annotate/                     ← commits-per-feature annotation
│ ├── config/                         ← YAML configs
│ │ ├── download_resources.yaml       ← what to fetch from R2 (server binaries)
│ │ ├── copy_resources.yaml           ← file copy operations
│ │ └── bundled_extensions.yaml       ← which CRXs ship pre-installed
│ ├── docs/                           ← build-system docs (e.g. nightly-macos-ci.md)
│ └── scripts/                        ← helper scripts
├── chromium_patches/                 ← 342 file replacements
│ └── chrome/browser/browseros/       ← where BrowserOS-specific C++ lives
│ ├── BUILD.gn
│ ├── core/                          ← prefs, switches, constants
│ ├── extensions/                    ← bundled_ext loader
│ ├── metrics/                       ← telemetry
│ └── server/                        ← bundled MCP server manager + IPC
├── chromium_files/                   ← brand-new files added to Chromium tree
├── series_patches/                   ← GNU-Quilt ordered patches
├── resources/                        ← icons, branding, BROWSEROS_VERSION
├── tools/patch                       ← BrowserOS patch CLI
├── browseros.egg-info/               ← pip metadata
├── pyrightconfig.json                ← pyright static analysis
└── logs/, releases/                  ← build artefacts (gitignored)
```

## Opinionated rules

These apply specifically to the Chromium side. Combined with
[`AGENTS-architecture.md`](../../AGENTS-architecture.md) R1–R14.

### F1 — `features.yaml` is the source of truth for patches
**Never** commit a `.cc` / `.h` to `chromium_patches/...` without also
listing it under a feature block in `features.yaml`. The
`browseros dev annotate` step uses the manifest to create one commit
per feature.

### F2 — Patch path mirrors Chromium source path
If a Chromium file lives at `chrome/browser/foo/bar.cc`, the patch
lives at `chromium_patches/chrome/browser/foo/bar.cc` — **same path**.
This is the rule that lets `patches/apply/` and `extract/` work
deterministically. Don't rename or symlink.

### F3 — New files go in `chromium_files/`
If the file doesn't exist in stock Chromium, drop it under
`chromium_files/...` and reference it from `features.yaml`. The
`chromium_replace` module handles wholesale replacement, but
prefer granular patches when possible.

### F4 — Series patches are last-resort
`series_patches/` is for GNU-Quilt-style ordered patches where
conflict resolution needs human review. Try the `chromium_patches/`
route first. The CLI runs them with
`browseros build -m series_patches` separately.

### F5 — Module order matters
Each module declares its dependencies via `produces` / `requires`
fields on `CommandModule` (see `build/common/module.py`). The
`resolver.py` builds the DAG. Don't re-order modules in
`build/cli/build.py` — fix the dependencies in the module class.

### F6 — Context, not globals
All build state flows through `Context` (in `common/context.py`).
Modules receive it as the first arg. Don't read env vars or
`~/.config/...` directly — declare them in
`common/env.py` and access via `ctx.env`.

### F7 — Tests in the module that owns them
If a module has unit tests, put them next to the module:
`modules/setup/clean.py` → `modules/setup/clean_test.py`. There's
also a top-level `build_annotate_test.py`, `server_binaries_test.py`,
etc., for cross-module concerns.

## Opinionated workflow

### "I'm adding a C++ feature"
1. Pick the file path. **Mirror Chromium's path** (F2).
2. If file exists in Chromium, write a full-replacement patch under
 `chromium_patches/<mirror-path>`.
3. If file is new, write it under `chromium_files/<path>`.
4. Add an entry to `features.yaml` under an existing or new feature.
 The `description:` is the eventual git commit message.
5. If new BUILD.gn is needed, add it at the right `chromium_patches/.../BUILD.gn`.
 Reference it from the parent BUILD.gn if appropriate.
6. Run on the Linux LAN box: `tools/ubuntu-launch.sh --phase 2 3`
 (applies patches + incremental ninja).
7. Verify the file lands at the expected path in `<chromium_src>/`.

### "I'm modifying an existing C++ file"
Same as above, but the file already exists in `chromium_patches/...`.
Edit it, run `browseros dev annotate` to convert in-tree edits back
to patch commits (or just re-edit the patch file directly — they're
the same).

### "I'm adding a new build phase"
Don't. Add a new module to one of the existing 14 areas, or
add a new area under `build/modules/<area>/`. The phase pipeline
in `cli/build.py` is fixed; modules self-declare ordering via
`produces` / `requires`.

### "I'm adding a new platform-specific packager"
1. Create `modules/package/<platform>.py` with the standard module
 shape (see `package/windows.py` for Windows).
2. Add a `MODULE_REGISTRY` entry.
3. Update `features.yaml` if any platform-conditional patches are needed.

### "I'm adding a new LLM provider icon / branding asset"
1. Drop the asset under `resources/<area>/`.
2. Add a copy operation to `build/config/copy_resources.yaml` with
 `os:` and `arch:` filters.

### "I'm updating Chromium version"
1. Edit `CHROMIUM_VERSION` (and `BASE_COMMIT` if you know the new SHA).
2. On the LAN box, blow away `~/browseros-build/src/` and let phase 1
 re-clone (1.5 hours cooldown if rate-limited).
3. Re-run phase 1 then phase 2.
4. `browseros dev annotate` against a fresh checkout will report
 patch conflicts; resolve them.

## Key files for quick lookup

| Need | File |
|---|---|
| Pin a Chromium version | `CHROMIUM_VERSION`, `BASE_COMMIT` |
| Add/remove patches | `build/features.yaml` |
| Add a build phase | `build/modules/<area>/<step>.py` |
| Change what's bundled | `build/config/copy_resources.yaml` (file copy) and `bundled_extensions.yaml` (CRX list) |
| Change R2 download layout | `build/config/download_resources.yaml` |
| Change module ordering | `build/common/module.py` (`CommandModule.produces` / `.requires`) |
| Change CLI | `build/cli/<subcommand>.py` |
| Add a Chrome flag | `chromium_patches/chrome/browser/browseros/core/browseros_switches.h` + `chrome/browser/ui/startup/...` |
| Change the bundled MCP server URL | `build/config/download_resources.yaml` (the `r2_key` for the relevant platform/arch) |
| Custom pre-build / post-build hooks | `build/common/pipeline.py` |

## Build-time artefacts

After a successful build, artefacts land at:

```
<chromium_src>/
└── out/Default/                 ← autoninja output (~100 GB)
 ├── chrome                    ← runnable binary
 ├── mini_installer.exe       ← Windows installer
 ├── *.dmg                    ← macOS
 ├── *.deb, *.AppImage        ← Linux
 └── *.zip                    ← Portable Windows
```

The packaged installer ends up at:
`packages/browseros/releases/<version>/BrowserOS_v<ver>_<platform>.exe`

R2 layout (after release):
```
browseros/
 <version>/
 BrowserOS_v<version>_win-x64.exe
 windows/
 update_check.xml
 appcast.xml
 latest.json
```

## Cross-references

- [`AGENTS-architecture.md`](../../AGENTS-architecture.md) — overall map.
- [`AGENTS-build.md`](../../AGENTS-build.md) — full build pipeline.
- [`AGENTS-ubuntu-dev.md`](../../AGENTS-ubuntu-dev.md) — Linux LAN build host.
- [`AGENTS-toolchain.md`](../../AGENTS-toolchain.md) — Ubuntu 24.04 toolchain.
- [`docs/CI_AND_RELEASES.md`](../../docs/CI_AND_RELEASES.md) — CI + release.
- [`README.md`](README.md) — original CLI reference (some sections outdated).
- [`packages/browseros-agent/AGENTS.md`](../browseros-agent/AGENTS.md) — Bun monorepo side.
