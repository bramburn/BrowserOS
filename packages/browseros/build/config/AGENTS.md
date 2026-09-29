# `build/config/` — YAML build recipes, GN args, appcast seeds

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros/build`.

## What's here

Every declarative input to the build that is not Python. Each
`release.*.yaml` / `sign.*.yaml` / `package.*.yaml` / `debug.yaml` is a
complete pipeline recipe: it declares `build.type`, `build.architecture`,
which GN flag file to use, the ordered list of module names to run, the
env vars that must be set, and whether Slack notifications fire. They
are passed as `browseros build --config build/config/<file>.yaml` and
loaded by `../common/config.py::load_config()`.

`gn/` holds the GN args copied verbatim into `<chromium_src>/out/<dir>/args.gn`
by the `configure` module (it appends `target_cpu` itself).
`appcast/` holds the *seed* appcast XML that the OTA module reads to
preserve existing `<item>` history. `BROWSEROS_BUILD_OFFSET` is a single
integer consumed by the nightly CI version bumper.

## Contents

| File | Pipeline it defines |
|---|---|
| `release.macos.yaml` | macOS **universal**: `clean, git_setup, sparkle_setup, download_resources, bundled_extensions, chromium_replace, string_replaces, series_patches, patches, universal_build` |
| `release.macos.arm64.yaml` | macOS single-arch arm64, the nightly CI default; ends in `sign_macos` + `package_macos` + `upload` |
| `release.macos.arm64.noupload.yaml` | Same as above minus the R2 `upload` step — manual-dispatch / artifact-only builds |
| `release.windows.yaml` | Windows x64 release |
| `release.linux.yaml` | Linux arm64 release: `clean, git_setup, download_resources, resources, bundled_extensions, chromium_replace, string_replaces, series_patches, patches, configure, compile, package_linux, upload` |
| `sign.windows.yaml` | Sign+package only (`sign_windows, package_windows`) against a pre-built `out/Default` |
| `sign.macos.yaml` | macOS sign+package only |
| `package.linux.yaml` | Linux packaging only (`package_linux`) |
| `debug.yaml` | macOS debug, single arm64, no `clean`, no signing, `slack: false` |
| `copy_resources.yaml` | `copy_operations:` — what the `resources` module copies into the Chromium tree, with `os:` / `arch:` / `build_type:` filters |
| `download_resources.yaml` | `download_operations:` — what the `download_resources` module pulls from R2 (`r2_key`, `download_type: file\|artifact_zip`) |
| `BROWSEROS_BUILD_OFFSET` | One integer (currently `148`); incremented by `../scripts/bump_version.py` |
| `gn/` | 6 GN arg files: `{macos,windows,linux}.{release,debug}.gn` |
| `appcast/` | `appcast-server.xml` (channel seed), `appcast-server.alpha.xml` |

## Rules

**CFG1 — Module order here is authoritative; `produces`/`requires` is
the safety net, not the scheduler.** `release.macos.yaml` runs
`universal_build` as the only compile step because that module
internally runs resources → configure → compile → sign → package →
upload per architecture. If you list `compile` before `universal_build`
you get two builds.

**CFG2 — Every new YAML gets a `gn_flags.file` that exists.** The
`configure` module raises `ValidationError` if `ctx.paths.gn_flags_file`
is unset or the file is missing on disk. Paths are repo-relative and
written as `build/config/gn/...`, not `build/gn/...`.

**CFG3 — `--fail-on-unused-args` is on for non-debug builds.**
`configure` adds it whenever `ctx.build_type != "debug"`, so a GN flag
added to a `.gn` file that no build path reads will hard-fail release
builds. When you add a flag, add it to every `.gn` variant that should
carry it, not just one.

**CFG4 — `os:` and `arch:` values are case-sensitive exact
strings.** `os` is one of `windows` / `macos` / `linux`; `arch` is
matched against `ctx.architecture` (`x64` / `arm64`). A typo silently
skips the operation with a log line, it does not error.

**CFG5 — `required_envs:` is a list of names, not values.** Names must
match what `common/env.py::EnvConfig` exposes. Secrets go in the
environment or a dotenv file — never in these YAML files.

**CFG6 — `notifications.slack` defaults matter.** `debug.yaml` sets it
`false`; every release config sets it `true`. Local iteration configs
should set it `false` so a stray build does not page the team.

**CFG7 — The `appcast/` files are seeds, not outputs.** The OTA module
*reads* them via `../modules/ota/common.py::parse_existing_appcast()` to
recover prior items, then writes the merged result elsewhere. Editing
them by hand drops update history; regenerate with
`browseros ota server release-appcast`.

## Workflows

**Adding a new release variant**
1. Copy the nearest `release.<platform>.yaml`.
2. Set `build.type` / `build.architecture` and point `gn_flags.file` at
   the matching `gn/flags.<platform>.<type>.gn`.
3. Edit the `modules:` list — names must exist in
   `../cli/build.py::AVAILABLE_MODULES` or the pipeline aborts.
4. Copy `required_envs:` from the variant you forked; keep
   `notifications.slack: false` unless it really publishes.

**Adding a file to the Chromium tree**
1. Put the file under `../../resources/<area>/`.
2. Add a `copy_operations:` entry in `copy_resources.yaml` with the
   right `type:` (`directory` / `files` / `file`) and `os:` / `arch:`
   filters. Without an entry nothing is copied.

**Changing what is fetched from R2**
1. Edit `download_resources.yaml`, not the Python. `r2_key` is the
   object key; `download_type: artifact_zip` means
   `../modules/storage/download.py` will validate
   `artifact-metadata.json` and extract the declared files.

**Changing a GN flag**
1. Edit the relevant `gn/flags.<platform>.<type>.gn`.
2. Apply the same edit to the sibling `<type>` variant if the change is
   platform-agnostic (rule CFG3).
3. If the flag only exists for a subset of pipelines, add a new `.gn`
   file rather than removing the flag from a shared one.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-system overview.
- [`../common/AGENTS.md`](../common/AGENTS.md) — `load_config()` / `EnvConfig` / `Context`.
- [`../cli/AGENTS.md`](../cli/AGENTS.md) — `AVAILABLE_MODULES`, the names these files reference.
- [`../modules/resources/AGENTS.md`](../modules/resources/AGENTS.md) — the consumer of `copy_resources.yaml`.
- [`../modules/storage/AGENTS.md`](../modules/storage/AGENTS.md) — the consumer of `download_resources.yaml`.
