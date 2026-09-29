# `build/config/gn/` — GN argument files per platform and build type

> Part of [`../AGENTS.md`](../AGENTS.md) in `packages/browseros`.

## What's here

Six GN argument files, one per (platform × build type) pair. Each is a
flat list of `flag = value` lines with no GN function calls. The
`configure` module (`../../modules/setup/configure.py`) copies the
selected file verbatim into `<chromium_src>/out/<out_dir>/args.gn`,
appends `target_cpu = "<architecture>"`, then runs `gn gen`.

Selection is entirely declarative: a `build/config/*.yaml` recipe sets
`gn_flags.file: build/config/gn/flags.<platform>.<type>.gn`.

## Contents

| File | Notable settings |
|---|---|
| `flags.macos.release.gn` | `is_official_build=true`, `enable_sparkle=true`, `enable_widevine=true`, `proprietary_codecs=true`, `enable_swiftshader=true`, empty Google API keys, `enable_updater=false` (Sparkle replaces it), `use_system_xcode=true`, `use_clang_modules=false`, `chrome_pgo_phase=0`, `symbol_level=0`. A long trailing block of commented-out flags is kept as a scratchpad. |
| `flags.macos.debug.gn` | Debug counterpart for macOS. |
| `flags.windows.release.gn` | Windows release flags. |
| `flags.windows.debug.gn` | Windows debug flags. |
| `flags.linux.release.gn` | Linux release flags. |
| `flags.linux.debug.gn` | Linux debug flags. |

## Rules

**GN1 — Do not write `target_cpu` here.** The `configure` module
appends it from `ctx.architecture` on every run. A hardcoded value
becomes a second, stale `target_cpu` line in `args.gn`, and
`--fail-on-unused-args` will reject the duplicate.

**GN2 — One assignment per line, no functions.** These files are
appended verbatim; a multi-line construct or a trailing comma in a list
that spans lines works, but anything requiring evaluation of
`import()` or a custom scope is out of contract.

**GN3 — Release builds run `gn gen --fail-on-unused-args`.** Every
flag you add must therefore be read by at least one target in the
build graph for that platform. Adding a flag only to the release
`.gn` file is fine; adding one that no `BUILD.gn` consumes will fail
the release pipeline with "unused args".

**GN4 — Google API keys stay empty.** The release flags set
`google_api_key`, `google_default_client_id`,
`google_default_client_secret` to `""` and
`use_official_google_api_keys=false`. Do not paste real credentials
here — this is tracked in git.

**GN5 — Change the sibling variant too.** Branding and codec flags
that are not platform-specific belong in all six files. Divergence is
the usual cause of "it works on macOS but not on Windows".

**GN6 — Keep the commented-out block at the bottom of the macOS
release file intact.** It is the working scratchpad for flag
experiments. Delete it only when the file is being genuinely
simplified, not as part of an unrelated flag change.

## Workflows

**Toggling a GN flag**
1. Edit the `.gn` file for the platform you are building.
2. Mirror the change into the `<platform>.debug.gn` sibling if the flag
   is not release-only.
3. Re-run `browseros build -m configure` (or the full pipeline) — GN
   will error on an unused arg in release, which is the intended
   feedback loop.

**Adding a flag for a new platform**
1. Copy the closest existing `flags.<platform>.release.gn`.
2. Create the matching `.debug.gn`.
3. Add a new `build/config/release.<platform>.yaml` that points
   `gn_flags.file` at the release file.

**Debugging a `gn gen` failure**
1. Read the generated `<chromium_src>/out/<out_dir>/args.gn` — it is
   the flags file plus an appended `target_cpu` line.
2. Compare it against the `.gn` file you intended to use.
3. If the architecture is wrong, the problem is the YAML's
   `build.architecture`, not the `.gn` file.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — build-system overview.
- [`../AGENTS.md`](../AGENTS.md#contents) — the `release.*.yaml` files that select these.
- [`../../modules/setup/AGENTS.md`](../../modules/setup/AGENTS.md) — the `configure` module that consumes them.
- [`../../modules/compile/AGENTS.md`](../../modules/compile/AGENTS.md) — the `autoninja` step that reads the generated `args.gn`.
