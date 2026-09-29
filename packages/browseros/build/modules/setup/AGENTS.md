# `build/modules/setup/` — clean, git sync, Sparkle, GN configure

> Part of [`../AGENTS.md`](../AGENTS.md) in
> [`packages/browseros/build/modules/`](../AGENTS.md).

## What's here

The four phase-1/phase-3 setup steps: `clean.py` resets the Chromium
working tree, `git.py` checks out the pinned Chromium version (and
downloads the Sparkle framework on macOS), and `configure.py` runs
`gn gen` from a selected `config/gn/flags.*.gn`. Three files, four
module classes — `git.py` holds two.

The area has no `__init__.py` — PEP 420 namespace package.

## Contents

| File | What it does |
|---|---|
| `clean.py` | `class CleanModule` — `description = "Clean build artifacts and reset git state"`. Removes build output directories and resets the Chromium tree. |
| `git.py` | `class GitSetupModule` — `description = "Checkout Chromium version and sync dependencies"`. `class SparkleSetupModule` — `description = "Download and setup Sparkle framework (macOS only)"`. |
| `configure.py` | `class ConfigureModule` — `description = "Configure build with GN"`. `_ensure_linux_sysroot()`. |

All four use `produces = []` and `requires = []`.

## Rules

**SET1 — `configure` appends `target_cpu`; it must not be in the `.gn`
file.** `args_content += f'\ntarget_cpu = "{ctx.architecture}"\n'`
then `args.gn` is written. Duplicating it in the source `.gn` triggers
`--fail-on-unused-args` in release builds.

**SET2 — `gn gen --fail-on-unused-args` is added for every non-debug
build.** `if ctx.build_type != "debug": gn_args.append(...)`. This is
why an unused flag in a release `.gn` file is a hard failure, and why
`debug.yaml` skipping it is convenient rather than a bug.

**SET3 — The Linux sysroot install is unconditional.** `_ensure_linux_sysroot()`
runs `build/linux/sysroot_scripts/install-sysroot.py` before `gn gen` on
every Linux build. The docstring explains why relying on `gclient sync`
DEPS hooks is fragile (the hook only fires when `.gclient` declared
`target_cpus` before sync). The script is idempotent — do not add a
"skip if present" guard.

**SET4 — `gn` is `gn.bat` on Windows, `gn` elsewhere.** Same rule as
`../compile/`.

**SET5 — `gn_flags_file` is a `Context` field, not a module setting.**
`ConfigureModule.validate()` requires `ctx.paths.gn_flags_file`; it
comes from the YAML's `gn_flags.file` via `../../common/resolver.py`.
Never hardcode a flags path here.

**SET6 — `clean` is destructive and should be the first module in a
release config only.** `config/release.*.yaml` puts `clean` at the top;
`config/debug.yaml` deliberately omits it for iteration speed. If you
add `clean` to a debug config you will lose the incremental build.

**SET7 — `sparkle_setup` is macOS-only and must precede packaging.**
It downloads the Sparkle framework that the app bundle embeds.
Windows and Linux have no equivalent; their configs omit the step.

## Workflows

**Starting a clean release build**
1. `clean` resets the tree and removes output directories.
2. `git_setup` checks out the version in `CHROMIUM_VERSION` (and
   `BASE_COMMIT`) and runs the dependency sync.
3. `sparkle_setup` (macOS only) fetches the update framework.
4. `download_resources`, `bundled_extensions`, `chromium_replace`,
   `string_replaces`, `series_patches`, `patches` mutate the tree.
5. `configure` writes `args.gn` and runs `gn gen`.
6. `compile` runs autoninja.

**Iterating on a debug build**
1. Use `config/debug.yaml` — no `clean`, no signing, `slack: false`.
2. Re-run only `configure` after changing a `.gn` file.
3. Re-run only `compile` after changing a patch that touches C++.

**Reconfiguring for a different architecture**
1. Change `build.architecture` in the YAML (or pass `--arch`).
2. Re-run `configure` — a different `target_cpu` means a different
   output directory and a fresh build.

**Debugging a `gn gen` failure**
1. Read `<chromium_src>/out/<out_dir>/args.gn` — it is the flags file
   plus the appended `target_cpu` line.
2. "Unused args" → a flag in the `.gn` file no target reads.
3. Linux sysroot errors → the target-arch sysroot is missing; check
   `_ensure_linux_sysroot()` ran and that the script exists in the
   Chromium checkout.

## Cross-references

- [`../AGENTS.md`](../AGENTS.md) — all 14 module areas.
- [`../../config/AGENTS.md`](../../config/AGENTS.md) — the `gn_flags.file` values and `modules:` lists consumed here.
- [`../compile/AGENTS.md`](../compile/AGENTS.md) — the step that reads the generated `args.gn`.
- [`../patches/AGENTS.md`](../patches/AGENTS.md) — the phase-2 steps that must run between `git_setup` and `configure`.
- [`../../../../CHROMIUM_VERSION`](../../../CHROMIUM_VERSION) — the version `git_setup` checks out.
